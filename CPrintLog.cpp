/*!
    \file
    \brief Class for system error logging to the console.
    \authors Bliznets R.A. (r.bliznets@gmail.com)
    \version 1.3.0.0
    \date 10.07.2020
    \details This class provides functionality for formatting and printing log messages
             to the console, including timestamp information and optional system restart
             on critical errors. It supports logging error codes, various data types
             in different formats (decimal, hexadecimal), and timing information.
*/

#include "CPrintLog.h"
#include "esp_log.h"

// Method to format the log header with a timestamp
void CPrintLog::printHeader(uint64_t time, uint32_t n)
{
    // Calculate time based on the provided divisor 'n'
    uint64_t res = time / n;

#if (CONFIG_TRACE_USEC == 1)
    // If microseconds are configured, format the header with microseconds
    std::snprintf(m_header, sizeof(m_header), "(+%liusec)", (long)res);
#else
    // Choose the appropriate time unit based on the calculated value 'res'
    if (res >= 10000000)
    {
        // If 'res' is greater than or equal to 10 million (assuming microseconds), convert to seconds
        std::snprintf(m_header, sizeof(m_header), "(+%lisec)", (long)(res / 1000000));
    }
    else
    {
        if (res < 10000)
        {
            if (res < 10)
            {
                // If 'res' is less than 10 (assuming microseconds), convert to nanoseconds with double precision
                double f = time / (double)n;
                std::snprintf(m_header, sizeof(m_header), "(+%linsec)", (long)(f * 1000));
            }
            else
            {
                // If 'res' is less than 10000 (assuming microseconds), format the header with microseconds
                std::snprintf(m_header, sizeof(m_header), "(+%liusec)", (long)res);
            }
        }
        else
        {
            // If 'res' is between 10000 and 9999999 (assuming microseconds), convert to milliseconds
            std::snprintf(m_header, sizeof(m_header), "(+%limsec)", (long)(res / 1000));
        }
    }
#endif
}

