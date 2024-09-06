/*!
	\file
	\brief Класс для вывода отладочной информации.
	\authors Близнец Р.А.(r.bliznets@gmail.com)
	\version 1.3.1.0
	\date 15.09.2022

	Один объект на приложение.
	Необходим чтобы не блокировать отлаживаемую задачу.
*/

#pragma once

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "CBaseTask.h"
#include "ITraceLog.h"

#define MSG_TRACE_ISR_STRING 5024	 ///< ID сообщения вывода строки из прерывания.
#define MSG_TRACE_STRING 5025		 ///< ID сообщения вывода строки.
#define MSG_TRACE_STRING_REBOOT 5026 ///< ID сообщения вывода строки и перезагрузки (из прерывания).
#define MSG_TRACE_UINT8 5027		 ///< ID сообщения вывода массива uint8_t.
#define MSG_TRACE_UINT16 5028		 ///< ID сообщения вывода массива uint16_t.
#define MSG_TRACE_UINT32 5029		 ///< ID сообщения вывода массива uint32_t.
#define MSG_STOP_TIME 5030			 ///< ID сообщения вычисления интервала.
#define MSG_TRACE_INT8 5031			 ///< ID сообщения вывода массива int8_t.
#define MSG_TRACE_INT16 5032		 ///< ID сообщения вывода массива int16_t.
#define MSG_TRACE_INT32 5033		 ///< ID сообщения вывода массива int32_t.
#define MSG_PRINT_STRING 5034		 ///< ID сообщения простого вывода строки.

#define MSG_TRACE2_UINT8 5127  ///< ID сообщения вывода массива uint8_t.
#define MSG_TRACE2_UINT16 5128 ///< ID сообщения вывода массива uint16_t.
#define MSG_TRACE2_UINT32 5129 ///< ID сообщения вывода массива uint32_t.
#define MSG_TRACE2_INT8 5131   ///< ID сообщения вывода массива int8_t.
#define MSG_TRACE2_INT16 5132  ///< ID сообщения вывода массива int16_t.
#define MSG_TRACE2_INT32 5133  ///< ID сообщения вывода массива int32_t.

/// Класс задачи вывода отладочной информации.
class CTraceTask : public CBaseTask, public ITraceLog
{
private:
	portMUX_TYPE mMut = portMUX_INITIALIZER_UNLOCKED; ///< Мьютекс для критической секции.
protected:
	char m_header[32]; ///< Буфер для времени

	/// Вывести интервал времени с предыдущего сообщения
	/*!
	  \param[in] time Сообщение об ошибке.
	  \param[in] n количество для усреднения.
	*/
	virtual void printHeader(uint64_t time, uint32_t n = 1);

	/// Обработка сообщения вывода
	/*!
	  \param[in] msg Сообщение об ошибке.
	  \return true если сообщение обработано.
	*/
	bool logMessage(STaskMessage &msg);

	/// Распечатать массив данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	  \param[in] tp ID типа данных.
	*/
	void traceData(const char *strError, void *data, uint32_t size, uint16_t tp = MSG_TRACE_UINT8);
	/// Распечатать массив данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	  \param[in] tp ID типа данных.
	*/
	void traceData2(const char *strError, void *data, uint32_t size, uint16_t tp = MSG_TRACE2_UINT8);

	/// Функция задачи.
	virtual void run() override;

