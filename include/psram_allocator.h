#pragma once
#include "sdkconfig.h" // Include configuration header (likely for ESP-IDF)

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