/*!
	\file
	\brief Class for outputting debug information.
	\authors Bliznets R.A. (r.bliznets@gmail.com)
	\version 1.3.1.0
	\date 15.09.2022

	\details One object per application.
	\details It is necessary to avoid blocking the task being debugged.
			 This file implements the CTraceTask class, which runs as a dedicated
			 FreeRTOS task to handle logging messages sent from other tasks or ISRs.
			 This prevents logging operations from blocking the originating tasks.
*/

#include "CTraceTask.h"
#include <cstring>
#include "esp_system.h"
#include "CTrace.h"
#include "esp_log.h"

// Define AUTO_TIMER based on configuration or default to false
#ifdef CONFIG_TRACE_AUTO_RESET
#define AUTO_TIMER CONFIG_TRACE_AUTO_RESET
#else
#define AUTO_TIMER false
#endif

// Formats the time header string based on the elapsed time and divisor 'n'.
// The output format depends on the CONFIG_TRACE_USEC setting and the magnitude of 'res'.
void CTraceTask::printHeader(uint64_t time, uint32_t n)
{
	uint64_t res = time / n; // Calculate scaled time

#if (CONFIG_TRACE_USEC == 1)
	// If configured for microseconds, format directly
	std::snprintf(m_header, sizeof(m_header), "(+%liusec)", (long)res);
#else
	// Otherwise, choose unit based on magnitude
	if (res >= 10000000) // If time >= 10 seconds (assuming microseconds)
	{
		std::snprintf(m_header, sizeof(m_header), "(+%lisec)", (long)(res / 1000000)); // Format in seconds
	}
	else
	{
		if (res < 10000) // If time < 10 milliseconds (assuming microseconds)
		{
			if (res < 10) // If time < 10 microseconds
			{
				// Calculate more precise value for nanoseconds
				double f = time / (double)n;
				std::snprintf(m_header, sizeof(m_header), "(+%linsec)", (long)(f * 1000)); // Format in nanoseconds
			}
			else
			{
				// Format in microseconds
				std::snprintf(m_header, sizeof(m_header), "(+%liusec)", (long)res);
			}
		}
		else // If 10ms <= time < 10s (assuming microseconds)
		{
			// Format in milliseconds
			std::snprintf(m_header, sizeof(m_header), "(+%limsec)", (long)(res / 1000));
		}
	}
#endif
}

// Processes a single log message received from the task's queue.
// Dispatches the message to the appropriate print method based on its ID.
// Returns true if the message was handled, false otherwise.
bool CTraceTask::logMessage(STaskMessage &msg)
{
	switch (msg.msgID) // Check the message ID
	{
	case MSG_TRACE_ISR_STRING: // Message from ISR
		printIsrString((char *)msg.msgBody, (int16_t)msg.shortParam);
		return true;
	case MSG_TRACE_STRING: // Standard trace message
		printString((char *)msg.msgBody);
		break;
	case MSG_STOP_TIME: // Timing message
		printStop((char *)msg.msgBody);
		break;
	case MSG_PRINT_STRING: // Simple print message
		printS((char *)msg.msgBody);
		break;
	case MSG_TRACE_STRING_REBOOT: // Trace message followed by system restart
		printString((char *)msg.msgBody);
		vTaskDelay(pdMS_TO_TICKS(150)); // Delay before restart
		esp_restart();
		break;
	case MSG_TRACE_UINT8: // uint8_t data array (hex)
		printData8h((char *)msg.msgBody);
		break;
	case MSG_TRACE2_UINT8: // uint8_t data array (hex) - version 2 (indirect pointer)
		printData8h_2((char *)msg.msgBody);
		break;
	case MSG_TRACE_INT8: // int8_t data array (decimal)
		printData8((char *)msg.msgBody);
		break;
	case MSG_TRACE2_INT8: // int8_t data array (decimal) - version 2 (indirect pointer)
		printData8_2((char *)msg.msgBody);
		break;
	case MSG_TRACE_UINT16: // uint16_t data array (hex)
		printData16h((char *)msg.msgBody);
		break;
	case MSG_TRACE2_UINT16: // uint16_t data array (hex) - version 2 (indirect pointer)
		printData16h_2((char *)msg.msgBody);
		break;
	case MSG_TRACE_INT16: // int16_t data array (decimal)
		printData16((char *)msg.msgBody);
		break;
	case MSG_TRACE2_INT16: // int16_t data array (decimal) - version 2 (indirect pointer)
		printData16_2((char *)msg.msgBody);
		break;
	case MSG_TRACE_UINT32: // uint32_t data array (hex)
		printData32h((char *)msg.msgBody);
		break;
	case MSG_TRACE2_UINT32: // uint32_t data array (hex) - version 2 (indirect pointer)
		printData32h_2((char *)msg.msgBody);
		break;
	case MSG_TRACE_INT32: // int32_t data array (decimal)
		printData32((char *)msg.msgBody);
		break;
	case MSG_TRACE2_INT32: // int32_t data array (decimal) - version 2 (indirect pointer)
		printData32_2((char *)msg.msgBody);
		break;
	default:
		return false; // Unknown message ID
	}
	vPortFree(msg.msgBody); // Free the dynamically allocated message body memory
	return true;			// Message handled successfully
}

