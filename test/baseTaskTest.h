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

#define BASETASKTEST_QUEUE_BIT 			(1)						///< Номер бита уведомления о сообщении в очереди.
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
	virtual ~CBaseTaskTest(){};
};
/*! @} */

#endif // CBASETASKTEST_H

