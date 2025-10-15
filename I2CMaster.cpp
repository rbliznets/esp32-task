/*!
    \file
    \brief Class for initializing I2C in master mode.
    \authors Bliznets R.A. (r.bliznets@gmail.com)
    \version 1.0.0.0
    \date 11.11.2024
    \details This class implements the Singleton pattern to manage I2C master resources
             across the application. It provides methods to initialize I2C buses, add
             devices, probe for devices, and release resources, ensuring thread safety
             through a lock mechanism.
*/

#include "I2CMaster.h"
#include "esp_log.h"

static const char *TAG = "I2CMaster"; // Tag for logging

I2CMaster *I2CMaster::theSingleInstance = nullptr; // The single instance of the class

// Default constructor
I2CMaster::I2CMaster() : CLock() // Initialize the base lock class
{
}

// Destructor
I2CMaster::~I2CMaster()
{
    // Nothing specific to do here, as resource cleanup is handled by the 'free' method
}

// Method to get the single instance of the class (Singleton)
I2CMaster *I2CMaster::Instance()
{
    if (theSingleInstance == nullptr)
    {
        theSingleInstance = new I2CMaster(); // Create the single instance if it doesn't exist
    }
    return theSingleInstance; // Return the single instance
}

// Method to free resources and delete the class instance
void I2CMaster::free()
{
    if (theSingleInstance != nullptr)
    {
        // Iterate through all possible I2C buses and release them
        for (int16_t i = 0; i < I2C_NUM_MAX; i++)
        {
            theSingleInstance->release(i); // Release each I2C bus
        }
        delete theSingleInstance;    // Delete the singleton instance
        theSingleInstance = nullptr; // Nullify the instance pointer
    }
}

// Initialize an I2C bus (set up SDA and SCL pins)
bool I2CMaster::init(int16_t i2c_num, int16_t i2c_sda, int16_t i2c_scl)
{
    // Assert that the provided parameters are within valid ranges
    assert(i2c_num >= 0 && i2c_num < I2C_NUM_MAX);
    assert(i2c_sda >= 0 && i2c_sda < GPIO_NUM_MAX);
    assert(i2c_scl >= 0 && i2c_scl < GPIO_NUM_MAX);

    // Check if the specified I2C bus is already initialized (pins already set)
    if ((m_i2c[i2c_num].i2c_sda >= 0) || (m_i2c[i2c_num].i2c_scl >= 0))
        return false; // Initialization failed, bus already initialized

    // Store the SDA and SCL pin numbers for the specified I2C bus
    m_i2c[i2c_num].i2c_sda = i2c_sda;
    m_i2c[i2c_num].i2c_scl = i2c_scl;
    m_i2c[i2c_num].count = 0; // Reset the usage counter
    return true;              // Initialization successful
}

// Take/claim an I2C bus (initialize the ESP-IDF bus handle if not already done)
bool I2CMaster::take(int16_t i2c_num)
{
    // Assert that the I2C bus number is valid
    assert(i2c_num >= 0 && i2c_num < I2C_NUM_MAX);

    // Check if the specified I2C bus has been initialized (pins set via init())
    if ((m_i2c[i2c_num].i2c_sda < 0) || (m_i2c[i2c_num].i2c_scl < 0))
        return false; // Cannot take, bus not initialized

    lock(); // Lock access to the shared resource (m_i2c array and bus_handle)

    // Check if this is the first time the bus is being taken (count == 0)
    if (m_i2c[i2c_num].count == 0)
    {
        // Configure the I2C master bus
        i2c_master_bus_config_t i2c_mst_config = {
            .i2c_port = i2c_num,                              // I2C port number
            .sda_io_num = (gpio_num_t)m_i2c[i2c_num].i2c_sda, // SDA pin
            .scl_io_num = (gpio_num_t)m_i2c[i2c_num].i2c_scl, // SCL pin
            .clk_source = I2C_CLK_SRC_DEFAULT,                // Clock source
            .glitch_ignore_cnt = 7,                           // Glitch filter count
            .intr_priority = 0,                               // Interrupt priority
            .trans_queue_depth = 0,                           // Transaction queue depth
            .flags = {1, 0}                                   // Flags (enable, disable internal pullup)
        };

        // Initialize the I2C master bus using ESP-IDF API
        esp_err_t err = i2c_new_master_bus(&i2c_mst_config, &m_i2c[i2c_num].bus_handle);
        if (err != ESP_OK)
        {
            unlock();                                    // Unlock before returning failure
            ESP_LOGE(TAG, "Init I2C%d failed", i2c_num); // Log error
            return false;                                // Initialization failed
        }
    }
    m_i2c[i2c_num].count++; // Increment the usage counter
    unlock();               // Unlock access to the shared resource
    return true;            // Take successful
}

