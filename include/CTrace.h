/*!
	\file
	\brief Error message output.
	\authors Bliznets R.A.(r.bliznets@gmail.com)
	\version 1.3.1.0
	\date 10.07.2020
	\details This header file defines macros and the CTraceList class for a flexible
			 logging system. It allows multiple loggers (implementing ITraceLog) to be
			 registered and called simultaneously for logging messages, errors, data,
			 and timing information. Macros provide convenient access to the global
			 traceLog instance.
*/

#pragma once

#include "sdkconfig.h"
#include <stdint.h>
#include "ITraceLog.h"
#include "CLock.h"
#include <list>
#include "esp_log.h"

#ifdef CONFIG_COMPILER_CXX_RTTI
#include <typeinfo>
#endif

#ifdef CONFIG_DEBUG_CODE
/// Output log message
/*!
  \param[in] str Message string.
*/
#define LOG(str) traceLog.log(str)

/// Output a message (using stopTime for timing context)
/*!
  \param[in] str Message std::string.
*/
#define PRINT(str) traceLog.stopTime(str.c_str(), 1)

/// Main trace method
/*!
	\param[in] str Error message string.
	\param[in] code Error code.
	\param[in] reboot Reboot flag.
*/
#define TRACE(str, code, reboot) traceLog.trace((char *)str, code, ESP_LOG_INFO, reboot)
#define TRACE_W(str, code, reboot) traceLog.trace((char *)str, code, ESP_LOG_WARN, reboot)
#define TRACE_E(str, code, reboot) traceLog.trace((char *)str, code, ESP_LOG_ERROR, reboot)

/// Print value in decimal
/*!
	\param[in] str Message string.
	\param[in] code Value to print.
*/
#define TDEC(str, code) traceLog.trace((char *)str, code, ESP_LOG_INFO, false)

/// Print value in hexadecimal
/*!
	\param[in] str Message string.
	\param[in] code Value to print.
*/
#define THEX(str, code)             \
	{                               \
		auto x = code;              \
		traceLog.trace(str, &x, 1); \
	}

/// Main trace method from interrupt
/*!
	\param[in] str Error message string.
	\param[in] code Error code.
	\param[in|out] pxHigherPriorityTaskWoken Task switch flag.
*/
#define TRACE_FROM_ISR(str, code, pxHigherPriorityTaskWoken) traceLog.traceFromISR((char *)str, code, pxHigherPriorityTaskWoken)

/// Trace data array method
/*!
	\param[in] str Error message string.
	\param[in] data Pointer to the data array.
	\param[in] size Size of the data array.
*/
#define TRACEDATA(str, data, size) traceLog.trace(str, data, size)

/// Start stopwatch
#define STARTTIMESHOT() traceLog.startTime()

/// Record stopwatch time
/*!
	\param[in] str Message string.
*/
#define STOPTIMESHOT(str) traceLog.stopTime(str, 1)

/// Print time interval
/*!
	\param[in] str Name of the interval.
	\param[in] N Number for averaging.
*/
#define STOPTIME(str, N) traceLog.stopTime(str, N)

/// Add a tracer
/*!
	\param[in] log Tracer object (ITraceLog*).
*/
#define ADDLOG(log) traceLog.add(log)

/// Remove a tracer
/*!
	\param[in] log Tracer object (ITraceLog*).
*/
#define REMOVELOG(log) traceLog.remove(log)

/// Clear the list of tracers
#define CLEARLOGS() traceLog.clear();

/// Initialize trace system
#define INIT_TRACE() traceLog.init();

#ifdef CONFIG_COMPILER_CXX_RTTI
/// Print error from a class method.
/*!
	\param[in] s Error message string.
	\param[in] x Error code.
*/
#define TRACE_ERROR(s, x)                                       \
	{                                                           \
		if (std::is_base_of_v<ITraceLog, typeof(*this)>)        \
			ESP_LOGE(typeid(*this).name(), "%s: %d", s, x);     \
		else                                                    \
			traceLog.trace((char *)s, x, ESP_LOG_ERROR, false); \
	}

/// Print warning from a class method.
/*!
	\param[in] s Error message string.
	\param[in] x Error code.
*/
#define TRACE_WARNING(s, x)                                    \
	{                                                          \
		if (std::is_base_of_v<ITraceLog, typeof(*this)>)       \
			ESP_LOGW(typeid(*this).name(), "%s: %d", s, x);    \
		else                                                   \
			traceLog.trace((char *)s, x, ESP_LOG_WARN, false); \
	}
#else
/// Print error from a class method.
/*!
	\param[in] s Error message string.
	\param[in] x Error code.
*/
#define TRACE_ERROR(s, x)                                       \
	{                                                           \
		if (std::is_base_of_v<ITraceLog, typeof(*this)>)        \
			ESP_LOGE("Trace", "%s: %d", s, x);                  \
		else                                                    \
			traceLog.trace((char *)s, x, ESP_LOG_ERROR, false); \
	}

/// Print warning from a class method.
/*!
	\param[in] s Error message string.
	\param[in] x Error code.
*/
#define TRACE_WARNING(s, x)                                    \
	{                                                          \
		if (std::is_base_of_v<ITraceLog, typeof(*this)>)       \
			ESP_LOGW("Trace", "%s: %d", s, x);                 \
		else                                                   \
			traceLog.trace((char *)s, x, ESP_LOG_WARN, false); \
	}
#endif

#else // CONFIG_DEBUG_CODE is not defined

// Define empty macros if debugging is disabled
#define LOG(str)
#define PRINT(str)

