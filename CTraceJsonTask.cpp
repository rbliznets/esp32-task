/*!
	\file
	\brief Class for outputting debug information in JSON format.
	\authors Bliznets R.A.(r.bliznets@gmail.com)
	\version 1.0.0.0
	\date 02.05.2024

	\details One object per application.
	\details It is necessary to avoid blocking the task being debugged.
			 This file contains implementations for various methods that format
			 different types of trace data (messages, errors, data arrays, timing)
			 into JSON strings stored in the `mAnswer` member variable.
			 It inherits from CTraceTask and redefines how data is serialized.
			 Includes helper macros for byte swapping (`__bswap_16`, `__bswap_32`).
*/

#include "CTraceJsonTask.h"

// Macro to swap bytes in a 16-bit value
#define __bswap_16(x) \
	(__extension__({ unsigned short int __bsx = (x);					      \
        ((((__bsx) >> 8) & 0xff) | (((__bsx) & 0xff) << 8)); }))

// Macro to swap bytes in a 32-bit value
#define __bswap_32(x) \
	(__extension__({ unsigned int __bsx = (x);					      \
        ((((__bsx) & 0xff000000) >> 24) | (((__bsx) & 0x00ff0000) >>  8) |    \
	 (((__bsx) & 0x0000ff00) <<  8) | (((__bsx) & 0x000000ff) << 24)); }))

// Formats the header part of the JSON log entry, including the timestamp.
void CTraceJsonTask::printHeader(uint64_t time, uint32_t n)
{
	// Call the parent class's printHeader to format the time string into m_header
	CTraceTask::printHeader(time, n);

	// Start building the JSON string with the log object and time field
	mAnswer = "{\"log\":{\"time\":\"";
	mAnswer += m_header;
	mAnswer += "\"";
}

// Formats an error message trace entry into JSON.
void CTraceJsonTask::printString(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;		// Time
	int32_t *errCode = (int32_t *)&data[8]; // Error code
	char *strError = &data[13];				// Error message string (assuming level is at data[12])
	// Note: The code assumes level is a single byte at data[12]
	char level = data[12];

	// Format the header part
	printHeader(*res);
	// Append error code, level, and message to the JSON string
	mAnswer += ",\"code\":";
	mAnswer += std::to_string(*errCode);
	mAnswer += ",\"level\":";
	mAnswer += std::to_string(level);
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\"}}";
}

// Formats a simple log message string into JSON.
void CTraceJsonTask::printS(char *str)
{
	// Create a JSON object with the provided string as the value
	mAnswer = "{\"log\":{\"value\":\"";
	mAnswer += str;
	mAnswer += "\"}}";
}

// Formats a timing stop trace entry into JSON.
void CTraceJsonTask::printStop(char *data)
{
	// Interpret the raw data buffer
	uint64_t *x = (uint64_t *)data;	  // Time
	int32_t *n = (int32_t *)&data[8]; // Divisor for averaging
	char *str = &data[12];			  // Description string

	// Format the header part
	printHeader(*x, *n);
	// Append the description to the JSON string
	mAnswer += ",\"value\":\"";
	mAnswer += str;
	mAnswer += "\"}}";
}

// Formats a uint8_t data array trace entry into JSON (hexadecimal string).
void CTraceJsonTask::printData8h(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;		  // Time
	uint32_t *size = (uint32_t *)&data[8];	  // Size of the data array
	uint8_t *pdata = (uint8_t *)&data[8 + 4]; // Pointer to the data array
	// Error message string comes after the data array
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint8_t))];

	// Format the header part
	printHeader(*res);
	// Append error message and start the data array as a hex string
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":\"";
	// Convert each byte to a 2-digit hex string
	for (int16_t i = 0; i < *size; i++)
	{
		std::sprintf(m_header, "%02x", pdata[i]); // Format byte as hex
		mAnswer += m_header;					  // Append to JSON string
	}
	mAnswer += "\"}}"; // Close the JSON object
}

// Formats a uint8_t data array trace entry into JSON (hexadecimal string) - version 2 (indirect pointer).
void CTraceJsonTask::printData8h_2(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;	   // Time
	uint32_t *size = (uint32_t *)&data[8]; // Size of the data array
	// Pointer to the data array is stored as a 32-bit address within the buffer (little-endian assumed)
	uint8_t *pdata = (uint8_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4]; // Error message string comes after the address

	// Format the header part
	printHeader(*res);
	// Append error message and start the data array as a hex string
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":\"";
	// Convert each byte to a 2-digit hex string
	for (int16_t i = 0; i < *size; i++)
	{
		std::sprintf(m_header, "%02x", pdata[i]); // Format byte as hex
		mAnswer += m_header;					  // Append to JSON string
	}
	mAnswer += "\"}}"; // Close the JSON object
}

