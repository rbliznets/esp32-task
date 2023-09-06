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
#include "esp_log.h"

static const char *TAG = "SoftwareTimer";

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
			ESP_LOGE(TAG,"xTimerStop failed (%ld)",period);
			return -3;
		}
		vTimerSetReloadMode(mTimerHandle, mAutoRefresh);
		if(xTimerChangePeriod(mTimerHandle, period, 0) == pdTRUE)
		{
			return 0;
		}
		else
		{
			ESP_LOGE(TAG,"xTimerChangePeriod failed (%ld)",period);
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
				ESP_LOGE(TAG,"xTimerStart failed (%ld)",period);
				return -2;
			}
		}
		else
		{
			ESP_LOGE(TAG,"xTimerCreate failed (%ld)",period);
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
				ESP_LOGE(TAG,"xTimerDelete failed");
				return -3;
			}
		}
		else
		{
			ESP_LOGE(TAG,"xTimerStop failed");
			return -2;
		}
	}
	else
	{
		ESP_LOGI(TAG,"mTimerHandle==NULL");
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
		else
		{
			ESP_LOGE(TAG,"xTimerDelete failed");
		}
	}
	xTaskNotify(mTaskToNotify,(1 << mNotifyBit),eSetBits);
}
