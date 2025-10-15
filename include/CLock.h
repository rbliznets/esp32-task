/*!
	\file
	\brief Base class for capturing FreeRTOS task resources.
	\authors Bliznets R.A. (r.bliznets@gmail.com)
	\version 1.1.0.0
	\date 28.04.2020
	\details This header file defines the CLock class, an abstract base class
			 intended to provide a locking mechanism (using a mutex) for managing
			 access to shared resources in a FreeRTOS environment. Classes inheriting
			 from CLock can use the protected lock() and unlock() methods to ensure
			 thread safety.
*/

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/// Base class for capturing a resource.
/// @details This class provides a mechanism to lock and unlock a resource
///          using a FreeRTOS mutex (SemaphoreHandle_t). It is designed to be
///          inherited by other classes that need to protect shared data or
///          critical sections from concurrent access by multiple FreeRTOS tasks.
class CLock
{
protected:
	/// Mutex handle used for locking/unlocking the resource.
	/// @details This handle must be initialized by a derived class or external code
	///          before the lock() or unlock() methods are called.
	SemaphoreHandle_t mMutex = nullptr;

	/// Initialize with a new mutex semaphore.
	/// @details This method allows setting the internal mutex handle.
	///          It's protected, intended for use by derived classes during their setup.
	/// @param[in] mutex Pointer to the semaphore to be used as the mutex.
	void init(SemaphoreHandle_t mutex)
	{
		mMutex = mutex;
	};

	/// Acquire the resource lock (mutex).
	/// @details This method blocks the calling task until the mutex is acquired.
	///          It should be called before accessing the protected resource.
	///          Uses portMAX_DELAY, meaning the task will wait indefinitely.
	void lock();

	/// Release the resource lock (mutex).
	/// @details This method releases the mutex, allowing other tasks to acquire it.
	///          It should be called after finishing access to the protected resource.
	void unlock();

public:
	/// Constructor for the CLock class.
	/// @details Initializes the base class. The mutex handle (mMutex) remains nullptr
	///          and must be set by the derived class or external initialization code.
	CLock();
};