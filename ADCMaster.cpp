/*!
    \file
    \brief Класс для работы с АЦП (ADC) в режиме однократного замера (one shot) с разрешением 12 бит.
    \authors Близнец Р.А. (r.bliznets@gmail.com)
    \version 0.1.0.0
    \date 23.11.2024
    \details Класс реализует паттерн Singleton для управления ресурсами ADC.
             Поддерживает работу с двумя модулями ADC (ADC1, ADC2) на ESP.
             Обеспечивает потокобезопасность за счет использования блокировок.
*/

#include "ADCMaster.h" // Заголовочный файл класса ADCMaster
#include "esp_log.h"   // Библиотека для логирования в среде ESP-IDF

static const char *TAG = "ADCMaster"; // Тег для идентификации логов данного класса

ADCMaster *ADCMaster::theSingleInstance = nullptr; // Указатель на единственный экземпляр класса

/**
 * \brief Конструктор по умолчанию.
 * \details Инициализирует объект блокировки CLock для синхронизации доступа к ресурсам ADC.
 */
ADCMaster::ADCMaster() : CLock() {}

/**
 * \brief Деструктор.
 * \details В текущей реализации не выполняет действий, так как управление ресурсами
 *          осуществляется через метод free().
 */
ADCMaster::~ADCMaster() {}

/**
 * \brief Возвращает единственный экземпляр класса (реализация паттерна Singleton).
 * \return Указатель на экземпляр ADCMaster.
 */
ADCMaster *ADCMaster::Instance()
{
    if (theSingleInstance == nullptr)
    {
        theSingleInstance = new ADCMaster();
    }
    return theSingleInstance;
}

/**
 * \brief Освобождает ресурсы ADC и удаляет экземпляр класса.
 * \details Вызывает release() для обоих модулей ADC (0 и 1) перед удалением.
 */
void ADCMaster::free()
{
    if (theSingleInstance != nullptr)
    {
        for (int16_t i = 0; i < 2; i++)
        {
            theSingleInstance->release((adc_unit_t)i); // Освобождение ресурсов ADC
        }
        delete theSingleInstance;
        theSingleInstance = nullptr;
    }
}

/**
 * \brief Инициализирует канал ADC для работы.
 * \param adc_num Номер модуля ADC (ADC_UNIT_1 или ADC_UNIT_2).
 * \param channel Номер канала ADC.
 * \return true - успешная инициализация, false - ошибка.
 * \details Если модуль ADC не инициализирован, создает новый юнит.
 *          Настраивает канал с параметрами по умолчанию: 12 бит, затухание 12 дБ.
 */
bool ADCMaster::take(adc_unit_t adc_num, adc_channel_t channel)
{
    esp_err_t err;

    lock(); // Блокировка доступа к ресурсам ADC

    // Инициализация модуля ADC, если он не был инициализирован ранее
    if (m_adc[adc_num].count == 0)
    {
        adc_oneshot_unit_init_cfg_t init_config;
        init_config.unit_id = adc_num;
        init_config.clk_src = ADC_RTC_CLK_SRC_DEFAULT; // Источник тактирования по умолчанию
        init_config.ulp_mode = ADC_ULP_MODE_DISABLE;   // Отключение режима для ультра-низкого потребления
        err = adc_oneshot_new_unit(&init_config, &m_adc[adc_num].adc_handle);
        if (err != ESP_OK)
        {
            unlock();
            ESP_LOGE(TAG, "Ошибка инициализации ADC%d", adc_num);
            return false;
        }
    }

    // Настройка канала ADC
    adc_oneshot_chan_cfg_t config;
    config.bitwidth = ADC_BITWIDTH_DEFAULT; // 12 бит
    config.atten = ADC_ATTEN_DB_12;         
    err = adc_oneshot_config_channel(m_adc[adc_num].adc_handle, channel, &config);
    if (err != ESP_OK)
    {
        unlock();
        ESP_LOGE(TAG, "Ошибка добавления канала %d в ADC%d", (uint16_t)channel, adc_num);
        return false;
    }

    m_adc[adc_num].count++; // Увеличение счетчика использования модуля
    unlock();
    return true;
}

/**
 * \brief Читает значение с указанного канала ADC.
 * \param adc_num Номер модуля ADC.
 * \param channel Номер канала ADC.
 * \param[out] value Прочитанное значение (0-4095 для 12 бит).
 * \return true - чтение успешно, false - ошибка.
 * \details При возникновении тайм-аута повторяет попытку чтения.
 */
bool ADCMaster::read(adc_unit_t adc_num, adc_channel_t channel, int &value)
{
    esp_err_t err = ESP_ERR_TIMEOUT;
    while (err != ESP_OK)
    {
        err = adc_oneshot_read(m_adc[adc_num].adc_handle, channel, &value);

        if (err == ESP_ERR_TIMEOUT)
        {
            ESP_LOGW(TAG, "Тайм-аут чтения ADC. Повторная попытка...");
            continue;
        }
        else if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Ошибка чтения ADC: %d", err);
            return false;
        }
    }
    return true;
}

/**
 * \brief Освобождает ресурсы модуля ADC.
 * \param adc_num Номер модуля ADC.
 * \details Уменьшает счетчик использования модуля.
 *          Если счетчик достигает нуля - удаляет юнит ADC.
 */
void ADCMaster::release(adc_unit_t adc_num)
{
    lock();
    if (m_adc[adc_num].count != 0)
    {
        m_adc[adc_num].count--;
        if (m_adc[adc_num].count == 0)
        {
            esp_err_t err = adc_oneshot_del_unit(m_adc[adc_num].adc_handle);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Ошибка удаления модуля ADC%d", adc_num);
            }
        }
    }
    unlock();
}