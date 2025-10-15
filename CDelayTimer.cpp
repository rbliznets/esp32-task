/*!
	\file
	\brief Hardware timer for FreeRTOS tasks.
	\authors Bliznets R.A. (r.bliznets@gmail.com)
	\version 1.3.0.0
	\date 31.03.2023
	\details The CDelayTimer class implements management of a hardware timer for use in FreeRTOS tasks.
			 It supports task notifications and sending messages when a specified time interval elapses.
*/

#include "CDelayTimer.h" // Header file for the CDelayTimer class
#include <cstdio>		 // Standard library for input/output operations
#include "CTrace.h"		 // Library for tracing and logging

/**
 * \brief Constructor for the CDelayTimer class.
 * \param xNotifyBit Notification bit (0-31) used for notifying the task.
 * \param timerCmd Timer command passed in the message.
 * \details Initializes the timer with default settings: frequency 1 MHz, counting up.
 *          Registers a callback function to handle timer events.
 */
CDelayTimer::CDelayTimer(uint8_t xNotifyBit, uint16_t timerCmd) : mNotifyBit(xNotifyBit), mTimerCmd(timerCmd)
{
	assert(xNotifyBit < 32); // Verify the notification bit is valid (0-31)

	// Configure the timer settings
	gptimer_config_t mTimer_config = {
		.clk_src = GPTIMER_CLK_SRC_DEFAULT, // Default clock source
		.direction = GPTIMER_COUNT_UP,		// Count up
		.resolution_hz = 1000000,			// Frequency 1 MHz (1 tick = 1 microsecond)
		.intr_priority = 0,					// Interrupt priority
		.flags = {1, 0, 0}					// Configuration flags
	};

	// Register the callback function for handling timer events
	gptimer_event_callbacks_t cbs = {
		.on_alarm = timer_on_alarm_cb // Pointer to the callback function
	};

	esp_err_t er;
	// Create a new timer
	if ((er = gptimer_new_timer(&mTimer_config, &mTimerHandle)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_new_timer failed", er); // Log error
	}
	else
	{
		// Register the callback function
		if ((er = gptimer_register_event_callbacks(mTimerHandle, &cbs, this)) != ESP_OK)
		{
			TRACE_ERROR("CDelayTimer:gptimer_register_event_callbacks failed", er); // Log error
			gptimer_del_timer(mTimerHandle);										// Delete timer on error
		}
	}
}

/**
 * \brief Destructor for the CDelayTimer class.
 * \details Stops the timer and deletes it.
 */
CDelayTimer::~CDelayTimer()
{
	stop();											// Stop the timer
	esp_err_t er = gptimer_del_timer(mTimerHandle); // Delete the timer
	if (er != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_del_timer failed", er); // Log error
	}
}

/**
 * \brief Callback function called when the timer triggers an alarm.
 * \param timer Timer handle.
 * \param edata Timer event data.
 * \param user_ctx Pointer to the CDelayTimer object.
 * \return Always returns true.
 * \details Calls the timer() method of the CDelayTimer object.
 */
bool IRAM_ATTR CDelayTimer::timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
	CDelayTimer *tm = (CDelayTimer *)user_ctx; // Cast the pointer
	tm->timer();							   // Call the timer handling method
	return true;
}

/**
 * \brief Starts the timer to notify a task.
 * \param xNotifyBit Notification bit (0-31).
 * \param period Timer period in microseconds.
 * \param autoRefresh Flag for automatic timer restart.
 * \return 0 on success, otherwise an error code.
 * \details Configures the timer to notify the current task when the period expires.
 */