// Method to print an error message with optional system restart
void CPrintLog::trace(const char *strError, int32_t errCode, esp_log_level_t level, bool reboot)
{
    // Get the current timer value
    uint64_t res = getTimer();

    // Check if the error code is not a special value (0x7fffffff) used to ignore output
    if (errCode != 0x7fffffff)
    {
        // Format the log header with the timestamp
        printHeader(res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
        // If ESP_LOG_LEVEL is configured, format the message using ESP-IDF logging
        if (reboot)
        {
            if (strError == nullptr)
            {
                ESP_LOG_LEVEL(level, m_header, "%ld: abort...", errCode);
            }
            else
            {
                ESP_LOG_LEVEL(level, m_header, "%ld:%s abort...", errCode, strError);
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
            esp_restart();
        }
        else
        {
            if (strError == nullptr)
            {
                ESP_LOG_LEVEL(level, m_header, "%ld", errCode);
            }
            else
            {
                ESP_LOG_LEVEL(level, m_header, "%ld:%s", errCode, strError);
            }
        }
#else
        // If ESP_LOG_LEVEL is not configured, format the message using std::printf
        std::printf(m_header);
        if (reboot)
        {
            if (strError == nullptr)
            {
                std::printf(": %d\n", (int)errCode);
                std::printf("abort\n");
            }
            else
            {
                std::printf(": %d:%s\n", (int)errCode, strError);
                std::printf("abort\n");
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
            esp_restart();
        }
        else
        {
            if (strError == nullptr)
            {
                std::printf(": %d\n", (int)errCode);
            }
            else
            {
                std::printf(": %d:%s\n", (int)errCode, strError);
            }
        }
#endif
    }
}

// Overloaded trace method for logging uint8_t arrays (as hex)
void CPrintLog::trace(const char *strError, uint8_t *data, uint32_t size)
{
    // Get the current timer value
    uint64_t res = getTimer();

    // Format the log header with the timestamp
    printHeader(res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
    // If ESP_LOG_LEVEL is configured, print data in hexadecimal format using ESP-IDF logging
    ESP_LOG_BUFFER_HEX(m_header, data, size);
#else
    // If ESP_LOG_LEVEL is not configured, use std::printf to print the data
    std::printf(m_header);
    std::printf("%s %ld:", strError, size);
    std::printf(" 0x%02x", data[0]);
    for (int16_t i = 1; i < size; i++)
    {
        std::printf(",0x%02x", data[i]);
    }
    std::printf("\n");
#endif
}

// Overloaded trace method for logging int8_t arrays (as decimal)
void CPrintLog::trace(const char *strError, int8_t *data, uint32_t size)
{
    // Get the current timer value
    uint64_t res = getTimer();

    // Format the log header with the timestamp
    printHeader(res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
    // If ESP_LOG_LEVEL is configured, print data in hexadecimal format using ESP-IDF logging
    ESP_LOG_BUFFER_HEX(m_header, data, size);
#else
    // If ESP_LOG_LEVEL is not configured, use std::printf to print the data
    std::printf(m_header);
    std::printf("%s %ld:", strError, size);
    std::printf(" %d", data[0]);
    for (int16_t i = 1; i < size; i++)
    {
        std::printf(",%d", data[i]);
    }
    std::printf("\n");
#endif
}

// Overloaded trace method for logging uint16_t arrays (as hex)
void CPrintLog::trace(const char *strError, uint16_t *data, uint32_t size)
{
    // Get the current timer value
    uint64_t res = getTimer();

    // Format the log header with the timestamp
    printHeader(res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
    // If ESP_LOG_LEVEL is configured, print data in hexadecimal format using ESP-IDF logging
    // Note: Size is multiplied by 2 to account for 16-bit elements when printing raw bytes
    ESP_LOG_BUFFER_HEX(m_header, data, size * 2);
#else
    // If ESP_LOG_LEVEL is not configured, use std::printf to print the data
    std::printf(m_header);
    std::printf("%s %ld:", strError, size);
    std::printf(" 0x%04x", data[0]);
    for (int16_t i = 1; i < size; i++)
    {
        std::printf(",0x%04x", data[i]);
    }
    std::printf("\n");
#endif
}

// Overloaded trace method for logging int16_t arrays (as decimal)
void CPrintLog::trace(const char *strError, int16_t *data, uint32_t size)
{
    // Get the current timer value
    uint64_t res = getTimer();

    // Format the log header with the timestamp
    printHeader(res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
    // If ESP_LOG_LEVEL is configured, print data in hexadecimal format using ESP-IDF logging
    // Note: Size is multiplied by 2 to account for 16-bit elements when printing raw bytes
    ESP_LOG_BUFFER_HEX(m_header, data, size * 2);
#else
    // If ESP_LOG_LEVEL is not configured, use std::printf to print the data
    std::printf(m_header);
    std::printf("%s %ld:", strError, size);
    std::printf(" %d", (int)data[0]);
    for (int16_t i = 1; i < size; i++)
    {
        std::printf(",%d", (int)data[i]);
    }
    std::printf("\n");
#endif
}

// Overloaded trace method for logging uint32_t arrays (as hex)
void CPrintLog::trace(const char *strError, uint32_t *data, uint32_t size)
{
    // Get the current timer value
    uint64_t res = getTimer();

    // Format the log header with the timestamp
    printHeader(res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
    // If ESP_LOG_LEVEL is configured, print data in hexadecimal format using ESP-IDF logging
    // Note: Size is multiplied by 4 to account for 32-bit elements when printing raw bytes
    ESP_LOG_BUFFER_HEX(m_header, data, size * 4);
#else
    // If ESP_LOG_LEVEL is not configured, use std::printf to print the data
    std::printf(m_header);
    std::printf("%s %ld:", strError, size);
    std::printf(" 0x%08x", (int)data[0]);
    for (int16_t i = 1; i < size; i++)
    {
        std::printf(",0x%08x", (int)data[i]);
    }
    std::printf("\n");
#endif
}

// Overloaded trace method for logging int32_t arrays (as decimal)
void CPrintLog::trace(const char *strError, int32_t *data, uint32_t size)
{
    // Get the current timer value
    uint64_t res = getTimer();

    // Format the log header with the timestamp
    printHeader(res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
    // If ESP_LOG_LEVEL is configured, print data in hexadecimal format using ESP-IDF logging
    // Note: Size is multiplied by 4 to account for 32-bit elements when printing raw bytes
    ESP_LOG_BUFFER_HEX(m_header, data, size * 4);
#else
    // If ESP_LOG_LEVEL is not configured, use std::printf to print the data
    std::printf(m_header);
    std::printf("%s %ld:", strError, size);
    std::printf(" %d", (int)data[0]);
    for (int16_t i = 1; i < size; i++)
    {
        std::printf(",%d", (int)data[i]);
    }
    std::printf("\n");
#endif
}

// Method to print timing information with a custom divisor 'n'
void CPrintLog::stopTime(const char *str, uint32_t n)
{
    // Get the current timer value
    uint64_t res = getTimer();

    // Format the log header with the timestamp and divisor 'n'
    printHeader(res, n);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
    // If ESP_LOG_LEVEL is configured, print the timing message using ESP-IDF logging
    ESP_LOGI(m_header, "%s", str);
#else
    // If ESP_LOG_LEVEL is not configured, use std::printf to print the message
    std::printf(m_header);
    std::printf(" %s\n", str);
#endif
}