/**
 * \file CTraceList.cpp
 * \brief Файл содержит реализацию класса CTraceList для логирования ошибок.
 * \authors Близнец Р.А. (r.bliznets@gmail.com)
 * \version 1.2.1.0
 * \date 10.07.2020
 */

#include "CTrace.h"
#include "esp_system.h"
#include "esp_log.h"
#include "CPrintLog.h"
#include "CTraceTask.h"

#ifdef CONFIG_DEBUG_CODE
/// \brief Лог ошибок.
CTraceList traceLog;
#endif

#ifdef CONFIG_DEBUG_TRACE_PRINT
/// \brief Лог для печати.
CPrintLog tracePrintLog;
#endif

static const char *TAG = "TraceList";

/**
 * \brief Конструктор класса CTraceList.
 */
CTraceList::CTraceList() : ITraceLog(), CLock()
{
	if (esp_timer_early_init() != ESP_OK)
	{
		ESP_LOGE(TAG, "esp_timer_early_init error");
	}
	vSemaphoreCreateBinary(mMutex);
}

/**
 * \brief Деструктор класса CTraceList.
 */
CTraceList::~CTraceList()
{
	clear();
	vSemaphoreDelete(mMutex);
}

/**
 * \brief Инициализация логгеров.
 */
void CTraceList::init()
{
#ifdef CONFIG_DEBUG_TRACE_PRINT
	ADDLOG(&tracePrintLog); // Добавление печатного лога
#endif

#ifdef CONFIG_DEBUG_TRACE_TASK
#ifdef CONFIG_DEBUG_TRACE_TASK0
	CTraceTask::Instance()->init(30, 0);
#else
	CTraceTask::Instance()->init(30, 1);
#endif
	ADDLOG(CTraceTask::Instance()); // Добавление лога задачи
#endif
}

/**
 * \brief Очистка списка ошибок.
 */
void CTraceList::clear()
{
	lock();
	m_list.clear();
	unlock();
}

/**
 * \brief Вывод сообщения об ошибке.
 * \param strError Указатель на строку с сообщением об ошибке.
 * \param errCode Код ошибки.
 * \param level Уровень логирования.
 * \param reboot Флаг, указывающий на необходимость перезагрузки устройства.
 */
void CTraceList::trace(const char *strError, int32_t errCode, esp_log_level_t level, bool reboot)
{
	lock();
	for (auto &x : m_list)
	{
		x->trace(strError, errCode, level, reboot);
	}
	unlock();

	if (reboot)
	{
		ESP_LOGW(TAG, "trace reboot...");
		vTaskDelay(pdMS_TO_TICKS(1000));
		esp_restart();
	}
}

/**
 * \brief Вывод сообщения об ошибке из прерывания.
 * \param strError Указатель на строку с сообщением об ошибке.
 * \param errCode Код ошибки.
 * \param pxHigherPriorityTaskWoken Указатель на флаг, который определяет, нужно ли разбудить задачу с высоким приоритетом.
 */
void IRAM_ATTR CTraceList::traceFromISR(const char *strError, int16_t errCode, BaseType_t *pxHigherPriorityTaskWoken)
{
	for (auto &x : m_list)
	{
		x->traceFromISR(strError, errCode, pxHigherPriorityTaskWoken);
	}
}
/**
 * \brief Вывод сообщения об ошибке с массивом данных типа uint8_t.
 * \param strError Указатель на строку с описанием ошибки.
 * \param data Указатель на массив данных типа uint8_t.
 * \param size Размер массива данных.
 */
void CTraceList::trace(const char *strError, uint8_t *data, uint32_t size)
{
	lock();
	for (auto &x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

/**
 * \brief Вывод сообщения об ошибке с массивом данных типа int8_t.
 * \param strError Указатель на строку с описанием ошибки.
 * \param data Указатель на массив данных типа int8_t.
 * \param size Размер массива данных.
 */
void CTraceList::trace(const char *strError, int8_t *data, uint32_t size)
{
	lock();
	for (auto &x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

/**
 * \brief Вывод сообщения об ошибке с массивом данных типа uint16_t.
 * \param strError Указатель на строку с описанием ошибки.
 * \param data Указатель на массив данных типа uint16_t.
 * \param size Размер массива данных.
 */
void CTraceList::trace(const char *strError, uint16_t *data, uint32_t size)
{
	lock();
	for (auto &x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

/**
 * \brief Вывод сообщения об ошибке с массивом данных типа int16_t.
 * \param strError Указатель на строку с описанием ошибки.
 * \param data Указатель на массив данных типа int16_t.
 * \param size Размер массива данных.
 */
void CTraceList::trace(const char *strError, int16_t *data, uint32_t size)
{
	lock();
	for (auto &x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

/**
 * \brief Вывод сообщения об ошибке с массивом данных типа uint32_t.
 * \param strError Указатель на строку с описанием ошибки.
 * \param data Указатель на массив данных типа uint32_t.
 * \param size Размер массива данных.
 */
void CTraceList::trace(const char *strError, uint32_t *data, uint32_t size)
{
	lock();
	for (auto &x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

/**
 * \brief Вывод сообщения об ошибке с массивом данных типа int32_t.
 * \param strError Указатель на строку с описанием ошибки.
 * \param data Указатель на массив данных типа int32_t.
 * \param size Размер массива данных.
 */
void CTraceList::trace(const char *strError, int32_t *data, uint32_t size)
{
	lock();
	for (auto &x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

/**
 * \brief Запись произвольной строки в лог.
 * \param str Указатель на строку для записи в лог.
 */
void CTraceList::log(const char *str)
{
	lock();
	for (auto &x : m_list)
	{
		x->log(str);
	}
	unlock();
}

/**
 * \brief Запуск таймера для измерения времени.
 */
void CTraceList::startTime()
{
	lock();
	for (auto &x : m_list)
	{
		x->startTime();
	}
	unlock();
}

/**
 * \brief Остановка таймера и запись результата.
 * \param str Указатель на строку с описанием измерения.
 * \param n Дополнительный параметр для записи.
 */
void CTraceList::stopTime(const char *str, uint32_t n)
{
	lock();
	for (auto &x : m_list)
	{
		x->stopTime(str, n);
	}
	unlock();
}

/**
 * \brief Добавление нового логгера в список.
 * \param log Указатель на объект логгера типа ITraceLog.
 */
void CTraceList::add(ITraceLog *log)
{
	lock();
	m_list.push_back(log);
	unlock();
}

/**
 * \brief Удаление логгера из списка.
 * \param log Указатель на объект логгера типа ITraceLog, который нужно удалить.
 */
void CTraceList::remove(ITraceLog *log)
{
	lock();
	m_list.remove(log);
	unlock();
}
