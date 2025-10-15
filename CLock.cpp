/*!
	\file
	\brief Base class for capturing FreeRTOS task resources.
	\authors Bliznets R.A. (r.bliznets@gmail.com)
	\version 1.1.0.0
	\date 28.04.2020
	\details This class provides basic locking mechanisms (mutex) for thread safety
			 in FreeRTOS environments. It serves as a base class that other classes
			 can inherit from to protect shared resources.
*/
#include "CLock.h"

// Constructor for the CLock class.
// Initializes the CLock object. In this implementation, the constructor is empty.
// It's assumed that the mutex (mMutex) will be initialized elsewhere,
// potentially in a derived class constructor or during object creation,
// before `lock()` or `unlock()` are called.
CLock::CLock()
{
	// The member variable mMutex is expected to be initialized by the derived class
	// or by the code creating the instance.
}

// The lock() method is used to lock access to a shared resource.
// If the mutex (mMutex) was successfully created and is not nullptr,
// it calls xSemaphoreTake() to acquire the mutex.
// The portMAX_DELAY parameter indicates that the calling task will wait indefinitely
// until the mutex becomes available.
void CLock::lock()
{
	if (mMutex != nullptr) // Check if the mutex exists and is valid.
	{
		xSemaphoreTake(mMutex, portMAX_DELAY); // Acquire the mutex.
	}
}

// The unlock() method is used to unlock access to a shared resource.
// If the mutex (mMutex) was successfully created and is not nullptr,
// it calls xSemaphoreGive() to release the mutex,
// allowing other tasks to gain access to the resource.
void CLock::unlock()
{
	if (mMutex != nullptr) // Check if the mutex exists and is valid.
	{
		xSemaphoreGive(mMutex); // Release the mutex.
	}
}