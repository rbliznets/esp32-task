/*!
	\file
	\brief Класс для вывода отладочной информации.
    \authors Близнец Р.А. (r.bliznets@gmail.com)
	\version 1.3.1.0
	\date 15.09.2022

	Один объект на приложение.
	Необходим чтобы не блокировать отлаживаемую задачу.
*/

#include "CTraceTask.h"
#include <cstring>
#include "esp_system.h"
#include "CTrace.h"
#include "esp_log.h"

#ifdef CONFIG_TRACE_AUTO_RESET
#define AUTO_TIMER CONFIG_TRACE_AUTO_RESET
#else
#define AUTO_TIMER false
#endif

void CTraceTask::printHeader(uint64_t time, uint32_t n)
{
	uint64_t res = time / n;
#if (CONFIG_TRACE_USEC == 1)
	std::snprintf(m_header, sizeof(m_header), "(+%liusec)", (long)res);
#else
    if (res >= 10000000)
    {
        std::snprintf(m_header, sizeof(m_header), "(+%lisec)", (long)(res / 1000000));
    }
    else
    {
        if (res < 10000)
        {
            if (res < 10)
            {
                double f = time / (double)n;
                std::snprintf(m_header, sizeof(m_header), "(+%linsec)", (long)(f * 1000));
            }
            else
            {
                std::snprintf(m_header, sizeof(m_header), "(+%liusec)", (long)res);
            }
        }
        else
        {
            std::snprintf(m_header, sizeof(m_header), "(+%limsec)", (long)(res / 1000));
        }
    }
#endif
}

bool CTraceTask::logMessage(STaskMessage& msg)
{
	switch (msg.msgID)
	{
	case MSG_TRACE_ISR_STRING:
		printIsrString((char *)msg.msgBody,(int16_t)msg.shortParam);
		return true;
	case MSG_TRACE_STRING:
		printString((char *)msg.msgBody);
		break;
	case MSG_STOP_TIME:
		printStop((char *)msg.msgBody);
		break;
	case MSG_PRINT_STRING:
		printS((char *)msg.msgBody);
		break;
	case MSG_TRACE_STRING_REBOOT:
		printString((char *)msg.msgBody);
		vTaskDelay(pdMS_TO_TICKS(150));
		esp_restart();
		break;
	case MSG_TRACE_UINT8:
		printData8h((char *)msg.msgBody);
		break;
	case MSG_TRACE2_UINT8:
		printData8h_2((char *)msg.msgBody);
		break;
	case MSG_TRACE_INT8:
		printData8((char *)msg.msgBody);
		break;
	case MSG_TRACE2_INT8:
		printData8_2((char *)msg.msgBody);
		break;
	case MSG_TRACE_UINT16:
		printData16h((char *)msg.msgBody);
		break;
	case MSG_TRACE2_UINT16:
		printData16h_2((char *)msg.msgBody);
		break;
	case MSG_TRACE_INT16:
		printData16((char *)msg.msgBody);
		break;
	case MSG_TRACE2_INT16:
		printData16_2((char *)msg.msgBody);
		break;
	case MSG_TRACE_UINT32:
		printData32h((char *)msg.msgBody);
		break;
	case MSG_TRACE2_UINT32:
		printData32h_2((char *)msg.msgBody);
		break;
	case MSG_TRACE_INT32:
		printData32((char *)msg.msgBody);
		break;
	case MSG_TRACE2_INT32:
		printData32_2((char *)msg.msgBody);
		break;
	default:
		return false;
	}
	vPortFree(msg.msgBody);
	return true;
}

void CTraceTask::run()
{
#ifndef CONFIG_FREERTOS_CHECK_STACKOVERFLOW_NONE
	UBaseType_t m1 = uxTaskGetStackHighWaterMark2(nullptr);
#endif 
	STaskMessage msg;

	while (getMessage(&msg, portMAX_DELAY))
	{
		if(!logMessage(msg))
		{
			ESP_LOGW("*","CTraceTask unknown message %d", msg.msgID);
		}
		vTaskDelay(pdMS_TO_TICKS(2));
#ifndef CONFIG_FREERTOS_CHECK_STACKOVERFLOW_NONE
		UBaseType_t m2 = uxTaskGetStackHighWaterMark2(nullptr);
		if (m2 != m1)
		{
			m1 = m2;
			if(m1 > 100)
				ESP_LOGI("*","free trace stack %d", m2);
			else
				ESP_LOGW("*","free trace stack %d", m2);
		}
#endif
	}
}

