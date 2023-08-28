/*!
	\file
	\brief Класс для тестирования CBaseTask.
	\authors Близнец Р.А.
	\version 0.0.0.1
	\date 05.05.2022
	\copyright (c) Copyright 2022, ООО "Глобал Ориент", Москва, Россия, http://www.glorient.ru/
*/

#if !defined CBASETASKTEST_H
#define CBASETASKTEST_H

#include "CBaseTask.h"

#define BASETASKTEST_QUEUE_BIT 			(31)						///< Номер бита уведомления о сообщении в очереди.
#define BASETASKTEST_QUEUE_FLAG 		(1 << BASETASKTEST_QUEUE_BIT)	///< Флаг уведомления о сообщении в очереди.

#define MSG_TERMINATE 0
#define MSG_ECHO 1

/// Класс для реализации задачи FreeRTOS основной логики работы.
class CBaseTaskTest : public CBaseTask
{
protected:
	/// Функция задачи.
	virtual void run() override;

public:
    bool mFlag=false;

	/// Послать сообщение в задачу.
	/*!
	  \param[in] msg Указатель на сообщение.
	  \param[in] xTicksToWait Время ожидания в тиках.
	  \param[in] free вернуть память в кучу в случае неудачи.
	  \return true в случае успеха.
	*/
	inline bool sendMessage(STaskMessage* msg,TickType_t xTicksToWait=0, bool free=false)
	{
		return CBaseTask::sendMessage(msg, BASETASKTEST_QUEUE_FLAG,xTicksToWait,free);
	};

	/// Послать сообщение в задачу из прерывания.
	/*!
	  \param[in] msg Указатель на сообщение.
	  \param[out] pxHigherPriorityTaskWoken Флаг переключения задач.
	  \return true в случае успеха.
	*/
	inline bool sendMessageFromISR(STaskMessage* msg,BaseType_t *pxHigherPriorityTaskWoken) override
	{
		return CBaseTask::sendMessageFromISR(msg,pxHigherPriorityTaskWoken,BASETASKTEST_QUEUE_FLAG);
	};
};
/*! @} */

#endif // CBASETASKTEST_H

