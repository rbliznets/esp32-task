/*!
	\file
	\brief Аппаратный таймер под задачи FreeRTOS.
	\authors Близнец Р.А.
	\version 1.1.0.0
	\date 31.03.2023
*/

#include "CDelayTimer.h"
#include <cstdio>
#include "CTrace.h"

CDelayTimer::CDelayTimer()
{
	gptimer_config_t mTimer_config = {
		.clk_src = GPTIMER_CLK_SRC_DEFAULT,
		.direction = GPTIMER_COUNT_UP,
		.resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
		.flags = {0}};
	gptimer_event_callbacks_t cbs = {
		.on_alarm = timer_on_alarm_cb // register user callback
	};
	esp_err_t er;
	if ((er = gptimer_new_timer(&mTimer_config, &mTimerHandle)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_new_timer failed", er);
	}
	else
	{
		if ((er = gptimer_register_event_callbacks(mTimerHandle, &cbs, this)) != ESP_OK)
		{
			TRACE_ERROR("CDelayTimer:gptimer_register_event_callbacks failed", er);
			gptimer_del_timer(mTimerHandle);
		}
	}
}

CDelayTimer::~CDelayTimer()
{
	stop();
	gptimer_event_callbacks_t cbs = {
		.on_alarm = nullptr // register user callback
	};
	gptimer_register_event_callbacks(mTimerHandle, &cbs, nullptr);
	gptimer_del_timer(mTimerHandle);
}

IRAM_ATTR bool CDelayTimer::timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
	CDelayTimer *tm = (CDelayTimer *)user_ctx;
	tm->timer();
	return true;
}

int CDelayTimer::start(uint8_t xNotifyBit, uint32_t period, bool autoRefresh)
{
	esp_err_t er;
	stop();

	mTaskToNotify = xTaskGetCurrentTaskHandle();
	mNotifyBit = xNotifyBit;
	m_alarm_config.alarm_count = period;
	m_alarm_config.flags.auto_reload_on_alarm = autoRefresh;
	if ((er = gptimer_set_alarm_action(mTimerHandle, &m_alarm_config)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_set_alarm_action failed!", er);
		return -3;
	}
	if ((er = gptimer_enable(mTimerHandle)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_enable failed", er);
		return -4;
	}
	if ((er = gptimer_start(mTimerHandle)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_start failed!", er);
		gptimer_disable(mTimerHandle);
		return -5;
	}
	mRun = true;
	return 0;
}

int CDelayTimer::stop()
{
	if (mRun)
	{
		gptimer_stop(mTimerHandle);
		gptimer_disable(mTimerHandle);
		mRun = false;
		return 0;
	}
	else
		return -1;
}

IRAM_ATTR void CDelayTimer::timer()
{
	BaseType_t do_yield = pdFALSE;
	xTaskNotifyFromISR(mTaskToNotify, (1 << mNotifyBit), eSetBits, &do_yield);
	// portYIELD_FROM_ISR(do_yield);
}
