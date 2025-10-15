/*!
	\file
	\brief Hardware timer for FreeRTOS tasks.
	\authors Bliznets R.A. (r.bliznets@gmail.com)
	\version 1.3.0.0
	\date 31.03.2023
	\details This header file defines the CDelayTimer class, which wraps an ESP-IDF
			 general-purpose hardware timer (gptimer) for use within FreeRTOS tasks.
			 It allows tasks to be notified or sent messages upon timer expiration.
*/

#pragma once

#include "driver/gptimer.h" // ESP-IDF driver header for general-purpose timers
#include "CBaseTask.h"		// Header for the CBaseTask class (likely used for message sending)
#include "CSoftwareTimer.h" // Header for CSoftwareTimer (defines ETimerEvent)

/// Microsecond timer for FreeRTOS tasks.
/// @details This class provides an interface to control an ESP-IDF hardware timer.
///          It can notify the current task using FreeRTOS task notifications or
///          send messages to a specified CBaseTask-derived task upon alarm.
class CDelayTimer
{
protected:
	/// Handle for the ESP-IDF hardware timer.
	gptimer_handle_t mTimerHandle = nullptr;

	/// Configuration structure for the timer alarm.
	/// @details Configured with default values: alarm at 1 second (assuming 1MHz resolution),
	///          reload counter to 0 on alarm.
	gptimer_alarm_config_t m_alarm_config = {
		.alarm_count = 1000000, // period = 1s @resolution 1MHz
		.reload_count = 0,		// counter will reload with 0 on alarm event
		.flags = {0}};

	/// Flag indicating if the timer is currently running.
	bool mRun = false;

	/// Timer alarm callback function.
	/// @details This static function is registered with the ESP-IDF timer driver.
	///          It calls the member function `timer()` on the associated CDelayTimer object.
	/// @param[in] timer Timer handle created by `gptimer_new_timer`
	/// @param[in] edata Alarm event data, provided by the driver.
	/// @param[in] user_ctx User data (pointer to this CDelayTimer instance), passed from `gptimer_register_event_callbacks`
	/// @return Whether a high-priority task has been woken up by this function (pdTRUE/pdFALSE).
	static bool timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx);

	/// Task handle for the task to be notified upon timer alarm.
	TaskHandle_t mTaskToNotify;

	/// Notification bit number (0-31) used to notify the task about the timer event.
	uint8_t mNotifyBit;

	/// (Note: Comment seems to describe mAutoRefresh, but code initializes m_alarm_config.flags.auto_reload_on_alarm)
	/// Flag for timer auto-reload (restarting after expiration).
	/// bool mAutoRefresh; // This member variable seems unused; auto-reload is controlled via m_alarm_config.flags.auto_reload_on_alarm

	/// Command ID number used when sending a message to notify the task about the timer event.
	uint16_t mTimerCmd;

	/// Pointer to the CBaseTask instance to which a message will be sent upon timer alarm.
	CBaseTask *mTask = nullptr;

	/// Specifies the method of communication upon timer alarm (Notify, SendBack, SendFront).
	ETimerEvent mEventType;

	/// Function called when a timer event occurs.
	/// @details Handles sending the notification or message based on `mEventType`.
	void timer();

public:
	/// Constructor.
	/// @details Initializes the hardware timer driver and registers the callback.
	/// @param[in] xNotifyBit Notification bit number (0-31) for task notifications (default 0).
	/// @param[in] timerCmd Command ID number used when sending messages (default 10000).
	CDelayTimer(uint8_t xNotifyBit = 0, uint16_t timerCmd = 10000);

	/// Destructor.
	/// @details Stops and deletes the associated hardware timer.
	~CDelayTimer();

	/// Start the timer with task notification.
	/// @warning Must be called only from a FreeRTOS task.
	/// @param[in] xNotifyBit Notification bit number (0-31) for the current task.
	/// @param[in] period Period in microseconds.
	/// @param[in] autoRefresh Flag for timer auto-reload. If false, the timer runs once.
	/// @return 0 on success, negative value on error.
	/// @sa stop()
	int start(uint8_t xNotifyBit, uint32_t period, bool autoRefresh = false);

	/// Start the timer to send a message to a CBaseTask.
	/// @warning Must be called only from a FreeRTOS task.
	/// @param[in] task Pointer to the CBaseTask to receive the message.
	/// @param[in] event Type of event (SendBack, SendFront) determining message placement in the queue.
	/// @param[in] period Period in microseconds.
	/// @param[in] autoRefresh Flag for timer auto-reload. If false, the timer runs once.
	/// @return 0 on success, negative value on error.
	/// @sa stop()
	int start(CBaseTask *task, ETimerEvent event, uint32_t period, bool autoRefresh = false);

	/// Stop the timer.
	/// @return 0 on success, -1 if the timer was already stopped.
	/// @sa start()
	int stop();

	/// Wait for the timer to expire.
	/// @warning Must be called only from a FreeRTOS task.
	/// @param[in] period Period in microseconds.
	/// @param[in] xNotifyBit Notification bit number (0-31) used for waiting (default 0).
	/// @return 0 on success, negative value on error (e.g., timeout).
	int wait(uint32_t period, uint8_t xNotifyBit = 0);
};