// Formats an int8_t data array trace entry into JSON (decimal array).
void CTraceJsonTask::printData8(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;		// Time
	uint32_t *size = (uint32_t *)&data[8];	// Size of the data array
	int8_t *pdata = (int8_t *)&data[8 + 4]; // Pointer to the data array
	// Error message string comes after the data array
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint8_t))]; // Note: sizeof(uint8_t) == sizeof(int8_t)

	// Format the header part
	printHeader(*res);
	// Append error message and start the data array as a JSON array
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":[";
	// Add the first element
	std::sprintf(m_header, "%d", pdata[0]);
	mAnswer += m_header;
	// Add remaining elements, prefixed with a comma
	for (int16_t i = 1; i < *size; i++)
	{
		std::sprintf(m_header, ",%d", pdata[i]);
		mAnswer += m_header;
	}
	mAnswer += "]}}"; // Close the JSON array and object
}

// Formats an int8_t data array trace entry into JSON (decimal array) - version 2 (indirect pointer).
void CTraceJsonTask::printData8_2(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;	   // Time
	uint32_t *size = (uint32_t *)&data[8]; // Size of the data array
	// Pointer to the data array is stored as a 32-bit address within the buffer (little-endian assumed)
	uint8_t *pdata = (uint8_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4]; // Error message string comes after the address

	// Format the header part
	printHeader(*res);
	// Append error message and start the data array as a JSON array
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":[";
	// Add the first element (cast uint8_t* to int8_t*)
	std::sprintf(m_header, "%d", ((int8_t *)pdata)[0]);
	mAnswer += m_header;
	// Add remaining elements, prefixed with a comma
	for (int16_t i = 1; i < *size; i++)
	{
		std::sprintf(m_header, ",%d", ((int8_t *)pdata)[i]);
		mAnswer += m_header;
	}
	mAnswer += "]}}"; // Close the JSON array and object
}

// Formats an int32_t data array trace entry into JSON (decimal array) - version 2 (indirect pointer).
void CTraceJsonTask::printData32_2(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;	   // Time
	uint32_t *size = (uint32_t *)&data[8]; // Size of the data array
	// Pointer to the data array is stored as a 32-bit address within the buffer (little-endian assumed)
	uint32_t *pdata = (uint32_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4]; // Error message string comes after the address

	// Format the header part
	printHeader(*res);
	// Append error message and start the data array as a JSON array
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":[";
	// Add the first element (cast uint32_t* to int32_t*)
	std::sprintf(m_header, "%ld", ((int32_t *)pdata)[0]);
	mAnswer += m_header;
	// Add remaining elements, prefixed with a comma
	for (int16_t i = 1; i < *size; i++)
	{
		std::sprintf(m_header, ",%ld", ((int32_t *)pdata)[i]);
		mAnswer += m_header;
	}
	mAnswer += "]}}"; // Close the JSON array and object
}

// Formats a uint32_t data array trace entry into JSON (hexadecimal string) - version 2 (indirect pointer).
void CTraceJsonTask::printData32h_2(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;	   // Time
	uint32_t *size = (uint32_t *)&data[8]; // Size of the data array
	// Pointer to the data array is stored as a 32-bit address within the buffer (little-endian assumed)
	uint32_t *pdata = (uint32_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4]; // Error message string comes after the address

	// Format the header part
	printHeader(*res);
	// Append error message and start the data array as a hex string
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":\"";
	// Convert each 32-bit word to an 8-digit hex string, swapping bytes first
	for (int16_t i = 0; i < *size; i++)
	{
		std::sprintf(m_header, "%08x", __bswap_32(pdata[i])); // Format word as hex with byte swap
		mAnswer += m_header;								  // Append to JSON string
	}
	mAnswer += "\"}}"; // Close the JSON object
}

// Formats a uint32_t data array trace entry into JSON (hexadecimal string).
void CTraceJsonTask::printData32h(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;			// Time
	uint32_t *size = (uint32_t *)&data[8];		// Size of the data array
	uint32_t *pdata = (uint32_t *)&data[8 + 4]; // Pointer to the data array
	// Error message string comes after the data array
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint32_t))];

	// Format the header part
	printHeader(*res);
	// Append error message and start the data array as a hex string
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":\"";
	// Convert each 32-bit word to an 8-digit hex string, swapping bytes first
	for (int16_t i = 0; i < *size; i++)
	{
		std::sprintf(m_header, "%08x", __bswap_32(pdata[i])); // Format word as hex with byte swap
		mAnswer += m_header;								  // Append to JSON string
	}
	mAnswer += "\"}}"; // Close the JSON object
}