// Main execution loop for the CTraceTask FreeRTOS task.
// Continuously receives messages from its queue and processes them.
void CTraceTask::run()
{
#ifndef CONFIG_FREERTOS_CHECK_STACKOVERFLOW_NONE
	// Get initial stack high water mark for monitoring
	UBaseType_t m1 = uxTaskGetStackHighWaterMark2(nullptr);
#endif
	STaskMessage msg; // Buffer for receiving messages

	// Infinite loop to process messages
	while (getMessage(&msg, portMAX_DELAY)) // Block indefinitely until a message is received
	{
		if (!logMessage(msg)) // Process the received message
		{
			// Log a warning if the message ID was unknown
			ESP_LOGW("*", "CTraceTask unknown message %d", msg.msgID);
		}
		vTaskDelay(pdMS_TO_TICKS(2)); // Small delay to prevent excessive CPU usage

#ifndef CONFIG_FREERTOS_CHECK_STACKOVERFLOW_NONE
		// Monitor stack usage
		UBaseType_t m2 = uxTaskGetStackHighWaterMark2(nullptr);
		if (m2 != m1) // If the high water mark changed
		{
			m1 = m2;									  // Update the stored mark
			if (m1 > 100)								  // If more than 100 words are free
				ESP_LOGI("*", "free trace stack %d", m2); // Log as info
			else										  // If 100 or fewer words are free
				ESP_LOGW("*", "free trace stack %d", m2); // Log as warning
		}
#endif
	}
}

