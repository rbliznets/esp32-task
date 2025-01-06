/*!
    \file
    \brief Класс инициализации i2c в режиме мастер.
    \authors Близнец Р.А. (r.bliznets@gmail.com)
    \version 1.0.0.0
    \date 11.11.2024
*/

#include "I2CMaster.h"
#include "esp_log.h"

static const char *TAG = "I2CMaster";

I2CMaster *I2CMaster::theSingleInstance = nullptr;

I2CMaster::I2CMaster() : CLock()
{
}

I2CMaster::~I2CMaster()
{
}

I2CMaster *I2CMaster::Instance()
{
    if (theSingleInstance == nullptr)
    {
        theSingleInstance = new I2CMaster();
    }
    return theSingleInstance;
}

void I2CMaster::free()
{
    if (theSingleInstance != nullptr)
    {
        for (int16_t i = 0; i < I2C_NUM_MAX; i++)
        {
            theSingleInstance->release(i);
        }
        delete theSingleInstance;
        theSingleInstance = nullptr;
    }
}

bool I2CMaster::init(int16_t i2c_num, int16_t i2c_sda, int16_t i2c_scl)
{
    assert(i2c_num >= 0 && i2c_num < I2C_NUM_MAX);
    assert(i2c_sda >= 0 && i2c_sda < GPIO_NUM_MAX);
    assert(i2c_scl >= 0 && i2c_scl < GPIO_NUM_MAX);

    if ((m_i2c[i2c_num].i2c_sda >= 0) || (m_i2c[i2c_num].i2c_scl >= 0))
        return false;
    else
    {
        m_i2c[i2c_num].i2c_sda = i2c_sda;
        m_i2c[i2c_num].i2c_scl = i2c_scl;
        m_i2c[i2c_num].count = 0;
        return true;
    }
}

bool I2CMaster::take(int16_t i2c_num)
{
    assert(i2c_num >= 0 && i2c_num < I2C_NUM_MAX);

    if ((m_i2c[i2c_num].i2c_sda < 0) || (m_i2c[i2c_num].i2c_scl < 0))
        return false;
    else
    {
        lock();
        if (m_i2c[i2c_num].count == 0)
        {
            i2c_master_bus_config_t i2c_mst_config = {
                .i2c_port = i2c_num,
                .sda_io_num = (gpio_num_t)m_i2c[i2c_num].i2c_sda,
                .scl_io_num = (gpio_num_t)m_i2c[i2c_num].i2c_scl,
                .clk_source = I2C_CLK_SRC_DEFAULT,
                .glitch_ignore_cnt = 7,
                .intr_priority = 0,
                .trans_queue_depth = 0,
                .flags = {1, 1}};
            esp_err_t err = i2c_new_master_bus(&i2c_mst_config, &m_i2c[i2c_num].bus_handle);
            if (err != ESP_OK)
            {
                unlock();
                ESP_LOGE(TAG, "Init I2C%d failed", i2c_num);
                return false;
            }
        }
        m_i2c[i2c_num].count++;
        unlock();
        return true;
    }
}

bool I2CMaster::probe(int16_t i2c_num, uint16_t address)
{
    assert(i2c_num >= 0 && i2c_num < I2C_NUM_MAX);

    if ((m_i2c[i2c_num].i2c_sda < 0) || (m_i2c[i2c_num].i2c_scl < 0))
        return false;

    lock();
    if (m_i2c[i2c_num].count != 0)
    {
        esp_err_t err = i2c_master_probe(m_i2c[i2c_num].bus_handle, address, -1);
        unlock();
        return err == ESP_OK;
    }
    else
    {
        unlock();
        return false;
    }
}

bool I2CMaster::add(int16_t i2c_num, const i2c_device_config_t *dev_config, i2c_master_dev_handle_t *ret_handle)
{
    assert(i2c_num >= 0 && i2c_num < I2C_NUM_MAX);
    assert(dev_config != nullptr);

    if ((m_i2c[i2c_num].i2c_sda < 0) || (m_i2c[i2c_num].i2c_scl < 0))
        return false;

    lock();
    if (m_i2c[i2c_num].count != 0)
    {
        esp_err_t err = i2c_master_bus_add_device(m_i2c[i2c_num].bus_handle, dev_config, ret_handle);
        if (err != ESP_OK)
        {
            unlock();
            ESP_LOGE(TAG, "I2CMaster::add %d failed", i2c_num);
            return false;
        }
        unlock();
        return true;
    }
    else
    {
        unlock();
        return false;
    }
}

void I2CMaster::release(int16_t i2c_num)
{
    assert(i2c_num >= 0 && i2c_num < I2C_NUM_MAX);

    lock();
    if (m_i2c[i2c_num].count != 0)
    {
        m_i2c[i2c_num].count--;
        if (m_i2c[i2c_num].count == 0)
        {
            esp_err_t err = i2c_del_master_bus(m_i2c[i2c_num].bus_handle);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Delete I2C%d failed", i2c_num);
            }
        }
    }
    unlock();
}
