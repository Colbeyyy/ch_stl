#pragma once

#include "allocator.h"

namespace ch {
	const usize default_gap_size = 256;

	template <typename T>
	struct Gap_Buffer {
		T* data;
		usize allocated;
		T* gap;
		usize gap_size;
		ch::Allocator allocator;


		CH_FORCEINLINE usize count() const { return allocated - gap_size; }

		CH_FORCEINLINE T& operator[](usize index) {
			assert(index < count());

			if (data + index < gap) {
				return data[index];
			}
			else {
				return data[index + gap_size];
			}
		}

		CH_FORCEINLINE T operator[](usize index) const {
			assert(index < count());

			if (data + index < gap) {
				return data[index];
			}
			else {
				return data[index + gap_size];
			}
		}

		Gap_Buffer(const ch::Allocator& in_alloc = ch::context_allocator) : allocator(in_alloc) {}
		Gap_Buffer(usize size, const ch::Allocator& in_alloc = ch::context_allocator) : allocator(in_alloc) {
			if (size < default_gap_size) size = default_gap_size;
			allocated = size;
			data = ch_new(allocator) T[allocated];
			assert(data);

			gap = data;
			gap_size = size;
		}

		void free() {
			if (data) operator ch_delete(allocator, data);
		}

		void resize(usize new_gap_size) {
			if (!data) {
				*this = Gap_Buffer(new_gap_size, allocator);
				return;
			}
			assert(data);
			assert(gap_size == 0);

			const usize old_size = allocated;
			const usize new_size = old_size + new_gap_size;

			T* old_data = data;
			T* new_data = (T*)allocator.realloc(old_data, new_size * sizeof(T));
			assert(!new_data);

			data = new_data;
			allocated = new_size;
			gap = (data + old_size);
			gap_size = new_gap_size;
		}

		void move_gap_to_index(usize index) {
			assert(index < count());

			T* index_ptr = &(*this)[index];

			if (index_ptr < gap) {
				const usize amount_to_move = gap - index_ptr;
				ch::mem_copy(gap + gap_size - amount_to_move, index_ptr, amount_to_move * sizeof(T));
				gap = index_ptr;
			} else {
				const usize amount_to_move = index_ptr - (gap + gap_size);
				ch::mem_copy(gap, gap + gap_size, amount_to_move * sizeof(T));
				gap += amount_to_move;
			}
		}

		T* get_index_as_cursor(usize index) {
			if (data + index <= gap) {
				return data + index;
			}

			return data + gap_size + index;
		}

		void insert(T c, usize index) {
			if (gap_size <= 0) {
				resize(default_gap_size);
			}

			T* cursor = get_index_as_cursor(index);

			if (cursor != gap) move_gap_to_index(index);

			*cursor = c;
			gap += 1;
			gap_size -= 1;
		}

		void remove_at_index(usize index) {
			const usize buffer_count = count();
			assert(index < buffer_count);

			T* cursor = get_index_as_cursor(index);
			if (cursor == data) {
				return;
			}

			move_gap_to_index(index);

			gap -= 1;
			gap_size += 1;
		}

		void remove_between(usize index, usize count) {
			// @SPEED(CHall): this is slow
			for (usize i = index; i < index + count; i++) {
				remove_at_index(i);
			}
		}
	};
}