/*!
	\file
	\brief Базовый класс для захвата ресурса задач FreeRTOS.
    \authors Близнец Р.А. (r.bliznets@gmail.com)
	\version 1.1.0.0
	\date 28.04.2020
*/

#include "CLock.h"

CLock::CLock()
{
}

void CLock::lock()
{
	if (mMutex != nullptr)
		xSemaphoreTake(mMutex, portMAX_DELAY);
}

void CLock::unlock()
{
	if (mMutex != nullptr)
		xSemaphoreGive(mMutex);
}
