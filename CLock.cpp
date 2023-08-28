/*!
	\file
	\brief Базовый класс для захвата ресурса задач FreeRTOS.
	\authors Близнец Р.А.
	\version 1.1.0.0
	\date 28.04.2020
*/

#include "CLock.h"

CLock::CLock()
{
}

void CLock::lock()
{
	if(mMutex != NULL)xSemaphoreTake(mMutex, portMAX_DELAY);
}

void CLock::unlock()
{
	if(mMutex != NULL)xSemaphoreGive(mMutex);
}
