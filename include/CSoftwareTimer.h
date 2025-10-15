/*!
	\file
	\brief Software timer for FreeRTOS tasks.
	\authors Bliznets R.A. (r.bliznets@gmail.com)
	\version 1.2.0.0
	\date 28.04.2020
	\details This header file defines the CSoftwareTimer class, which wraps FreeRTOS
			 software timers (TimerHandle_t) to provide a mechanism for triggering
			 actions in FreeRTOS tasks after a specified time period. It supports
			 notifying the task directly or sending a message via a CBaseTask's queue.
*/

#pragma once

#include "CBaseTask.h"		 // Include header for CBaseTask (used for message sending)
#include "freertos/timers.h" // Include FreeRTOS timer header
#include "esp_pm.h"			 // Include ESP-IDF power management header (currently commented out)

/// Method for the timer to communicate its event.
/// @details Defines how the timer signals its expiration to the target task.
enum class ETimerEvent
{
	Notify,	  ///< Notify the task using FreeRTOS task notifications.
	SendBack, ///< Send a message to the back of the task's queue (CBaseTask).
	SendFront ///< Send a message to the front of the task's queue (CBaseTask).
};

/// Software timer for FreeRTOS tasks.
/// @details This class encapsulates a FreeRTOS software timer. It allows configuring
///          the timer to either notify a task using a specific bit or send a command
///          message to a CBaseTask-derived task upon expiration. It supports both
///          one-shot and auto-reload modes.
class CSoftwareTimer
{
protected:
	// #if CONFIG_PM_ENABLE
	// 	esp_pm_lock_handle_t mPMLock; ///< Flag to prevent sleep mode (currently commented out)
	// #endif

	/// FreeRTOS timer handle for this instance.
	TimerHandle_t mTimerHandle = nullptr;

	/// Notification bit number (0-31) used to notify the task about the timer event.
	uint8_t mNotifyBit;

	/// Command ID number used when sending a message to notify the task about the timer event.
	uint16_t mTimerCmd;

	/// Task handle for the task to be notified upon timer expiration.
	TaskHandle_t mTaskToNotify;

	/// Pointer to the CBaseTask instance to which a message will be sent upon timer expiration.
	CBaseTask *mTask = nullptr;

	/// Specifies the method of communication upon timer expiration (Notify, SendBack, SendFront).
	ETimerEvent mEventType;

	/// Timer event handler callback.
	/// @details This static function is registered with the FreeRTOS timer system.
	///          It calls the `timer()` method on the associated CSoftwareTimer object.
	/// @param[in] xTimer The FreeRTOS TimerHandle_t that expired.
	static void vTimerCallback(TimerHandle_t xTimer);

	/// Function called when a timer event occurs.
	/// @details Handles sending the notification or message based on `mEventType`.
	inline void timer();

public:
	/// Constructor.
	/// @details Creates the underlying FreeRTOS timer with default settings.
	/// @param[in] xNotifyBit Notification bit number (0-31) for task notifications.
	/// @param[in] timerCmd Command ID number used when sending messages (default 10000).
	CSoftwareTimer(uint8_t xNotifyBit, uint16_t timerCmd = 10000);

	/// Destructor.
	/// @details Stops and deletes the underlying FreeRTOS timer.
	~CSoftwareTimer();

	/// Start the timer (event via task notification).
	/// @warning Must be called only from a FreeRTOS task.
	/// @param[in] period Period in milliseconds.
	/// @param[in] autoRefresh Flag for timer auto-reload. If false, the timer runs once.
	/// @return 0 on success, negative value on error.
	/// @sa stop()
	int start(uint32_t period, bool autoRefresh = false);

	/// Start the timer (event via message to CBaseTask).
	/// @param[in] task Pointer to the CBaseTask to receive the message.
	/// @param[in] event Type of event (Notify, SendBack, SendFront).
	/// @param[in] period Period in milliseconds.
	/// @param[in] autoRefresh Flag for timer auto-reload. If false, the timer runs once.
	/// @return 0 on success, negative value on error.
	/// @sa stop()
	int start(CBaseTask *task, ETimerEvent event, uint32_t period, bool autoRefresh = false);

	/// Stop the timer.
	/// @return 0 on success, negative value on error.
	/// @sa start()
	int stop();

	/// Check if the timer is running.
	/// @return true if the timer is active (running), false otherwise.
	inline bool isRun()
	{
		return xTimerIsTimerActive(mTimerHandle) == pdTRUE;
	};
};