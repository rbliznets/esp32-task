/*!
    \file
    \brief Class for working with ADC (Analog-to-Digital Converter) in single-shot mode with 12-bit resolution.
    \authors Bliznets R.A. (r.bliznets@gmail.com)
    \version 1.0.0.0
    \date 23.11.2024
    \details The class implements the Singleton pattern for managing ADC resources.
             Supports working with two ADC modules (ADC1, ADC2) on ESP.
             Ensures thread safety through the use of locks.
*/

#include "ADCMaster.h" // Header file for the ADCMaster class
#include "esp_log.h"   // Library for logging in the ESP-IDF environment

static const char *TAG = "ADCMaster"; // Tag for identifying logs from this class

ADCMaster *ADCMaster::theSingleInstance = nullptr; // Pointer to the single instance of the class

/**
 * \brief Default constructor.
 * \details Initializes the object's lock (CLock) for synchronizing access to ADC resources.
 */
ADCMaster::ADCMaster() : CLock() {}

/**
 * \brief Destructor.
 * \details Does nothing in the current implementation, as resource management
 *          is handled through the free() method.
 */
ADCMaster::~ADCMaster() {}

/**
 * \brief Returns the single instance of the class (Singleton pattern implementation).
 * \return Pointer to the ADCMaster instance.
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
 * \brief Frees ADC resources and deletes the class instance.
 * \details Calls release() for both ADC modules (0 and 1) before deletion.
 */
void ADCMaster::free()
{
    if (theSingleInstance != nullptr)
    {
        for (int16_t i = 0; i < 2; i++)
        {
            theSingleInstance->release((adc_unit_t)i); // Freeing ADC resources
        }
        delete theSingleInstance;
        theSingleInstance = nullptr;
    }
}

/**
 * \brief Initializes an ADC channel for operation.
 * \param adc_num The ADC module number (ADC_UNIT_1 or ADC_UNIT_2).
 * \param channel The ADC channel number.
 * \return true - successful initialization, false - error.
 * \details If the ADC module is not initialized, it creates a new unit.
 *          Configures the channel with default parameters: 12 bit, 12 dB attenuation.
 */
bool ADCMaster::take(adc_unit_t adc_num, adc_channel_t channel)
{
    esp_err_t err;

    lock(); // Lock access to ADC resources

    // Initialize the ADC module if it hasn't been initialized before
    if (m_adc[adc_num].count == 0)
    {
        adc_oneshot_unit_init_cfg_t init_config;
        init_config.unit_id = adc_num;
        init_config.clk_src = ADC_RTC_CLK_SRC_DEFAULT; // Default clock source
        init_config.ulp_mode = ADC_ULP_MODE_DISABLE;   // Disable Ultra-Low Power mode
        err = adc_oneshot_new_unit(&init_config, &m_adc[adc_num].adc_handle);
        if (err != ESP_OK)
        {
            unlock();
            ESP_LOGE(TAG, "adc_oneshot_new_unit failed(%d) ADC%d", err, adc_num);
            return false;
        }
    }

    // Configure the ADC channel
    adc_oneshot_chan_cfg_t config;
    config.bitwidth = ADC_BITWIDTH_DEFAULT; // 12 bits
    config.atten = ADC_ATTEN_DB_12;
    err = adc_oneshot_config_channel(m_adc[adc_num].adc_handle, channel, &config);
    if (err != ESP_OK)
    {
        unlock();
        ESP_LOGE(TAG, "adc_oneshot_config_channel failed(%d) channel %d in ADC%d", err, (uint16_t)channel, adc_num);
        return false;
    }

    m_adc[adc_num].count++; // Increment the module usage counter
    unlock();
    return true;
}

adc_oneshot_unit_handle_t *ADCMaster::take(adc_unit_t adc_num)
{
    esp_err_t err;

    lock(); // Lock access to ADC resources

    // Initialize the ADC module if it hasn't been initialized before
    if (m_adc[adc_num].count == 0)
    {
        adc_oneshot_unit_init_cfg_t init_config;
        init_config.unit_id = adc_num;
        init_config.clk_src = ADC_RTC_CLK_SRC_DEFAULT; // Default clock source
        init_config.ulp_mode = ADC_ULP_MODE_DISABLE;   // Disable Ultra-Low Power mode
        err = adc_oneshot_new_unit(&init_config, &m_adc[adc_num].adc_handle);
        if (err != ESP_OK)
        {
            unlock();
            ESP_LOGE(TAG, "adc_oneshot_new_unit failed(%d) ADC%d", err, adc_num);
            return nullptr;
        }
    }

    m_adc[adc_num].count++; // Increment the module usage counter
    unlock();
    return &(m_adc[adc_num].adc_handle);
}

/**
 * \brief Reads the value from the specified ADC channel.
 * \param adc_num The ADC module number.
 * \param channel The ADC channel number.
 * \param[out] value The read value (0-4095 for 12 bits).
 * \return true - read successful, false - error.
 * \details On timeout, retries the read attempt.
 */
bool ADCMaster::read(adc_unit_t adc_num, adc_channel_t channel, int &value)
{
    esp_err_t err = ESP_ERR_TIMEOUT;
    while (err != ESP_OK)
    {
        err = adc_oneshot_read(m_adc[adc_num].adc_handle, channel, &value);

        if (err == ESP_ERR_TIMEOUT)
        {
            ESP_LOGW(TAG, "adc_oneshot_read ESP_ERR_TIMEOUT");
            continue;
        }
        else if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "adc_oneshot_read ADC%d: %d", adc_num, err);
            return false;
        }
    }
    return true;
}

/**
 * \brief Frees the resources of the ADC module.
 * \param adc_num The ADC module number.
 * \details Decrements the module usage counter.
 *          If the counter reaches zero, deletes the ADC unit.
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
                ESP_LOGE(TAG, "adc_oneshot_del_unit failed(%d) ADC%d", err, adc_num);
            }
        }
    }
    unlock();
}