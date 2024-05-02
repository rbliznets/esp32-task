/*!
	\file
	\brief Базовый класс для захвата ресурса задач FreeRTOS.
    \authors Близнец Р.А. (r.bliznets@gmail.com)
	\version 1.1.0.0
	\date 28.04.2020
*/

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/// Базовый класс для захвата ресурса.
class CLock
{
protected:
	SemaphoreHandle_t mMutex = nullptr; ///< Хэндлер мьютекса.

	/// Инициализация новыми параметрами.
	/*!
	  \param[in] mutex Указатель на на семафор для мьютекса.
	*/
	void init(SemaphoreHandle_t mutex)
	{
		mMutex = mutex;
	};

	/// Захват ресурса.
	void lock();
	/// Освобождение ресурса.
	void unlock();

public:
	/// Конструктор класса.
	CLock();
};
