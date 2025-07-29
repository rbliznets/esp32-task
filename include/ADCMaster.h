/*!
    \file
    \brief Класс инициализации adc в режиме one short 12bit.
    \authors Близнец Р.А. (r.bliznets@gmail.com)
    \version 0.1.0.0
    \date 23.11.2024
*/

#pragma once
#include "esp_adc/adc_oneshot.h"
#include "CLock.h"

/// @brief состояние ADC
struct SADCMaster
{
    int16_t count = 0;                    ///< Количество инициализаций
    adc_oneshot_unit_handle_t adc_handle; ///< Хэндлер модуля adc
};

/// @brief Класс инициализации adc в режиме one short 12bit.
class ADCMaster : public CLock
{
private:
    static ADCMaster *theSingleInstance; ///< Единственный экземпляр

protected:
    SADCMaster m_adc[2]; ///< Структура для хранения параметров

    /// @brief Конструктор
    ADCMaster();
    /// @brief Деструктор
    ~ADCMaster();

public:
    /// @brief  Единственный экземпляр класса.
    /// @return Указатель на ADCMaster
    static ADCMaster *Instance();
    /// @brief Освобождение ресурсов задачи
    static void free();

    /// @brief зарегистрировать использование ADC
    /// @param adc_num Номер ADC
    /// @param channel Канал ADC
    /// @return true - успешно, false - ошибка
    bool take(adc_unit_t adc_num, adc_channel_t channel);

    adc_oneshot_unit_handle_t* take(adc_unit_t adc_num);

    /// @brief считать значение ADC
    /// @param adc_num Номер ADC
    /// @param channel Канал ADC
    /// @param value значение
    /// @return true - успешно, false - ошибка
    bool read(adc_unit_t adc_num, adc_channel_t channel, int &value);

    /// @brief отменить регистрацию устройства
    /// @param adc_num Номер ADC
    void release(adc_unit_t adc_num);
};