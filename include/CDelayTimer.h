/*!
	\file
	\brief Аппаратный таймер под задачи FreeRTOS.
	\authors Близнец Р.А. (r.bliznets@gmail.com)
	\version 1.3.0.0
	\date 31.03.2023
*/

#pragma once

#include "driver/gptimer.h"
#include "CBaseTask.h"
#include "CSoftwareTimer.h"

/// Микросекундный таймер под задачи FreeRTOS.
class CDelayTimer
{
protected:
	gptimer_handle_t mTimerHandle = nullptr; ///< Хэндлер таймера.
	gptimer_alarm_config_t m_alarm_config = {
		.alarm_count = 1000000, // period = 1s @resolution 1MHz
		.reload_count = 0,		// counter will reload with 0 on alarm event
		.flags = {0}};

	bool mRun = false; ///< Флаг включенного таймера.
	/// Timer alarm callback
	/*!
	 \param[in] timer Timer handle created by `gptimer_new_timer`
	 \param[in] edata Alarm event data, fed by driver
	 \param[in] user_ctx User data, passed from `gptimer_register_event_callbacks`
	 \return Whether a high priority task has been waken up by this function
	 */
	static bool timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx);

	TaskHandle_t mTaskToNotify; ///< Указатель на задачу, ожидающую события от таймера.
	uint8_t mNotifyBit;			///< Номер бита для оповещения задачи о событии таймера (не более 31).
	bool mAutoRefresh;			///< Флаг автозагрузки таймера.
	uint16_t mTimerCmd;			///< Номер команды для оповещения задачи о событии таймера (не более 31).
	CBaseTask *mTask = nullptr; ///< Указатель на задачу, ожидающую события от таймера.
	ETimerEvent mEventType;		///< Метод сообщения.

	/// Функция, вызываемая по событию в таймере.
	void timer();

public:
	/// Конструктор.
	/*!
		Захват таймера
	  \param[in] xNotifyBit Номер бита для оповещения задачи о событии таймера.
	  \param[in] timerCmd Номер команды для оповещения задачи о событии таймера.
	*/
	CDelayTimer(uint8_t xNotifyBit = 0, uint16_t timerCmd = 10000);
	/// Деструктор.
	~CDelayTimer();

	/// Запуск таймера.
	/*!
	  \warning Вызывать только из задачи FreeRTOS.
	  \param[in] xNotifyBit Номер бита для оповещения задачи о событии таймера.
	  \param[in] period Период в микросекундах.
	  \param[in] autoRefresh Флаг автозагрузки таймера. Если false, то таймер запускается один раз.
	  \return 0 - в случае успеха.
	  \sa Stop()
	*/
	int start(uint8_t xNotifyBit, uint32_t period, bool autoRefresh = false);
	int start(CBaseTask *task, ETimerEvent event, uint32_t period, bool autoRefresh = false);
	/// Остановка таймера.
	/*!
	  \return 0 - в случае успеха.
	  \sa Start()
	*/
	int stop();

	/// Ожидание окончания таймера.
	/*!
	  \warning Вызывать только из задачи FreeRTOS.
	  \param[in] period Период в микросекундах.
	  \param[in] xNotifyBit Номер бита для оповещения задачи о событии таймера.
	  \return 0 - в случае успеха.
	*/
	int wait(uint32_t period, uint8_t xNotifyBit = 0);
};
