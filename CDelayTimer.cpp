/*!
	\file
	\brief Аппаратный таймер под задачи FreeRTOS.
	\authors Близнец Р.А.
	\version 1.0.1.0
	\date 31.03.2023
*/

#include "CDelayTimer.h"
#include <cstdio>
#include "esp_log.h"

static const char *TAG = "DelayTimer";

bool IRAM_ATTR CDelayTimer::timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
	CDelayTimer *tm = (CDelayTimer *)user_ctx;
	tm->timer();
	return true;
}

int CDelayTimer::start(uint8_t xNotifyBit, uint32_t period, bool autoRefresh)
{
	if (is_run())
	{
		gptimer_stop(mTimerHandle);
		mTaskToNotify = xTaskGetCurrentTaskHandle();
		mNotifyBit = xNotifyBit;
		m_alarm_config.alarm_count = period;
		m_alarm_config.flags.auto_reload_on_alarm = autoRefresh;
		if (gptimer_set_alarm_action(mTimerHandle, &m_alarm_config) != ESP_OK)
		{
			ESP_LOGE(TAG, "gptimer_set_alarm_action failed");
			gptimer_del_timer(mTimerHandle);
			mTimerHandle = nullptr;
			return -3;
		}
		if (gptimer_start(mTimerHandle) != ESP_OK)
		{
			ESP_LOGE(TAG, "gptimer_start failed");
			gptimer_disable(mTimerHandle);
			gptimer_del_timer(mTimerHandle);
			mTimerHandle = nullptr;
			return -4;
		}
		return 0;
	}
	else
	{
		if (gptimer_new_timer(&mTimer_config, &mTimerHandle) != ESP_OK)
		{
			ESP_LOGE(TAG, "gptimer_new_timer failed");
			return -1;
		}
		if (gptimer_register_event_callbacks(mTimerHandle, &m_cbs, this) != ESP_OK)
		{
			ESP_LOGE(TAG, "gptimer_register_event_callbacks failed");
			gptimer_del_timer(mTimerHandle);
			mTimerHandle = nullptr;
			return -2;
		}
		mTaskToNotify = xTaskGetCurrentTaskHandle();
		mNotifyBit = xNotifyBit;
		m_alarm_config.alarm_count = period;
		m_alarm_config.flags.auto_reload_on_alarm = autoRefresh;
		if (gptimer_set_alarm_action(mTimerHandle, &m_alarm_config) != ESP_OK)
		{
			ESP_LOGE(TAG, "gptimer_set_alarm_action failed");
			gptimer_del_timer(mTimerHandle);
			mTimerHandle = nullptr;
			return -3;
		}
		if (gptimer_enable(mTimerHandle) != ESP_OK)
		{
			ESP_LOGE(TAG, "gptimer_enable failed");
			gptimer_del_timer(mTimerHandle);
			mTimerHandle = nullptr;
			return -4;
		}
		if (gptimer_start(mTimerHandle) != ESP_OK)
		{
			ESP_LOGE(TAG, "gptimer_start failed");
			gptimer_disable(mTimerHandle);
			gptimer_del_timer(mTimerHandle);
			mTimerHandle = nullptr;
			return -4;
		}
		return 0;
	}
	return 0;
}

int CDelayTimer::stop()
{
	if (mTimerHandle != nullptr)
	{
		gptimer_stop(mTimerHandle);
		gptimer_disable(mTimerHandle);
		gptimer_del_timer(mTimerHandle);
		mTimerHandle = nullptr;
	}
	else
	{
		ESP_LOGI(TAG, "mTimerHandle==NULL");
		return -1;
	}
	return 0;
}

void CDelayTimer::timer()
{
	xTaskNotify(mTaskToNotify, (1 << mNotifyBit), eSetBits);
	if (m_alarm_config.flags.auto_reload_on_alarm == 0)
	{
		gptimer_stop(mTimerHandle);
		gptimer_disable(mTimerHandle);
		gptimer_del_timer(mTimerHandle);
		mTimerHandle = nullptr;
	}
}