void CTraceTask::printData32_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint32_t *pdata = (uint32_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOGI(m_header,"32 %s(%ld)",strError,(*size));
	ESP_LOG_BUFFER_HEX(strError,pdata,(*size)*4);
#else
    std::printf(m_header);
	std::printf("%s %ld:", strError, *size);
	std::printf(" %d", (int)pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",%d", (int)pdata[i]);
	}
	std::printf("\n");
#endif
}

void CTraceTask::printData32h_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint32_t *pdata = (uint32_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOGI(m_header,"32 %s(%ld)",strError,(*size));
	ESP_LOG_BUFFER_HEX(strError,pdata,(*size)*4);
#else
    std::printf(m_header);
	std::printf("%s %ld:", strError, *size);
	std::printf(" 0x%08x", (int)pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",0x%08x", (int)pdata[i]);
	}
	std::printf("\n");
#endif
}

void CTraceTask::printData32h(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint32_t *pdata = (uint32_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint32_t))];

	printHeader(*res);
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOGI(m_header,"32 %s(%ld)",strError,(*size));
	ESP_LOG_BUFFER_HEX(strError,pdata,(*size)*4);
#else
    std::printf(m_header);
	std::printf("%s %ld:", strError, *size);
	std::printf(" 0x%08x", (int)pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",0x%08x", (int)pdata[i]);
	}
	std::printf("\n");
#endif
}

void CTraceTask::printData32(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	int32_t *pdata = (int32_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint32_t))];

	printHeader(*res);
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOGI(m_header,"32 %s(%ld)",strError,(*size));
	ESP_LOG_BUFFER_HEX(strError,pdata,(*size)*4);
#else
    std::printf(m_header);
	std::printf("%s %ld:", strError, *size);
	std::printf(" %d", (int)pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",%d", (int)pdata[i]);
	}
	std::printf("\n");
#endif
}

void CTraceTask::printData16h(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint16_t *pdata = (uint16_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint16_t))];

	printHeader(*res);
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOGI(m_header,"16 %s(%ld)",strError,(*size));
	ESP_LOG_BUFFER_HEX(strError,pdata,(*size)*2);
#else
    std::printf(m_header);
	std::printf("%s %ld:", strError, *size);
	std::printf(" 0x%04x", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",0x%04x", pdata[i]);
	}
	std::printf("\n");
#endif
}

void CTraceTask::printData16h_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint16_t *pdata = (uint16_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOGI(m_header,"16 %s(%ld)",strError,(*size));
	ESP_LOG_BUFFER_HEX(strError,pdata,(*size)*2);
#else
    std::printf(m_header);
	std::printf("%s %ld:", strError, *size);
	std::printf(" 0x%04x", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",0x%04x", pdata[i]);
	}
	std::printf("\n");
#endif
}

void CTraceTask::printData16(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	int16_t *pdata = (int16_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint16_t))];

	printHeader(*res);
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOGI(m_header,"16 %s(%ld)",strError,(*size));
	ESP_LOG_BUFFER_HEX(strError,pdata,(*size)*2);
#else
    std::printf(m_header);
	std::printf("%s %ld:", strError, *size);
	std::printf(" %d", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",%d", pdata[i]);
	}
	std::printf("\n");
#endif
}

void CTraceTask::printData16_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint16_t *pdata = (uint16_t *)(data[8 + 2] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOGI(m_header,"16 %s(%ld)",strError,(*size));
	ESP_LOG_BUFFER_HEX(strError,pdata,(*size)*2);
#else
    std::printf(m_header);
	std::printf("%s %ld:", strError, *size);
	std::printf(" %d", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",%d", pdata[i]);
	}
	std::printf("\n");
