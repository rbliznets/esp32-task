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

void CBaseTask::vTask(void *pvParameters)
{
	((CBaseTask *)pvParameters)->run();
	vQueueDelete(((CBaseTask *)pvParameters)->mTaskQueue);
	((CBaseTask *)pvParameters)->mTaskQueue = nullptr;
	ESP_LOGI(pcTaskGetName(((CBaseTask *)pvParameters)->mTaskHandle), "exit");
#if (INCLUDE_vTaskDelete == 1) //????
	((CBaseTask *)pvParameters)->mTaskHandle = nullptr;
	vTaskDelete(nullptr);
#else
	for (;;)
		vTaskDelay(pdMS_TO_TICKS(1000));
#endif
}

CBaseTask::~CBaseTask()
{
#if (INCLUDE_vTaskDelete == 1) //????
	if (mTaskHandle != nullptr)
	{
		vTaskDelete(mTaskHandle);
		if (mTaskQueue != nullptr)
			vQueueDelete(mTaskQueue);
	}
#endif
}

void CBaseTask::init(const char *name, unsigned short usStack, UBaseType_t uxPriority, UBaseType_t queueLength, BaseType_t coreID)
{
	assert(uxPriority <= configMAX_PRIORITIES);
	assert(usStack >= configMINIMAL_STACK_SIZE);
	assert(std::strlen(name) < configMAX_TASK_NAME_LEN);

	mTaskQueue = xQueueCreate(queueLength, sizeof(STaskMessage));
	xTaskCreatePinnedToCore(vTask, name, usStack, this, uxPriority, &mTaskHandle, coreID);
}

bool CBaseTask::sendMessage(STaskMessage *msg, TickType_t xTicksToWait, bool free_mem)
{
	assert(msg != nullptr);

	if (xQueueSend(mTaskQueue, msg, xTicksToWait) == pdPASS)
	{
		if (mNotify != 0)
		{
			return (xTaskNotify(mTaskHandle, mNotify, eSetBits) == pdPASS);
		}
		else
			return true;
	}
	else
	{
		if (free_mem)
			vPortFree(msg->msgBody);
		TRACE_WARNING(pcTaskGetName(mTaskHandle), msg->msgID);
		return false;
	}
}

bool CBaseTask::sendMessageFront(STaskMessage *msg, TickType_t xTicksToWait, bool free_mem)
{
	assert(msg != nullptr);

	if (xQueueSendToFront(mTaskQueue, msg, xTicksToWait) == pdPASS)
	{
		if (mNotify != 0)
		{
			return (xTaskNotify(mTaskHandle, mNotify, eSetBits) == pdPASS);
		}
		else
			return true;
	}
	else
	{
		if (free_mem)
			vPortFree(msg->msgBody);
		TRACE_WARNING(pcTaskGetName(mTaskHandle), msg->msgID);
		return false;
	}
}

bool IRAM_ATTR CBaseTask::sendMessageFromISR(STaskMessage *msg, BaseType_t *pxHigherPriorityTaskWoken)
{
	assert(msg != nullptr);

	if (xQueueOverwriteFromISR(mTaskQueue, msg, pxHigherPriorityTaskWoken) == pdPASS)
	{
		if (mNotify != 0)
		{
			return (xTaskNotifyFromISR(mTaskHandle, mNotify, eSetBits, pxHigherPriorityTaskWoken) == pdPASS);
		}
		else
			return true;
	}
	else
		return false;
}

bool IRAM_ATTR CBaseTask::sendMessageFrontFromISR(STaskMessage *msg, BaseType_t *pxHigherPriorityTaskWoken)
{
	assert(msg != nullptr);

	if (xQueueSendToFrontFromISR(mTaskQueue, msg, pxHigherPriorityTaskWoken) != pdPASS)
	{
		if (xQueueOverwriteFromISR(mTaskQueue, msg, pxHigherPriorityTaskWoken) != pdPASS)
		{
			return false;
		}
	}
	if (mNotify != 0)
	{
		return (xTaskNotifyFromISR(mTaskHandle, mNotify, eSetBits, pxHigherPriorityTaskWoken) == pdPASS);
	}
	else
		return true;
}

bool CBaseTask::getMessage(STaskMessage *msg, TickType_t xTicksToWait)
{
	assert(msg != nullptr);

	return (xQueueReceive(mTaskQueue, msg, xTicksToWait) == pdTRUE);
}

uint8_t *CBaseTask::allocNewMsg(STaskMessage *msg, uint16_t cmd, uint16_t size)
{
	assert(msg != nullptr);
	assert(size > 0);

	msg->msgID = cmd;
	msg->shortParam = size;
	msg->msgBody = pvPortMalloc(msg->shortParam);
	return (uint8_t *)msg->msgBody;
}
