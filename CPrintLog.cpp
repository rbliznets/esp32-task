/*!
    \file
    \brief Класс журнала ошибок системы для консоли.
    \authors Близнец Р.А. (r.bliznets@gmail.com)
    \version 1.3.0.0
    \date 10.07.2020
*/

#include "CPrintLog.h"
#include "esp_log.h"

// Метод для формирования заголовка лога с временной меткой
void CPrintLog::printHeader(uint64_t time, uint32_t n)
{
    // Вычисление времени в единицах измерения, зависящих от конфигурации
    uint64_t res = time / n;

#if (CONFIG_TRACE_USEC == 1)
    // Если используется микросекунды, формируем заголовок с микросекундами
    std::snprintf(m_header, sizeof(m_header), "(+%liusec)", (long)res);
#else
    // В зависимости от значения res выбираем подходящую единицу измерения времени
    if (res >= 10000000)
    {
        // Если res больше или равно 10 миллионам микросекунд, преобразуем в секунды
        std::snprintf(m_header, sizeof(m_header), "(+%lisec)", (long)(res / 1000000));
    }
    else
    {
        if (res < 10000)
        {
            if (res < 10)
            {
                // Если res меньше 10 микросекунд, преобразуем в наносекунды с двойной точностью
                double f = time / (double)n;
                std::snprintf(m_header, sizeof(m_header), "(+%linsec)", (long)(f * 1000));
            }
            else
            {
                // Если res меньше 10000 микросекунд, формируем заголовок с микросекундами
                std::snprintf(m_header, sizeof(m_header), "(+%liusec)", (long)res);
            }
        }
        else
        {
            // Если res между 10000 и 9999999 микросекундами, преобразуем в миллисекунды
            std::snprintf(m_header, sizeof(m_header), "(+%limsec)", (long)(res / 1000));
        }
    }
#endif
}

// Метод для вывода сообщения об ошибке с возможностью перезагрузки системы
void CPrintLog::trace(const char *strError, int32_t errCode, esp_log_level_t level, bool reboot)
{
    // Получение текущего времени таймера
    uint64_t res = getTimer();

    // Проверка, что код ошибки не равен специальному значению (0x7fffffff), используемому для игнорирования вывода
    if (errCode != 0x7fffffff)
    {
        // Формирование заголовка лога с временной меткой
        printHeader(res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
        // Если используется ESP_LOG_LEVEL, формируем сообщение с использованием этой функции
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
        // Если не используется ESP_LOG_LEVEL, формируем сообщение с использованием std::printf
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
void CPrintLog::trace(const char *strError, uint8_t *data, uint32_t size)
{
    // Получение текущего времени таймера
    uint64_t res = getTimer();

    // Формирование заголовка лога с временной меткой
    printHeader(res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
    // Если используется ESP_LOG_LEVEL, выводим данные в шестнадцатеричном формате
    ESP_LOG_BUFFER_HEX(m_header, data, size);
#else
    // Если не используется ESP_LOG_LEVEL, используем std::printf для вывода данных
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

void CPrintLog::trace(const char *strError, int8_t *data, uint32_t size)
{
    // Получение текущего времени таймера
    uint64_t res = getTimer();

    // Формирование заголовка лога с временной меткой
    printHeader(res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
    // Если используется ESP_LOG_LEVEL, выводим данные в шестнадцатеричном формате
    ESP_LOG_BUFFER_HEX(m_header, data, size);
#else
    // Если не используется ESP_LOG_LEVEL, используем std::printf для вывода данных
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

void CPrintLog::trace(const char *strError, uint16_t *data, uint32_t size)
{
    // Получение текущего времени таймера
    uint64_t res = getTimer();

    // Формирование заголовка лога с временной меткой
    printHeader(res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
    // Если используется ESP_LOG_LEVEL, выводим данные в шестнадцатеричном формате
    ESP_LOG_BUFFER_HEX(m_header, data, size * 2);
#else
    // Если не используется ESP_LOG_LEVEL, используем std::printf для вывода данных
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

void CPrintLog::trace(const char *strError, int16_t *data, uint32_t size)
{
    // Получение текущего времени таймера
    uint64_t res = getTimer();

    // Формирование заголовка лога с временной меткой
    printHeader(res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
    // Если используется ESP_LOG_LEVEL, выводим данные в шестнадцатеричном формате
    ESP_LOG_BUFFER_HEX(m_header, data, size * 2);
#else
    // Если не используется ESP_LOG_LEVEL, используем std::printf для вывода данных
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

void CPrintLog::trace(const char *strError, uint32_t *data, uint32_t size)
{
    // Получение текущего времени таймера
    uint64_t res = getTimer();

    // Формирование заголовка лога с временной меткой
    printHeader(res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
    // Если используется ESP_LOG_LEVEL, выводим данные в шестнадцатеричном формате
    ESP_LOG_BUFFER_HEX(m_header, data, size * 4);
#else
    // Если не используется ESP_LOG_LEVEL, используем std::printf для вывода данных
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

void CPrintLog::trace(const char *strError, int32_t *data, uint32_t size)
{
    // Получение текущего времени таймера
    uint64_t res = getTimer();

    // Формирование заголовка лога с временной меткой
    printHeader(res);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
    // Если используется ESP_LOG_LEVEL, выводим данные в шестнадцатеричном формате
    ESP_LOG_BUFFER_HEX(m_header, data, size * 4);
#else
    // Если не используется ESP_LOG_LEVEL, используем std::printf для вывода данных
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

void CPrintLog::stopTime(const char *str, uint32_t n)
{
    // Получение текущего времени таймера
    uint64_t res = getTimer();

    // Формирование заголовка лога с временной меткой и делителем n
    printHeader(res, n);

#ifdef CONFIG_DEBUG_TRACE_ESPLOG
    // Если используется ESP_LOG_LEVEL, выводим сообщение об остановке времени
    ESP_LOGI(m_header, "%s", str);
#else
    // Если не используется ESP_LOG_LEVEL, используем std::printf для вывода сообщения
    std::printf(m_header);
    std::printf(" %s\n", str);
#endif
}
