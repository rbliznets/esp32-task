/**
 * \file CTraceList.cpp
 * \brief This file contains the implementation of the CTraceList class for error logging.
 * \authors Bliznets R.A. (r.bliznets@gmail.com)
 * \version 1.3.1.0
 * \date 10.07.2020
 */

#include "CTrace.h"
#include "esp_system.h"
#include "esp_log.h"
#include "CPrintLog.h"
#include "CTraceTask.h"

#ifdef CONFIG_DEBUG_CODE
/// \brief Error log instance.
CTraceList traceLog;
#endif

#ifdef CONFIG_DEBUG_TRACE_PRINT
/// \brief Print log instance.
CPrintLog tracePrintLog;
#endif

static const char *TAG = "TraceList";

/**
 * \brief Constructor for the CTraceList class.
 * \details Initializes the base ITraceLog and CLock classes.
 *          Creates a binary semaphore (mutex) for thread safety.
 *          Calls esp_timer_early_init() for timer initialization.
 */
CTraceList::CTraceList() : ITraceLog(), CLock()
{
	if (esp_timer_early_init() != ESP_OK)
	{
		ESP_LOGE(TAG, "esp_timer_early_init error");
	}
	vSemaphoreCreateBinary(mMutex);
}

/**
 * \brief Destructor for the CTraceList class.
 * \details Clears the logger list and deletes the binary semaphore (mutex).
 */
CTraceList::~CTraceList()
{
	clear();
	vSemaphoreDelete(mMutex);
}

/**
 * \brief Initializes loggers.
 * \details Adds configured loggers (like CPrintLog or CTraceTask) to the internal list
 *          based on compile-time configuration flags (CONFIG_DEBUG_TRACE_PRINT, CONFIG_DEBUG_TRACE_TASK).
 */
void CTraceList::init()
{
#ifdef CONFIG_DEBUG_TRACE_PRINT
	ADDLOG(&tracePrintLog); // Add the print log
#endif

#ifdef CONFIG_DEBUG_TRACE_TASK
#ifdef CONFIG_DEBUG_TRACE_TASK0
	CTraceTask::Instance()->init(30, 0); // Initialize trace task on core 0
#else
	CTraceTask::Instance()->init(30, 1); // Initialize trace task on core 1
#endif
	ADDLOG(CTraceTask::Instance()); // Add the task log
#endif
}

/**
 * \brief Clears the list of errors/loggers.
 * \details Locks the object, clears the internal logger list, and unlocks.
 */
void CTraceList::clear()
{
	lock();
	m_list.clear(); // Assumes m_list is a container like std::list<ITraceLog*>
	unlock();
}

/**
 * \brief Outputs an error message.
 * \param strError Pointer to the error message string.
 * \param errCode The error code.
 * \param level The logging level (e.g., ESP_LOGE, ESP_LOGW).
 * \param reboot Flag indicating whether the device should reboot after logging.
 * \details Locks the object, iterates through all registered loggers and calls their trace method,
 *          unlocks, and optionally reboots the device.
 */
void CTraceList::trace(const char *strError, int32_t errCode, esp_log_level_t level, bool reboot)
{
	lock();
	for (auto &x : m_list) // Iterate through the list of loggers
	{
		x->trace(strError, errCode, level, reboot); // Call trace on each logger
	}
	unlock();

	if (reboot)
	{
		ESP_LOGW(TAG, "trace reboot...");
		vTaskDelay(pdMS_TO_TICKS(1000));
		esp_restart();
	}
}

/**
 * \brief Outputs an error message from an Interrupt Service Routine (ISR).
 * \param strError Pointer to the error message string.
 * \param errCode The error code.
 * \param pxHigherPriorityTaskWoken Pointer to a flag indicating if a higher priority task should be woken.
 * \details Iterates through registered loggers and calls their traceFromISR method.
 *          This function is marked with IRAM_ATTR for faster execution from ISR context.
 */
void IRAM_ATTR CTraceList::traceFromISR(const char *strError, int16_t errCode, BaseType_t *pxHigherPriorityTaskWoken)
{
	for (auto &x : m_list) // Iterate through the list of loggers
	{
		x->traceFromISR(strError, errCode, pxHigherPriorityTaskWoken); // Call traceFromISR on each logger
	}
}

/**
 * \brief Outputs an error message with a uint8_t data array.
 * \param strError Pointer to the error description string.
 * \param data Pointer to the uint8_t data array.
 * \param size Size of the data array.
 * \details Locks the object, iterates through all registered loggers and calls their trace method for uint8_t*,
 *          unlocks.
 */
void CTraceList::trace(const char *strError, uint8_t *data, uint32_t size)
{
	lock();
	for (auto &x : m_list) // Iterate through the list of loggers
	{
		x->trace(strError, data, size); // Call trace on each logger for uint8_t array
	}
	unlock();
}

