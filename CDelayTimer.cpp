/*!
	\file
	\brief Аппаратный таймер под задачи FreeRTOS.
	\authors Близнец Р.А.
	\version 1.0.1.0
	\date 31.03.2023
*/

#include "CDelayTimer.h"
#include <cstdio>
#include "CTrace.h"

IRAM_ATTR bool CDelayTimer::timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
	CDelayTimer *tm = (CDelayTimer *)user_ctx;
	tm->timer();
	return true;
}

int CDelayTimer::start(uint8_t xNotifyBit, uint32_t period, bool autoRefresh)
{
	esp_err_t er;
	if (is_run())
	{
		gptimer_stop(mTimerHandle);
		mTaskToNotify = xTaskGetCurrentTaskHandle();
		mNotifyBit = xNotifyBit;
		m_alarm_config.alarm_count = period;
		m_alarm_config.flags.auto_reload_on_alarm = autoRefresh;
		if ((er = gptimer_set_alarm_action(mTimerHandle, &m_alarm_config)) != ESP_OK)
		{
			TRACE_ERROR("CDelayTimer:gptimer_set_alarm_action failed!", er);
			gptimer_del_timer(mTimerHandle);
			mTimerHandle = nullptr;
			return -3;
		}
		if ((er = gptimer_start(mTimerHandle)) != ESP_OK)
		{
			TRACE_ERROR("CDelayTimer:gptimer_start failed!", er);
			gptimer_disable(mTimerHandle);
			gptimer_del_timer(mTimerHandle);
			mTimerHandle = nullptr;
			return -4;
		}
		return 0;
	}
	else
	{
		if ((er = gptimer_new_timer(&mTimer_config, &mTimerHandle)) != ESP_OK)
		{
			TRACE_ERROR("CDelayTimer:gptimer_new_timer failed", er);
			return -1;
		}
		if ((er = gptimer_register_event_callbacks(mTimerHandle, &m_cbs, this)) != ESP_OK)
		{
			TRACE_ERROR("CDelayTimer:gptimer_register_event_callbacks failed", er);
			gptimer_del_timer(mTimerHandle);
			mTimerHandle = nullptr;
			return -2;
		}
		mTaskToNotify = xTaskGetCurrentTaskHandle();
		mNotifyBit = xNotifyBit;
		m_alarm_config.alarm_count = period;
		m_alarm_config.flags.auto_reload_on_alarm = autoRefresh;
		if ((er = gptimer_set_alarm_action(mTimerHandle, &m_alarm_config)) != ESP_OK)
		{
			TRACE_ERROR("CDelayTimer:gptimer_set_alarm_action failed", er);
			gptimer_del_timer(mTimerHandle);
			mTimerHandle = nullptr;
			return -3;
		}
		if ((er = gptimer_enable(mTimerHandle)) != ESP_OK)
		{
			TRACE_ERROR("CDelayTimer:gptimer_enable failed", er);
			gptimer_del_timer(mTimerHandle);
			mTimerHandle = nullptr;
			return -4;
		}
		if ((er = gptimer_start(mTimerHandle)) != ESP_OK)
		{
			TRACE_ERROR("CDelayTimer:gptimer_start failed", er);
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
		ESP_LOGD("CDelayTimer", "mTimerHandle==NULL");
		return -1;
	}
	return 0;
}

IRAM_ATTR void CDelayTimer::timer()
{
	BaseType_t do_yield = pdFALSE;
	xTaskNotifyFromISR(mTaskToNotify, (1 << mNotifyBit), eSetBits, &do_yield);
	if (m_alarm_config.flags.auto_reload_on_alarm == 0)
	{
		gptimer_stop(mTimerHandle);
		gptimer_disable(mTimerHandle);
		gptimer_del_timer(mTimerHandle);
		taskENTER_CRITICAL_ISR(&mMut);
		mTimerHandle = nullptr;
		taskEXIT_CRITICAL_ISR(&mMut);
	}
	portYIELD_FROM_ISR(do_yield);
}
