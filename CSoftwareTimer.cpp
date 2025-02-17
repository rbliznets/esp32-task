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

// Конструктор класса CSoftwareTimer
CSoftwareTimer::CSoftwareTimer(uint8_t xNotifyBit, uint16_t timerCmd)
{
	// Проверка, что номер уведомления меньше 32 (так как используется bitmask)
	assert(xNotifyBit < 32);
	
	// Если включена поддержка управления питанием, создаем блокировку для предотвращения перехода в режим неглубокого сна
// #if CONFIG_PM_ENABLE
// 	esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, "st", &mPMLock);
// #endif

	// Инициализация полей класса
	mNotifyBit = xNotifyBit;
	mTimerCmd = timerCmd;

	// Создание таймера с именем "STimer", периодом 1000 мс (pdMS_TO_TICKS(1000)), одноразовым запуском (pdFALSE)
	mTimerHandle = xTimerCreate("STimer", pdMS_TO_TICKS(1000), pdFALSE, this, CSoftwareTimer::vTimerCallback);
	
	// Проверка успешности создания таймера
	if (mTimerHandle == nullptr)
		TRACE_ERROR("CSoftwareTimer has not created", -1);
}

// Деструктор класса CSoftwareTimer
CSoftwareTimer::~CSoftwareTimer()
{
	// Если таймер был создан, останавливаем и удаляем его
	if (mTimerHandle != nullptr)
	{
		stop();
		xTimerDelete(mTimerHandle, 1);
	}
	
	// Удаление блокировки управления питанием, если она была создана
// #if CONFIG_PM_ENABLE
// 	esp_pm_lock_delete(mPMLock);
// #endif
}

// Функция обратного вызова таймера (выполняется при срабатывании таймера)
void CSoftwareTimer::vTimerCallback(TimerHandle_t xTimer)
{
	// Получение указателя на объект CSoftwareTimer из данных таймера
	CSoftwareTimer *tm = (CSoftwareTimer *)pvTimerGetTimerID(xTimer);
	
	// Вызов метода timer у объекта CSoftwareTimer
	tm->timer();
}

// Метод для запуска таймера с заданным периодом и режимом автоматического перезапуска
int CSoftwareTimer::start(uint32_t period, bool autoRefresh)
{
	// Проверка, что период больше 0
	assert(pdMS_TO_TICKS(period) > 0);

	// Остановка таймера перед запуском с новыми параметрами
	stop();
	
	// Установка типа события на уведомление и получения текущей задачи для уведомления
	mEventType = ETimerEvent::Notify;
	mTaskToNotify = xTaskGetCurrentTaskHandle();

	// Установка режима перезапуска таймера (одноразовый или периодический)
	vTimerSetReloadMode(mTimerHandle, autoRefresh);
	
	// Изменение периода таймера на заданный
	if (xTimerChangePeriod(mTimerHandle, pdMS_TO_TICKS(period), 1) != pdTRUE)
	{
		TRACE_ERROR("CSoftwareTimer:xTimerChangePeriod failed", (uint16_t)period);
		return -2;
	}

	// Запуск таймера
	if (xTimerStart(mTimerHandle, 1) == pdTRUE)
	{
// #if CONFIG_PM_ENABLE
// 		// Получение блокировки управления питанием для предотвращения перехода в режим неглубокого сна
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

int CSoftwareTimer::start(CBaseTask *task, ETimerEvent event, uint32_t period, bool autoRefresh)
{
	// Проверка, что переданный указатель на задачу не равен nullptr
	assert(task != nullptr);
	
	// Проверка, что период больше 0
	assert(pdMS_TO_TICKS(period) > 0);

	// Остановка таймера перед запуском с новыми параметрами
	stop();
	
	// Установка типа события и задачи для уведомления
	mEventType = event;
	mTask = task;
	mTaskToNotify = mTask->getTask();

	// Установка режима перезапуска таймера (одноразовый или периодический)
	vTimerSetReloadMode(mTimerHandle, autoRefresh);
	
	// Изменение периода таймера на заданный
	if (xTimerChangePeriod(mTimerHandle, pdMS_TO_TICKS(period), 1) != pdTRUE)
	{
		TRACE_ERROR("CSoftwareTimer:xTimerChangePeriod failed", (uint16_t)period);
		return -2;
	}

	// Запуск таймера
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

int CSoftwareTimer::stop()
{
	// Проверка, что таймер запущен
	if (isRun())
	{
		// Остановка таймера
		if (xTimerStop(mTimerHandle, 1) == pdTRUE)
		{
// #if CONFIG_PM_ENABLE
// 			// Освобождение блокировки управления питанием
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
		// Логирование информации о том, что таймер не запущен (закомментировано)
		// ESP_LOGI("CSoftwareTimer", "mTimerHandle==NULL");
		return -1;
	}
}

void CSoftwareTimer::timer()
{
	// Обработка события уведомления
	if (mEventType == ETimerEvent::Notify)
		xTaskNotify(mTaskToNotify, (1 << mNotifyBit), eSetBits);
	
	// Обработка события отправки команды обратно
	else if (mEventType == ETimerEvent::SendBack)
		mTask->sendCmd(mTimerCmd, 0, 0, 1);
}
