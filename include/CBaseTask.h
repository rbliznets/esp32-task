/*!
	\file
	\brief Base class for implementing FreeRTOS tasks on multi-core CPUs.
	\authors Bliznets R.A. (r.bliznets@gmail.com)
	\version 1.3.0.0
	\date 28.04.2020
	\details This header file defines the CBaseTask class, an abstract base class
			 for creating FreeRTOS tasks. It provides mechanisms for message passing
			 via queues, task lifecycle management, and integration with FreeRTOS
			 features like task notifications and CPU core affinity.
*/

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define MSG_END_TASK (0)        ///< Force task termination

/// Structure representing a message passed between tasks.
/// It uses a union to allow different interpretations of the data payload.
struct STaskMessage
{
	uint16_t msgID;		 ///< Type or ID of the message.
	uint16_t shortParam; ///< Short parameter associated with the message command.

	// Union to provide multiple ways to access the payload data.
	union
	{
		// Structure allowing access via two 16-bit parameters.
		struct
		{
			uint16_t param1; ///< First 16-bit parameter of the message.
			uint16_t param2; ///< Second 16-bit parameter of the message.
		};

		// Single 32-bit parameter.
		uint32_t paramID; ///< A single 32-bit parameter.

		// Pointer to a larger data block.
		void *msgBody; ///< Pointer to a larger message body or data payload.
	};
};

/// Base abstract class for implementing FreeRTOS tasks.
/// @details This class provides a framework for creating tasks with built-in message queues.
///          Derived classes must implement the `run()` method containing the task's main logic.
class CBaseTask
{
protected:
	/// FreeRTOS task handle for this instance.
	TaskHandle_t mTaskHandle = nullptr;

	/// FreeRTOS queue handle for receiving messages.
	QueueHandle_t mTaskQueue = nullptr;

	/// Notification flag used with FreeRTOS task notifications.
	/// If 0, task notifications are not used when sending messages.
	uint32_t mNotify = 0;

	/// Static FreeRTOS task function.
	/// @details This is the entry point function passed to FreeRTOS when creating the task.
	///          It calls the virtual `run()` method of the associated CBaseTask object.
	/// @param[in] pvParameters Pointer to the CBaseTask instance (this).
	static void vTask(void *pvParameters);

	/// Virtual function for the task's main execution logic.
	/// @details This must be overridden by derived classes to define the task's behavior.
	virtual void run() = 0;

	/// Receive a message from the task's queue.
	/// @param[out] msg Pointer to the STaskMessage structure to store the received message.
	/// @param[in] xTicksToWait Maximum time to wait for a message, in FreeRTOS ticks.
	///                         Use 0 for non-blocking receive.
	/// @return true if a message was successfully received, false if the timeout expired.
	bool getMessage(STaskMessage *msg, TickType_t xTicksToWait = 0);

public:
	/// Initial task setup.
	/// @details Creates the message queue and starts the FreeRTOS task pinned to a specific core.
	/// @param[in] name Name of the task (length must be less than configMAX_TASK_NAME_LEN).
	/// @param[in] usStack Size of the task stack in words (4 bytes each).
	/// @param[in] uxPriority Priority of the task (must be less than configMAX_PRIORITIES).
	/// @param[in] queueLength Maximum length of the message queue.
	/// @param[in] coreID CPU core ID to pin the task to (0, 1, or tskNO_AFFINITY for no affinity).
	void init(const char *name, unsigned short usStack, UBaseType_t uxPriority, UBaseType_t queueLength, BaseType_t coreID = tskNO_AFFINITY);

	/// Destructor.
	/// @details Ensures the associated FreeRTOS task and queue are cleaned up if possible.
	virtual ~CBaseTask();

	/// Send a message to the task from an Interrupt Service Routine (ISR).
	/// @param[in] msg Pointer to the message to send.
	/// @param[out] pxHigherPriorityTaskWoken Set to pdTRUE if sending the message caused a higher priority task to become unblocked.
	/// @return true if the message was successfully sent from the ISR, false otherwise.
	bool sendMessageFromISR(STaskMessage *msg, BaseType_t *pxHigherPriorityTaskWoken);

	/// Send a message to the task.
	/// @param[in] msg Pointer to the message to send.
	/// @param[in] xTicksToWait Maximum time to wait for the queue to have space, in FreeRTOS ticks.
	/// @param[in] free If true, frees the memory pointed to by `msg->msgBody` if sending fails.
	/// @return true if the message was successfully sent, false otherwise.
	bool sendMessage(STaskMessage *msg, TickType_t xTicksToWait = 0, bool free = false);

	/// Send a message to the front of the task's queue from an Interrupt Service Routine (ISR).
	/// @param[in] msg Pointer to the message to send.
	/// @param[out] pxHigherPriorityTaskWoken Set to pdTRUE if sending the message caused a higher priority task to become unblocked.
	/// @return true if the message was successfully sent from the ISR, false otherwise.
	bool sendMessageFrontFromISR(STaskMessage *msg, BaseType_t *pxHigherPriorityTaskWoken);

	/// Send a message to the front of the task's queue (highest priority).
	/// @param[in] msg Pointer to the message to send.
	/// @param[in] xTicksToWait Maximum time to wait for the queue to have space, in FreeRTOS ticks.
	/// @param[in] free_mem If true, frees the memory pointed to by `msg->msgBody` if sending fails.
	/// @return true if the message was successfully sent, false otherwise.
	bool sendMessageFront(STaskMessage *msg, TickType_t xTicksToWait = 0, bool free_mem = false);

	/// Send a simple command message to the task (inline helper).
	/// @details This is a convenience function for sending messages without a separate body.
	/// @param[in] msgID Type or ID of the message.
	/// @param[in] shortParam Short parameter associated with the command.
	/// @param[in] paramID 32-bit parameter associated with the command.
	/// @param[in] xTicksToWait Maximum time to wait for the queue to have space, in FreeRTOS ticks.
	/// @return true if the message was successfully sent, false otherwise.
	inline bool sendCmd(uint16_t msgID, uint16_t shortParam = 0, uint32_t paramID = 0, TickType_t xTicksToWait = 0)
	{
		STaskMessage msg;
		msg.msgID = msgID;
		msg.shortParam = shortParam;
		msg.paramID = paramID;
		return sendMessage(&msg, xTicksToWait, false); // Sends without freeing memory on failure
	}

	/// Allocate memory for the message body.
	/// @details Allocates memory for the `msgBody` field of an STaskMessage structure.
	///          Optionally allocates from PSRAM if available.
	/// @param[in] msg Pointer to the STaskMessage structure to initialize.
	/// @param[in] cmd Command ID (assigned to msgID).
	/// @param[in] size Size of the memory block to allocate.
	/// @param[in] psram If true, attempts to allocate from PSRAM; otherwise uses standard heap.
	/// @return Pointer to the allocated memory block, or nullptr on failure.
	static uint8_t *allocNewMsg(STaskMessage *msg, uint16_t cmd, uint16_t size, bool psram = false);

	/// Check if the task is running.
	/// @details A task is considered running if its message queue handle is valid.
	/// @return true if the task is running (mTaskQueue is not nullptr), false otherwise.
	inline bool isRun() { return mTaskQueue != nullptr; };

	/// Get the FreeRTOS task handle.
	/// @return The TaskHandle_t associated with this task instance.
	inline TaskHandle_t getTask() { return mTaskHandle; };
};