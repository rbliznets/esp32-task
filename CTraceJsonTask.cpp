/*!
	\file
	\brief Класс для вывода отладочной информации в json формате.
	\authors Близнец Р.А.(r.bliznets@gmail.com)
	\version 1.0.0.0
	\date 02.05.2024

	Один объект на приложение.
	Необходим чтобы не блокировать отлаживаемую задачу.
*/

#include "CTraceJsonTask.h"

# define __bswap_16(x) \
    (__extension__							      \
     ({ unsigned short int __bsx = (x);					      \
        ((((__bsx) >> 8) & 0xff) | (((__bsx) & 0xff) << 8)); }))

# define __bswap_32(x) \
    (__extension__							      \
     ({ unsigned int __bsx = (x);					      \
        ((((__bsx) & 0xff000000) >> 24) | (((__bsx) & 0x00ff0000) >>  8) |    \
	 (((__bsx) & 0x0000ff00) <<  8) | (((__bsx) & 0x000000ff) << 24)); }))


void CTraceJsonTask::printHeader(uint64_t time, uint32_t n)
{
	CTraceTask::printHeader(time, n);

	mAnswer = "{\"log\":{\"time\":\"";
	mAnswer += m_header;
	mAnswer += "\"";
}

void CTraceJsonTask::printString(char *data)
{
	uint64_t *res = (uint64_t *)data;
	int32_t *errCode = (int32_t *)&data[8];
	char *strError = &data[13];

	printHeader(*res);
	mAnswer += ",\"code\":";
	mAnswer += std::to_string(*errCode);
	mAnswer += ",\"level\":";
	mAnswer += std::to_string(data[12]);
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\"}}";
}

void CTraceJsonTask::printS(char *str)
{
	mAnswer = "{\"log\":{\"value\":\"";
	mAnswer += str;
	mAnswer += "\"}}";
}

void CTraceJsonTask::printStop(char *data)
{
	uint64_t *x = (uint64_t *)data;
	int32_t *n = (int32_t *)&data[8];
	char *str = &data[12];

	printHeader(*x, *n);
	mAnswer += ",\"value\":\"";
	mAnswer += str;
	mAnswer += "\"}}";
}

void CTraceJsonTask::printData8h(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint8_t *pdata = (uint8_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint8_t))];

	printHeader(*res);
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	// mJsonAmswer += "\",\"size\":";
	// mJsonAmswer += std::to_string(*size);
	// mJsonAmswer += ",\"data\":\"";
	// std::sprintf(m_header,"0x%02x", pdata[0]);
	// mJsonAmswer += m_header;
	// for (int16_t i = 1; i < *size; i++)
	// {
	// 	std::sprintf(m_header,",0x%02x", pdata[i]);
	// 	mJsonAmswer += m_header;
	// }
	// mJsonAmswer += "\"}}";
	mAnswer += "\",\"data\":\"";
	for (int16_t i = 0; i < *size; i++)
	{
		std::sprintf(m_header, "%02x", pdata[i]);
		mAnswer += m_header;
	}
	mAnswer += "\"}}";
}

void CTraceJsonTask::printData8h_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint8_t *pdata = (uint8_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":\"";
	for (int16_t i = 0; i < *size; i++)
	{
		std::sprintf(m_header, "%02x", pdata[i]);
		mAnswer += m_header;
	}
	mAnswer += "\"}}";
}

void CTraceJsonTask::printData8(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	int8_t *pdata = (int8_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint8_t))];

	printHeader(*res);
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":[";
	std::sprintf(m_header, "%d", pdata[0]);
	mAnswer += m_header;
	for (int16_t i = 1; i < *size; i++)
	{
		std::sprintf(m_header, ",%d", pdata[i]);
		mAnswer += m_header;
	}
	mAnswer += "]}}";
}

void CTraceJsonTask::printData8_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint8_t *pdata = (uint8_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":[";
	std::sprintf(m_header, "%d", pdata[0]);
	mAnswer += m_header;
	for (int16_t i = 1; i < *size; i++)
	{
		std::sprintf(m_header, ",%d", pdata[i]);
		mAnswer += m_header;
	}
	mAnswer += "]}}";
}

void CTraceJsonTask::printData32_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint32_t *pdata = (uint32_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":[";
	std::sprintf(m_header, "%ld", pdata[0]);
	mAnswer += m_header;
	for (int16_t i = 1; i < *size; i++)
	{
		std::sprintf(m_header, ",%ld", pdata[i]);
		mAnswer += m_header;
	}
	mAnswer += "]}}";
}

void CTraceJsonTask::printData32h_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint32_t *pdata = (uint32_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":\"";
	for (int16_t i = 0; i < *size; i++)
	{
		std::sprintf(m_header, "%08x", __bswap_32(pdata[i]));
		mAnswer += m_header;
	}
	mAnswer += "\"}}";
}

void CTraceJsonTask::printData32h(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint32_t *pdata = (uint32_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint32_t))];

	printHeader(*res);
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":\"";
	for (int16_t i = 0; i < *size; i++)
	{
		std::sprintf(m_header, "%08x", __bswap_32(pdata[i]));
		mAnswer += m_header;
	}
	mAnswer += "\"}}";
}

void CTraceJsonTask::printData32(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	int32_t *pdata = (int32_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint32_t))];

	printHeader(*res);
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":[";
	std::sprintf(m_header, "%ld", pdata[0]);
	mAnswer += m_header;
	for (int16_t i = 1; i < *size; i++)
	{
		std::sprintf(m_header, ",%ld", pdata[i]);
		mAnswer += m_header;
	}
	mAnswer += "]}}";
}

void CTraceJsonTask::printData16h(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint16_t *pdata = (uint16_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint16_t))];

	printHeader(*res);
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":\"";
	for (int16_t i = 0; i < *size; i++)
	{
		std::sprintf(m_header, "%04x", __bswap_16(pdata[i]));
		mAnswer += m_header;
	}
	mAnswer += "\"}}";
}

void CTraceJsonTask::printData16h_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint16_t *pdata = (uint16_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":\"";
	for (int16_t i = 0; i < *size; i++)
	{
		std::sprintf(m_header, "%04x", __bswap_16(pdata[i]));
		mAnswer += m_header;
	}
	mAnswer += "\"}}";
}

void CTraceJsonTask::printData16(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	int16_t *pdata = (int16_t *)&data[8 + 4];
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint16_t))];

	printHeader(*res);
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":[";
	std::sprintf(m_header, "%d", pdata[0]);
	mAnswer += m_header;
	for (int16_t i = 1; i < *size; i++)
	{
		std::sprintf(m_header, ",%d", pdata[i]);
		mAnswer += m_header;
	}
	mAnswer += "]}}";
}

void CTraceJsonTask::printData16_2(char *data)
{
	uint64_t *res = (uint64_t *)data;
	uint32_t *size = (uint32_t *)&data[8];
	uint16_t *pdata = (uint16_t *)(data[8 + 2] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4];

	printHeader(*res);
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":[";
	std::sprintf(m_header, "%d", pdata[0]);
	mAnswer += m_header;
	for (int16_t i = 1; i < *size; i++)
	{
		std::sprintf(m_header, ",%d", pdata[i]);
		mAnswer += m_header;
	}
	mAnswer += "]}}";
}
