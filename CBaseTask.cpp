/*!
	\file
	\brief Базовый класс для реализации задачи FreeRTOS в многоядерном CPU.
	\authors Близнец Р.А. (r.bliznets@gmail.com)
	\version 1.3.0.0
	\date 28.04.2020
*/
#include "CBaseTask.h"
#include <cstdio>
#include <cstring>
#include "sdkconfig.h"
#include "CTrace.h"
#include "esp_heap_caps.h"

// Основная функция задачи, которая вызывается FreeRTOS при запуске задачи.
// Эта функция выполняет следующие действия:
// 1. Вызывает метод `run()` для выполнения основной логики задачи.
// 2. После завершения работы удаляет очередь сообщений (`mTaskQueue`).
// 3. Если включена поддержка `vTaskDelete`, удаляет саму задачу.
void CBaseTask::vTask(void *pvParameters)
{
	// Приводим параметр к типу CBaseTask* и вызываем метод run().
	((CBaseTask *)pvParameters)->run();

	// Удаляем очередь сообщений после завершения работы задачи.
	vQueueDelete(((CBaseTask *)pvParameters)->mTaskQueue);
	((CBaseTask *)pvParameters)->mTaskQueue = nullptr;

#if (INCLUDE_vTaskDelete == 1)
	// Если поддерживается удаление задач, логируем завершение и удаляем задачу.
	ESP_LOGD(pcTaskGetName(((CBaseTask *)pvParameters)->mTaskHandle), "exit");
	((CBaseTask *)pvParameters)->mTaskHandle = nullptr;
	vTaskDelete(nullptr);
#else
	// Если удаление задач не поддерживается, задача зацикливается.
	for (;;)
		vTaskDelay(pdMS_TO_TICKS(1000));
#endif
}

// Деструктор класса CBaseTask.
// Удаляет задачу и связанную с ней очередь сообщений, если это возможно.
CBaseTask::~CBaseTask()
{
#if (INCLUDE_vTaskDelete == 1)
	if (mTaskHandle != nullptr)
	{
		// Удаляем очередь сообщений, если она существует.
		if (mTaskQueue != nullptr)
			vQueueDelete(mTaskQueue);

		// Удаляем саму задачу.
		vTaskDelete(mTaskHandle);
	}
#endif
}

// Метод для инициализации задачи.
// Создает очередь сообщений и регистрирует задачу в планировщике FreeRTOS.
void CBaseTask::init(const char *name, unsigned short usStack, UBaseType_t uxPriority, UBaseType_t queueLength, BaseType_t coreID)
{
	// Проверяем корректность входных параметров.
	assert(uxPriority <= configMAX_PRIORITIES);
	assert(usStack >= configMINIMAL_STACK_SIZE);
	assert(std::strlen(name) < configMAX_TASK_NAME_LEN);

	// Создаем очередь сообщений заданной длины.
	mTaskQueue = xQueueCreate(queueLength, sizeof(STaskMessage));

	// Создаем задачу и привязываем ее к указанному ядру процессора.
	xTaskCreatePinnedToCore(vTask, name, usStack, this, uxPriority, &mTaskHandle, coreID);
}

// Отправляет сообщение в конец очереди.
// Возвращает true, если сообщение успешно отправлено.
bool CBaseTask::sendMessage(STaskMessage *msg, TickType_t xTicksToWait, bool free_mem)
{
	assert(msg != nullptr); // Проверяем, что указатель на сообщение не равен nullptr.

	// Отправляем сообщение в очередь.
	if (xQueueSend(mTaskQueue, msg, xTicksToWait) == pdPASS)
	{
		// Если установлен флаг уведомления, отправляем уведомление задаче.
		if (mNotify != 0)
		{
			return (xTaskNotify(mTaskHandle, mNotify, eSetBits) == pdPASS);
		}
		else
			return true; // Сообщение успешно отправлено без уведомления.
	}
	else
	{
		// Если отправка не удалась, освобождаем память (если требуется) и записываем предупреждение.
		if (free_mem)
			vPortFree(msg->msgBody);
		TRACE_WARNING(pcTaskGetName(mTaskHandle), msg->msgID);
		return false;
	}
}

// Отправляет сообщение в начало очереди.
// Возвращает true, если сообщение успешно отправлено.
bool CBaseTask::sendMessageFront(STaskMessage *msg, TickType_t xTicksToWait, bool free_mem)
{
	assert(msg != nullptr); // Проверяем, что указатель на сообщение не равен nullptr.

	// Отправляем сообщение в начало очереди.
	if (xQueueSendToFront(mTaskQueue, msg, xTicksToWait) == pdPASS)
	{
		// Если установлен флаг уведомления, отправляем уведомление задаче.
		if (mNotify != 0)
		{
			return (xTaskNotify(mTaskHandle, mNotify, eSetBits) == pdPASS);
		}
		else
			return true; // Сообщение успешно отправлено без уведомления.
	}
	else
	{
		// Если отправка не удалась, освобождаем память (если требуется) и записываем предупреждение.
		if (free_mem)
			vPortFree(msg->msgBody);
		TRACE_WARNING(pcTaskGetName(mTaskHandle), msg->msgID);
		return false;
	}
}

