#pragma once

#include "types.h"

namespace ch {
    /**
     * Allocator concept
     * 
     * All actual allocator data is stored inside the allocators data as a header
     * We can throw this around anyway we like and it should always work
     */
    struct Allocator {
        using Allocator_Func = void* (*)(const Allocator& allocator, void* ptr, usize size);
        
        u8* data;
        Allocator_Func func;

        explicit operator bool() const {
            return func != nullptr;
        }

        void* alloc(usize size);
        void* realloc(void* ptr, usize size);
        void free(void* ptr);

        template <typename T> 
        T* get_header() const {
            return (T*)data;
        }
    };

    ch::Allocator get_heap_allocator();

    struct Arena_Allocator_Header {
        usize allocated;
        usize current;
        void* data;
    };

    ch::Allocator make_arena_allocator(usize size);
    void free_arena_allocator(ch::Allocator* allocator);
    void reset_arena_allocator(ch::Allocator* allocator);

	/** Essentially an empty allocator to prevent seg faults. */
	ch::Allocator get_stack_allocator();

	extern Allocator context_allocator;
}

void* operator new(usize size, ch::Allocator allocator);
void* operator new[](usize size, ch::Allocator allocator);

void operator delete(void* ptr, ch::Allocator allocator);
void operator delete[](void* ptr, ch::Allocator allocator);