// Prints a uint32_t data array trace message (version 2 - indirect pointer).
// Interprets the raw data buffer, extracts time, size, pointer to data, and error message string.
// Outputs the information formatted as decimal numbers.
void CTraceTask::printData32_2(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;	   // Time
	uint32_t *size = (uint32_t *)&data[8]; // Size of the data array
	// Pointer to the data array is stored as a 32-bit address within the buffer (little-endian assumed)
	uint32_t *pdata = (uint32_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4]; // Error message string comes after the address

	// Format the header part
	printHeader(*res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging
	ESP_LOGI(m_header, "32 %s(%ld)", strError, (*size)); // Log size and description
	ESP_LOG_BUFFER_HEX(strError, pdata, (*size) * 4);	 // Log data as hex (note: uses description as tag)
#else
	// Use standard printf
	std::printf(m_header);					 // Print the formatted time header
	std::printf("%s %ld:", strError, *size); // Print description and size
	std::printf(" %d", (int)pdata[0]);		 // Print the first element as decimal
	for (int16_t i = 1; i < *size; i++)		 // Print remaining elements
	{
		std::printf(",%d", (int)pdata[i]);
	}
	std::printf("\n"); // Print newline
#endif
}

// Prints a uint32_t data array trace message (version 2 - indirect pointer) in hexadecimal format.
// Interprets the raw data buffer, extracts time, size, pointer to data, and error message string.
// Outputs the information formatted as hexadecimal numbers.
void CTraceTask::printData32h_2(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;	   // Time
	uint32_t *size = (uint32_t *)&data[8]; // Size of the data array
	// Pointer to the data array is stored as a 32-bit address within the buffer (little-endian assumed)
	uint32_t *pdata = (uint32_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4]; // Error message string comes after the address

	// Format the header part
	printHeader(*res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging
	ESP_LOGI(m_header, "32 %s(%ld)", strError, (*size)); // Log size and description
	ESP_LOG_BUFFER_HEX(strError, pdata, (*size) * 4);	 // Log data as hex (note: uses description as tag)
#else
	// Use standard printf
	std::printf(m_header);					 // Print the formatted time header
	std::printf("%s %ld:", strError, *size); // Print description and size
	std::printf(" 0x%08x", (int)pdata[0]);	 // Print the first element as 8-digit hex
	for (int16_t i = 1; i < *size; i++)		 // Print remaining elements
	{
		std::printf(",0x%08x", (int)pdata[i]);
	}
	std::printf("\n"); // Print newline
#endif
}

// Prints a uint32_t data array trace message in hexadecimal format.
// Interprets the raw data buffer, extracts time, size, data array, and error message string.
// Outputs the information formatted as hexadecimal numbers.
void CTraceTask::printData32h(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;							  // Time
	uint32_t *size = (uint32_t *)&data[8];						  // Size of the data array
	uint32_t *pdata = (uint32_t *)&data[8 + 4];					  // Pointer to the data array within the buffer
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint32_t))]; // Error message string comes after the data array

	// Format the header part
	printHeader(*res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging
	ESP_LOGI(m_header, "32 %s(%ld)", strError, (*size)); // Log size and description
	ESP_LOG_BUFFER_HEX(strError, pdata, (*size) * 4);	 // Log data as hex (note: uses description as tag)
#else
	// Use standard printf
	std::printf(m_header);					 // Print the formatted time header
	std::printf("%s %ld:", strError, *size); // Print description and size
	std::printf(" 0x%08x", (int)pdata[0]);	 // Print the first element as 8-digit hex
	for (int16_t i = 1; i < *size; i++)		 // Print remaining elements
	{
		std::printf(",0x%08x", (int)pdata[i]);
	}
	std::printf("\n"); // Print newline
#endif
}

// Prints an int32_t data array trace message in decimal format.
// Interprets the raw data buffer, extracts time, size, data array, and error message string.
// Outputs the information formatted as decimal numbers.
void CTraceTask::printData32(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;							  // Time
	uint32_t *size = (uint32_t *)&data[8];						  // Size of the data array
	int32_t *pdata = (int32_t *)&data[8 + 4];					  // Pointer to the data array within the buffer
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint32_t))]; // Error message string comes after the data array

	// Format the header part
	printHeader(*res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging
	ESP_LOGI(m_header, "32 %s(%ld)", strError, (*size)); // Log size and description
	ESP_LOG_BUFFER_HEX(strError, pdata, (*size) * 4);	 // Log data as hex (note: uses description as tag)
#else
	// Use standard printf
	std::printf(m_header);					 // Print the formatted time header
	std::printf("%s %ld:", strError, *size); // Print description and size
	std::printf(" %d", (int)pdata[0]);		 // Print the first element as decimal
	for (int16_t i = 1; i < *size; i++)		 // Print remaining elements
	{
		std::printf(",%d", (int)pdata[i]);
	}
	std::printf("\n"); // Print newline
#endif
}

