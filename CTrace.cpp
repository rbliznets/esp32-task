/*!
	\file
	\brief Вывод сообщения об ошибке.
	\authors Близнец Р.А.
	\version 1.2.0.0
	\date 10.07.2020
*/

#include "CTrace.h"
#include "esp_system.h"

#ifdef CONFIG_DEBUG_CODE
/// лог ошибок
CTraceList traceLog;
#endif

CTraceList::CTraceList():ITraceLog(),CLock()
{
    if(esp_timer_early_init() != ESP_OK)
    {
       printf("esp_timer_early_init error.\n");
    }
	vSemaphoreCreateBinary( mMutex );
}

CTraceList::~CTraceList()
{
	clear();
	vSemaphoreDelete( mMutex );
}

void CTraceList::clear()
{
	lock();
	for( auto x : m_list)
	{
		delete x;
	}
	m_list.clear();
	unlock();
}

void CTraceList::trace(const char* strError, int32_t errCode, bool reboot)
{
	lock();
	for( auto x : m_list)
	{
		x->trace(strError, errCode, reboot);
	}
	unlock();

	if(reboot)
	{
		std::printf("trace reboot...\n");
		vTaskDelay(pdMS_TO_TICKS(1000));
		esp_restart();
	}
}

void IRAM_ATTR CTraceList::traceFromISR(const char* strError, int32_t errCode, bool reboot, BaseType_t *pxHigherPriorityTaskWoken)
{
	//lock();
	for( auto x : m_list)
	{
		x->traceFromISR(strError, errCode, reboot, pxHigherPriorityTaskWoken);
	}
	//unlock();
}

void CTraceList::trace(const char* strError, uint8_t* data, uint32_t size)
{
	lock();
	for( auto x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

void CTraceList::trace(const char* strError, int8_t* data, uint32_t size)
{
	lock();
	for( auto x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

void CTraceList::trace(const char* strError, uint16_t* data, uint32_t size)
{
	lock();
	for( auto x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

void CTraceList::trace(const char* strError, int16_t* data, uint32_t size)
{
	lock();
	for( auto x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

void CTraceList::trace(const char* strError, uint32_t* data, uint32_t size)
{
	lock();
	for( auto x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

void CTraceList::trace(const char* strError, int32_t* data, uint32_t size)
{
	lock();
	for( auto x : m_list)
	{
		x->trace(strError, data, size);
	}
	unlock();
}

void CTraceList::log(const char *str)
{
	lock();
	for( auto x : m_list)
	{
		x->log(str);
	}
	unlock();
}

void CTraceList::startTime()
{
	lock();
	for( auto x : m_list)
	{
		x->startTime();
	}
	unlock();
}

void CTraceList::stopTime(const char* str, uint32_t n)
{
	lock();
	for( auto x : m_list)
	{
		x->stopTime(str, n);
	}
	unlock();
}

void CTraceList::add(ITraceLog* log)
{
	lock();
	m_list.push_back(log);
	unlock();
}

void CTraceList::remove(ITraceLog* log)
{
	lock();
	m_list.remove(log);
	unlock();
}
