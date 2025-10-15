/*!
	\file
	\brief Software timer for FreeRTOS tasks.
	\authors Bliznets R.A. (r.bliznets@gmail.com)
	\version 1.2.0.0
	\date 28.04.2020
	\copyright (c) Copyright 2021, LLC "Global Orient", Moscow, Russia, http://www.glorient.ru/
	\details This file implements the CSoftwareTimer class, which wraps FreeRTOS software timers
			 to provide a convenient way to trigger actions (notifications or message sends)
			 in FreeRTOS tasks after a specified time period.
*/

#include "CSoftwareTimer.h"
#include <cstdio>
#include "CTrace.h"

// Constructor for the CSoftwareTimer class
CSoftwareTimer::CSoftwareTimer(uint8_t xNotifyBit, uint16_t timerCmd)
{
	// Verify that the notification bit number is less than 32 (as it's used in a bitmask)
	assert(xNotifyBit < 32);

	// If power management support is enabled, create a lock to prevent light sleep mode
	// #if CONFIG_PM_ENABLE
	// 	esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, "st", &mPMLock);
	// #endif

	// Initialize class members
	mNotifyBit = xNotifyBit;
	mTimerCmd = timerCmd;

	// Create a FreeRTOS timer named "STimer", with a period of 1000 ms (pdMS_TO_TICKS(1000)),
	// one-shot mode (pdFALSE), passing 'this' as the timer ID, and setting the callback function
	mTimerHandle = xTimerCreate("STimer", pdMS_TO_TICKS(1000), pdFALSE, this, CSoftwareTimer::vTimerCallback);

	// Check if the timer was created successfully
	if (mTimerHandle == nullptr)
		TRACE_ERROR("CSoftwareTimer has not created", -1);
}

// Destructor for the CSoftwareTimer class
CSoftwareTimer::~CSoftwareTimer()
{
	// If the timer was created, stop and delete it
	if (mTimerHandle != nullptr)
	{
		stop();
		xTimerDelete(mTimerHandle, 1);
	}

	// Delete the power management lock if it was created
	// #if CONFIG_PM_ENABLE
	// 	esp_pm_lock_delete(mPMLock);
	// #endif
}

// Timer callback function (executed when the timer expires)
void CSoftwareTimer::vTimerCallback(TimerHandle_t xTimer)
{
	// Get the pointer to the CSoftwareTimer object from the timer's ID data
	CSoftwareTimer *tm = (CSoftwareTimer *)pvTimerGetTimerID(xTimer);

	// Call the timer() method on the CSoftwareTimer object
	tm->timer();
}

// Method to start the timer with a specified period and auto-reload mode
int CSoftwareTimer::start(uint32_t period, bool autoRefresh)
{
	// Verify that the period is greater than 0
	assert(pdMS_TO_TICKS(period) > 0);

	// Stop the timer before starting with new parameters
	stop();

	// Set the event type to notification and get the current task handle for notification
	mEventType = ETimerEvent::Notify;
	mTaskToNotify = xTaskGetCurrentTaskHandle();

	// Set the timer's reload mode (one-shot or periodic)
	vTimerSetReloadMode(mTimerHandle, autoRefresh);

	// Change the timer's period to the specified value
	if (xTimerChangePeriod(mTimerHandle, pdMS_TO_TICKS(period), 1) != pdTRUE)
	{
		TRACE_ERROR("CSoftwareTimer:xTimerChangePeriod failed", (uint16_t)period);
		return -2;
	}

	// Start the timer
	if (xTimerStart(mTimerHandle, 1) == pdTRUE)
	{
		// #if CONFIG_PM_ENABLE
		// 		// Acquire the power management lock to prevent light sleep mode
		// 		esp_pm_lock_acquire(mPMLock);
		// #endif
		return 0;
	}
	else
	{
		TRACE_ERROR("CSoftwareTimer:xTimerStart failed", (uint16_t)period);
		return -1;
	}
}

// Overloaded start method to send a message to a CBaseTask
int CSoftwareTimer::start(CBaseTask *task, ETimerEvent event, uint32_t period, bool autoRefresh)
{
	// Verify that the passed task pointer is not nullptr
	assert(task != nullptr);

	// Verify that the period is greater than 0
	assert(pdMS_TO_TICKS(period) > 0);

	// Stop the timer before starting with new parameters
	stop();

	// Set the event type and the task to notify/send message to
	mEventType = event;
	mTask = task;
	mTaskToNotify = mTask->getTask();

	// Set the timer's reload mode (one-shot or periodic)
	vTimerSetReloadMode(mTimerHandle, autoRefresh);

	// Change the timer's period to the specified value
	if (xTimerChangePeriod(mTimerHandle, pdMS_TO_TICKS(period), 1) != pdTRUE)
	{
		TRACE_ERROR("CSoftwareTimer:xTimerChangePeriod failed", (uint16_t)period);
		return -2;
	}

	// Start the timer
	if (xTimerStart(mTimerHandle, 1) == pdTRUE)
	{
		return 0;
	}
	else
	{
		TRACE_ERROR("CSoftwareTimer:xTimerStart failed", (uint16_t)period);
		return -1;
	}
}

// Method to stop the timer
int CSoftwareTimer::stop()
{
	// Check if the timer is running
	if (isRun())
	{
		// Stop the timer
		if (xTimerStop(mTimerHandle, 1) == pdTRUE)
		{
			// #if CONFIG_PM_ENABLE
			// 			// Release the power management lock
			// 			esp_pm_lock_release(mPMLock);
			// #endif
			return 0;
		}
		else
		{
			TRACE_ERROR("CSoftwareTimer:xTimerStop failed", -2);
			return -2;
		}
	}
	else
	{
		// Log information that the timer is not running (commented out)
		// ESP_LOGI("CSoftwareTimer", "mTimerHandle==NULL");
		return -1;
	}
}

// Method called by the timer callback to handle the timer event
void CSoftwareTimer::timer()
{
	// Handle notification event
	if (mEventType == ETimerEvent::Notify)
		xTaskNotify(mTaskToNotify, (1 << mNotifyBit), eSetBits);

	// Handle send-back event (sends a command message to the task)
	else if (mEventType == ETimerEvent::SendBack)
		mTask->sendCmd(mTimerCmd, 0, 0, 1);
}