// Prints a uint16_t data array trace message in hexadecimal format.
// Interprets the raw data buffer, extracts time, size, data array, and error message string.
// Outputs the information formatted as hexadecimal numbers.
void CTraceTask::printData16h(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;							  // Time
	uint32_t *size = (uint32_t *)&data[8];						  // Size of the data array
	uint16_t *pdata = (uint16_t *)&data[8 + 4];					  // Pointer to the data array within the buffer
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint16_t))]; // Error message string comes after the data array

	// Format the header part
	printHeader(*res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging
	ESP_LOGI(m_header, "16 %s(%ld)", strError, (*size)); // Log size and description
	ESP_LOG_BUFFER_HEX(strError, pdata, (*size) * 2);	 // Log data as hex (note: uses description as tag)
#else
	// Use standard printf
	std::printf(m_header);					 // Print the formatted time header
	std::printf("%s %ld:", strError, *size); // Print description and size
	std::printf(" 0x%04x", pdata[0]);		 // Print the first element as 4-digit hex
	for (int16_t i = 1; i < *size; i++)		 // Print remaining elements
	{
		std::printf(",0x%04x", pdata[i]);
	}
	std::printf("\n"); // Print newline
#endif
}

// Prints a uint16_t data array trace message (version 2 - indirect pointer) in hexadecimal format.
// Interprets the raw data buffer, extracts time, size, pointer to data, and error message string.
// Outputs the information formatted as hexadecimal numbers.
void CTraceTask::printData16h_2(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;	   // Time
	uint32_t *size = (uint32_t *)&data[8]; // Size of the data array
	// Pointer to the data array is stored as a 32-bit address within the buffer (little-endian assumed)
	uint16_t *pdata = (uint16_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4]; // Error message string comes after the address

	// Format the header part
	printHeader(*res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging
	ESP_LOGI(m_header, "16 %s(%ld)", strError, (*size)); // Log size and description
	ESP_LOG_BUFFER_HEX(strError, pdata, (*size) * 2);	 // Log data as hex (note: uses description as tag)
#else
	// Use standard printf
	std::printf(m_header);					 // Print the formatted time header
	std::printf("%s %ld:", strError, *size); // Print description and size
	std::printf(" 0x%04x", pdata[0]);		 // Print the first element as 4-digit hex
	for (int16_t i = 1; i < *size; i++)		 // Print remaining elements
	{
		std::printf(",0x%04x", pdata[i]);
	}
	std::printf("\n"); // Print newline
#endif
}

// Prints an int16_t data array trace message in decimal format.
// Interprets the raw data buffer, extracts time, size, data array, and error message string.
// Outputs the information formatted as decimal numbers.
void CTraceTask::printData16(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;							  // Time
	uint32_t *size = (uint32_t *)&data[8];						  // Size of the data array
	int16_t *pdata = (int16_t *)&data[8 + 4];					  // Pointer to the data array within the buffer
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint16_t))]; // Error message string comes after the data array

	// Format the header part
	printHeader(*res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging
	ESP_LOGI(m_header, "16 %s(%ld)", strError, (*size)); // Log size and description
	ESP_LOG_BUFFER_HEX(strError, pdata, (*size) * 2);	 // Log data as hex (note: uses description as tag)
#else
	// Use standard printf
	std::printf(m_header);					 // Print the formatted time header
	std::printf("%s %ld:", strError, *size); // Print description and size
	std::printf(" %d", pdata[0]);			 // Print the first element as decimal
	for (int16_t i = 1; i < *size; i++)		 // Print remaining elements
	{
		std::printf(",%d", pdata[i]);
	}
	std::printf("\n"); // Print newline
#endif
}

