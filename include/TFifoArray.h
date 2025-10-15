/*!
	\file
	\brief Template for a FIFO buffer.
	\authors Bliznets R.A.(r.bliznets@gmail.com)
	\version 1.0.0.0
	\date 21.03.2022
	\details This header file defines the TFifoArray template class, which implements
			 a fixed-size circular FIFO (First-In, First-Out) buffer. Elements can be
			 added (pushed) to the buffer, and older elements are overwritten when
			 the buffer becomes full. It supports accessing elements by index, including
			 negative indices relative to the current write position.
*/

#pragma once

#include <cstring>

template <typename T>
/// Template for a circular FIFO buffer.
/// @details This class provides a fixed-size, circular buffer where data is written
///          sequentially. When the buffer is full, new data overwrites the oldest data.
///          It uses an internal index to track the current write position.
class TFifoArray
{
protected:
	/// Internal buffer array to store elements of type T.
	T *mBuffer;

	/// Size of the buffer (total number of elements it can hold).
	int mSize;

	/// Current write index. Points to the position where the next element will be written.
	/// It wraps around when it reaches the end of the buffer.
	int mIndex;

public:
	/// Constructor.
	/// @details Allocates memory for the internal buffer of the specified size.
	/// @param[in] size The size of the buffer to create.
	TFifoArray(int size) : mSize(size), mIndex(0)
	{
		mBuffer = new T[mSize];
	}

	/// Destructor.
	/// @details Deallocates the memory used by the internal buffer.
	~TFifoArray()
	{
		delete[] mBuffer; // Use delete[] for arrays allocated with new[]
	}

	/// Get the size of the buffer.
	/// @return The total size of the buffer.
	inline int getSize() { return mSize; };

	/// Push multiple data elements into the FIFO.
	/// @details Copies the provided data array into the buffer. If the size of the
	///          input data is greater than or equal to the buffer size, the buffer
	///          is completely overwritten with the last `mSize` elements from the input.
	///          Otherwise, data is copied sequentially, wrapping around the buffer if necessary.
	/// @param[in] data Pointer to the array of data to push.
	/// @param[in] size Number of elements in the input data array.
	void push(T *data, int size)
	{
		if (size >= mSize)
		{
			// If input data is larger than or equal to buffer, overwrite buffer
			// with the last mSize elements from the input data.
			std::memcpy(mBuffer, &data[size - mSize], sizeof(T) * mSize);
			mIndex = 0; // Reset index after overwrite
		}
		else
		{
			int n = mSize - mIndex; // Space remaining from current index to end of buffer
			if (size < n)
			{
				// Data fits in the remaining space without wrapping
				std::memcpy(&mBuffer[mIndex], data, sizeof(T) * size);
				mIndex += size;
			}
			else if (size == n)
			{
				// Data fits exactly in the remaining space
				std::memcpy(&mBuffer[mIndex], data, sizeof(T) * size);
				mIndex = 0; // Wrap index to the beginning
			}
			else
			{
				// Data needs to wrap around the buffer
				// Copy part that fits to the end
				std::memcpy(&mBuffer[mIndex], data, sizeof(T) * n);
				// Copy the remaining part to the beginning
				std::memcpy(mBuffer, &data[n], sizeof(T) * (size - n));
				mIndex = size - n; // New index after wrapping
			}
		}
	}

	/// Push a single data element into the FIFO.
	/// @details Adds a single element to the buffer at the current index and advances the index.
	///          If the index reaches the end of the buffer, it wraps around to 0.
	/// @param[in] value The single element to push.
	void push(T value)
	{
		mBuffer[mIndex] = value;
		if (mIndex == (mSize - 1))
			mIndex = 0; // Wrap around to the beginning
		else
			mIndex++; // Advance index
	}

	/// Access an element by index relative to the current write position.
	/// @details Allows access using operator[]. An index of 0 refers to the element
	///          that will be overwritten next (oldest if full). Negative indices
	///          refer to previous elements in the sequence (e.g., -1 is the last written).
	///          Positive indices wrap around to older elements.
	/// @param[in] index Relative index (can be negative).
	/// @return A reference to the requested element.
	T &operator[](int index)
	{
		// Calculate the absolute index, wrapping around the buffer size
		int i = (mIndex + index) % mSize;
		if (i < 0)
			i += mSize; // Handle negative modulo result
		return mBuffer[i];
	}

	/// Align the buffer so that the logical start is at index 0.
	/// @details Rearranges the internal buffer memory so that the sequence of elements
	///          starts from mBuffer[0], making it easier to access the data linearly
	///          if needed. This operation modifies the internal buffer and resets mIndex to 0.
	/// @return A pointer to the beginning of the aligned buffer (mBuffer).
	T *align()
	{
		if (mIndex != 0) // Only align if necessary
		{
			int n = mSize - mIndex;	 // Number of elements from current index to end
			T *data = new T[mIndex]; // Temporary buffer for elements at the beginning
			// Copy elements from the start (which are logically "older")
			std::memcpy(data, mBuffer, sizeof(T) * mIndex);
			// Move elements from current index to end to the beginning of the buffer
			std::memmove(mBuffer, &mBuffer[mIndex], sizeof(T) * n);
			// Append the elements from the temporary buffer to the end
			std::memcpy(&mBuffer[n], data, sizeof(T) * mIndex);
			delete[] data; // Free temporary buffer
			mIndex = 0;	   // After alignment, the logical start is at index 0
		}
		return mBuffer; // Return pointer to the aligned buffer
	}

	/// Get a pointer to the internal buffer and the current index.
	/// @details Provides access to the raw buffer pointer and the current write index.
	///          Note that the order of elements in the raw buffer depends on the `mIndex`.
	/// @param[out] index Reference to an integer where the current `mIndex` will be stored.
	/// @return Pointer to the internal buffer array.
	inline T *getBuffer(int &index)
	{
		index = mIndex; // Output the current index
		return mBuffer; // Return the buffer pointer
	}

	/// Clear the FIFO buffer.
	/// @details Fills the entire internal buffer with zeros and resets the write index to 0.
	inline void clear()
	{
		std::memset(mBuffer, 0, sizeof(T) * mSize); // Fill buffer with zeros
		mIndex = 0;									// Reset index
	}
};