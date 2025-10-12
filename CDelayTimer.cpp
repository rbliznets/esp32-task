/*!
	\file
	\brief Аппаратный таймер для задач FreeRTOS.
	\authors Близнец Р.А. (r.bliznets@gmail.com)
	\version 1.3.0.0
	\date 31.03.2023
	\details Класс CDelayTimer реализует управление аппаратным таймером для использования в задачах FreeRTOS.
			 Поддерживает уведомления задач и отправку сообщений по истечении заданного интервала времени.
*/

#include "CDelayTimer.h" // Заголовочный файл класса CDelayTimer
#include <cstdio>		 // Стандартная библиотека для работы с вводом-выводом
#include "CTrace.h"		 // Библиотека для трассировки и логирования

/**
 * \brief Конструктор класса CDelayTimer.
 * \param xNotifyBit Бит уведомления (0-31), используемый для уведомления задачи.
 * \param timerCmd Команда таймера, передаваемая в сообщении.
 * \details Инициализирует таймер с настройками по умолчанию: частота 1 МГц, счет вверх.
 *          Регистрирует callback-функцию для обработки событий таймера.
 */
CDelayTimer::CDelayTimer(uint8_t xNotifyBit, uint16_t timerCmd) : mNotifyBit(xNotifyBit), mTimerCmd(timerCmd)
{
	assert(xNotifyBit < 32); // Проверка корректности бита уведомления

	// Настройка конфигурации таймера
	gptimer_config_t mTimer_config = {
		.clk_src = GPTIMER_CLK_SRC_DEFAULT, // Источник тактирования по умолчанию
		.direction = GPTIMER_COUNT_UP,		// Счет вверх
		.resolution_hz = 1000000,			// Частота 1 МГц (1 тик = 1 мкс)
		.intr_priority = 0,					// Приоритет прерывания
		.flags = {1, 0, 0}					// Флаги конфигурации
	};

	// Регистрация callback-функции для обработки событий таймера
	gptimer_event_callbacks_t cbs = {
		.on_alarm = timer_on_alarm_cb // Указатель на функцию-обработчик
	};

	esp_err_t er;
	// Создание нового таймера
	if ((er = gptimer_new_timer(&mTimer_config, &mTimerHandle)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_new_timer failed", er); // Логирование ошибки
	}
	else
	{
		// Регистрация callback-функции
		if ((er = gptimer_register_event_callbacks(mTimerHandle, &cbs, this)) != ESP_OK)
		{
			TRACE_ERROR("CDelayTimer:gptimer_register_event_callbacks failed", er); // Логирование ошибки
			gptimer_del_timer(mTimerHandle);										// Удаление таймера в случае ошибки
		}
	}
}

/**
 * \brief Деструктор класса CDelayTimer.
 * \details Останавливает таймер и удаляет его.
 */
CDelayTimer::~CDelayTimer()
{
	stop();											// Остановка таймера
	esp_err_t er = gptimer_del_timer(mTimerHandle); // Удаление таймера
	if (er != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_del_timer failed", er); // Логирование ошибки
	}
}

/**
 * \brief Callback-функция, вызываемая при срабатывании таймера.
 * \param timer Дескриптор таймера.
 * \param edata Данные события таймера.
 * \param user_ctx Указатель на объект CDelayTimer.
 * \return Всегда возвращает true.
 * \details Вызывает метод timer() объекта CDelayTimer.
 */
bool IRAM_ATTR CDelayTimer::timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
	CDelayTimer *tm = (CDelayTimer *)user_ctx; // Приведение указателя
	tm->timer();							   // Вызов метода обработки таймера
	return true;
}

/**
 * \brief Запуск таймера с уведомлением задачи.
 * \param xNotifyBit Бит уведомления (0-31).
 * \param period Период таймера в микросекундах.
 * \param autoRefresh Флаг автоматического перезапуска таймера.
 * \return 0 в случае успеха, иначе код ошибки.
 * \details Настраивает таймер на уведомление текущей задачи по истечении периода.
 */
