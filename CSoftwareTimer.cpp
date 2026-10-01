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

// Реестр живых таймеров. xTimerStop/xTimerDelete - асинхронные команды демону
// таймеров (Tmr Svc): при удалении объекта C++ колбэк истёкшего таймера может
// выполниться позже (и на другом ядре) с висячим pvTimerID. До фикса это
// приводило к периодической порче кучи под нагрузкой (WiFi scan): демон с
// приоритетом 1 голодал, команды stop/delete с таймаутом 1 тик не вставали в
// очередь (код возврата игнорировался), а "живой" таймер стрелял по
// освобождённой памяти.
static CSoftwareTimer *sAliveTimers[64];
static portMUX_TYPE sAliveMux = portMUX_INITIALIZER_UNLOCKED;
// Объект, чей колбэк демон таймеров выполняет прямо сейчас. Демон один на систему,
// поэтому одного указателя достаточно. Нужен, чтобы деструктор дождался выхода из
// уже начатого колбэка: одной проверки по реестру мало - между проверкой и вызовом
// tm->timer() объект может быть удалён на другом ядре (демон Tmr Svc прибит к CPU0,
// задачи bt/logic - к CPU1), и колбэк уйдёт по освобождённой памяти.
static CSoftwareTimer *sRunningCallback = nullptr;

static void alive_add(CSoftwareTimer *t)
{
	portENTER_CRITICAL(&sAliveMux);
	for (auto &slot : sAliveTimers)
	{
		if (slot == nullptr)
		{
			slot = t;
			break;
		}
	}
	portEXIT_CRITICAL(&sAliveMux);
}

static void alive_remove(CSoftwareTimer *t)
{
	portENTER_CRITICAL(&sAliveMux);
	for (auto &slot : sAliveTimers)
	{
		if (slot == t)
		{
			slot = nullptr;
			break;
		}
	}
	portEXIT_CRITICAL(&sAliveMux);
}

/// Пометить объект как занятый колбэком, если он ещё жив.
/// Проверка реестра и пометка выполняются атомарно - иначе деструктор,
/// стартовавший между ними, не узнает о колбэке в работе.
static bool alive_enter_callback(CSoftwareTimer *t)
{
	bool res = false;
	portENTER_CRITICAL(&sAliveMux);
	for (auto &slot : sAliveTimers)
	{
		if (slot == t)
		{
			res = true;
			sRunningCallback = t;
			break;
		}
	}
	portEXIT_CRITICAL(&sAliveMux);
	return res;
}

static void alive_exit_callback()
{
	portENTER_CRITICAL(&sAliveMux);
	sRunningCallback = nullptr;
	portEXIT_CRITICAL(&sAliveMux);
}

static bool alive_in_callback(CSoftwareTimer *t)
{
	portENTER_CRITICAL(&sAliveMux);
	bool res = (sRunningCallback == t);
	portEXIT_CRITICAL(&sAliveMux);
	return res;
}

/// Перевод миллисекунд в тики без переполнения.
/// pdMS_TO_TICKS считает в TickType_t (32 бита): при configTICK_RATE_HZ = 1000 произведение
/// ms * 1000 переполняется для периодов длиннее ~71.6 мин, и таймер срабатывает раньше срока.
static inline TickType_t msToTicks(uint32_t ms)
{
	return (TickType_t)(((uint64_t)ms * (uint64_t)configTICK_RATE_HZ) / 1000U);
}

