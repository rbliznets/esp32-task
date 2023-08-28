/*!
	\file
	\brief Программный таймер под задачи FreeRTOS.
	\authors Близнец Р.А.
	\version 1.1.0.0
	\date 28.04.2020
	\copyright (c) Copyright 2021, ООО "Глобал Ориент", Москва, Россия, http://www.glorient.ru/
*/

#include "CSoftwareTimer.h"
#include <cstdio>
#include "CTrace.h"

void CSoftwareTimer::vTimerCallback( TimerHandle_t xTimer )
{
	CSoftwareTimer* tm = (CSoftwareTimer*) pvTimerGetTimerID(xTimer);
	tm->timer();
}

int CSoftwareTimer::start(uint8_t xNotifyBit, uint32_t period, bool autoRefresh)
{
	if(is_run())
	{
		if(xTimerStop(mTimerHandle, 0) != pdTRUE)
		{
#ifdef CONFIG_DEBUG_CODE
			TRACE("CSoftwareTimer::Start xTimerStop failed",period,false);
#endif
			return -3;
		}
		vTimerSetReloadMode(mTimerHandle, mAutoRefresh);
		if(xTimerChangePeriod(mTimerHandle, period, 0) == pdTRUE)
		{
			return 0;
		}
		else
		{
#ifdef CONFIG_DEBUG_CODE
			TRACE("CSoftwareTimer::Start xTimerChangePeriod failed",period,false);
#endif
			return -4;
		}
	}
	else
	{
		mTaskToNotify=xTaskGetCurrentTaskHandle();
		mNotifyBit=xNotifyBit;
		mAutoRefresh=autoRefresh;
		mTimerHandle=xTimerCreate("Timer", pdMS_TO_TICKS(period), mAutoRefresh, this, CSoftwareTimer::vTimerCallback);
		if(mTimerHandle != nullptr)
		{
			if(xTimerStart(mTimerHandle, 0) == pdTRUE)
			{
				return 0;
			}
			else
			{
#ifdef CONFIG_DEBUG_CODE
				TRACE("CSoftwareTimer::Start xTimerStart failed",period,false);
#endif
				return -2;
			}
		}
		else
		{
#ifdef CONFIG_DEBUG_CODE
			TRACE("CSoftwareTimer::Start xTimerCreate failed",period,false);
#endif
			return -1;
		}
	}
}

int CSoftwareTimer::stop()
{
	if(mTimerHandle != nullptr)
	{
		if(xTimerStop(mTimerHandle, 0) == pdTRUE)
		{
			if(xTimerDelete(mTimerHandle, 0) == pdTRUE)
			{
				mTimerHandle=nullptr;
				return 0;
			}
			else
			{
#ifdef CONFIG_DEBUG_CODE
				TRACE("CSoftwareTimer::Stop xTimerDelete failed",0,false);
#endif
				return -3;
			}
		}
		else
		{
#ifdef CONFIG_DEBUG_CODE
			TRACE("CSoftwareTimer::Stop xTimerStop failed",0,false);
#endif
			return -2;
		}
	}
	else
	{
#ifdef CONFIG_DEBUG_CODE
		//TRACE("CSoftwareTimer::Stop mTimerHandle==NULL",0,false);
#endif
		return -1;
	}
}

void CSoftwareTimer::timer()
{
	if(!mAutoRefresh)
	{
		if(xTimerDelete(mTimerHandle, 0) == pdTRUE)
		{
			mTimerHandle=nullptr;
		}
#ifdef CONFIG_DEBUG_CODE
		else
		{
			TRACE("CSoftwareTimer::Timer xTimerDelete failed",0,false);
		}
#endif
	}
	xTaskNotify(mTaskToNotify,(1 << mNotifyBit),eSetBits);
}