	/// Вывести сообщение.
	/*!
	  \param[in] strError Указатель строку.
	  \param[in] errCode Код ошибки.
	*/
	virtual void printIsrString(char *strError, int16_t errCode);
	/// Вывести сообщение.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_STRING или MSG_TRACE_STRING_REBOOT.
	*/
	virtual void printString(char *data);
	/// Вывести сообщение.
	/*!
	  \param[in] str Указатель строку.
	*/
	virtual void printS(char *str);
	/// Вывести сообщение об интервале времени.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_STOP_TIME.
	*/
	virtual void printStop(char *data);
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_UINT8.
	*/
	virtual void printData8h(char *data);
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_UINT8.
	*/
	virtual void printData8h_2(char *data);
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_UINT16.
	*/
	virtual void printData16h(char *data);
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_UINT16.
	*/
	virtual void printData16h_2(char *data);
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_UINT32.
	*/
	virtual void printData32h(char *data);
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_UINT32.
	*/
	virtual void printData32h_2(char *data);
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_INT8.
	*/
	virtual void printData8(char *data);
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_INT8.
	*/
	virtual void printData8_2(char *data);
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_INT16.
	*/
	virtual void printData16(char *data);
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_INT16.
	*/
	virtual void printData16_2(char *data);
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_INT32.
	*/
	virtual void printData32(char *data);
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_INT32.
	*/
	virtual void printData32_2(char *data);

	/// Деструктор.
	virtual ~CTraceTask() {};

public:
	/// Единственный экземпляр класса.
	/*!
	  \return Указатель на CTraceTask
	*/
	static CTraceTask *Instance()
	{
		static CTraceTask theSingleInstance;
		return &theSingleInstance;
	}

	/// Начальная инициализация.
	/*!
	  \param[in] queueLength Максимальная длина очереди сообщений.
	  \param[in] coreID Ядро CPU (0,1).
	*/
	virtual void init(UBaseType_t queueLength = 30, BaseType_t coreID = 1)
	{
		CBaseTask::init("trace", 2048 + 1024, 1, queueLength, coreID);
	};

	/// Виртуальный метод трассировки
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] errCode Код ошибки.
	  \param[in] level Уровень вывода сообщения.
	  \param[in] reboot Флаг перезагрузки.
	*/
	virtual void trace(const char *strError, int32_t errCode, esp_log_level_t level, bool reboot) override;
	/// Виртуальный метод трассировки из прерывания.
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] errCode Код ошибки.
	*/
	virtual void IRAM_ATTR traceFromISR(const char *strError, int16_t errCode, BaseType_t *pxHigherPriorityTaskWoken) override;

	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual inline void trace(const char *strError, uint8_t *data, uint32_t size) override
	{
		if (size > 4096)
			traceData2(strError, data, size, MSG_TRACE2_UINT8);
		else
			traceData(strError, data, size, MSG_TRACE_UINT8);
	};
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual inline void trace(const char *strError, int8_t *data, uint32_t size) override
	{
		if (size > 4096)
			traceData2(strError, data, size, MSG_TRACE2_INT8);
		else
			traceData(strError, data, size, MSG_TRACE_INT8);
	};
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual inline void trace(const char *strError, uint16_t *data, uint32_t size) override
	{
		if (size > 2048)
			traceData2(strError, data, size, MSG_TRACE2_UINT16);
		else
			traceData(strError, data, size, MSG_TRACE_UINT16);
	};
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual inline void trace(const char *strError, int16_t *data, uint32_t size) override
	{
		if (size > 2048)
			traceData2(strError, data, size, MSG_TRACE2_INT16);
		else
			traceData(strError, data, size, MSG_TRACE_INT16);
	};
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual inline void trace(const char *strError, uint32_t *data, uint32_t size) override
	{
		if (size > 1024)
			traceData2(strError, data, size, MSG_TRACE2_UINT32);
		else
			traceData(strError, data, size, MSG_TRACE_UINT32);
	};
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual inline void trace(const char *strError, int32_t *data, uint32_t size) override
	{
		if (size > 1024)
			traceData2(strError, data, size, MSG_TRACE2_INT32);
		else
			traceData(strError, data, size, MSG_TRACE_INT32);
	};

	/// Обнулить метку времени
	virtual void startTime() override;
	/// Вывести интервал времени
	/*!
	  \param[in] str название интервала.
	  \param[in] n количество для усреднения.
	*/
	virtual void stopTime(const char *str, uint32_t n = 1) override;

	/// Вывести сообщение
	/*!
	  \param[in] str Сообщение.
	*/
	void log(const char *str) override;
};
