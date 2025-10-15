/*!
	\file
	\brief Class for outputting debug information in JSON format.
	\authors Bliznets R.A.(r.bliznets@gmail.com)
	\version 1.0.0.0
	\date 02.05.2024

	\details One object per application.
	\details It is necessary to avoid blocking the task being debugged.
			 This header file defines the CTraceJsonTask class, which inherits from
			 CTraceTask. It overrides the data formatting methods to serialize trace
			 information (messages, errors, data arrays, timing) into JSON strings
			 instead of plain text or other formats.
*/

#pragma once

#include "CTraceTask.h" // Include the base trace task class header
#include <string>		// Include std::string for JSON output

/// Class for outputting debug information in JSON format as a FreeRTOS task.
/// @details This class implements the Singleton pattern (Instance() method).
///          It inherits from CTraceTask and overrides the protected `print*` methods
///          to format the received trace data into JSON strings, which are stored
///          in the `mAnswer` member variable. This allows the trace output to be
///          consumed by systems expecting JSON format, potentially for remote
///          logging or parsing.
class CTraceJsonTask : public CTraceTask
{
protected:
	/// String to hold the generated JSON output.
	std::string mAnswer;

	/// Print the time interval since the previous message in JSON format.
	/// @details Overrides the base class method to start formatting the JSON string
	///          with the log object and the time field.
	/// @param[in] time Raw time value (likely from a timer function).
	/// @param[in] n A divisor used for averaging or scaling the time unit (default is 1).
	void printHeader(uint64_t time, uint32_t n = 1) override;

	/// Print an error message string in JSON format.
	/// @details Overrides the base class method to format the trace data
	///          (time, error code, level, message) into a JSON object.
	/// @param[in] data Pointer to the message body for MSG_TRACE_STRING or MSG_TRACE_STRING_REBOOT.
	void printString(char *data) override;

	/// Print a simple message string in JSON format.
	/// @details Overrides the base class method to format a plain string
	///          into a minimal JSON log object.
	/// @param[in] str Pointer to the string message.
	void printS(char *str) override;

	/// Print a time interval message in JSON format.
	/// @details Overrides the base class method to format the timing data
	///          (time, divisor, description) into a JSON object.
	/// @param[in] data Pointer to the message body for MSG_STOP_TIME.
	void printStop(char *data) override;

	/// Print a uint8_t data array in JSON format (as a hexadecimal string).
	/// @details Overrides the base class method to format the trace data
	///          (time, message, data array) into a JSON object with the data as a hex string.
	/// @param[in] data Pointer to the message body for MSG_TRACE_UINT8.
	void printData8h(char *data) override;

	/// Print a uint8_t data array in JSON format (as a hexadecimal string) - version 2 (indirect pointer).
	/// @details Similar to printData8h, but handles data where the array pointer is stored within the message body.
	/// @param[in] data Pointer to the message body for MSG_TRACE2_UINT8.
	void printData8h_2(char *data) override;

	/// Print a uint16_t data array in JSON format (as a hexadecimal string).
	/// @details Overrides the base class method to format the trace data
	///          (time, message, data array) into a JSON object with the data as a hex string.
	/// @param[in] data Pointer to the message body for MSG_TRACE_UINT16.
	void printData16h(char *data) override;

	/// Print a uint16_t data array in JSON format (as a hexadecimal string) - version 2 (indirect pointer).
	/// @details Similar to printData16h, but handles data where the array pointer is stored within the message body.
	/// @param[in] data Pointer to the message body for MSG_TRACE2_UINT16.
	void printData16h_2(char *data) override;

	/// Print a uint32_t data array in JSON format (as a hexadecimal string).
	/// @details Overrides the base class method to format the trace data
	///          (time, message, data array) into a JSON object with the data as a hex string.
	/// @param[in] data Pointer to the message body for MSG_TRACE_UINT32.
	void printData32h(char *data) override;

	/// Print a uint32_t data array in JSON format (as a hexadecimal string) - version 2 (indirect pointer).
	/// @details Similar to printData32h, but handles data where the array pointer is stored within the message body.
	/// @param[in] data Pointer to the message body for MSG_TRACE2_UINT32.
	void printData32h_2(char *data) override;

	/// Print an int8_t data array in JSON format (as a decimal array).
	/// @details Overrides the base class method to format the trace data
	///          (time, message, data array) into a JSON object with the data as a JSON array of decimals.
	/// @param[in] data Pointer to the message body for MSG_TRACE_INT8.
	void printData8(char *data) override;

	/// Print an int8_t data array in JSON format (as a decimal array) - version 2 (indirect pointer).
	/// @details Similar to printData8, but handles data where the array pointer is stored within the message body.
	/// @param[in] data Pointer to the message body for MSG_TRACE2_INT8.
	void printData8_2(char *data) override;

	/// Print an int16_t data array in JSON format (as a decimal array).
	/// @details Overrides the base class method to format the trace data
	///          (time, message, data array) into a JSON object with the data as a JSON array of decimals.
	/// @param[in] data Pointer to the message body for MSG_TRACE_INT16.
	void printData16(char *data) override;

	/// Print an int16_t data array in JSON format (as a decimal array) - version 2 (indirect pointer).
	/// @details Similar to printData16, but handles data where the array pointer is stored within the message body.
	/// @param[in] data Pointer to the message body for MSG_TRACE2_INT16.
	void printData16_2(char *data) override;

	/// Print an int32_t data array in JSON format (as a decimal array).
	/// @details Overrides the base class method to format the trace data
	///          (time, message, data array) into a JSON object with the data as a JSON array of decimals.
	/// @param[in] data Pointer to the message body for MSG_TRACE_INT32.
	void printData32(char *data) override;

	/// Print an int32_t data array in JSON format (as a decimal array) - version 2 (indirect pointer).
	/// @details Similar to printData32, but handles data where the array pointer is stored within the message body.
	/// @param[in] data Pointer to the message body for MSG_TRACE2_INT32.
	void printData32_2(char *data) override;

	/// Virtual destructor.
	/// @details Provides a virtual destructor with a default implementation.
	///          This is good practice for base classes with virtual functions.
	virtual ~CTraceJsonTask() {};

public:
	/// Get the single instance of the CTraceJsonTask class (Singleton).
	/// @details Uses the Meyer's Singleton pattern. The static instance is created
	///          on the first call to this function.
	/// @return Pointer to the single CTraceJsonTask instance.
	static CTraceJsonTask *Instance()
	{
		static CTraceJsonTask theSingleInstance;
		return &theSingleInstance;
	}
};