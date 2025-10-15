/*!
	\file
	\brief Interface class for the system error log.
	\authors Bliznets R.A.(r.bliznets@gmail.com)
	\version 1.3.0.0
	\date 10.07.2020
	\details This header file defines the ITraceLog abstract base class.
			 It specifies the interface that concrete logger classes must implement
			 to provide logging functionality for errors, data, and timing information.
			 It includes methods for standard logging, interrupt-safe logging,
			 data array logging, and time measurement.
*/

#pragma once

#include <cstdio>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "esp_timer.h"
#include "esp_log.h"

/// Interface class for error message tracing.
/// @details This abstract class defines the contract for logging implementations.
///          Concrete classes like CPrintLog or CTraceTask inherit from this
///          and provide specific ways to output log information.
class ITraceLog
{
protected:
	/// Time of the last message (used for calculating intervals).
	int64_t mTime;

	/// Get the current timer value.
	/// @details Calculates the time elapsed since the last call (or since startTime())
	///          using esp_timer_get_time(). Optionally updates the internal reference time `mTime`.
	/// @param[in] refresh Flag indicating whether to update the reference time `mTime` (default true).
	/// @return The elapsed time in microseconds since the last reference point.
	int64_t getTimer(bool refresh = true)
	{
		int64_t res = 0;
		int64_t time = esp_timer_get_time(); // Get current time in microseconds
		res = time - mTime;					 // Calculate elapsed time
		if (refresh)
			mTime = time; // Update the reference time
		return res;
	};

public:
	/// Constructor.
	/// @details Initializes the internal time marker `mTime` to 0.
	ITraceLog() : mTime{0} {};

	/// Virtual destructor.
	/// @details Uses default implementation.
	virtual ~ITraceLog() = default;

	/// Virtual trace method for error messages.
	/// @details This pure virtual function must be implemented by derived classes.
	/// @param[in] strError Error message string. Can be nullptr.
	/// @param[in] errCode Error code.
	/// @param[in] level ESP-IDF log level (e.g., ESP_LOGE, ESP_LOGW).
	/// @param[in] reboot If true, the system should restart after logging.
	virtual void trace(const char *strError, int32_t errCode, esp_log_level_t level, bool reboot) = 0;

	/// Virtual trace method from interrupt.
	/// @details Provides a default empty implementation. Can be overridden by derived classes
	///          to handle logging from ISR context. Marked with IRAM_ATTR for faster execution.
	/// @param[in] strError Error message string. Can be nullptr.
	/// @param[in] errCode Error code.
	/// @param[out] pxHigherPriorityTaskWoken Set to pdTRUE if a higher priority task should be woken.
	virtual void IRAM_ATTR traceFromISR(const char *strError, int16_t errCode, BaseType_t *pxHigherPriorityTaskWoken) {};

	/// Virtual method for logging uint8_t data arrays.
	/// @details This pure virtual function must be implemented by derived classes.
	/// @param[in] strError Error message string associated with the data. Can be nullptr.
	/// @param[in] data Pointer to the uint8_t data array.
	/// @param[in] size Size of the data array.
	virtual void trace(const char *strError, uint8_t *data, uint32_t size) = 0;

	/// Virtual method for logging int8_t data arrays.
	/// @details Provides a default implementation that casts the int8_t array to uint8_t
	///          and calls the uint8_t version.
	/// @param[in] strError Error message string associated with the data. Can be nullptr.
	/// @param[in] data Pointer to the int8_t data array.
	/// @param[in] size Size of the data array.
	virtual void trace(const char *strError, int8_t *data, uint32_t size) { trace(strError, (uint8_t *)data, size); };

	/// Virtual method for logging uint16_t data arrays.
	/// @details This pure virtual function must be implemented by derived classes.
	/// @param[in] strError Error message string associated with the data. Can be nullptr.
	/// @param[in] data Pointer to the uint16_t data array.
	/// @param[in] size Size of the data array.
	virtual void trace(const char *strError, uint16_t *data, uint32_t size) = 0;

	/// Virtual method for logging int16_t data arrays.
	/// @details Provides a default implementation that casts the int16_t array to uint16_t
	///          and calls the uint16_t version.
	/// @param[in] strError Error message string associated with the data. Can be nullptr.
	/// @param[in] data Pointer to the int16_t data array.
	/// @param[in] size Size of the data array.
	virtual void trace(const char *strError, int16_t *data, uint32_t size) { trace(strError, (uint16_t *)data, size); };

	/// Virtual method for logging uint32_t data arrays.
	/// @details This pure virtual function must be implemented by derived classes.
	/// @param[in] strError Error message string associated with the data. Can be nullptr.
	/// @param[in] data Pointer to the uint32_t data array.
	/// @param[in] size Size of the data array.
	virtual void trace(const char *strError, uint32_t *data, uint32_t size) = 0;

	/// Virtual method for logging int32_t data arrays.
	/// @details Provides a default implementation that casts the int32_t array to uint32_t
	///          and calls the uint32_t version.
	/// @param[in] strError Error message string associated with the data. Can be nullptr.
	/// @param[in] data Pointer to the int32_t data array.
	/// @param[in] size Size of the data array.
	virtual void trace(const char *strError, int32_t *data, uint32_t size) { trace(strError, (uint32_t *)data, size); };

	/// Print a message.
	/// @details Provides a default implementation that calls stopTime with the message string.
	/// @param[in] str Message string to print. Can be nullptr.
	virtual void log(const char *str)
	{
		stopTime(str);
	};

	/// Reset the time marker.
	/// @details Calls getTimer(true) to set the internal reference time `mTime`
	///          to the current time, effectively starting a new time interval measurement.
	inline virtual void startTime() { getTimer(); };

	/// Print the time interval.
	/// @details Provides a default implementation that calls the main trace method
	///          with the message string, the divisor `n`, and `ESP_LOG_INFO` level.
	/// @param[in] str Name or description of the interval. Can be nullptr.
	/// @param[in] n Number used for averaging or scaling the time unit (default 1).
	virtual void stopTime(const char *str, uint32_t n = 1) { trace(str, n, ESP_LOG_INFO, false); };
};