// Отправляет сообщение из ISR в конец очереди.
// Возвращает true, если сообщение успешно отправлено.
bool IRAM_ATTR CBaseTask::sendMessageFromISR(STaskMessage *msg, BaseType_t *pxHigherPriorityTaskWoken)
{
	assert(msg != nullptr); // Проверяем, что указатель на сообщение не равен nullptr.

	// Отправляем сообщение из ISR в конец очереди.
	if (xQueueSendToBackFromISR(mTaskQueue, msg, pxHigherPriorityTaskWoken) == pdPASS)
	{
		// Если установлен флаг уведомления, отправляем уведомление задаче из ISR.
		if (mNotify != 0)
		{
			if (xTaskNotifyFromISR(mTaskHandle, mNotify, eSetBits, pxHigherPriorityTaskWoken) == pdPASS)
			{
				return true;
			}
			else
			{
				// Если отправка не удалась, записываем предупреждение.
				TRACE_FROM_ISR("sendMessageFromISR2", msg->msgID, pxHigherPriorityTaskWoken);
				return false;
			}
		}
		else
			return true; // Сообщение успешно отправлено без уведомления.
	}
	else
	{
		// Если отправка не удалась, записываем предупреждение.
		TRACE_FROM_ISR("sendMessageFromISR", msg->msgID, pxHigherPriorityTaskWoken);
		return false;
	}
}

// Отправляет сообщение из ISR в начало очереди.
// Возвращает true, если сообщение успешно отправлено.
bool IRAM_ATTR CBaseTask::sendMessageFrontFromISR(STaskMessage *msg, BaseType_t *pxHigherPriorityTaskWoken)
{
	assert(msg != nullptr); // Проверяем, что указатель на сообщение не равен nullptr.

	// Отправляем сообщение из ISR в начало очереди.
	if (xQueueSendToFrontFromISR(mTaskQueue, msg, pxHigherPriorityTaskWoken) == pdPASS)
	{
		// Если установлен флаг уведомления, отправляем уведомление задаче из ISR.
		if (mNotify != 0)
		{
			if (xTaskNotifyFromISR(mTaskHandle, mNotify, eSetBits, pxHigherPriorityTaskWoken) == pdPASS)
			{
				return true;
			}
			else
			{
				// Если отправка не удалась, записываем предупреждение.
				TRACE_FROM_ISR("sendMessageFrontFromISR2", msg->msgID, pxHigherPriorityTaskWoken);
				return false;
			}
		}
		else
			return true; // Сообщение успешно отправлено без уведомления.
	}
	else
	{
		// Если отправка не удалась, записываем предупреждение.
		TRACE_FROM_ISR("sendMessageFrontFromISR", msg->msgID, pxHigherPriorityTaskWoken);
		return false;
	}
}

// Получает сообщение из очереди.
// Возвращает true, если сообщение успешно получено.
bool CBaseTask::getMessage(STaskMessage *msg, TickType_t xTicksToWait)
{
	assert(msg != nullptr); // Проверяем, что указатель на сообщение не равен nullptr.

	// Получаем сообщение из очереди.
	return (xQueueReceive(mTaskQueue, msg, xTicksToWait) == pdTRUE);
}

// Выделяет память для тела сообщения.
// Возвращает указатель на выделенную память.
uint8_t *CBaseTask::allocNewMsg(STaskMessage *msg, uint16_t cmd, uint16_t size, bool psram)
{
	assert(msg != nullptr); // Проверяем, что указатель на сообщение не равен nullptr.
	assert(size > 0);		// Размер должен быть больше нуля.

	// Заполняем заголовок сообщения.
	msg->msgID = cmd;
	msg->shortParam = size;

#ifdef CONFIG_SPIRAM
	// Если доступна внешняя PSRAM, используем её для выделения памяти.
	if (psram)
		msg->msgBody = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
	else
		msg->msgBody = heap_caps_malloc(size, MALLOC_CAP_DEFAULT);
#else
	// Используем стандартное выделение памяти.
	msg->msgBody = pvPortMalloc(msg->shortParam);
#endif // CONFIG_SPIRAM

	return (uint8_t *)msg->msgBody; // Возвращаем указатель на выделенную память.
}