int IRAM_ATTR CDelayTimer::start(uint8_t xNotifyBit, uint32_t period, bool autoRefresh)
{
	assert(xNotifyBit < 32);		   // Verify the notification bit is valid (0-31)
	assert(pdMS_TO_TICKS(period) > 0); // Verify the period is valid

	esp_err_t er;
	stop(); // Stop the timer before configuring

	mTaskToNotify = xTaskGetCurrentTaskHandle();			 // Get the handle of the current task
	mNotifyBit = xNotifyBit;								 // Set the notification bit
	mEventType = ETimerEvent::Notify;						 // Set the event type
	m_alarm_config.alarm_count = period;					 // Set the timer period
	m_alarm_config.flags.auto_reload_on_alarm = autoRefresh; // Configure auto-reload

	// Configure the timer action on alarm
	if ((er = gptimer_set_alarm_action(mTimerHandle, &m_alarm_config)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_set_alarm_action failed!", er); // Log error
		return -3;
	}

	// Enable the timer
	if ((er = gptimer_enable(mTimerHandle)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_enable failed", er); // Log error
		return -4;
	}

	// Reset the timer counter
	if ((er = gptimer_set_raw_count(mTimerHandle, 0)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_set_raw_count failed", er); // Log error
		return -5;
	}

	// Start the timer
	if ((er = gptimer_start(mTimerHandle)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_start failed!", er); // Log error
		gptimer_disable(mTimerHandle);						  // Disable timer on error
		return -6;
	}

	mRun = true; // Set the timer running flag
	return 0;
}

/**
 * \brief Starts the timer to send a message to a task.
 * \param task Pointer to the task to which the message will be sent.
 * \param event Type of event (sending message to front or back of queue).
 * \param period Timer period in microseconds.
 * \param autoRefresh Flag for automatic timer restart.
 * \return 0 on success, otherwise an error code.
 * \details Configures the timer to send a message to the task when the period expires.
 */
int IRAM_ATTR CDelayTimer::start(CBaseTask *task, ETimerEvent event, uint32_t period, bool autoRefresh)
{
	assert(task != nullptr);		   // Verify the task pointer is valid
	assert(pdMS_TO_TICKS(period) > 0); // Verify the period is valid

	esp_err_t er;
	stop(); // Stop the timer before configuring

	mEventType = event;				  // Set the event type
	mTask = task;					  // Set the target task
	mTaskToNotify = mTask->getTask(); // Get the task handle

	m_alarm_config.alarm_count = period;					 // Set the timer period
	m_alarm_config.flags.auto_reload_on_alarm = autoRefresh; // Configure auto-reload

	// Configure the timer action on alarm
	if ((er = gptimer_set_alarm_action(mTimerHandle, &m_alarm_config)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_set_alarm_action failed!", er); // Log error
		return -3;
	}

	// Enable the timer
	if ((er = gptimer_enable(mTimerHandle)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_enable failed", er); // Log error
		return -4;
	}

	// Reset the timer counter
	if ((er = gptimer_set_raw_count(mTimerHandle, 0)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_set_raw_count failed", er); // Log error
		return -5;
	}

	// Start the timer
	if ((er = gptimer_start(mTimerHandle)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_start failed!", er); // Log error
		gptimer_disable(mTimerHandle);						  // Disable timer on error
		return -6;
	}

	mRun = true; // Set the timer running flag
	return 0;
}

/**
 * \brief Stops the timer.
 * \return 0 on success, otherwise -1.
 * \details Stops and disables the timer.
 */
int IRAM_ATTR CDelayTimer::stop()
{
	if (mRun)
	{
		gptimer_stop(mTimerHandle);	   // Stop the timer
		gptimer_disable(mTimerHandle); // Disable the timer
		mRun = false;				   // Clear the timer running flag
		return 0;
	}
	else
	{
		return -1; // Timer is already stopped
	}
}

/**
 * \brief Waits for the timer to trigger.
 * \param period Wait period in microseconds.
 * \param xNotifyBit Notification bit (0-31).
 * \return 0 on success, otherwise an error code.
 * \details Starts the timer and waits for its notification.
 */
int CDelayTimer::wait(uint32_t period, uint8_t xNotifyBit)
{
	if (start(xNotifyBit, period, false) != 0) // Start the timer
		return -1;

	uint32_t flag = 0;
	// Wait for the timer notification
	if (xTaskNotifyWait(0, (1 << xNotifyBit), &flag, pdMS_TO_TICKS((period / 1000) + 10)) != pdTRUE)
	{
		stop(); // Stop the timer on timeout
		return -2;
	}

	stop(); // Stop the timer after successful notification
	return 0;
}

/**
 * \brief Handler for the timer trigger event.
 * \details Sends a notification or message to the task based on the settings.
 */
void IRAM_ATTR CDelayTimer::timer()
{
	BaseType_t do_yield = pdFALSE;
	if (mEventType == ETimerEvent::Notify)
	{
		// Notify the task
		xTaskNotifyFromISR(mTaskToNotify, (1 << mNotifyBit), eSetBits, &do_yield);
	}
	else
	{
		// Send a message to the task
		STaskMessage msg;
		msg.msgID = mTimerCmd; // Set the message command
		if (mEventType == ETimerEvent::SendBack)
		{
			mTask->sendMessageFromISR(&msg, &do_yield); // Send to the back of the queue
		}
		else
		{
			mTask->sendMessageFrontFromISR(&msg, &do_yield); // Send to the front of the queue
		}
	}
	// portYIELD_FROM_ISR(do_yield); // Potential call for rescheduling (commented out)
}