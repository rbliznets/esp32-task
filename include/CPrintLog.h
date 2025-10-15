/*!
	\file
	\brief Class for system error logging to the console.
	\authors Bliznets R.A. (r.bliznets@gmail.com)
	\version 1.3.0.0
	\date 10.07.2020
	\details This header file defines the CPrintLog class, which implements the ITraceLog interface.
			 It provides concrete implementations for logging error messages, data arrays, and timing
			 information to the console. It includes formatting for timestamps and integrates with
			 ESP-IDF logging if configured.
*/

#pragma once

#include "ITraceLog.h" // Include the base interface for trace logging

/// Console error message tracing class
/// @details This class inherits from ITraceLog and provides specific implementations
///          for logging to the console (e.g., via std::printf or ESP_LOG).
class CPrintLog : public ITraceLog
{
protected:
	/// Buffer for storing the formatted timestamp header.
	char m_header[32];

	/// Print the time interval since the previous message.
	/// @details Formats the time interval into `m_header` based on the provided `time` and divisor `n`.
	/// @param[in] time The raw time value (likely from a timer function).
	/// @param[in] n A divisor used for averaging or scaling the time unit (default is 1).
	void printHeader(uint64_t time, uint32_t n = 1);

public:
	/// Constructor.
	/// @details Initializes the base class ITraceLog.
	CPrintLog() : ITraceLog() {};

	/// Virtual destructor.
	/// @details Uses default implementation.
	virtual ~CPrintLog() = default;

	/// Virtual trace method for error messages.
	/// @details Logs an error message with an optional error code, log level, and system restart flag.
	/// @param[in] strError Error message string. Can be nullptr.
	/// @param[in] errCode Error code.
	/// @param[in] level ESP-IDF log level (e.g., ESP_LOGE, ESP_LOGW).
	/// @param[in] reboot If true, the system will restart after logging the message.
	void trace(const char *strError, int32_t errCode, esp_log_level_t level, bool reboot) override;

	/// Virtual trace method for uint8_t data arrays.
	/// @details Logs an array of uint8_t values, typically in hexadecimal format.
	/// @param[in] strError Error message string associated with the data. Can be nullptr.
	/// @param[in] data Pointer to the uint8_t array.
	/// @param[in] size Number of elements in the array.
	void trace(const char *strError, uint8_t *data, uint32_t size) override;

	/// Virtual trace method for int8_t data arrays.
	/// @details Logs an array of int8_t values, typically in decimal format.
	/// @param[in] strError Error message string associated with the data. Can be nullptr.
	/// @param[in] data Pointer to the int8_t array.
	/// @param[in] size Number of elements in the array.
	void trace(const char *strError, int8_t *data, uint32_t size) override;

	/// Virtual trace method for uint16_t data arrays.
	/// @details Logs an array of uint16_t values, typically in hexadecimal format.
	/// @param[in] strError Error message string associated with the data. Can be nullptr.
	/// @param[in] data Pointer to the uint16_t array.
	/// @param[in] size Number of elements in the array.
	void trace(const char *strError, uint16_t *data, uint32_t size) override;

	/// Virtual trace method for int16_t data arrays.
	/// @details Logs an array of int16_t values, typically in decimal format.
	/// @param[in] strError Error message string associated with the data. Can be nullptr.
	/// @param[in] data Pointer to the int16_t array.
	/// @param[in] size Number of elements in the array.
	void trace(const char *strError, int16_t *data, uint32_t size) override;

	/// Virtual trace method for uint32_t data arrays.
	/// @details Logs an array of uint32_t values, typically in hexadecimal format.
	/// @param[in] strError Error message string associated with the data. Can be nullptr.
	/// @param[in] data Pointer to the uint32_t array.
	/// @param[in] size Number of elements in the array.
	void trace(const char *strError, uint32_t *data, uint32_t size) override;

	/// Virtual trace method for int32_t data arrays.
	/// @details Logs an array of int32_t values, typically in decimal format.
	/// @param[in] strError Error message string associated with the data. Can be nullptr.
	/// @param[in] data Pointer to the int32_t array.
	/// @param[in] size Number of elements in the array.
	void trace(const char *strError, int32_t *data, uint32_t size) override;

	/// Print a timing interval.
	/// @details Logs a message prefixed with the formatted time interval since the last call,
	///          using the divisor `n` for scaling.
	/// @param[in] str Name or description of the interval.
	/// @param[in] n A divisor used for averaging or scaling the time unit (default is 1).
	void stopTime(const char *str, uint32_t n = 1) override;

	/// Print a simple message.
	/// @details Logs a plain string message to the console.
	/// @param[in] str The message string to print. Can be nullptr.
	inline void log(const char *str) override
	{
		if (str != nullptr)
			std::printf(str); // Print the message if it's not nullptr
		std::printf("\n");	  // Always print a newline
	};
};