/**
 * \brief Outputs an error message with an int8_t data array.
 * \param strError Pointer to the error description string.
 * \param data Pointer to the int8_t data array.
 * \param size Size of the data array.
 * \details Locks the object, iterates through all registered loggers and calls their trace method for int8_t*,
 *          unlocks.
 */
void CTraceList::trace(const char *strError, int8_t *data, uint32_t size)
{
	lock();
	for (auto &x : m_list) // Iterate through the list of loggers
	{
		x->trace(strError, data, size); // Call trace on each logger for int8_t array
	}
	unlock();
}

/**
 * \brief Outputs an error message with a uint16_t data array.
 * \param strError Pointer to the error description string.
 * \param data Pointer to the uint16_t data array.
 * \param size Size of the data array.
 * \details Locks the object, iterates through all registered loggers and calls their trace method for uint16_t*,
 *          unlocks.
 */
void CTraceList::trace(const char *strError, uint16_t *data, uint32_t size)
{
	lock();
	for (auto &x : m_list) // Iterate through the list of loggers
	{
		x->trace(strError, data, size); // Call trace on each logger for uint16_t array
	}
	unlock();
}

/**
 * \brief Outputs an error message with an int16_t data array.
 * \param strError Pointer to the error description string.
 * \param data Pointer to the int16_t data array.
 * \param size Size of the data array.
 * \details Locks the object, iterates through all registered loggers and calls their trace method for int16_t*,
 *          unlocks.
 */
void CTraceList::trace(const char *strError, int16_t *data, uint32_t size)
{
	lock();
	for (auto &x : m_list) // Iterate through the list of loggers
	{
		x->trace(strError, data, size); // Call trace on each logger for int16_t array
	}
	unlock();
}

/**
 * \brief Outputs an error message with a uint32_t data array.
 * \param strError Pointer to the error description string.
 * \param data Pointer to the uint32_t data array.
 * \param size Size of the data array.
 * \details Locks the object, iterates through all registered loggers and calls their trace method for uint32_t*,
 *          unlocks.
 */
void CTraceList::trace(const char *strError, uint32_t *data, uint32_t size)
{
	lock();
	for (auto &x : m_list) // Iterate through the list of loggers
	{
		x->trace(strError, data, size); // Call trace on each logger for uint32_t array
	}
	unlock();
}

/**
 * \brief Outputs an error message with an int32_t data array.
 * \param strError Pointer to the error description string.
 * \param data Pointer to the int32_t data array.
 * \param size Size of the data array.
 * \details Locks the object, iterates through all registered loggers and calls their trace method for int32_t*,
 *          unlocks.
 */
void CTraceList::trace(const char *strError, int32_t *data, uint32_t size)
{
	lock();
	for (auto &x : m_list) // Iterate through the list of loggers
	{
		x->trace(strError, data, size); // Call trace on each logger for int32_t array
	}
	unlock();
}

/**
 * \brief Writes an arbitrary string to the log.
 * \param str Pointer to the string to write to the log.
 * \details Locks the object, iterates through all registered loggers and calls their log method,
 *          unlocks.
 */
void CTraceList::log(const char *str)
{
	lock();
	for (auto &x : m_list) // Iterate through the list of loggers
	{
		x->log(str); // Call log on each logger
	}
	unlock();
}

/**
 * \brief Starts a timer for measuring time.
 * \details Locks the object, iterates through all registered loggers and calls their startTime method,
 *          unlocks.
 */
void CTraceList::startTime()
{
	lock();
	for (auto &x : m_list) // Iterate through the list of loggers
	{
		x->startTime(); // Call startTime on each logger
	}
	unlock();
}

/**
 * \brief Stops the timer and records the result.
 * \param str Pointer to a string describing the measurement.
 * \param n An additional parameter for recording.
 * \details Locks the object, iterates through all registered loggers and calls their stopTime method,
 *          unlocks.
 */
void CTraceList::stopTime(const char *str, uint32_t n)
{
	lock();
	for (auto &x : m_list) // Iterate through the list of loggers
	{
		x->stopTime(str, n); // Call stopTime on each logger
	}
	unlock();
}

/**
 * \brief Adds a new logger to the list.
 * \param log Pointer to the ITraceLog logger object to add.
 * \details Locks the object, adds the logger to the internal list, unlocks.
 */
void CTraceList::add(ITraceLog *log)
{
	lock();
	m_list.push_back(log); // Assumes m_list supports push_back
	unlock();
}

/**
 * \brief Removes a logger from the list.
 * \param log Pointer to the ITraceLog logger object to remove.
 * \details Locks the object, removes the logger from the internal list, unlocks.
 */
void CTraceList::remove(ITraceLog *log)
{
	lock();
	m_list.remove(log); // Assumes m_list supports remove
	unlock();
}