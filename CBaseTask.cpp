/*!
	\file
	\brief Base class for implementing FreeRTOS tasks on multi-core CPUs.
	\authors Bliznets R.A. (r.bliznets@gmail.com)
	\version 1.3.0.0
	\date 28.04.2020
	\details This class provides a framework for creating and managing FreeRTOS tasks.
			 It includes message queue handling, task lifecycle management,
			 and support for both standard task functions and ISR-safe functions.
*/
#include "CBaseTask.h"
#include <cstdio>
#include <cstring>
#include "sdkconfig.h"
#include "CTrace.h"
#include "esp_heap_caps.h"

// Static task function that serves as the entry point for the FreeRTOS task.
// This function is passed to xTaskCreatePinnedToCore and executes the main logic
// defined in the run() method of the CBaseTask instance.
// After run() completes, it cleans up the message queue and deletes the task itself
// (if vTaskDelete is supported).
void CBaseTask::vTask(void *pvParameters)
{
	// Cast the parameter back to CBaseTask* and call the instance's run method.
	((CBaseTask *)pvParameters)->run();

	// Delete the associated message queue after the task logic finishes.
	vQueueDelete(((CBaseTask *)pvParameters)->mTaskQueue);
	((CBaseTask *)pvParameters)->mTaskQueue = nullptr;

#if (INCLUDE_vTaskDelete == 1)
	// If task deletion is supported by FreeRTOS, log exit and delete the task.
	ESP_LOGD(pcTaskGetName(((CBaseTask *)pvParameters)->mTaskHandle), "exit");
	((CBaseTask *)pvParameters)->mTaskHandle = nullptr;
	vTaskDelete(nullptr); // Delete the currently running task.
#else
	// If task deletion is not supported, the task enters an infinite loop.
	for (;;)
		vTaskDelay(pdMS_TO_TICKS(1000)); // Delay to prevent excessive CPU usage in the loop.
#endif
}

// Destructor for the CBaseTask class.
// Ensures that the associated FreeRTOS task and its message queue are deleted
// if task deletion is supported by FreeRTOS.
CBaseTask::~CBaseTask()
{
#if (INCLUDE_vTaskDelete == 1)
	if (mTaskHandle != nullptr) // Check if the task handle is valid.
	{
		// Delete the message queue if it exists.
		if (mTaskQueue != nullptr)
			vQueueDelete(mTaskQueue);

		// Delete the FreeRTOS task itself.
		vTaskDelete(mTaskHandle);
	}
#endif
}

// Initializes the task by creating the message queue and starting the task in the FreeRTOS scheduler.
// Pins the task to the specified CPU core.
void CBaseTask::init(const char *name, unsigned short usStack, UBaseType_t uxPriority, UBaseType_t queueLength, BaseType_t coreID)
{
	// Validate input parameters against FreeRTOS configuration limits.
	assert(uxPriority <= configMAX_PRIORITIES);
	assert(usStack >= configMINIMAL_STACK_SIZE);
	assert(std::strlen(name) < configMAX_TASK_NAME_LEN);

	// Create the FreeRTOS queue for task messages.
	mTaskQueue = xQueueCreate(queueLength, sizeof(STaskMessage));

	// Create the FreeRTOS task, pin it to the specified core, and store its handle.
	xTaskCreatePinnedToCore(vTask, name, usStack, this, uxPriority, &mTaskHandle, coreID);
}

// Sends a message to the end of the task's queue.
// Optionally sends a task notification and frees memory on failure.
// Returns true if the message was successfully sent, false otherwise.
bool CBaseTask::sendMessage(STaskMessage *msg, TickType_t xTicksToWait, bool free_mem)
{
	assert(msg != nullptr); // Ensure the message pointer is valid.

	// Attempt to send the message to the back of the queue.
	if (xQueueSend(mTaskQueue, msg, xTicksToWait) == pdPASS)
	{
		// If notification is enabled, notify the task.
		if (mNotify != 0)
		{
			return (xTaskNotify(mTaskHandle, mNotify, eSetBits) == pdPASS);
		}
		else
			return true; // Message sent successfully without notification.
	}
	else
	{
		// If sending failed, free the message body memory if requested and log a warning.
		if (free_mem)
			vPortFree(msg->msgBody);
		TRACE_WARNING(pcTaskGetName(mTaskHandle), msg->msgID);
		return false;
	}
}

// Sends a message to the front of the task's queue (highest priority).
// Optionally sends a task notification and frees memory on failure.
// Returns true if the message was successfully sent, false otherwise.
bool CBaseTask::sendMessageFront(STaskMessage *msg, TickType_t xTicksToWait, bool free_mem)
{
	assert(msg != nullptr); // Ensure the message pointer is valid.

	// Attempt to send the message to the front of the queue.
	if (xQueueSendToFront(mTaskQueue, msg, xTicksToWait) == pdPASS)
	{
		// If notification is enabled, notify the task.
		if (mNotify != 0)
		{
			return (xTaskNotify(mTaskHandle, mNotify, eSetBits) == pdPASS);
		}
		else
			return true; // Message sent successfully without notification.
	}
	else
	{
		// If sending failed, free the message body memory if requested and log a warning.
		if (free_mem)
			vPortFree(msg->msgBody);
		TRACE_WARNING(pcTaskGetName(mTaskHandle), msg->msgID);
		return false;
	}
}