// Formats an int32_t data array trace entry into JSON (decimal array).
void CTraceJsonTask::printData32(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;		  // Time
	uint32_t *size = (uint32_t *)&data[8];	  // Size of the data array
	int32_t *pdata = (int32_t *)&data[8 + 4]; // Pointer to the data array
	// Error message string comes after the data array
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint32_t))];

	// Format the header part
	printHeader(*res);
	// Append error message and start the data array as a JSON array
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":[";
	// Add the first element
	std::sprintf(m_header, "%ld", pdata[0]);
	mAnswer += m_header;
	// Add remaining elements, prefixed with a comma
	for (int16_t i = 1; i < *size; i++)
	{
		std::sprintf(m_header, ",%ld", pdata[i]);
		mAnswer += m_header;
	}
	mAnswer += "]}}"; // Close the JSON array and object
}

// Formats a uint16_t data array trace entry into JSON (hexadecimal string).
void CTraceJsonTask::printData16h(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;			// Time
	uint32_t *size = (uint32_t *)&data[8];		// Size of the data array
	uint16_t *pdata = (uint16_t *)&data[8 + 4]; // Pointer to the data array
	// Error message string comes after the data array
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint16_t))];

	// Format the header part
	printHeader(*res);
	// Append error message and start the data array as a hex string
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":\"";
	// Convert each 16-bit word to a 4-digit hex string, swapping bytes first
	for (int16_t i = 0; i < *size; i++)
	{
		std::sprintf(m_header, "%04x", __bswap_16(pdata[i])); // Format word as hex with byte swap
		mAnswer += m_header;								  // Append to JSON string
	}
	mAnswer += "\"}}"; // Close the JSON object
}

// Formats a uint16_t data array trace entry into JSON (hexadecimal string) - version 2 (indirect pointer).
void CTraceJsonTask::printData16h_2(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;	   // Time
	uint32_t *size = (uint32_t *)&data[8]; // Size of the data array
	// Pointer to the data array is stored as a 32-bit address within the buffer (little-endian assumed)
	uint16_t *pdata = (uint16_t *)(data[8 + 4] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4]; // Error message string comes after the address

	// Format the header part
	printHeader(*res);
	// Append error message and start the data array as a hex string
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":\"";
	// Convert each 16-bit word to a 4-digit hex string, swapping bytes first
	for (int16_t i = 0; i < *size; i++)
	{
		std::sprintf(m_header, "%04x", __bswap_16(pdata[i])); // Format word as hex with byte swap
		mAnswer += m_header;								  // Append to JSON string
	}
	mAnswer += "\"}}"; // Close the JSON object
}

// Formats an int16_t data array trace entry into JSON (decimal array).
void CTraceJsonTask::printData16(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;		  // Time
	uint32_t *size = (uint32_t *)&data[8];	  // Size of the data array
	int16_t *pdata = (int16_t *)&data[8 + 4]; // Pointer to the data array
	// Error message string comes after the data array
	char *strError = &data[8 + 4 + ((*size) * sizeof(uint16_t))];

	// Format the header part
	printHeader(*res);
	// Append error message and start the data array as a JSON array
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":[";
	// Add the first element
	std::sprintf(m_header, "%d", pdata[0]);
	mAnswer += m_header;
	// Add remaining elements, prefixed with a comma
	for (int16_t i = 1; i < *size; i++)
	{
		std::sprintf(m_header, ",%d", pdata[i]);
		mAnswer += m_header;
	}
	mAnswer += "]}}"; // Close the JSON array and object
}

// Formats an int16_t data array trace entry into JSON (decimal array) - version 2 (indirect pointer).
// Note: There's a potential bug in the address calculation: `data[8 + 2]` instead of `data[8 + 4]`
void CTraceJsonTask::printData16_2(char *data)
{
	// Interpret the raw data buffer
	uint64_t *res = (uint64_t *)data;	   // Time
	uint32_t *size = (uint32_t *)&data[8]; // Size of the data array
	// Pointer to the data array is stored as a 32-bit address within the buffer (little-endian assumed)
	// Note: This uses data[8 + 2] which might be incorrect based on the standard pattern seen elsewhere.
	uint16_t *pdata = (uint16_t *)(data[8 + 2] + data[8 + 4 + 1] * 256 + data[8 + 4 + 2] * 256 * 256 + data[8 + 4 + 3] * 256 * 256 * 256);
	char *strError = &data[8 + 4 + 4]; // Error message string comes after the address

	// Format the header part
	printHeader(*res);
	// Append error message and start the data array as a JSON array
	mAnswer += ",\"value\":\"";
	mAnswer += strError;
	mAnswer += "\",\"data\":[";
	// Add the first element (cast uint16_t* to int16_t*)
	std::sprintf(m_header, "%d", ((int16_t *)pdata)[0]);
	mAnswer += m_header;
	// Add remaining elements, prefixed with a comma
	for (int16_t i = 1; i < *size; i++)
	{
		std::sprintf(m_header, ",%d", ((int16_t *)pdata)[i]);
		mAnswer += m_header;
	}
	mAnswer += "]}}"; // Close the JSON array and object
}