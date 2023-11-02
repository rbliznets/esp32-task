/*!
	\file
	\brief Аппаратный таймер под задачи FreeRTOS.
	\authors Близнец Р.А.
	\version 1.0.1.0
	\date 31.03.2023
*/

#if !defined CDELAYTIMER_H
#define CDELAYTIMER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gptimer.h"

/// Микросекундный таймер под задачи FreeRTOS.
class CDelayTimer
{
private:
	portMUX_TYPE mMut = portMUX_INITIALIZER_UNLOCKED; ///< Мьютекс для критической секции.
protected:
	gptimer_handle_t mTimerHandle = nullptr; ///< Хэндлер таймера.
	gptimer_config_t mTimer_config = {
		.clk_src = GPTIMER_CLK_SRC_DEFAULT,
		.direction = GPTIMER_COUNT_UP,
		.resolution_hz = 1 * 1000 * 1000, // 1MHz, 1 tick = 1us
		.flags = {0}};
	gptimer_alarm_config_t m_alarm_config = {
		.alarm_count = 1000000, // period = 1s @resolution 1MHz
		.reload_count = 0,		// counter will reload with 0 on alarm event
		.flags = 0};

	static bool timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx);
	gptimer_event_callbacks_t m_cbs = {
		.on_alarm = timer_on_alarm_cb // register user callback
	};

	TaskHandle_t mTaskToNotify; ///< Указатель на задачу, ожидающую события от таймера.
	uint8_t mNotifyBit;			///< Номер бита для оповещения задачи о событии таймера (не более 31).
	bool mAutoRefresh;			///< Флаг автозагрузки таймера.
	
	/// Функция, вызываемая по событию в таймере.
	void timer();

public:
	/// Запуск таймера.
	/*!
	  \warning Вызывать только из задачи FreeRTOS.
	  \param[in] xNotifyBit Номер бита для оповещения задачи о событии таймера.
	  \param[in] period Период в микросекундах.
	  \param[in] autoRefresh Флаг автозагрузки таймера. Если false, то таймер запускается один раз.
	  \return 0 - в случае успеха.
	  \sa Stop()
	*/
	int start(uint8_t xNotifyBit, uint32_t period, bool autoRefresh = false);
	/// Остановка таймера.
	/*!
	  \return 0 - в случае успеха.
	  \sa Start()
	*/
	int stop();

	/// Состояние таймера.
	/*!
	  \return Состояние таймера.
	*/
	bool is_run()
	{
		taskENTER_CRITICAL(&mMut);
		bool res = mTimerHandle != nullptr;
		taskEXIT_CRITICAL(&mMut);
		return res;
	};
};

#endif // CDELAYTIMER_H