#define TRACE(str, code, reboot)
#define TRACE_W(str, code, reboot)
#define TRACE_E(str, code, reboot)
#define TDEC(str, code)
#define THEX(str, code)
#define TRACE_FROM_ISR(str, code, pxHigherPriorityTaskWoken)
#define TRACEDATA(str, data, size)

#define STARTTIMESHOT()
#define STOPTIMESHOT(str)
#define STOPTIME(str, N)

#define ADDLOG(log)
#define REMOVELOG(log)
#define CLEARLOGS()

#define INIT_TRACE()

#ifdef CONFIG_COMPILER_CXX_RTTI
/// Print error from a class method (when global tracing is disabled).
/*!
	\param[in] s Error message string.
	\param[in] x Error code.
*/
#define TRACE_ERROR(s, x) ESP_LOGE(typeid(*this).name(), "%s: %d", s, x)

/// Print warning from a class method (when global tracing is disabled).
/*!
	\param[in] s Error message string.
	\param[in] x Error code.
*/
#define TRACE_WARNING(s, x) ESP_LOGW(typeid(*this).name(), "%s: %d", s, x)
#else
/// Print error from a class method (when global tracing is disabled).
/*!
	\param[in] s Error message string.
	\param[in] x Error code.
*/
#define TRACE_ERROR(s, x) ESP_LOGE("Trace", "%s: %d", s, x)

/// Print warning from a class method (when global tracing is disabled).
/*!
	\param[in] s Error message string.
	\param[in] x Error code.
*/
#define TRACE_WARNING(s, x) ESP_LOGW("Trace", "%s: %d", s, x)
#endif
#endif // CONFIG_DEBUG_CODE

/// Class for the list of registered tracers/loggers
/// @details This class aggregates multiple ITraceLog implementations. When a logging
///          method is called on CTraceList, it iterates through its internal list
///          and calls the corresponding method on each registered ITraceLog object.
///          It inherits from CLock to provide thread safety for the internal list
///          operations and iteration.
class CTraceList : public ITraceLog, public CLock
{
protected:
	/// List of registered tracer/loggers (pointers to ITraceLog).
	std::list<ITraceLog *> m_list;

public:
	/// Constructor
	/// @details Initializes base classes ITraceLog and CLock.
	///          Creates the mutex (mMutex) via CLock.
	CTraceList();

	/// Destructor
	/// @details Virtual destructor.
	virtual ~CTraceList();

	/// Initial default setup
	/// @details Adds default loggers (like CPrintLog or CTraceTask) based on
	///          compile-time configuration flags.
	void init();

	/// Virtual trace method
	/*!
	  \param[in] strError Error message string.
	  \param[in] errCode Error code.
	  \param[in] level Log level (e.g., ESP_LOG_INFO, ESP_LOG_WARN).
	  \param[in] reboot Reboot flag.
	*/
	virtual void trace(const char *strError, int32_t errCode, esp_log_level_t level, bool reboot) override;

	/// Virtual trace method from interrupt.
	/*!
	  \param[in] strError Error message string.
	  \param[in] errCode Error code.
	  \param[in|out] pxHigherPriorityTaskWoken Task switch flag.
	*/
	virtual void traceFromISR(const char *strError, int16_t errCode, BaseType_t *pxHigherPriorityTaskWoken) override;

	/// Virtual data array trace method
	/*!
	  \param[in] strError Error message string.
	  \param[in] data Pointer to the data array.
	  \param[in] size Size of the data array.
	*/
	virtual void trace(const char *strError, uint8_t *data, uint32_t size) override;

	/// Virtual data array trace method
	/*!
	  \param[in] strError Error message string.
	  \param[in] data Pointer to the data array.
	  \param[in] size Size of the data array.
	*/
	virtual void trace(const char *strError, int8_t *data, uint32_t size) override;

	/// Virtual data array trace method
	/*!
	  \param[in] strError Error message string.
	  \param[in] data Pointer to the data array.
	  \param[in] size Size of the data array.
	*/
	virtual void trace(const char *strError, uint16_t *data, uint32_t size) override;

	/// Virtual data array trace method
	/*!
	  \param[in] strError Error message string.
	  \param[in] data Pointer to the data array.
	  \param[in] size Size of the data array.
	*/
	virtual void trace(const char *strError, int16_t *data, uint32_t size) override;

	/// Virtual data array trace method
	/*!
	  \param[in] strError Error message string.
	  \param[in] data Pointer to the data array.
	  \param[in] size Size of the data array.
	*/
	virtual void trace(const char *strError, uint32_t *data, uint32_t size) override;

	/// Virtual data array trace method
	/*!
	  \param[in] strError Error message string.
	  \param[in] data Pointer to the data array.
	  \param[in] size Size of the data array.
	*/
	virtual void trace(const char *strError, int32_t *data, uint32_t size) override;

	/// Print a message
	/*!
	  \param[in] str Message string.
	*/
	virtual void log(const char *str) override;

	/// Reset the time marker
	virtual void startTime() override;

	/// Print the time interval
	/*!
	  \param[in] str Name of the interval.
	  \param[in] n Number for averaging (default 1).
	*/
	virtual void stopTime(const char *str, uint32_t n = 1) override;

	/// Add a tracer to the list
	/*!
	  \param[in] log Tracer object (ITraceLog*).
	*/
	void add(ITraceLog *log);

	/// Remove a tracer from the list
	/*!
	  \param[in] log Tracer object (ITraceLog*) to remove.
	*/
	void remove(ITraceLog *log);

	/// Clear the list of tracers
	void clear();
};

#ifdef CONFIG_DEBUG_CODE
/// Global instance of the list of registered tracers/loggers.
extern CTraceList traceLog;
#endif