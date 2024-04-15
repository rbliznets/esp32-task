/*!
	\file
	\brief Вывод сообщения об ошибке.
    \authors Близнец Р.А. (r.bliznets@gmail.com)
	\version 1.2.1.0
	\date 10.07.2020
*/

#include "CTrace.h"
#include "esp_system.h"
#include "esp_log.h"
#include "CPrintLog.h"
#include "CTraceTask.h"

#ifdef CONFIG_DEBUG_CODE
/// лог ошибок
CTraceList traceLog;
#endif
#ifdef CONFIG_DEBUG_TRACE_PRINT
CPrintLog tracePrintLog;
#endif

static const char *TAG = "TraceList";

CTraceList::CTraceList() : ITraceLog(), CLock()
{
	if (esp_timer_early_init() != ESP_OK)
	{
		ESP_LOGE(TAG, "esp_timer_early_init error");
	}
	vSemaphoreCreateBinary(mMutex);
}

CTraceList::~CTraceList()
{
	clear();
	vSemaphoreDelete(mMutex);
}

void CTraceList::init()
{
#ifdef CONFIG_DEBUG_TRACE_PRINT
	ADDLOG(&tracePrintLog);
#endif
#ifdef CONFIG_DEBUG_TRACE_TASK
#ifdef CONFIG_DEBUG_TRACE_TASK0
	CTraceTask::Instance()->init(30, 0);
#else
	CTraceTask::Instance()->init(30, 1);
#endif
	ADDLOG(CTraceTask::Instance());
#endif
}

void CTraceList::clear()
{
	lock();
	// for (auto x : m_list)
	// {
	// 	delete x;
	// }
	m_list.clear();
	unlock();
}

void CTraceList::trace(const char *strError, int32_t errCode, esp_log_level_t level, bool reboot)
{
	lock();
	for (auto x : m_list)
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

void CTraceList::traceFromISR(const char *strError, int32_t errCode, esp_log_level_t level, bool reboot, BaseType_t *pxHigherPriorityTaskWoken)
{
	// lock();
	for (auto x : m_list)
	{
		x->traceFromISR(strError, errCode, level, reboot, pxHigherPriorityTaskWoken);
	}
	// unlock();
}

void CTraceList::trace(const char *strError, uint8_t *data, uint32_t size)
{
	lock();
	for (auto x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

void CTraceList::trace(const char *strError, int8_t *data, uint32_t size)
{
	lock();
	for (auto x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

void CTraceList::trace(const char *strError, uint16_t *data, uint32_t size)
{
	lock();
	for (auto x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

void CTraceList::trace(const char *strError, int16_t *data, uint32_t size)
{
	lock();
	for (auto x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

void CTraceList::trace(const char *strError, uint32_t *data, uint32_t size)
{
	lock();
	for (auto x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

void CTraceList::trace(const char *strError, int32_t *data, uint32_t size)
{
	lock();
	for (auto x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

void CTraceList::log(const char *str)
{
	lock();
	for (auto x : m_list)
	{
		x->log(str);
	}
	unlock();
}

void CTraceList::startTime()
{
	lock();
	for (auto x : m_list)
	{
		x->startTime();
	}
	unlock();
}

void CTraceList::stopTime(const char *str, uint32_t n)
{
	lock();
	for (auto x : m_list)
	{
		x->stopTime(str, n);
	}
	unlock();
}

void CTraceList::add(ITraceLog *log)
{
	lock();
	m_list.push_back(log);
	unlock();
}

void CTraceList::remove(ITraceLog *log)
{
	lock();
	m_list.remove(log);
	unlock();
}
