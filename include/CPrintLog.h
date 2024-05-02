/*!
	\file
	\brief Класс журнала ошибок системы для консоли.
    \authors Близнец Р.А. (r.bliznets@gmail.com)
	\version 1.3.0.0
	\date 10.07.2020
*/

#pragma once

#include "ITraceLog.h"

/// Класс трассировки сообщения об ошибке для консоли
class CPrintLog : public ITraceLog
{
protected:
	char m_header[32]; ///< Буфер для времени

	/// Вывести интрвал времени с предыдущего собщения
	/*!
	  \param[in] time Сообщение об ошибке.
	  \param[in] n количество для усреднения.
	*/
	void printHeader(uint64_t time, uint32_t n = 1);

public:
	/// Конструктор
	CPrintLog() : ITraceLog(){};
	/// Виртуальный деструктор
	virtual ~CPrintLog() = default;

	/// Виртуальный метод трассировки
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] errCode Код ошибки.
	  \param[in] level Уровень вывода сообщения.
	  \param[in] reboot Флаг перезагрузки.
	*/
	void trace(const char *strError, int32_t errCode, esp_log_level_t level, bool reboot) override;

	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	void trace(const char *strError, uint8_t *data, uint32_t size) override;
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	void trace(const char *strError, int8_t *data, uint32_t size) override;

	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	void trace(const char *strError, uint16_t *data, uint32_t size) override;
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	void trace(const char *strError, int16_t *data, uint32_t size) override;

	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	void trace(const char *strError, uint32_t *data, uint32_t size) override;
	/// Виртуальный метод массива данных
	/*!
	  \param[in] strError Сообщение об ошибке.
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	void trace(const char *strError, int32_t *data, uint32_t size) override;

	/// Вывести интервал времени
	/*!
	  \param[in] str название интервала.
	  \param[in] n количество для усреднения.
	*/
	void stopTime(const char *str, uint32_t n = 1) override;

	/// Вывести сообщение
	/*!
	  \param[in] str Сообщение.
	*/
	inline void log(const char *str) override
	{
		if (str != nullptr)
			std::printf(str);
		std::printf("\n");
	};
};

