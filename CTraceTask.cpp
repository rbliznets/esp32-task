/*!
	\file
	\brief Класс для вывода отладочной информации.
	\authors Близнец Р.А.
	\version 1.2.0.0
	\date 15.09.2022

	Один объект на приложение.
	Необходим чтобы не блокировать отлаживаемую задачу.
*/

#include "CTraceTask.h"
#include <cstring>
#include "esp_system.h"
#include "CTrace.h"

#ifdef CONFIG_TRACE_AUTO_RESET
#define AUTO_TIMER CONFIG_TRACE_AUTO_RESET
#else
#define AUTO_TIMER false
#endif

void CTraceTask::printHeader(uint64_t time, uint32_t n)
{
	uint64_t res = time / n;
#if (CONFIG_TRACE_USEC == 1)
	std::printf("(+%liusec)", (long)res);
#else
	if (res >= 10000000)
	{
		std::printf("(+%lisec)", (long)(res / 1000000));
	}
	else
	{
		if (res < 10000)
		{
			if (res < 10)
			{
				double f = time / (double)n;
				std::printf("(+%linsec)", (long)(f * 1000));
			}
			else
			{
				std::printf("(+%liusec)", (long)res);
			}
		}
		else
		{
			std::printf("(+%limsec)", (long)(res / 1000));
		}
	}
#endif
}

void CTraceTask::run()
{
	STaskMessage msg;

	while (getMessage(&msg, portMAX_DELAY))
	{
		switch (msg.msgID)
		{
		case MSG_TRACE_STRING:
			printString((char *)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_STOP_TIME:
			printStop((char *)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_PRINT_STRING:
			std::printf((char *)msg.msgBody);
			vPortFree(msg.msgBody);
			std::printf("\n");
			break;
		case MSG_TRACE_STRING_REBOOT:
			printString((char *)msg.msgBody);
			std::printf("trace reboot...\n");
			// vPortFree(msg.msgBody);
			fflush(stdout);
			esp_restart();
			break;
		case MSG_TRACE_UINT8:
			printData8h((char *)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE2_UINT8:
			printData8h_2((char *)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE_INT8:
			printData8((char *)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE2_INT8:
			printData8_2((char *)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE_UINT16:
			printData16h((char *)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE2_UINT16:
			printData16h_2((char *)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE_INT16:
			printData16((char *)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE2_INT16:
			printData16_2((char *)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE_UINT32:
			printData32h((char *)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE2_UINT32:
			printData32h_2((char *)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE_INT32:
			printData32((char *)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		case MSG_TRACE2_INT32:
			printData32_2((char *)msg.msgBody);
			vPortFree(msg.msgBody);
			break;
		default:
			TRACE_WARNING("CTraceTask unknown message", msg.msgID);
			break;
		}
		// vTaskDelay(pdMS_TO_TICKS(250));//@@@@@@@@@@@@@@
	}
}

void CTraceTask::printData32_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint32_t *pdata = (uint32_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);

	std::printf("%s %ld:", strError, *size);
	std::printf(" %d", (int)pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",%d", (int)pdata[i]);
	}
	std::printf("\n");
}

void CTraceTask::printData32h_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint32_t *pdata = (uint32_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);

	std::printf("%s %ld:", strError, *size);
	std::printf(" 0x%08x", (int)pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",0x%08x", (int)pdata[i]);
	}
	std::printf("\n");
}

void CTraceTask::printData32h(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint32_t *pdata = (uint32_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint32_t))];

	printHeader(*res);

	std::printf("%s %ld:", strError, *size);
	std::printf(" 0x%08x", (int)pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",0x%08x", (int)pdata[i]);
	}
	std::printf("\n");
}

void CTraceTask::printData32(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	int32_t *pdata = (int32_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint32_t))];

	printHeader(*res);

	std::printf("%s %ld:", strError, *size);
	std::printf(" %d", (int)pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",%d", (int)pdata[i]);
	}
	std::printf("\n");
}

void CTraceTask::printData16h(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint16_t *pdata = (uint16_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint16_t))];

	printHeader(*res);

	std::printf("%s %ld:", strError, *size);
	std::printf(" 0x%04x", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",0x%04x", pdata[i]);
	}
	std::printf("\n");
}

