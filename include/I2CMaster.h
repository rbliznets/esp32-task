/*!
    \file
    \brief Класс инициализации i2c в режиме мастер.
    \authors Близнец Р.А. (r.bliznets@gmail.com)
    \version 1.0.0.0
    \date 11.11.2024
*/

#pragma once
#include "driver/i2c_master.h"
#include "CLock.h"

/// @brief состояние I2C
struct SI2CMaster
{
    int16_t i2c_sda = -1;               ///< GPIO для I2C SDA
    int16_t i2c_scl = -1;               ///< GPIO для I2C SCL
    int16_t count = 0;                  ///< Количество инициализаций
    i2c_master_bus_handle_t bus_handle; ///< I2C bus handle
};

/// @brief Класс инициализации i2c в режиме мастер.
class I2CMaster : public CLock
{
private:
    static I2CMaster *theSingleInstance; ///< Единственный экземпляр

protected:
    SI2CMaster m_i2c[I2C_NUM_MAX]; ///< Структура для хранения параметров

    /// @brief Конструктор
    I2CMaster();
    /// @brief Деструктор
    ~I2CMaster();

public:
    /// @brief  Единственный экземпляр класса.
    /// @return Указатель на I2CMaster
    static I2CMaster *Instance();
    /// @brief Освобождение ресурсов задачи
    static void free();

    /// @brief Инициализация I2C
    /// @param i2c_num Номер I2C
    /// @param i2c_sda GPIO для I2C SDA
    /// @param i2c_scl GPIO для I2C SCL
    /// @return true - успешно, false - ошибка
    bool init(int16_t i2c_num, int16_t i2c_sda, int16_t i2c_scl);

    /// @brief зарегистрировать использование I2C
    /// @param i2c_num Номер I2C
    /// @return true - успешно, false - ошибка
    bool take(int16_t i2c_num);

    /// @brief Проверить наличие устройства
    /// @param i2c_num Номер I2C
    /// @param address Адрес устройства
    /// @return true - успешно, false - ошибка
    bool probe(int16_t i2c_num, uint16_t address);

    /// @brief добавить устройство
    /// @param i2c_num Номер I2C
    /// @param dev_config конфигурация устройства
    /// @param ret_handle указатель на возвращаемый handle
    /// @return true - успешно, false - ошибка
    bool add(int16_t i2c_num, const i2c_device_config_t *dev_config, i2c_master_dev_handle_t *ret_handle);

    /// @brief отменить регистрацию устройства
    /// @param i2c_num Номер I2C
    void release(int16_t i2c_num);
};