// Sends a message to the task's queue from an Interrupt Service Routine (ISR).
// This function is marked with IRAM_ATTR for faster execution from ISR context.
// Optionally sends a task notification from ISR context.
// Returns true if the message was successfully sent, false otherwise.
bool IRAM_ATTR CBaseTask::sendMessageFromISR(STaskMessage *msg, BaseType_t *pxHigherPriorityTaskWoken)
{
	assert(msg != nullptr); // Ensure the message pointer is valid.

	// Attempt to send the message to the back of the queue from ISR context.
	if (xQueueSendToBackFromISR(mTaskQueue, msg, pxHigherPriorityTaskWoken) == pdPASS)
	{
		// If notification is enabled, notify the task from ISR context.
		if (mNotify != 0)
		{
			if (xTaskNotifyFromISR(mTaskHandle, mNotify, eSetBits, pxHigherPriorityTaskWoken) == pdPASS)
			{
				return true;
			}
			else
			{
				// If notification failed, log the error from ISR context.
				TRACE_FROM_ISR("sendMessageFromISR2", msg->msgID, pxHigherPriorityTaskWoken);
				return false;
			}
		}
		else
			return true; // Message sent successfully without notification.
	}
	else
	{
		// If sending failed, log the error from ISR context.
		TRACE_FROM_ISR("sendMessageFromISR", msg->msgID, pxHigherPriorityTaskWoken);
		return false;
	}
}

// Sends a message to the front of the task's queue from an Interrupt Service Routine (ISR).
// This function is marked with IRAM_ATTR for faster execution from ISR context.
// Optionally sends a task notification from ISR context.
// Returns true if the message was successfully sent, false otherwise.
bool IRAM_ATTR CBaseTask::sendMessageFrontFromISR(STaskMessage *msg, BaseType_t *pxHigherPriorityTaskWoken)
{
	assert(msg != nullptr); // Ensure the message pointer is valid.

	// Attempt to send the message to the front of the queue from ISR context.
	if (xQueueSendToFrontFromISR(mTaskQueue, msg, pxHigherPriorityTaskWoken) == pdPASS)
	{
		// If notification is enabled, notify the task from ISR context.
		if (mNotify != 0)
		{
			if (xTaskNotifyFromISR(mTaskHandle, mNotify, eSetBits, pxHigherPriorityTaskWoken) == pdPASS)
			{
				return true;
			}
			else
			{
				// If notification failed, log the error from ISR context.
				TRACE_FROM_ISR("sendMessageFrontFromISR2", msg->msgID, pxHigherPriorityTaskWoken);
				return false;
			}
		}
		else
			return true; // Message sent successfully without notification.
	}
	else
	{
		// If sending failed, log the error from ISR context.
		TRACE_FROM_ISR("sendMessageFrontFromISR", msg->msgID, pxHigherPriorityTaskWoken);
		return false;
	}
}

// Receives a message from the task's queue.
// Blocks for up to xTicksToWait ticks if the queue is empty.
// Returns true if a message was successfully received, false if the timeout expired.
bool CBaseTask::getMessage(STaskMessage *msg, TickType_t xTicksToWait)
{
	assert(msg != nullptr); // Ensure the message pointer is valid for receiving data.

	// Attempt to receive a message from the queue.
	return (xQueueReceive(mTaskQueue, msg, xTicksToWait) == pdTRUE);
}

// Allocates memory for the message body and initializes the message header.
// Uses PSRAM if available and requested, otherwise uses standard heap.
// Returns a pointer to the allocated memory block for the message body.
uint8_t *CBaseTask::allocNewMsg(STaskMessage *msg, uint16_t cmd, uint16_t size, bool psram)
{
	assert(msg != nullptr); // Ensure the message pointer is valid.
	assert(size > 0);		// Size must be greater than zero.

	// Initialize the message header fields.
	msg->msgID = cmd;
	msg->shortParam = size;

#ifdef CONFIG_SPIRAM
	// If PSRAM is configured and requested, allocate memory from PSRAM.
	if (psram)
		msg->msgBody = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
	else
		// Otherwise, allocate from the default heap (likely DRAM).
		msg->msgBody = heap_caps_malloc(size, MALLOC_CAP_DEFAULT);
#else
	// If PSRAM is not configured, allocate from the standard FreeRTOS heap.
	msg->msgBody = pvPortMalloc(msg->shortParam);
#endif // CONFIG_SPIRAM

	// Return a pointer to the allocated message body memory.
	return (uint8_t *)msg->msgBody;
}