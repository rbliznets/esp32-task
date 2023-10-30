/*!
	\file
	\brief Шаблон для FIFO буфера.
	\authors Близнец Р.А.
	\version 1.0.0.0
	\date 21.03.2022
	\copyright (c) Copyright 2021, ООО "Глобал Ориент", Москва, Россия, http://www.glorient.ru/
*/

#if !defined TFIFOARRAY_H
#define TFIFOARRAY_H

#include <cstring>

template <typename T>
/// Шаблон для циклического FIFO буфера.
class TFifoArray
{
protected:
	T *mBuffer; ///< буфер.
	int mSize;	///< размер.
	int mIndex; ///< текущий индекс.
public:
	/// Конструктор.
	/*!
	  \param[in] size размер.
	*/
	TFifoArray(int size) : mSize(size), mIndex(0)
	{
		mBuffer = new T[mSize];
	}

	/// Деструктор.
	~TFifoArray()
	{
		delete mBuffer;
	}

	/// Получить размер буфера.
	/*!
	  \return размер.
	*/
	inline int getSize() { return mSize; };

	/// Внести данные в FIFO.
	/*!
	  \param[in] data данные.
	  \param[in] size размер данных.
	*/
	void push(T *data, int size)
	{
		if (size >= mSize)
		{
			std::memcpy(mBuffer, &data[size - mSize], sizeof(T) * mSize);
			mIndex = 0;
		}
		else
		{
			int n = mSize - mIndex;
			if (size < n)
			{
				std::memcpy(&mBuffer[mIndex], data, sizeof(T) * size);
				mIndex += size;
			}
			else if (size == n)
			{
				std::memcpy(&mBuffer[mIndex], data, sizeof(T) * size);
				mIndex = 0;
			}
			else
			{
				std::memcpy(&mBuffer[mIndex], data, sizeof(T) * n);
				std::memcpy(mBuffer, &data[n], sizeof(T) * (size - n));
				mIndex = size - n;
			}
		}
	}

	/// Внести данные в FIFO.
	/*!
	  \param[in] value данные.
	*/
	void push(T value)
	{
		mBuffer[mIndex] = value;
		if (mIndex == (mSize - 1))
			mIndex = 0;
		else
			mIndex++;
	}

	/// Получить элемент по индексу.
	/*!
	  \param[in] index индекс, может быть отрицательным.
	  \return элемент.
	*/
	T &operator[](int index)
	{
		int i = (mIndex + index) % mSize;
		if (i < 0)
			i += mSize;
		return mBuffer[i];
	}

	/// Выравнивание по 0 индексу.
	T *align()
	{
		if (mIndex != 0)
		{
			int n = mSize - mIndex;
			T *data = new T[mIndex];
			std::memcpy(data, mBuffer, sizeof(T) * mIndex);
			std::memmove(mBuffer, &mBuffer[mIndex], sizeof(T) * n);
			std::memcpy(&mBuffer[n], data, sizeof(T) * mIndex);
			delete[] data;
			mIndex = 0;
		}
		return mBuffer;
	}

	/// Получить указатель на элемент по индексу.
	/*!
	  \param[in] index индекс, может быть отрицательным.
	  \return указатель на элемент.
	*/
	inline T *getBuffer(int &index)
	{
		index = mIndex;
		return mBuffer;
	}

	/// Очистка FIFO.
	inline void clear()
	{
		std::memset(mBuffer, 0, sizeof(T) * mSize);
		mIndex = 0;
	}
};

#endif // TFIFOARRAY_H