// Prints an int16_t data array trace message (version 2 - indirect pointer) in decimal format.
// Interprets the raw data buffer, extracts time, size, pointer to data, and error message string.
// Outputs the information formatted as decimal numbers.
void CTraceTask::printData16_2(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;	   // Time
	uint32_t *size = (uint32_t *)&data[8]; // Size of the data array
	// Pointer to the data array is stored as a 32-bit address within the buffer (little-endian assumed)
	int16_t *pdata = (int16_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4]; // Error message string comes after the address

	// Format the header part
	printHeader(*res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging
	ESP_LOGI(m_header, "16 %s(%ld)", strError, (*size)); // Log size and description
	ESP_LOG_BUFFER_HEX(strError, pdata, (*size) * 2);	 // Log data as hex (note: uses description as tag)
#else
	// Use standard printf
	std::printf(m_header);					 // Print the formatted time header
	std::printf("%s %ld:", strError, *size); // Print description and size
	std::printf(" %d", pdata[0]);			 // Print the first element as decimal
	for (int16_t i = 1; i < *size; i++)		 // Print remaining elements
	{
		std::printf(",%d", pdata[i]);
	}
	std::printf("\n"); // Print newline
#endif
}

// Prints a uint8_t data array trace message in hexadecimal format.
// Interprets the raw data buffer, extracts time, size, data array, and error message string.
// Outputs the information formatted as hexadecimal numbers.
void CTraceTask::printData8h(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;							 // Time
	uint32_t *size = (uint32_t *)&data[8];						 // Size of the data array
	uint8_t *pdata = (uint8_t *)&data[8 + 4];					 // Pointer to the data array within the buffer
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint8_t))]; // Error message string comes after the data array

	// Format the header part
	printHeader(*res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging
	ESP_LOGI(m_header, "8 %s(%ld)", strError, (*size)); // Log size and description
	ESP_LOG_BUFFER_HEX(strError, pdata, (*size));		// Log data as hex (note: uses description as tag)
#else
	// Use standard printf
	std::printf(m_header);					 // Print the formatted time header
	std::printf("%s %ld:", strError, *size); // Print description and size
	std::printf(" 0x%02x", pdata[0]);		 // Print the first element as 2-digit hex
	for (int16_t i = 1; i < *size; i++)		 // Print remaining elements
	{
		std::printf(",0x%02x", pdata[i]);
	}
	std::printf("\n"); // Print newline
#endif
}

// Prints a uint8_t data array trace message (version 2 - indirect pointer) in hexadecimal format.
// Interprets the raw data buffer, extracts time, size, pointer to data, and error message string.
// Outputs the information formatted as hexadecimal numbers.
void CTraceTask::printData8h_2(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;	   // Time
	uint32_t *size = (uint32_t *)&data[8]; // Size of the data array
	// Pointer to the data array is stored as a 32-bit address within the buffer (little-endian assumed)
	uint8_t *pdata = (uint8_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4]; // Error message string comes after the address

	// Format the header part
	printHeader(*res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging
	ESP_LOGI(m_header, "8 %s(%ld)", strError, (*size)); // Log size and description
	ESP_LOG_BUFFER_HEX(strError, pdata, (*size));		// Log data as hex (note: uses description as tag)
#else
	// Use standard printf
	std::printf(m_header);					 // Print the formatted time header
	std::printf("%s %ld:", strError, *size); // Print description and size
	std::printf(" 0x%02x", pdata[0]);		 // Print the first element as 2-digit hex
	for (int16_t i = 1; i < *size; i++)		 // Print remaining elements
	{
		std::printf(",0x%02x", pdata[i]);
	}
	std::printf("\n"); // Print newline
#endif
}

// Prints an int8_t data array trace message in decimal format.
// Interprets the raw data buffer, extracts time, size, data array, and error message string.
// Outputs the information formatted as decimal numbers.
void CTraceTask::printData8(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;							 // Time
	uint32_t *size = (uint32_t *)&data[8];						 // Size of the data array
	int8_t *pdata = (int8_t *)&data[8 + 4];						 // Pointer to the data array within the buffer
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint8_t))]; // Error message string comes after the data array

	// Format the header part
	printHeader(*res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging
	ESP_LOGI(m_header, "8 %s(%ld)", strError, (*size)); // Log size and description
	ESP_LOG_BUFFER_HEX(strError, pdata, (*size));		// Log data as hex (note: uses description as tag)
#else
	// Use standard printf
	std::printf(m_header);					 // Print the formatted time header
	std::printf("%s %ld:", strError, *size); // Print description and size
	std::printf(" %d", pdata[0]);			 // Print the first element as decimal
	for (int16_t i = 1; i < *size; i++)		 // Print remaining elements
	{
		std::printf(",%d", pdata[i]);
	}
	std::printf("\n"); // Print newline
#endif
}

