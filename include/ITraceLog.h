/*!
	\file
	\brief Интерфейс класса журнала ошибок системы.
	\authors Близнец Р.А.
	\version 1.1.1.0
	\date 10.07.2020
*/

#if !defined ITRACELOG_H
#define ITRACELOG_H

#include <cstdio>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "esp_timer.h"

/*! \defgroup debug Отладка
    @{
*/

//#define configCPU_CLOCK_HZ (CONFIG_ESP32S3_DEFAULT_CPU_FREQ_MHZ*1000000)

/// Интерфейс класса трассировки сообщения об ошибке
class ITraceLog
{
protected:
	int64_t mTime;///< Время последнего сообщения

    /// Текущее значение таймера.
	/*!
	  \return Текущее значение таймера.
	*/
    /// <returns>Текущее значение таймера</returns>
	int64_t getTimer(bool refresh = true)
	{
		int64_t res=0;
		int64_t time = esp_timer_get_time();
		res=time-mTime;		
		if(refresh)mTime=time;
		return res;
	};

public:
	/// Конструктор
	ITraceLog():mTime{0}{};
	/// Виртуальный деструктор
	virtual ~ITraceLog() = default;

	/// Виртуальный метод трассировки
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] errCode Код ошибки.
	  \param[in] reboot Флаг перезагрузки.
	*/
	virtual void trace(const char *strError, int32_t errCode, bool reboot) = 0;
	/// Виртуальный метод трассировки из прерывания.
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] errCode Код ошибки.
	  \param[in] reboot Флаг перезагрузки.
	  \param[out] pxHigherPriorityTaskWoken Флаг переключения задач.
	*/
	virtual void IRAM_ATTR traceFromISR(const char *strError, int32_t errCode, bool reboot, BaseType_t *pxHigherPriorityTaskWoken){};
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual void trace(const char *strError, uint8_t *data, uint32_t size) = 0;
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual void trace(const char *strError, int8_t *data, uint32_t size){trace(strError, (uint8_t*)data, size);};
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual void trace(const char *strError, uint16_t *data, uint32_t size) = 0;
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual void trace(const char *strError, int16_t *data, uint32_t size){trace(strError, (uint16_t*)data, size);};
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual void trace(const char *strError, uint32_t *data, uint32_t size) = 0;
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual void trace(const char *strError, int32_t *data, uint32_t size){trace(strError, (uint32_t*)data, size);};
	/// Вывести сообщение
	/*!
	  \param[in] str Сообщение.
	*/
	virtual void log(const char *str)
	{
		stopTime(str);
	};

	/// Обнулить метку времени
	inline virtual void startTime() {getTimer();};
	/// Вывести интервал времени
	/*!
	  \param[in] str название интервала.
	  \param[in] n количество для усреднения.
	*/
	virtual void stopTime(const char *str, uint32_t n=1) {trace(str, n, false);};

};
/*! @} */

#endif //ITRACELOG_H
