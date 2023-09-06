/*!
	\file
	\brief Базовый класс для реализации задачи FreeRTOS в многоядерном CPU.
	\authors Близнец Р.А.
	\version 1.1.0.1
	\date 28.04.2020
*/

#include "CBaseTask.h"
#include <cstdio>
#include <cstring>
#include "sdkconfig.h"
#include "esp_log.h"

static const char *TAG = "BaseTask";

void CBaseTask::vTask( void *pvParameters )
{
	((CBaseTask*)pvParameters)->run();
	vQueueDelete(((CBaseTask*)pvParameters)->mTaskQueue);
	((CBaseTask*)pvParameters)->mTaskQueue=nullptr;
	ESP_LOGI(TAG, "%s exit", pcTaskGetName(((CBaseTask*)pvParameters)->mTaskHandle));
#if (INCLUDE_vTaskDelete == 1)//????
	((CBaseTask*)pvParameters)->mTaskHandle=nullptr;
	vTaskDelete(nullptr);
#else
	for(;;) vTaskDelay(pdMS_TO_TICKS(1000));
#endif
}

CBaseTask::~CBaseTask()
{
#if (INCLUDE_vTaskDelete == 1)//????
	if(mTaskHandle != nullptr)
	{
		vTaskDelete(mTaskHandle);
		if(mTaskQueue != nullptr) vQueueDelete(mTaskQueue);
	}
#endif
}

void CBaseTask::init(const char * name,unsigned short usStack, UBaseType_t uxPriority, UBaseType_t queueLength, BaseType_t coreID)
{
#ifdef CONFIG_EXT_CHECK
	configASSERT(uxPriority <=  configMAX_PRIORITIES);
	configASSERT(usStack >=  configMINIMAL_STACK_SIZE);
	configASSERT(std::strlen(name) <  configMAX_TASK_NAME_LEN);
#endif
	mTaskQueue=xQueueCreate( queueLength, sizeof(STaskMessage) );
	xTaskCreatePinnedToCore( vTask, name, usStack, this, uxPriority, &mTaskHandle, coreID );
}

bool CBaseTask::sendMessage(STaskMessage* msg,uint32_t nFlag, TickType_t xTicksToWait, bool free_mem)
{
#ifdef CONFIG_EXT_CHECK
	configASSERT(msg !=  nullptr);
#endif
	if(xQueueSend(mTaskQueue,msg,xTicksToWait)==pdPASS)
	{
		if(nFlag != 0)
		{
			return(xTaskNotify(mTaskHandle,nFlag,eSetBits) == pdPASS);
		}
		else return true;
	}
	else
	{
		if(free_mem) vPortFree(msg->msgBody);
		ESP_LOGW(TAG,"%s:SendMessage failed",pcTaskGetName(mTaskHandle));
		return false;
	}
}

bool CBaseTask::sendMessageFront(STaskMessage* msg,uint32_t nFlag, TickType_t xTicksToWait, bool free_mem)
{
#ifdef CONFIG_EXT_CHECK
	configASSERT(msg !=  nullptr);
#endif
	if(xQueueSendToFront(mTaskQueue,msg,xTicksToWait)==pdPASS)
	{
		if(nFlag != 0)
		{
			return(xTaskNotify(mTaskHandle,nFlag,eSetBits) == pdPASS);
		}
		else return true;
	}
	else
	{
		if(free_mem) vPortFree(msg->msgBody);
		ESP_LOGW(TAG,"%s:SendMessage failed",pcTaskGetName(mTaskHandle));
		return false;
	}
}

bool IRAM_ATTR CBaseTask::sendMessageFromISR(STaskMessage* msg, BaseType_t *pxHigherPriorityTaskWoken,uint32_t nFlag)
{
#ifdef CONFIG_EXT_CHECK
	configASSERT(msg !=  nullptr);
#endif
	if(xQueueSendFromISR(mTaskQueue,msg,pxHigherPriorityTaskWoken)==pdPASS)
	{
		if(nFlag != 0)
		{
			return(xTaskNotifyFromISR(mTaskHandle,nFlag,eSetBits,pxHigherPriorityTaskWoken) == pdPASS);
		}
		else return true;
	}
	else return false;
}

bool CBaseTask::getMessage(STaskMessage* msg, TickType_t xTicksToWait)
{
#ifdef CONFIG_EXT_CHECK
	configASSERT(msg !=  nullptr);
#endif
	return (xQueueReceive(mTaskQueue,msg,xTicksToWait)== pdTRUE);
}

uint8_t* CBaseTask::allocNewMsg(STaskMessage* msg, uint16_t cmd, uint16_t size)
{
#ifdef CONFIG_EXT_CHECK
	configASSERT(msg !=  NULL);
	configASSERT(size >  0);
#endif
	msg->msgID=cmd;
	msg->shortParam=size;
	msg->msgBody=pvPortMalloc(msg->shortParam);
	return (uint8_t*)msg->msgBody;
}