// Prints an int8_t data array trace message (version 2 - indirect pointer) in decimal format.
// Interprets the raw data buffer, extracts time, size, pointer to data, and error message string.
// Outputs the information formatted as decimal numbers.
void CTraceTask::printData8_2(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;	   // Time
	uint32_t *size = (uint32_t *)&data[8]; // Size of the data array
	// Pointer to the data array is stored as a 32-bit address within the buffer (little-endian assumed)
	uint8_t *pdata = (uint8_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4]; // Error message string comes after the address

	// Format the header part
	printHeader(*res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging
	ESP_LOGI(m_header, "8 %s(%ld)", strError, (*size)); // Log size and description
	ESP_LOG_BUFFER_HEX(strError, pdata, (*size));		// Log data as hex (note: uses description as tag)
#else
	// Use standard printf
	std::printf(m_header);					 // Print the formatted time header
	std::printf("%s %ld:", strError, *size); // Print description and size
	std::printf(" %d", pdata[0]);			 // Print the first element as decimal
	for (int16_t i = 1; i < *size; i++)		 // Print remaining elements
	{
		std::printf(",%d", pdata[i]);
	}
	std::printf("\n"); // Print newline
#endif
}

// Prints an error message received from an Interrupt Service Routine (ISR).
// This method is designed for quick output from ISRs, often using printf directly.
void CTraceTask::printIsrString(char *strError, int16_t errCode)
{
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging with "ISR" tag
	ESP_LOGE("ISR", "%d:%s", errCode, strError);
#else
	// Use standard printf
	std::printf("%d:%s\n", errCode, strError);
#endif
}

// Prints a standard trace message string.
// Interprets the raw data buffer, extracts time, error code, level, and message string.
void CTraceTask::printString(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;		// Time
	int32_t *errCode = (int32_t *)&data[8]; // Error code
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	esp_log_level_t level = (esp_log_level_t)data[12]; // Log level (if using ESP_LOG)
#endif
	char *strError = &data[13]; // Error message string

	// Format the header part
	printHeader(*res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging with the formatted header as the tag
	ESP_LOG_LEVEL(level, m_header, "%ld:%s", (*errCode), strError);
#else
	// Use standard printf
	std::printf(m_header);								 // Print the formatted time header
	std::printf(": %d:%s\n", (int)(*errCode), strError); // Print error code and message
#endif
}

// Prints a simple string message.
void CTraceTask::printS(char *str)
{
#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging with "*" tag
	ESP_LOGI("*", "%s", str);
#else
	// Use standard printf
	std::printf("%s\n", str);
#endif
}

// Prints a timing stop message.
// Interprets the raw data buffer, extracts time, divisor, and description string.
void CTraceTask::printStop(char *data)
{
	// Interpret the raw data buffer
	uint64_t *x = (uint64_t *)data;	  // Time
	int32_t *n = (int32_t *)&data[8]; // Divisor for averaging
	char *str = &data[12];			  // Description string

	// Format the header part using time and divisor
	printHeader(*x, *n);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
	// Use ESP-IDF logging with the formatted header as the tag
	ESP_LOGI(m_header, "%s", str);
#else
	// Use standard printf
	std::printf(m_header);	   // Print the formatted time header
	std::printf(" %s\n", str); // Print the description
#endif
}

// Sends a trace message (error code and string) to the task queue for processing.
void CTraceTask::trace(const char *strError, int32_t errCode, esp_log_level_t level, bool reboot)
{
	STaskMessage msg; // Message structure to send

	// Get the current time, protected by a critical section
	taskENTER_CRITICAL(&mMut);
	uint64_t tm = getTimer(AUTO_TIMER);
	taskEXIT_CRITICAL(&mMut);

	// Check if the error code is not a special ignore value
	if (errCode != 0x7fffffff)
	{
		// Calculate the required buffer size for the message body
		int ln = 8 + 4 + 1 + 1;	 // Time (8) + Error Code (4) + Level (1) + Null terminator (1)
		if (strError != nullptr) // Add length of the error string if present
		{
			ln += std::strlen(strError);
		}

		char *str;
		// Allocate memory for the message body and set the message ID based on reboot flag
		if (reboot)
			str = (char *)allocNewMsg(&msg, MSG_TRACE_STRING_REBOOT, ln);
		else
			str = (char *)allocNewMsg(&msg, MSG_TRACE_STRING, ln);

		// Copy time, error code, level, and error string into the allocated buffer
		std::memcpy(str, &tm, 8);		   // Copy time
		std::memcpy(&str[8], &errCode, 4); // Copy error code
		str[12] = (uint8_t)level;		   // Copy log level
		if (strError != nullptr)		   // Copy error message string if provided
		{
			std::strcpy(&str[13], strError);
		}
		else
		{
			str[ln - 1] = 0; // Ensure null termination if no string
		}

		// Send the message to the task queue
		sendMessage(&msg, 0, true); // Timeout 0, free memory on failure
	}
}

// Sends a trace message from an Interrupt Service Routine (ISR) to the task queue.
// This method is marked with IRAM_ATTR for faster execution from ISR context.
void IRAM_ATTR CTraceTask::traceFromISR(const char *strError, int16_t errCode, BaseType_t *pxHigherPriorityTaskWoken)
{
	STaskMessage msg; // Message structure to send
	// Set message ID for ISR trace
	msg.msgID = MSG_TRACE_ISR_STRING;
	// Store error code in the shortParam field
	msg.shortParam = (uint16_t)errCode;
	// Store pointer to the error string in the msgBody field (be cautious with ISR pointers!)
	msg.msgBody = (void *)strError;

	// Send the message to the front of the queue from ISR context
	sendMessageFrontFromISR(&msg, pxHigherPriorityTaskWoken);
}

// Sends a data array trace message (version 1 - inline data) to the task queue.
void CTraceTask::traceData(const char *strError, void *data, uint32_t size, uint16_t tp)
{
	STaskMessage msg; // Message structure to send

	// Get the current time, protected by a critical section
	taskENTER_CRITICAL(&mMut);
	uint64_t tm = getTimer(AUTO_TIMER);
	taskEXIT_CRITICAL(&mMut);

	// Determine the size of each data element based on the message type
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
		break; // Default to uint8_t size
	}

	// Calculate the required buffer size for the message body
	// Time (8) + Size (4) + Data array (size * sz) + Null terminator (1)
	int ln = 8 + 4 + (size * sz) + 1;
	if (strError != nullptr) // Add length of the error string if present
	{
		ln += std::strlen(strError);
	}

	// Allocate memory for the message body and set the message ID
	char *str = (char *)allocNewMsg(&msg, tp, ln);

	// Copy time, size, data array, and error string into the allocated buffer
	std::memcpy(str, &tm, 8);				   // Copy time
	std::memcpy(&str[8], &size, 4);			   // Copy array size
	std::memcpy(&str[8 + 4], data, size * sz); // Copy the data array
	if (strError != nullptr)				   // Copy error message string if provided
	{
		std::strcpy(&str[8 + 4 + (size * sz)], strError);
	}
	else
	{
		str[ln - 1] = 0; // Ensure null termination if no string
	}

	// Send the message to the task queue
	sendMessage(&msg, 0, true); // Timeout 0, free memory on failure
}

// Sends a data array trace message (version 2 - indirect pointer) to the task queue.
void CTraceTask::traceData2(const char *strError, void *data, uint32_t size, uint16_t tp)
{
	STaskMessage msg; // Message structure to send

	// Get the current time, protected by a critical section
	taskENTER_CRITICAL(&mMut);
	uint64_t tm = getTimer(AUTO_TIMER);
	taskEXIT_CRITICAL(&mMut);

	// Calculate the required buffer size for the message body
	// Time (8) + Size (4) + Pointer (4) + Null terminator (1)
	int ln = 8 + 4 + 4 + 1;
	if (strError != nullptr) // Add length of the error string if present
	{
		ln += std::strlen(strError);
	}

	// Allocate memory for the message body and set the message ID
	char *str = (char *)allocNewMsg(&msg, tp, ln);

	// Copy time, size, and the *pointer* to the data into the allocated buffer
	std::memcpy(str, &tm, 8);			// Copy time
	std::memcpy(&str[8], &size, 4);		// Copy array size
	std::memcpy(&str[8 + 4], &data, 4); // Copy the *address* of the data (4 bytes)

	if (strError != nullptr) // Copy error message string if provided
	{
		std::strcpy(&str[8 + 4 + 4], strError); // String starts after time, size, and pointer (4 bytes each)
	}
	else
	{
		str[ln - 1] = 0; // Ensure null termination if no string
	}

	// Send the message to the task queue
	sendMessage(&msg, 0, true); // Timeout 0, free memory on failure
}

// Resets the internal timer used for measuring time intervals.
// Uses a critical section to ensure thread safety.
void CTraceTask::startTime()
{
	taskENTER_CRITICAL(&mMut); // Enter critical section
	getTimer();				   // Call getTimer to reset the internal time reference
	taskEXIT_CRITICAL(&mMut);  // Exit critical section
};

// Sends a timing stop message to the task queue for processing.
void CTraceTask::stopTime(const char *str, uint32_t n)
{
	STaskMessage msg; // Message structure to send

	// Get the elapsed time, protected by a critical section
	taskENTER_CRITICAL(&mMut);
	uint64_t tm = getTimer(); // getTimer() resets the internal reference time
	taskEXIT_CRITICAL(&mMut);

	// Calculate the required buffer size for the message body
	// Time (8) + Divisor (4) + Null terminator (1)
	int ln = 8 + 4 + 1;
	if (str != nullptr) // Add length of the description string if present
	{
		ln += std::strlen(str);
	}

	// Allocate memory for the message body and set the message ID
	char *dt = (char *)allocNewMsg(&msg, MSG_STOP_TIME, ln);

	// Copy time, divisor, and description string into the allocated buffer
	std::memcpy(dt, &tm, 8);	// Copy time
	std::memcpy(&dt[8], &n, 4); // Copy divisor
	if (str != nullptr)			// Copy description string if provided
	{
		std::strcpy(&dt[8 + 4], str);
	}
	else
	{
		dt[ln - 1] = 0; // Ensure null termination if no string
	}

	// Send the message to the task queue
	sendMessage(&msg, 0, true); // Timeout 0, free memory on failure
};

// Sends a simple print message to the task queue for processing.
void CTraceTask::log(const char *str)
{
	STaskMessage msg; // Message structure to send

	if (str != nullptr) // If a string is provided
	{
		// Allocate memory for the message body (string length + null terminator) and set the message ID
		char *dt = (char *)allocNewMsg(&msg, MSG_PRINT_STRING, std::strlen(str) + 1);
		std::strcpy(dt, str); // Copy the string into the allocated buffer
	}
	else // If no string is provided
	{
		// Allocate memory for an empty string (just null terminator) and set the message ID
		char *dt = (char *)allocNewMsg(&msg, MSG_PRINT_STRING, 1);
		dt[0] = 0; // Set the null terminator
	}

	// Send the message to the task queue
	sendMessage(&msg, 0, true); // Timeout 0, free memory on failure
}