#endif
}

void CTraceTask::printData8h(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint8_t *pdata = (uint8_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint8_t))];

	printHeader(*res);
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOGI(m_header,"8 %s(%ld)",strError,(*size));
	ESP_LOG_BUFFER_HEX(strError,pdata,(*size));
#else
    std::printf(m_header);
	std::printf("%s %ld:", strError, *size);
	std::printf(" 0x%02x", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",0x%02x", pdata[i]);
	}
	std::printf("\n");
#endif
}

void CTraceTask::printData8h_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint8_t *pdata = (uint8_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOGI(m_header,"8 %s(%ld)",strError,(*size));
	ESP_LOG_BUFFER_HEX(strError,pdata,(*size));
#else
    std::printf(m_header);
	std::printf("%s %ld:", strError, *size);
	std::printf(" 0x%02x", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",0x%02x", pdata[i]);
	}
	std::printf("\n");
#endif
}

void CTraceTask::printData8(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	int8_t *pdata = (int8_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint8_t))];

	printHeader(*res);
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOGI(m_header,"8 %s(%ld)",strError,(*size));
	ESP_LOG_BUFFER_HEX(strError,pdata,(*size));
#else
    std::printf(m_header);
	std::printf("%s %ld:", strError, *size);
	std::printf(" %d", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",%d", pdata[i]);
	}
	std::printf("\n");
#endif
}

void CTraceTask::printData8_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint8_t *pdata = (uint8_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOGI(m_header,"8 %s(%ld)",strError,(*size));
	ESP_LOG_BUFFER_HEX(strError,pdata,(*size));
#else
    std::printf(m_header);
	std::printf("%s %ld:", strError, *size);
	std::printf(" %d", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",%d", pdata[i]);
	}
	std::printf("\n");
#endif
}

void CTraceTask::printIsrString(char *strError, int16_t errCode)
{
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOGE("ISR","%d:%s",errCode, strError);
#else
	std::printf("%d:%s\n", errCode, strError);
#endif
}

void CTraceTask::printString(char *data)
{
	uint64_t *res = (uint64_t *)data;
	int32_t *errCode = (int32_t *)&data[8];
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	esp_log_level_t level = (esp_log_level_t)data[12];
#endif
	char *strError = &data[13];

	printHeader(*res);
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOG_LEVEL(level,m_header,"%ld:%s",(*errCode), strError);
#else
    std::printf(m_header);
	std::printf(": %d:%s\n", (int)(*errCode), strError);
#endif
}

void CTraceTask::printS(char *str)
{
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOGI("*","%s",str);
#else
	std::printf("%s\n", str);
#endif
}

void CTraceTask::printStop(char *data)
{
	uint64_t *x = (uint64_t *)data;
	int32_t *n = (int32_t *)&data[8];
	char *str = &data[12];

	printHeader(*x, *n);
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	ESP_LOGI(m_header,"%s", str);
#else
    std::printf(m_header);
	std::printf(" %s\n", str);
#endif
}

void CTraceTask::trace(const char *strError, int32_t errCode, esp_log_level_t level, bool reboot)
{
	STaskMessage msg;
	taskENTER_CRITICAL(&mMut);
	uint64_t tm = getTimer(AUTO_TIMER);
	taskEXIT_CRITICAL(&mMut);
	if (errCode != 0x7fffffff)
	{
		int ln = 8 + 4 + 1 + 1;
		if (strError != nullptr)
		{
			ln += std::strlen(strError);
		}
		char *str;
		if (reboot)
			str = (char *)allocNewMsg(&msg, MSG_TRACE_STRING_REBOOT, ln);
		else
			str = (char *)allocNewMsg(&msg, MSG_TRACE_STRING, ln);
		std::memcpy(str, &tm, 8);
		std::memcpy(&str[8], &errCode, 4);
		str[12] = (uint8_t)level;
		if (strError != nullptr)
		{
			std::strcpy(&str[13], strError);
		}
		else
		{
			str[ln - 1] = 0;
		}
		sendMessage(&msg, 0, true);
	}
}