void CTraceTask::printData16h_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint16_t *pdata = (uint16_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);

	std::printf("%s %ld:", strError, *size);
	std::printf(" 0x%04x", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",0x%04x", pdata[i]);
	}
	std::printf("\n");
}

void CTraceTask::printData16(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	int16_t *pdata = (int16_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint16_t))];

	printHeader(*res);

	std::printf("%s %ld:", strError, *size);
	std::printf(" %d", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",%d", pdata[i]);
	}
	std::printf("\n");
}

void CTraceTask::printData16_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint16_t *pdata = (uint16_t *)(data[8 + 2] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);

	std::printf("%s %ld:", strError, *size);
	std::printf(" %d", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",%d", pdata[i]);
	}
	std::printf("\n");
}

void CTraceTask::printData8h(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint8_t *pdata = (uint8_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint8_t))];

	printHeader(*res);

	std::printf("%s %ld:", strError, *size);
	std::printf(" 0x%02x", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",0x%02x", pdata[i]);
	}
	std::printf("\n");
}

void CTraceTask::printData8h_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint8_t *pdata = (uint8_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);

	std::printf("%s %ld:", strError, *size);
	std::printf(" 0x%02x", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",0x%02x", pdata[i]);
	}
	std::printf("\n");
}

void CTraceTask::printData8(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	int8_t *pdata = (int8_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint8_t))];

	printHeader(*res);

	std::printf("%s %ld:", strError, *size);
	std::printf(" %d", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",%d", pdata[i]);
	}
	std::printf("\n");
}

void CTraceTask::printData8_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint8_t *pdata = (uint8_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);

	std::printf("%s %ld:", strError, *size);
	std::printf(" %d", pdata[0]);
	for (int16_t i = 1; i < *size; i++)
	{
		std::printf(",%d", pdata[i]);
	}
	std::printf("\n");
}

void CTraceTask::printString(char *data)
{
	uint64_t *res = (uint64_t *)data;
	int32_t *errCode = (int32_t *)&data[8];
	char *strError = &data[12];

	printHeader(*res);
	std::printf("->%d:%s\n", (int)(*errCode), strError);
}

void CTraceTask::printStop(char *data)
{
	uint64_t *x = (uint64_t *)data;
	int32_t *n = (int32_t *)&data[8];
	char *str = &data[12];

	printHeader(*x, *n);

	std::printf(" %s\n", str);
}

void CTraceTask::trace(const char *strError, int32_t errCode, bool reboot)
{
	STaskMessage msg;
	taskENTER_CRITICAL(&mMut);
	uint64_t tm = getTimer(AUTO_TIMER);
	taskEXIT_CRITICAL(&mMut);
	if (errCode != 0x7fffffff)
	{
		int ln = 8 + 4 + 1;
		if (strError != nullptr)
		{
			ln += std::strlen(strError);
		}
		char *str = (char *)allocNewMsg(&msg, MSG_TRACE_STRING, ln);
		std::memcpy(str, &tm, 8);
		std::memcpy(&str[8], &errCode, 4);
		if (strError != nullptr)
		{
			std::strcpy(&str[12], strError);
		}
		else
		{
			str[ln - 1] = 0;
		}
		sendMessage(&msg, 0, true);
	}
}

void CTraceTask::traceFromISR(const char *strError, int32_t errCode, bool reboot, BaseType_t *pxHigherPriorityTaskWoken)
{
	STaskMessage msg;
	taskENTER_CRITICAL_ISR(&mMut);
	uint64_t tm = getTimer(AUTO_TIMER);
	taskEXIT_CRITICAL_ISR(&mMut);
	if (errCode != 0x7fffffff)
	{
		int ln = 8 + 4 + 1;
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
		if (strError != nullptr)
		{
			std::strcpy(&str[12], strError);
		}
		else
		{
			str[12] = 0;
		}
		sendMessageFromISR(&msg, pxHigherPriorityTaskWoken);
	}
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
