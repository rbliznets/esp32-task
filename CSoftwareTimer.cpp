/*!
	\file
	\brief Программный таймер под задачи FreeRTOS.
    \authors Близнец Р.А. (r.bliznets@gmail.com)
	\version 1.2.0.0
	\date 28.04.2020
	\copyright (c) Copyright 2021, ООО "Глобал Ориент", Москва, Россия, http://www.glorient.ru/
*/

#include "CSoftwareTimer.h"
#include <cstdio>
#include "CTrace.h"

CSoftwareTimer::CSoftwareTimer(uint8_t xNotifyBit, uint16_t timerCmd)
{
	assert(xNotifyBit < 32);

	mNotifyBit = xNotifyBit;
	mTimerCmd = timerCmd;
	mTimerHandle = xTimerCreate("STimer", pdMS_TO_TICKS(1000), pdFALSE, this, CSoftwareTimer::vTimerCallback);
	if (mTimerHandle == nullptr)
		TRACE_ERROR("CSoftwareTimer has not created", -1);
}

CSoftwareTimer::~CSoftwareTimer()
{
	if (mTimerHandle != nullptr)
	{
		stop();
		xTimerDelete(mTimerHandle, 1);
	}
}

void CSoftwareTimer::vTimerCallback(TimerHandle_t xTimer)
{
	CSoftwareTimer *tm = (CSoftwareTimer *)pvTimerGetTimerID(xTimer);
	tm->timer();
}

int CSoftwareTimer::start(uint32_t period, bool autoRefresh)
{
	assert(pdMS_TO_TICKS(period) > 0);

	stop();
	mEventType = ETimerEvent::Notify;
	mTaskToNotify = xTaskGetCurrentTaskHandle();
	vTimerSetReloadMode(mTimerHandle, autoRefresh);
	if (xTimerChangePeriod(mTimerHandle, pdMS_TO_TICKS(period), 0) != pdTRUE)
	{
		TRACE_ERROR("CSoftwareTimer:xTimerChangePeriod failed", (uint16_t)period);
		return -2;
	}
	if (xTimerStart(mTimerHandle, 0) == pdTRUE)
	{
		return 0;
	}
	else
	{
		TRACE_ERROR("CSoftwareTimer:xTimerStart failed", (uint16_t)period);
		return -1;
	}
}

int CSoftwareTimer::start(CBaseTask *task, ETimerEvent event, uint32_t period, bool autoRefresh)
{
	assert(task != nullptr);
	assert(pdMS_TO_TICKS(period) > 0);

	stop();
	mEventType = event;
	mTask = task;
	mTaskToNotify = mTask->getTask();
	vTimerSetReloadMode(mTimerHandle, autoRefresh);
	if (xTimerChangePeriod(mTimerHandle, pdMS_TO_TICKS(period), 0) != pdTRUE)
	{
		TRACE_ERROR("CSoftwareTimer:xTimerChangePeriod failed", (uint16_t)period);
		return -2;
	}
	if (xTimerStart(mTimerHandle, 0) == pdTRUE)
	{
		return 0;
	}
	else
	{
		TRACE_ERROR("CSoftwareTimer:xTimerStart failed", (uint16_t)period);
		return -1;
	}

	return 0;
}

int CSoftwareTimer::stop()
{
	if (isRun())
	{
		if (xTimerStop(mTimerHandle, 0) == pdTRUE)
		{
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
		// ESP_LOGI("CSoftwareTimer", "mTimerHandle==NULL");
		return -1;
	}
}

void CSoftwareTimer::timer()
{
	if (mEventType == ETimerEvent::Notify)
		xTaskNotify(mTaskToNotify, (1 << mNotifyBit), eSetBits);
	else if (mEventType == ETimerEvent::SendBack)
		mTask->sendCmd(mTimerCmd);
}
