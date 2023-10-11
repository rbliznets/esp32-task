/*!
	\file
	\brief Базовый класс для реализации задачи FreeRTOS в многоядерном CPU.
	\authors Близнец Р.А.
	\version 1.1.0.0
	\date 28.04.2020
*/

#if !defined CBASETASK_H
#define CBASETASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/// Структура сообщения между задачами.
struct STaskMessage
{
	uint16_t msgID;		 ///< Тип сообщения.
	uint16_t shortParam; ///< Параметр команды.
	union
	{
		struct
		{
			uint16_t param1; ///< Параметр сообщения.
			uint16_t param2; ///< Параметр сообщения.
		};
		uint32_t paramID; ///< Параметр сообщения.
		void *msgBody;	  ///< Указатель на тело сообщение.
	};
};

/// Базовый абстрактный класс для реализации задачи FreeRTOS.
class CBaseTask
{
protected:
	TaskHandle_t mTaskHandle = nullptr; ///< Хэндлер задачи FreeRTOS.
	QueueHandle_t mTaskQueue = nullptr; ///< Приемная очередь сообщений.

	/// Функция задачи FreeRTOS.
	/*!
	  \param[in] pvParameters Параметр (указатель на объект CBaseTask).
	*/
	static void vTask(void *pvParameters);

	/// Функция задачи для переопределения в потомках.
	virtual void run() = 0;

	/// Получить сообщение из очереди.
	/*!
	  \param[out] msg Указатель на сообщение.
	  \param[in] xTicksToWait Время ожидания в тиках.
	  \return true в случае успеха.
	*/
	bool getMessage(STaskMessage *msg, TickType_t xTicksToWait = 0);

	/// Послать сообщение в задачу.
	/*!
	  \param[in] msg Указатель на сообщение.
	  \param[in] nFlag Флаг сообщения о не пустой очереди для задачи. Если 0, то механизм Notify не используется.
	  \param[in] xTicksToWait Время ожидания в тиках.
	  \param[in] free_mem вернуть память в кучу в случае неудачи.
	  \return true в случае успеха.
	*/
	bool sendMessage(STaskMessage *msg, uint32_t nFlag, TickType_t xTicksToWait = 0, bool free_mem = false);
	/// Послать сообщение в задачу в начало очереди.
	/*!
	  \param[in] msg Указатель на сообщение.
	  \param[in] nFlag Флаг сообщения о не пустой очереди для задачи. Если 0, то механизм Notify не используется.
	  \param[in] xTicksToWait Время ожидания в тиках.
	  \param[in] free_mem вернуть память в кучу в случае неудачи.
	  \return true в случае успеха.
	*/
	bool sendMessageFront(STaskMessage *msg, uint32_t nFlag, TickType_t xTicksToWait = 0, bool free_mem = false);
	/// Послать сообщение в задачу из прерывания.
	/*!
	  \param[in] msg Указатель на сообщение.
	  \param[out] pxHigherPriorityTaskWoken Флаг переключения задач.
	  \param[in] nFlag Флаг сообщения о не пустой очереди для задачи. Если 0, то механизм Notify не используется.
	  \return true в случае успеха.
	*/
	bool IRAM_ATTR sendMessageFromISR(STaskMessage *msg, BaseType_t *pxHigherPriorityTaskWoken, uint32_t nFlag);

public:
	/// Начальная инициализация.
	/*!
	  \param[in] name Имя задачи длиной не более configMAX_TASK_NAME_LEN.
	  \param[in] usStack Размер стека в двойных словах (4 байта).
	  \param[in] uxPriority Приоритет. Не более configMAX_PRIORITIES.
	  \param[in] queueLength Максимальная длина очереди сообщений.
	  \param[in] coreID Ядро CPU (0,1).
	*/
	virtual void init(const char *name, unsigned short usStack, UBaseType_t uxPriority, UBaseType_t queueLength, BaseType_t coreID = tskNO_AFFINITY);
	/// Деструктор.
	virtual ~CBaseTask();

	/// Послать сообщение в задачу из прерывания.
	/*!
	  \param[in] msg Указатель на сообщение.
	  \param[out] pxHigherPriorityTaskWoken Флаг переключения задач.
	  \return true в случае успеха.
	*/
	virtual bool sendMessageFromISR(STaskMessage *msg, BaseType_t *pxHigherPriorityTaskWoken) = 0;
	/// Послать сообщение в задачу.
	/*!
	  \param[in] msg Указатель на сообщение.
	  \param[in] xTicksToWait Время ожидания в тиках.
	  \param[in] free вернуть память в кучу в случае неудачи.
	  \return true в случае успеха.
	*/
	virtual bool sendMessage(STaskMessage *msg, TickType_t xTicksToWait = 0, bool free = false) = 0;
	/// Послать простое сообщение в задачу.
	/*!
	  \param[in] msgID Тип сообщения.
	  \param[in] shortParam Параметр команды.
	  \param[in] paramID Поле команды.
	  \return true в случае успеха.
	*/
	inline bool sendCmd(uint16_t msgID, uint16_t shortParam = 0, uint32_t paramID = 0)
	{
		STaskMessage msg;
		msg.msgID = msgID;
		msg.shortParam = shortParam;
		msg.paramID = paramID;
		return sendMessage(&msg, 0, false);
	}

	/// Выделить память сообщению.
	/*!
	  \param[in] msg Указатель на сообщение.
	  \param[in] cmd Номер команды.
	  \param[in] size Размер выделяемой памяти.
	  \return указатель на выделенную память.
	*/
	static uint8_t *allocNewMsg(STaskMessage *msg, uint16_t cmd, uint16_t size);

	/// Признак запущенной задачи.
	/*!
	  \return Признак запущенной задачи.
	*/
	inline bool isRun() { return mTaskQueue != nullptr; };
};

#endif // CBASETASK_H
