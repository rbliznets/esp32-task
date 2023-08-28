/*!
	\file
	\brief Программный таймер под задачи FreeRTOS.
	\authors Близнец Р.А.
	\version 1.1.0.0
	\date 28.04.2020
*/

#if !defined CSOFTWARETIMER_H
#define CSOFTWARETIMER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

/// Программный таймер под задачи FreeRTOS.
class CSoftwareTimer
{
protected:
	TimerHandle_t mTimerHandle=nullptr; 	///< Хэндлер таймера FreeRTOS.
	TaskHandle_t mTaskToNotify; 		///< Указатель на задачу, ожидающую события от таймера.
	uint8_t mNotifyBit;					///< Номер бита для оповещения задачи о событии таймера (не более 31).
	bool mAutoRefresh;					///< Флаг автозагрузки таймера.

	/// Обработчик события таймера.
	/*!
	\param[in] xTimer Хэндлер таймера FreeRTOS.
	*/
	static void vTimerCallback( TimerHandle_t xTimer );

	/// Функция, вызываемая по событию в таймере.
	void timer();
public:
	/// Запуск таймера.
	/*!
  	  \warning Вызывать только из задачи FreeRTOS.
	  \param[in] xNotifyBit Номер бита для оповещения задачи о событии таймера.
	  \param[in] period Период в миллисекундах.
	  \param[in] autoRefresh Флаг автозагрузки таймера. Если false, то таймер запускается один раз.
	  \return 0 - в случае успеха.
	  \sa Stop()
	*/
	int start(uint8_t xNotifyBit, uint32_t period, bool autoRefresh=false);
	/// Остановка таймера.
	/*!
	  \return 0 - в случае успеха.
	  \sa Start()
	*/
	int stop();

	/// Состояние таймера.
	/*!
	  \return Состояние таймера.
	*/
	bool is_run()
	{
		return mTimerHandle != nullptr;
	};
};

#endif // CSOFTWARETIMER_H

