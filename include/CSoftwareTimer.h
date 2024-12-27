/*!
	\file
	\brief Программный таймер под задачи FreeRTOS.
    \authors Близнец Р.А. (r.bliznets@gmail.com)
	\version 1.2.0.0
	\date 28.04.2020
*/

#pragma once

#include "CBaseTask.h"
#include "freertos/timers.h"
#include "esp_pm.h"

/// Метод сообщения от таймера.
enum class ETimerEvent
{
	Notify,	 ///< Через notify.
	SendBack, ///< Через очередь.
	SendFront ///< Через очередь.
};

/// Программный таймер под задачи FreeRTOS.
class CSoftwareTimer
{
protected:
#if CONFIG_PM_ENABLE
	esp_pm_lock_handle_t mPMLock; ///< флаг запрета на sleep
#endif
	TimerHandle_t mTimerHandle = nullptr; ///< Хэндлер таймера FreeRTOS.
	uint8_t mNotifyBit;					  ///< Номер бита для оповещения задачи о событии таймера (не более 31).
	uint16_t mTimerCmd;					  ///< Номер команды для оповещения задачи о событии таймера (не более 31).

	TaskHandle_t mTaskToNotify; ///< Указатель на задачу, ожидающую события от таймера.
	CBaseTask *mTask = nullptr; ///< Указатель на задачу, ожидающую события от таймера.
	ETimerEvent mEventType;		///< Метод сообщения.

	/// Обработчик события таймера.
	/*!
	\param[in] xTimer Хэндлер таймера FreeRTOS.
	*/
	static void vTimerCallback(TimerHandle_t xTimer);

	/// Функция, вызываемая по событию в таймере.
	inline void timer();

public:
	/// Конструктор.
	/*!
	  \param[in] xNotifyBit Номер бита для оповещения задачи о событии таймера.
	  \param[in] timerCmd Номер команды для оповещения задачи о событии таймера.
	*/
	CSoftwareTimer(uint8_t xNotifyBit, uint16_t timerCmd = 10000);
	/// Деструктор.
	~CSoftwareTimer();

	/// Запуск таймера (событие через notify).
	/*!
	  \warning Вызывать только из задачи FreeRTOS.
	  \param[in] period Период в миллисекундах.
	  \param[in] autoRefresh Флаг автозагрузки таймера. Если false, то таймер запускается один раз.
	  \return 0 - в случае успеха.
	  \sa Stop()
	*/
	int start(uint32_t period, bool autoRefresh = false);
	/// Запуск таймера.
	/*!
	  \param[in] task Задача для сообщений таймера
	  \param[in] event Тип сообщения
	  \param[in] period Период в миллисекундах.
	  \param[in] autoRefresh Флаг автозагрузки таймера. Если false, то таймер запускается один раз.
	  \return 0 - в случае успеха.
	  \sa Stop()
	*/
	int start(CBaseTask *task, ETimerEvent event, uint32_t period, bool autoRefresh = false);
	/// Остановка таймера.
	/*!
	  \return 0 - в случае успеха.
	  \sa Start()
	*/
	int stop();

	/// Состояние таймера.
	/*!
	  \return Состояние таймера.
	*/
	inline bool isRun()
	{
		return xTimerIsTimerActive(mTimerHandle) == pdTRUE;
	};
};

