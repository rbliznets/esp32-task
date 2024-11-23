/*!
    \file
    \brief Класс инициализации adc в режиме one short 12bit.
    \authors Близнец Р.А. (r.bliznets@gmail.com)
    \version 0.1.0.0
    \date 23.11.2024
*/

#include "ADCMaster.h"
#include "esp_log.h"

static const char *TAG = "ADCMaster";

ADCMaster *ADCMaster::theSingleInstance = nullptr;

ADCMaster::ADCMaster() : CLock()
{
}

ADCMaster::~ADCMaster()
{
}

ADCMaster *ADCMaster::Instance()
{
    if (theSingleInstance == nullptr)
    {
        theSingleInstance = new ADCMaster();
    }
    return theSingleInstance;
}

void ADCMaster::free()
{
    if (theSingleInstance != nullptr)
    {
        for (int16_t i = 0; i < 2; i++)
        {
            theSingleInstance->release((adc_unit_t)i);
        }
        delete theSingleInstance;
        theSingleInstance = nullptr;
    }
}

bool ADCMaster::take(adc_unit_t adc_num, adc_channel_t channel)
{
    esp_err_t err;

    lock();
    if (m_adc[adc_num].count == 0)
    {
        adc_oneshot_unit_init_cfg_t init_config;
        init_config.unit_id = (adc_unit_t)adc_num;
        init_config.clk_src = ADC_RTC_CLK_SRC_DEFAULT;
        init_config.ulp_mode = ADC_ULP_MODE_DISABLE;
        err = adc_oneshot_new_unit(&init_config, &m_adc[adc_num].adc_handle);
        if (err != ESP_OK)
        {
            unlock();
            ESP_LOGE(TAG, "Init ADC%d failed", adc_num);
            return false;
        }
    }
    adc_oneshot_chan_cfg_t config;
    config.bitwidth = ADC_BITWIDTH_DEFAULT;
    config.atten = ADC_ATTEN_DB_12;
    err = adc_oneshot_config_channel(m_adc[adc_num].adc_handle, channel, &config);
    if (err != ESP_OK)
    {
        unlock();
        ESP_LOGE(TAG, "add channel %d to ADC%d failed", (uint16_t)channel, adc_num);
        return false;
    }

    m_adc[adc_num].count++;
    unlock();
    return true;
}

bool ADCMaster::read(adc_unit_t adc_num, adc_channel_t channel, int &value)
{
    esp_err_t err = adc_oneshot_read(m_adc[adc_num].adc_handle, channel, &value);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "adc_oneshot_read faeled %d", err);
        return false;
    }
    else
        return true;
}

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
                ESP_LOGE(TAG, "Delete I2C%d failed", adc_num);
            }
        }
    }
    unlock();
}
