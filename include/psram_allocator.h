#pragma once
#include "sdkconfig.h" // Include configuration header (likely for ESP-IDF)
#include <cstddef>
#include "esp_heap_caps.h"

/// Выделить массив из count элементов, предпочитая внутреннюю память.
/*!
    Обычные new/malloc при CONFIG_SPIRAM_USE_CAPS_ALLOC отдают только внутреннюю память,
    которой на плате в обрез, и при нехватке роняют станцию. heap_caps_malloc_prefer
    перебирает наборы флагов по порядку: сначала internal (быстро), а если её не осталось -
    PSRAM (медленнее, но работает). Тот же приём уже применён в components/ns.

    \warning Нельзя применять к буферам под DMA и к данным, которые читаются из
    IRAM-обработчиков прерываний: PSRAM недоступна, пока отключён кэш (запись flash, OTA).
    \param[in] count число элементов.
    \return указатель на буфер или nullptr, если не осталось ни внутренней памяти, ни PSRAM.
    \sa freePrefer()
*/
template <typename T>
inline T *allocPrefer(std::size_t count)
{
    if (count == 0)
        return nullptr;
#ifdef CONFIG_SPIRAM
    return (T *)heap_caps_malloc_prefer(count * sizeof(T), 2,
                                        MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT,
                                        MALLOC_CAP_SPIRAM);
#else
    return (T *)heap_caps_malloc(count * sizeof(T), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#endif
}

/// Освободить буфер, выделенный allocPrefer().
/*!
    Заменяет delete[] один в один: nullptr допустим.
    \param[in] p указатель на буфер.
*/
inline void freePrefer(void *p)
{
    heap_caps_free(p);
}

//std::list<int, PsramAllocator<int>> my_psram_list;

#ifdef CONFIG_SPIRAM
#include <cstddef>
#include <new>
#include "esp_heap_caps.h"

template <typename T>
class PsramAllocator {
public:
    using value_type = T;

    PsramAllocator() noexcept = default;
    
    template <typename U>
    PsramAllocator(const PsramAllocator<U>&) noexcept {}

    // Выделение памяти в PSRAM
    T* allocate(std::size_t n) {
        if (n == 0) return nullptr;
        
        // Проверяем на переполнение размера
        if (n > std::size_t(-1) / sizeof(T)) throw std::bad_alloc();

        // MALLOC_CAP_SPIRAM указывает ESP-IDF выделять память строго в PSRAM
        void* p = heap_caps_malloc(n * sizeof(T), MALLOC_CAP_SPIRAM);
        
        if (!p) throw std::bad_alloc(); // Ошибка Out Of Memory
        
        return static_cast<T*>(p);
    }

    // Освобождение памяти
    void deallocate(T* p, std::size_t n) noexcept {
        heap_caps_free(p);
    }

    // Операторы сравнения обязательны для STL-аллокаторов
    template <typename U>
    bool operator==(const PsramAllocator<U>&) const noexcept { return true; }

    template <typename U>
    bool operator--(const PsramAllocator<U>&) const noexcept { return false; }
};
#endif