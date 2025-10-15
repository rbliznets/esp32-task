/*!
    \file
    \brief Class for initializing I2C in master mode.
    \authors Bliznets R.A. (r.bliznets@gmail.com)
    \version 1.0.0.0
    \date 11.11.2024
    \details This header file defines the I2CMaster class, which implements the Singleton pattern
             to manage I2C master resources (buses and devices) across the application.
             It provides methods to initialize I2C buses (configure SDA/SCL pins), claim usage of a bus,
             probe for devices, add device handles, and release resources safely across multiple threads
             using a lock mechanism inherited from CLock.
*/

#pragma once
#include "driver/i2c_master.h" // Include the ESP-IDF I2C master driver header
#include "CLock.h"             // Include the custom lock mechanism header

/// @brief Structure to hold state and handle for each I2C bus
struct SI2CMaster
{
    int16_t i2c_sda = -1;               ///< GPIO pin number for I2C SDA line. -1 if not configured.
    int16_t i2c_scl = -1;               ///< GPIO pin number for I2C SCL line. -1 if not configured.
    int16_t count = 0;                  ///< Reference count for the number of active users of this bus.
                                        ///< Used for resource management (initialize once, deinitialize when count reaches 0).
    i2c_master_bus_handle_t bus_handle; ///< Handle for the ESP-IDF I2C master bus instance.
                                        ///< This handle is used in calls to the I2C driver functions for this bus.
};

/// @brief Class for initializing I2C in master mode.
/// @details This class inherits from CLock to provide thread safety for its operations.
///          It manages I2C buses identified by `i2c_num`. The `init` method sets up the
///          pins for a bus. The `take` method creates the ESP-IDF bus handle if needed.
///          The `add` method creates device handles for specific addresses on the bus.
///          The `release` method decrements the usage counter and deletes the bus handle
///          when the counter reaches zero. The `probe` method checks for device presence.
class I2CMaster : public CLock // Inherits locking mechanism from CLock
{
private:
    /// @brief Static pointer to the single instance of this class.
    /// @details Initialized to nullptr. Instance() method ensures only one object exists.
    static I2CMaster *theSingleInstance;

protected:
    /// @brief Array to store state information for all possible I2C buses (e.g., I2C_NUM_0, I2C_NUM_1).
    SI2CMaster m_i2c[I2C_NUM_MAX];

    /// @brief Private constructor.
    /// @details Initializes the base CLock object. Made private to enforce Singleton pattern.
    ///          Instance() should be used to get the object.
    I2CMaster();

    /// @brief Private destructor.
    /// @details Made private to control deletion. Resources should be freed using the static free() method.
    ///          The free() method calls release() on all buses and then deletes the instance.
    ~I2CMaster();

public:
    /// @brief  Get the single instance of the I2CMaster class.
    /// @details If the instance doesn't exist, it creates one. Otherwise, it returns the existing one.
    /// @return Pointer to the single I2CMaster instance.
    static I2CMaster *Instance();

    /// @brief Free task resources.
    /// @details This method should be called when I2C usage is finished to clean up handles
    ///          and allow the I2C peripherals to be reinitialized if needed later.
    ///          It calls release() for all possible I2C buses before deleting itself.
    static void free();

    /// @brief Initialize an I2C bus by setting its SDA and SCL pins.
    /// @details This method *must* be called before `take` for a specific `i2c_num`.
    ///          It stores the pin configuration but does not yet create the ESP-IDF bus handle.
    /// @param i2c_num The I2C bus number (e.g., I2C_NUM_0, I2C_NUM_1).
    /// @param i2c_sda GPIO pin number for the SDA line.
    /// @param i2c_scl GPIO pin number for the SCL line.
    /// @return true if initialization (pin assignment) was successful, false otherwise (e.g., if already initialized).
    bool init(int16_t i2c_num, int16_t i2c_sda, int16_t i2c_scl);

    /// @brief Claim usage of an I2C bus and initialize its ESP-IDF handle if necessary.
    /// @details This method locks access, checks if the bus is initialized via `init()`,
    ///          creates the ESP-IDF I2C master bus handle if it's the first user (`count` was 0),
    ///          and increments the usage counter (`count`).
    /// @param i2c_num The I2C bus number to claim.
    /// @return true if claiming and (if needed) initializing the bus handle was successful, false otherwise.
    bool take(int16_t i2c_num);

    /// @brief Probe the I2C bus for a device at the given address.
    /// @details Checks if a device responds to the specified address on the bus.
    /// @param i2c_num The I2C bus number where the device should be located.
    /// @param address The 7-bit I2C address of the device to probe.
    /// @return true if a device responded at the address, false otherwise or on error.
    bool probe(int16_t i2c_num, uint16_t address);

    /// @brief Add a device to the I2C bus and get its handle.
    /// @details This method creates an ESP-IDF device handle for a specific device on the bus.
    ///          The bus identified by `i2c_num` must have been claimed using `take()` first.
    /// @param i2c_num The I2C bus number where the device is located.
    /// @param dev_config Pointer to the configuration structure for the I2C device.
    /// @param ret_handle Pointer to an `i2c_master_dev_handle_t` where the resulting device handle will be stored.
    /// @return true if the device was successfully added and the handle returned, false otherwise.
    bool add(int16_t i2c_num, const i2c_device_config_t *dev_config, i2c_master_dev_handle_t *ret_handle);

    /// @brief Release usage of an I2C bus and delete its handle if no longer used.
    /// @details This method decrements the usage counter for the specified bus.
    ///          If the counter reaches zero, it means no devices on this bus are active,
    ///          and the underlying ESP-IDF I2C master bus handle is deleted, freeing the hardware resource.
    /// @param i2c_num The I2C bus number to release.
    void release(int16_t i2c_num);
};