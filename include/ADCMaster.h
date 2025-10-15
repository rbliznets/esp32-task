/*!
    \file
    \brief ADC initialization class in one-shot 12-bit mode.
    \authors Bliznets R.A. (r.bliznets@gmail.com)
    \version 1.0.0.0
    \date 23.11.2024
    \details This class manages ADC resources using the Singleton pattern for ESP-IDF.
             It provides thread-safe initialization, configuration, and reading from
             ADC channels in single-shot mode with 12-bit resolution.
             It supports two ADC units (typically ADC1 and ADC2 on ESP32).
*/

#pragma once
#include "esp_adc/adc_oneshot.h" // Include the ESP-IDF one-shot ADC driver header
#include "CLock.h"               // Include the custom lock mechanism header

/// @brief Structure to hold state and handle for each ADC unit
struct SADCMaster
{
    int16_t count = 0;                    ///< Counter for the number of active users/channels for this unit.
                                          ///< Used for reference counting to manage resource allocation/deallocation.
    adc_oneshot_unit_handle_t adc_handle; ///< Handle for the ESP-IDF ADC one-shot unit instance.
                                          ///< This handle is used in calls to the ADC driver functions for this unit.
};

/// @brief ADC initialization class in one-shot 12-bit mode.
/// @details This class implements the Singleton pattern to ensure only one instance
///          manages the ADC resources. It provides methods to initialize ADC units and channels,
///          read values, and release resources safely across multiple threads using CLock.
class ADCMaster : public CLock // Inherits locking mechanism from CLock
{
private:
    /// @brief Static pointer to the single instance of this class.
    /// @details Initialized to nullptr. Instance() method ensures only one object exists.
    static ADCMaster *theSingleInstance;

protected:
    /// @brief Array to store state information for two ADC units (e.g., ADC1, ADC2).
    /// @details Index 0 likely corresponds to ADC_UNIT_1, index 1 to ADC_UNIT_2.
    SADCMaster m_adc[2];

    /// @brief Private constructor.
    /// @details Initializes the base CLock object. Made private to enforce Singleton pattern.
    ///          Instance() should be used to get the object.
    ADCMaster();

    /// @brief Private destructor.
    /// @details Made private to control deletion. Resources should be freed using the static free() method.
    ///          The free() method calls release() on all units and then deletes the instance.
    ~ADCMaster();

public:
    /// @brief Get the single instance of the ADCMaster class.
    /// @details If the instance doesn't exist, it creates one. Otherwise, it returns the existing one.
    /// @return Pointer to the single ADCMaster instance.
    static ADCMaster *Instance();

    /// @brief Frees resources allocated by ADCMaster and deletes the singleton instance.
    /// @details This method should be called when ADC usage is finished to clean up handles
    ///          and allow the ADC peripheral to be reinitialized if needed later.
    ///          It calls release() for both potential ADC units before deleting itself.
    static void free();

    /// @brief Initializes and configures a specific ADC channel on the given unit.
    /// @details This method locks access, checks if the ADC unit handle exists,
    ///          creates the unit handle if necessary, configures the specified channel
    ///          with default settings (12-bit, 12dB attenuation), and increments the unit's usage counter.
    /// @param adc_num The ADC unit number (e.g., ADC_UNIT_1, ADC_UNIT_2).
    /// @param channel The ADC channel number to configure on the specified unit.
    /// @return true if initialization and configuration were successful, false otherwise.
    bool take(adc_unit_t adc_num, adc_channel_t channel);

    /// @brief Initializes an ADC unit without configuring a specific channel.
    /// @details This method locks access, checks if the ADC unit handle exists,
    ///          creates the unit handle if necessary, and increments the unit's usage counter.
    ///          It allows external code to manage channels directly after the unit is initialized.
    /// @param adc_num The ADC unit number (e.g., ADC_UNIT_1, ADC_UNIT_2) to initialize.
    /// @return Pointer to the ADC unit handle if successful, nullptr otherwise.
    adc_oneshot_unit_handle_t *take(adc_unit_t adc_num);

    /// @brief Reads a value from the specified ADC channel.
    /// @details Performs the read operation using the stored handle for the unit.
    ///          Retries on timeout errors (ESP_ERR_TIMEOUT). Handles other errors by logging and returning false.
    /// @param adc_num The ADC unit number where the channel resides.
    /// @param channel The ADC channel number to read from.
    /// @param[out] value Reference to an integer where the read ADC value (0-4095 for 12-bit) will be stored.
    /// @return true if the read was successful, false if an error occurred (other than timeout).
    bool read(adc_unit_t adc_num, adc_channel_t channel, int &value);

    /// @brief Decrements the usage counter for the specified ADC unit and deletes the unit handle if counter reaches zero.
    /// @details This method implements the reference counting mechanism. When `take()` is called,
    ///          the counter is incremented. When `release()` is called, the counter is decremented.
    ///          If the counter reaches zero, it means no channels on this unit are in use,
    ///          and the underlying ESP-IDF ADC unit handle is deleted, freeing the hardware resource.
    /// @param adc_num The ADC unit number to release.
    void release(adc_unit_t adc_num);
};