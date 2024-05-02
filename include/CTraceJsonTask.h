/*!
	\file
	\brief Класс для вывода отладочной информации в json формате.
	\authors Близнец Р.А.(r.bliznets@gmail.com)
	\version 1.0.0.0
	\date 02.05.2024

	Один объект на приложение.
	Необходим чтобы не блокировать отлаживаемую задачу.
*/

#pragma once

#include "CTraceTask.h"
#include <string>

/// Класс задачи вывода отладочной информации в json формате.
class CTraceJsonTask : public CTraceTask
{
protected:
	std::string mAnswer; ///< Строка json.

	/// Вывести интервал времени с предыдущего сообщения
	/*!
	  \param[in] time Сообщение об ошибке.
	  \param[in] n количество для усреднения.
	*/
	void printHeader(uint64_t time, uint32_t n = 1) override;
	/// Вывести сообщение.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_STRING или MSG_TRACE_STRING_REBOOT.
	*/
	void printString(char *data) override;
	/// Вывести сообщение.
	/*!
	  \param[in] str Указатель строку.
	*/
	void printS(char *str) override;
	/// Вывести сообщение об интервале времени.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_STOP_TIME.
	*/
	void printStop(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_UINT8.
	*/
	void printData8h(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_UINT8.
	*/
	void printData8h_2(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_UINT16.
	*/
	void printData16h(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_UINT16.
	*/
	void printData16h_2(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_UINT32.
	*/
	void printData32h(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_UINT32.
	*/
	void printData32h_2(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_INT8.
	*/
	void printData8(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_INT8.
	*/
	void printData8_2(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_INT16.
	*/
	void printData16(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_INT16.
	*/
	void printData16_2(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE_INT32.
	*/
	void printData32(char *data) override;
	/// Вывести массив.
	/*!
	  \param[in] data Указатель на тело сообщения MSG_TRACE2_INT32.
	*/
	void printData32_2(char *data) override;
	
	/// Деструктор.
	virtual ~CTraceJsonTask(){};

public:
	/// Единственный экземпляр класса.
	/*!
	  \return Указатель на CTraceJsonTask
	*/
	static CTraceJsonTask *Instance()
	{
		static CTraceJsonTask theSingleInstance;
		return &theSingleInstance;
	}
};