// Constructor for the CSoftwareTimer class
CSoftwareTimer::CSoftwareTimer(uint8_t xNotifyBit, uint16_t timerCmd)
{
	// Verify that the notification bit number is less than 32 (as it's used in a bitmask)
	assert(xNotifyBit < 32);

	// Initialize class members
	mNotifyBit = xNotifyBit;
	mTimerCmd = timerCmd;

	// Create a FreeRTOS timer named "STimer", with a period of 1000 ms (pdMS_TO_TICKS(1000)),
	// one-shot mode (pdFALSE), passing 'this' as the timer ID, and setting the callback function
	mTimerHandle = xTimerCreate("STimer", pdMS_TO_TICKS(1000), pdFALSE, this, CSoftwareTimer::vTimerCallback);

	// Check if the timer was created successfully
	if (mTimerHandle == nullptr)
	{
		TRACE_ERROR("CSoftwareTimer has not created", -1);
	}
	else
	{
		alive_add(this);
	}
}

// Destructor for the CSoftwareTimer class
CSoftwareTimer::~CSoftwareTimer()
{
	// If the timer was created, stop and delete it
	if (mTimerHandle != nullptr)
	{
		// Сначала выводим себя из реестра: новый колбэк по этому объекту уже
		// не стартует.
		alive_remove(this);
		// Колбэк мог начаться до alive_remove() - тогда демон прямо сейчас
		// находится внутри timer() и обращается к нашим полям (mTask, mTimerCmd).
		// Ждём его выхода, иначе объект (а следом и задача-адресат) освободится
		// у демона под руками: xTimerStop/xTimerDelete этого не гарантируют,
		// они лишь ставят команду в очередь и возвращают управление сразу.
		// Из самого демона (колбэк удаляет свой же таймер) ждать нельзя - deadlock.
		if (xTimerGetTimerDaemonTaskHandle() != xTaskGetCurrentTaskHandle())
		{
			while (alive_in_callback(this))
				vTaskDelay(1);
		}
		// Команды демону ставим в очередь с бесконечным ожиданием: таймаут в
		// 1 тик при полной очереди команд оставлял таймер живым навсегда.
		while (xTimerStop(mTimerHandle, portMAX_DELAY) != pdPASS)
			vTaskDelay(1);
		while (xTimerDelete(mTimerHandle, portMAX_DELAY) != pdPASS)
			vTaskDelay(1);
		mTimerHandle = nullptr;
	}
}

// Timer callback function (executed when the timer expires)
void CSoftwareTimer::vTimerCallback(TimerHandle_t xTimer)
{
	// Get the pointer to the CSoftwareTimer object from the timer's ID data
	CSoftwareTimer *tm = (CSoftwareTimer *)pvTimerGetTimerID(xTimer);

	// Объект мог быть удалён между истечением таймера и вызовом колбэка -
	// проверяем по реестру живых таймеров, прежде чем разыменовывать, и на
	// время колбэка помечаем объект занятым (деструктор дождётся выхода).
	if (!alive_enter_callback(tm))
		return;

	// Call the timer() method on the CSoftwareTimer object
	tm->timer();

	alive_exit_callback();
}

// Method to start the timer with a specified period and auto-reload mode
int CSoftwareTimer::start(uint32_t period, bool autoRefresh)
{
	// Verify that the period is greater than 0
	assert(msToTicks(period) > 0);

	// Stop the timer before starting with new parameters
	stop();

	// Set the event type to notification and get the current task handle for notification
	mEventType = ETimerEvent::Notify;
	mTaskToNotify = xTaskGetCurrentTaskHandle();

	// Set the timer's reload mode (one-shot or periodic)
	vTimerSetReloadMode(mTimerHandle, autoRefresh);

	// Change the timer's period to the specified value
	if (xTimerChangePeriod(mTimerHandle, msToTicks(period), 1) != pdTRUE)
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
	assert(msToTicks(period) > 0);

	// Stop the timer before starting with new parameters
	stop();

	// Set the event type and the task to notify/send message to
	mEventType = event;
	mTask = task;
	mTaskToNotify = mTask->getTask();

	// Set the timer's reload mode (one-shot or periodic)
	vTimerSetReloadMode(mTimerHandle, autoRefresh);

	// Change the timer's period to the specified value
	if (xTimerChangePeriod(mTimerHandle, msToTicks(period), 1) != pdTRUE)
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