int IRAM_ATTR CDelayTimer::start(uint8_t xNotifyBit, uint32_t period, bool autoRefresh)
{
	assert(xNotifyBit < 32);		   // Проверка корректности бита уведомления
	assert(pdMS_TO_TICKS(period) > 0); // Проверка корректности периода

	esp_err_t er;
	stop(); // Остановка таймера перед настройкой

	mTaskToNotify = xTaskGetCurrentTaskHandle();			 // Получение дескриптора текущей задачи
	mNotifyBit = xNotifyBit;								 // Установка бита уведомления
	mEventType = ETimerEvent::Notify;						 // Установка типа события
	m_alarm_config.alarm_count = period;					 // Установка периода таймера
	m_alarm_config.flags.auto_reload_on_alarm = autoRefresh; // Настройка автообновления

	// Настройка действия таймера при срабатывании
	if ((er = gptimer_set_alarm_action(mTimerHandle, &m_alarm_config)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_set_alarm_action failed!", er); // Логирование ошибки
		return -3;
	}

	// Включение таймера
	if ((er = gptimer_enable(mTimerHandle)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_enable failed", er); // Логирование ошибки
		return -4;
	}

	// Сброс счетчика таймера
	if ((er = gptimer_set_raw_count(mTimerHandle, 0)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_set_raw_count failed", er); // Логирование ошибки
		return -5;
	}

	// Запуск таймера
	if ((er = gptimer_start(mTimerHandle)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_start failed!", er); // Логирование ошибки
		gptimer_disable(mTimerHandle);						  // Отключение таймера в случае ошибки
		return -6;
	}

	mRun = true; // Установка флага работы таймера
	return 0;
}

/**
 * \brief Запуск таймера с отправкой сообщения задаче.
 * \param task Указатель на задачу, которой будет отправлено сообщение.
 * \param event Тип события (отправка сообщения в начало или конец очереди).
 * \param period Период таймера в микросекундах.
 * \param autoRefresh Флаг автоматического перезапуска таймера.
 * \return 0 в случае успеха, иначе код ошибки.
 * \details Настраивает таймер на отправку сообщения задаче по истечении периода.
 */
int IRAM_ATTR CDelayTimer::start(CBaseTask *task, ETimerEvent event, uint32_t period, bool autoRefresh)
{
	assert(task != nullptr);		   // Проверка корректности указателя на задачу
	assert(pdMS_TO_TICKS(period) > 0); // Проверка корректности периода

	esp_err_t er;
	stop(); // Остановка таймера перед настройкой

	mEventType = event;				  // Установка типа события
	mTask = task;					  // Установка задачи
	mTaskToNotify = mTask->getTask(); // Получение дескриптора задачи

	m_alarm_config.alarm_count = period;					 // Установка периода таймера
	m_alarm_config.flags.auto_reload_on_alarm = autoRefresh; // Настройка автообновления

	// Настройка действия таймера при срабатывании
	if ((er = gptimer_set_alarm_action(mTimerHandle, &m_alarm_config)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_set_alarm_action failed!", er); // Логирование ошибки
		return -3;
	}

	// Включение таймера
	if ((er = gptimer_enable(mTimerHandle)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_enable failed", er); // Логирование ошибки
		return -4;
	}

	// Сброс счетчика таймера
	if ((er = gptimer_set_raw_count(mTimerHandle, 0)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_set_raw_count failed", er); // Логирование ошибки
		return -5;
	}

	// Запуск таймера
	if ((er = gptimer_start(mTimerHandle)) != ESP_OK)
	{
		TRACE_ERROR("CDelayTimer:gptimer_start failed!", er); // Логирование ошибки
		gptimer_disable(mTimerHandle);						  // Отключение таймера в случае ошибки
		return -6;
	}

	mRun = true; // Установка флага работы таймера
	return 0;
}

/**
 * \brief Остановка таймера.
 * \return 0 в случае успеха, иначе -1.
 * \details Останавливает и отключает таймер.
 */
int IRAM_ATTR CDelayTimer::stop()
{
	if (mRun)
	{
		gptimer_stop(mTimerHandle);	   // Остановка таймера
		gptimer_disable(mTimerHandle); // Отключение таймера
		mRun = false;				   // Сброс флага работы таймера
		return 0;
	}
	else
	{
		return -1; // Таймер уже остановлен
	}
}

/**
 * \brief Ожидание срабатывания таймера.
 * \param period Период ожидания в микросекундах.
 * \param xNotifyBit Бит уведомления (0-31).
 * \return 0 в случае успеха, иначе код ошибки.
 * \details Запускает таймер и ожидает уведомления от него.
 */
int CDelayTimer::wait(uint32_t period, uint8_t xNotifyBit)
{
	if (start(xNotifyBit, period, false) != 0) // Запуск таймера
		return -1;

	uint32_t flag = 0;
	// Ожидание уведомления от таймера
	if (xTaskNotifyWait(0, (1 << xNotifyBit), &flag, pdMS_TO_TICKS((period / 1000) + 10)) != pdTRUE)
	{
		stop(); // Остановка таймера в случае тайм-аута
		return -2;
	}

	stop(); // Остановка таймера после успешного уведомления
	return 0;
}

/**
 * \brief Обработчик срабатывания таймера.
 * \details Отправляет уведомление или сообщение задаче в зависимости от настроек.
 */
void IRAM_ATTR CDelayTimer::timer()
{
	BaseType_t do_yield = pdFALSE;
	if (mEventType == ETimerEvent::Notify)
	{
		// Уведомление задачи
		xTaskNotifyFromISR(mTaskToNotify, (1 << mNotifyBit), eSetBits, &do_yield);
	}
	else
	{
		// Отправка сообщения задаче
		STaskMessage msg;
		msg.msgID = mTimerCmd; // Установка команды сообщения
		if (mEventType == ETimerEvent::SendBack)
		{
			mTask->sendMessageFromISR(&msg, &do_yield); // Отправка в конец очереди
		}
		else
		{
			mTask->sendMessageFrontFromISR(&msg, &do_yield); // Отправка в начало очереди
		}
	}
	// portYIELD_FROM_ISR(do_yield); // Возможный вызов перепланирования (закомментирован)
}