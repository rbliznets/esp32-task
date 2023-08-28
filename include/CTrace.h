/*!
	\file
	\brief Вывод сообщения об ошибке.
	\authors Близнец Р.А.
	\version 1.2.0.0
	\date 10.07.2020
*/

#if !defined CTRACE_H
#define CTRACE_H

#include <stdint.h>
#include "ITraceLog.h"
#include "CLock.h"
#include <list>
#include "sdkconfig.h"

/*! \addtogroup  debug

    @{
*/

#ifdef CONFIG_DEBUG_CODE
/// Вывод лога
/*!
  \param[in] str Сообщение.
*/
#define LOG(str)	traceLog.log(str)
/// Вывод сообщения
/*!
  \param[in] str Сообщение std::string.
*/
#define PRINT(str)	traceLog.stopTime(str.c_str(), 1)
/// Основной метод трассировки
/*!
	\param[in] str Сообщение об ошибке.
	\param[in] code Код ошибки.
	\param[in] reboot Флаг перезагрузки.
*/
#define TRACE(str, code, reboot)	traceLog.trace((char*)str, code, reboot)
/// Вывести значение в десятичном виде
/*!
	\param[in] str Сообщение.
	\param[in] code значение.
*/
#define TDEC(str, code)	traceLog.trace((char*)str, code, false)
/// Вывести значение в hex виде
/*!
	\param[in] str Сообщение.
	\param[in] code значение.
*/
#define THEX(str, code)	{auto x = code; traceLog.trace(str, &x, 1);}
/// Основной метод трассировки из прерывания
/*!
	\param[in] str Сообщение об ошибке.
	\param[in] code Код ошибки.
	\param[in] reboot Флаг перезагрузки.
	\param[in|out] pxHigherPriorityTaskWoken Флаг переключения задач.
*/
#define TRACE_FROM_ISR(str, code, reboot, pxHigherPriorityTaskWoken)	traceLog.traceFromISR((char*)str, code, reboot, pxHigherPriorityTaskWoken)

/// Метод трассировки массива данных
/*!
	\param[in] str Сообщение об ошибке.
	\param[in] data данные.
	\param[in] size размер данных.
*/
#define TRACEDATA(str, data, size)	traceLog.trace(str, data, size)

/// Старт секундомера
#define STARTTIMESHOT()	traceLog.startTime()
/// Фиксация времени секундомера
/*!
	\param[in] str Сообщение.
*/
#define STOPTIMESHOT(str)	traceLog.stopTime(str, 1)
/// Вывести интервал времени
/*!
	\param[in] str название интервала.
	\param[in] N количество для усреднения.
*/
#define STOPTIME(str, N)	traceLog.stopTime(str, N)

/// Добавить трассировщика
/*!
	\param[in] log трассировщик
*/
#define ADDLOG(log)	traceLog.add(log)
/// Убрать трассировщика
/*!
	\param[in] log трассировщик
*/
#define REMOVELOG(log)	traceLog.remove(log)
/// Очистить список
#define CLEARLOGS() traceLog.clear();

#else
#define LOG(str)	
#define PRINT(str)	

#define TRACE(str, code, reboot)	
#define TDEC(str, code)	
#define THEX(str, code)	
#define TRACE_FROM_ISR(str, code, reboot, pxHigherPriorityTaskWoken)	
#define TRACEDATA(str, data, size)

#define STARTTIMESHOT()	
#define STOPTIMESHOT(str)
#define STOPTIME(str, N)

#define ADDLOG(log)	
#define REMOVELOG(log)	
#define CLEARLOGS() 
#endif


/// Класс списка зарегестрированных трассировщиков
class CTraceList:public ITraceLog, public CLock
{
protected:
	std::list<ITraceLog*> m_list;///< Список зарегестрированных трассировщиков

public:
	/// Конструктор
	CTraceList();
	/// Деструктор
	virtual ~CTraceList();

	/// Виртуальный метод трассировки
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] errCode Код ошибки.
	  \param[in] reboot Флаг перезагрузки.
	*/
	virtual void trace(const char* strError, int32_t errCode, bool reboot) override;
	/// Виртуальный метод трассировки из прерывания.
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] errCode Код ошибки.
	  \param[in] reboot Флаг перезагрузки.
	  \param[in|out] pxHigherPriorityTaskWoken Флаг переключения задач.
	*/
	virtual void IRAM_ATTR traceFromISR(const char* strError, int32_t errCode, bool reboot, BaseType_t *pxHigherPriorityTaskWoken) override;

	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual void trace(const char* strError, uint8_t* data, uint32_t size) override;
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual void trace(const char* strError, int8_t* data, uint32_t size) override;
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual void trace(const char* strError, uint16_t* data, uint32_t size) override;
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual void trace(const char* strError, int16_t* data, uint32_t size) override;
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual void trace(const char* strError, uint32_t* data, uint32_t size) override;
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	virtual void trace(const char* strError, int32_t* data, uint32_t size) override;
	
	/// Вывести сообщение
	/*!
	  \param[in] str Сообщение.
	*/
	virtual void log(const char *str) override;

	/// Обнулить метку времену
	virtual void startTime() override;
	/// Вывести интервал времени
	/*!
	  \param[in] str название интервала.
	  \param[in] n количество для усреднения.
	*/
	virtual void stopTime(const char* str, uint32_t n=1) override;

	/// Добавить трассировщик в список
	/*!
	  \param[in] log трассировщик
	*/
	void add(ITraceLog* log);
	/// Удалить трассировщика из списка
	/*!
	  \param[in] log трассировщик
	*/
	void remove(ITraceLog* log);
	/// Очистить список
	void clear();
};

#ifdef CONFIG_DEBUG_CODE
extern CTraceList traceLog;///< Объект глобального списка зарегестрированных трассировщиков
#endif
/*! @} */

#endif // CTRACE_H