// Check for the presence of a device on the I2C bus
bool I2CMaster::probe(int16_t i2c_num, uint16_t address)
{
    // Assert that the I2C bus number is valid
    assert(i2c_num >= 0 && i2c_num < I2C_NUM_MAX);

    // Check if the specified I2C bus has been initialized (pins set via init())
    if ((m_i2c[i2c_num].i2c_sda < 0) || (m_i2c[i2c_num].i2c_scl < 0))
        return false; // Cannot probe, bus not initialized

    lock(); // Lock access to the shared resource (bus_handle)

    // Check if the bus handle exists (bus is taken/active)
    if (m_i2c[i2c_num].count != 0)
    {
        // Probe the I2C bus for a device at the given address using ESP-IDF API
        esp_err_t err = i2c_master_probe(m_i2c[i2c_num].bus_handle, address, -1); // -1 for default timeout
        unlock();                                                                 // Unlock access to the shared resource
        return err == ESP_OK;                                                     // Return true if probe was successful (ESP_OK)
    }
    else
    {
        unlock();     // Unlock access to the shared resource
        return false; // Cannot probe, bus handle doesn't exist (not taken)
    }
}

// Add a device to the I2C bus
bool I2CMaster::add(int16_t i2c_num, const i2c_device_config_t *dev_config, i2c_master_dev_handle_t *ret_handle)
{
    // Assert that the I2C bus number is valid and device config is not null
    assert(i2c_num >= 0 && i2c_num < I2C_NUM_MAX);
    assert(dev_config != nullptr);

    // Check if the specified I2C bus has been initialized (pins set via init())
    if ((m_i2c[i2c_num].i2c_sda < 0) || (m_i2c[i2c_num].i2c_scl < 0))
        return false; // Cannot add device, bus not initialized

    lock(); // Lock access to the shared resource (bus_handle)

    // Check if the bus handle exists (bus is taken/active)
    if (m_i2c[i2c_num].count != 0)
    {
        // Add the device to the I2C master bus using ESP-IDF API
        esp_err_t err = i2c_master_bus_add_device(m_i2c[i2c_num].bus_handle, dev_config, ret_handle);
        if (err != ESP_OK)
        {
            unlock();                                           // Unlock before returning failure
            ESP_LOGE(TAG, "I2CMaster::add %d failed", i2c_num); // Log error
            return false;                                       // Adding device failed
        }
        unlock();    // Unlock access to the shared resource
        return true; // Adding device successful
    }
    else
    {
        unlock();     // Unlock access to the shared resource
        return false; // Cannot add device, bus handle doesn't exist (not taken)
    }
}

// Release an I2C bus (deinitialize the ESP-IDF bus handle if no longer needed)
void I2CMaster::release(int16_t i2c_num)
{
    // Assert that the I2C bus number is valid
    assert(i2c_num >= 0 && i2c_num < I2C_NUM_MAX);

    lock(); // Lock access to the shared resource (m_i2c array and bus_handle)

    // Check if the bus is currently taken (count > 0)
    if (m_i2c[i2c_num].count != 0)
    {
        m_i2c[i2c_num].count--; // Decrement the usage counter

        // Check if this was the last user of the bus (count reached 0)
        if (m_i2c[i2c_num].count == 0)
        {
            // Delete the I2C master bus handle using ESP-IDF API as it's no longer needed
            esp_err_t err = i2c_del_master_bus(m_i2c[i2c_num].bus_handle);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Delete I2C%d failed", i2c_num); // Log error
            }
        }
    }
    unlock(); // Unlock access to the shared resource
}