void CTraceTask::traceFromISR(const char *strError, int16_t errCode, BaseType_t *pxHigherPriorityTaskWoken)
{
	STaskMessage msg;
	msg.msgID= MSG_TRACE_ISR_STRING;
	msg.shortParam = (uint16_t)errCode;
	msg.msgBody = (void*)strError;
	sendMessageFrontFromISR(&msg, pxHigherPriorityTaskWoken);
}

void CTraceTask::traceData(const char *strError, void *data, uint32_t size, uint16_t tp)
{
	STaskMessage msg;
	taskENTER_CRITICAL(&mMut);
	uint64_t tm = getTimer(AUTO_TIMER);
	taskEXIT_CRITICAL(&mMut);

	int sz = 1;
	switch (tp)
	{
	case MSG_TRACE_INT8:
		sz = sizeof(int8_t);
		break;
	case MSG_TRACE_UINT16:
		sz = sizeof(uint16_t);
		break;
	case MSG_TRACE_INT16:
		sz = sizeof(int16_t);
		break;
	case MSG_TRACE_UINT32:
		sz = sizeof(uint32_t);
		break;
	case MSG_TRACE_INT32:
		sz = sizeof(int32_t);
		break;
	default:
		sz = sizeof(uint8_t);
		break;
	}

	int ln = 8 + 4 + (size * sz) + 1;
	if (strError != nullptr)
	{
		ln += std::strlen(strError);
	}
	char *str = (char *)allocNewMsg(&msg, tp, ln);
	std::memcpy(str, &tm, 8);
	std::memcpy(&str[8], &size, 4);
	std::memcpy(&str[8 + 4], data, size * sz);
	if (strError != nullptr)
	{
		std::strcpy(&str[8 + 4 + (size * sz)], strError);
	}
	else
	{
		str[ln - 1] = 0;
	}
	sendMessage(&msg, 0, true);
}

void CTraceTask::traceData2(const char *strError, void *data, uint32_t size, uint16_t tp)
{
	STaskMessage msg;
	taskENTER_CRITICAL(&mMut);
	uint64_t tm = getTimer(AUTO_TIMER);
	taskEXIT_CRITICAL(&mMut);
	int ln = 8 + 4 + 4 + 1;
	if (strError != nullptr)
	{
		ln += std::strlen(strError);
	}
	char *str = (char *)allocNewMsg(&msg, tp, ln);
	std::memcpy(str, &tm, 8);
	std::memcpy(&str[8], &size, 4);
	std::memcpy(&str[8 + 4], &data, 4);
	if (strError != nullptr)
	{
		std::strcpy(&str[8 + 4 + 4], strError);
	}
	else
	{
		str[ln - 1] = 0;
	}
	sendMessage(&msg, 0, true);
}

void CTraceTask::startTime()
{
	taskENTER_CRITICAL(&mMut);
	getTimer();
	taskEXIT_CRITICAL(&mMut);
};

void CTraceTask::stopTime(const char *str, uint32_t n)
{
	STaskMessage msg;
	taskENTER_CRITICAL(&mMut);
	uint64_t tm = getTimer();
	taskEXIT_CRITICAL(&mMut);

	int ln = 8 + 4 + 1;
	if (str != nullptr)
	{
		ln += std::strlen(str);
	}
	char *dt = (char *)allocNewMsg(&msg, MSG_STOP_TIME, ln);
	std::memcpy(dt, &tm, 8);
	std::memcpy(&dt[8], &n, 4);
	if (str != nullptr)
	{
		std::strcpy(&dt[8 + 4], str);
	}
	else
	{
		dt[ln - 1] = 0;
	}
	sendMessage(&msg, 0, true);
};

void CTraceTask::log(const char *str)
{
	STaskMessage msg;
	if (str != nullptr)
	{
		char *dt = (char *)allocNewMsg(&msg, MSG_PRINT_STRING, std::strlen(str) + 1);
		std::strcpy(dt, str);
	}
	else
	{
		char *dt = (char *)allocNewMsg(&msg, MSG_PRINT_STRING, 1);
		dt[0] = 0;
	}
	sendMessage(&msg, 0, true);
}
