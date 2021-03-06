#pragma once

#include "array.h"

namespace ch {
	/**
	 * Totally fast and efficient hash table
	 *
	 * All Key's passed in need a hash function
	 */
	template <typename Key, typename Value>
	struct Hash_Table {
		struct Pair {
			Key key;
			Value value;
			ch::Hash_Table<Key, Value>::Pair* next = nullptr;

			Pair() = default;
		};

		ch::Array<Pair> buckets;
		ch::Array<Pair*> layout;

		Hash_Table(const ch::Allocator& in_alloc = ch::context_allocator) : buckets(in_alloc), layout(in_alloc) {}
		
		ch::Hash_Table<Key, Value> copy(const ch::Allocator& in_alloc = ch::context_allocator) const {
			ch::Hash_Table result(in_alloc);
			result.buckets = buckets.copy(in_alloc);
			result.layout = layout.copy(in_alloc);
			return result;
		}

		void free() {
			buckets.free();
			layout.free();
		}

		Pair* begin() {
			return buckets.data;
		}

		Pair* end() {
			return buckets.data + buckets.count;
		}

		const Pair* cbegin() const {
			return buckets.data;
		}

		const Pair* cend() const {
			return buckets.data + buckets.count;
		}

		operator bool() const { return buckets && layout; }
		bool operator==(const ch::Hash_Table<Key, Value>& right) { return right.buckets == buckets; }
		bool operator!=(const ch::Hash_Table<Key, Value>& right) { return right.buckets != buckets; }
		CH_FORCEINLINE Value& operator[](usize index) { return buckets[index].value; }
		CH_FORCEINLINE const Value& operator[](usize index) const { return buckets[index].value; }

		usize key_to_index(const Key& k) const{
			const u64 the_hash = hash(k);
			return the_hash % buckets.count;
		}

		void refresh_layout() {
			assert(buckets && layout.allocated > 0);

			ch::mem_zero(layout.data, sizeof(Pair*) * layout.allocated);
			layout.count = 0;

			for (Pair& it : buckets) {
				const usize index = key_to_index(it.key);

				it.next = nullptr;

				// @TODO(chall): clean this up
				Pair** found = &layout.data[index];
				if (!(*found)) {
					*found = &it;
					layout.count += 1;
				} else {
					while (*found) {
						found = &(*found)->next;
					}
					
					assert(!(*found));
					*found = &it;
				}
			}
		}

		void reserve(usize size) {
			buckets.reserve(size);
			layout.reserve(size);
		}

		usize push(const Key& k, const Value& v) {
			Pair r;
			r.key = k;
			r.value = v;

			const usize result = buckets.push(r);
			layout.reserve(1);

			refresh_layout();

			return result;
		}

        usize push_zero(const Key& key) {
            const usize result = buckets.push_empty();
            buckets[result].key = key;
            layout.reserve(1);

            refresh_layout();

            return result;
        }

		Value* find(const Key& key) {
			if (!buckets.count) return nullptr;

			const usize index = key_to_index(key);

			Pair* found = layout.data[index];
			if (!found) return nullptr;

			if (found->key == key) return &found->value;

			while (found) {
				found = found->next;

				if (found && found->key == key) return &found->value;
			}

			return nullptr;
		}

        const Value* find(const Key& key) const {
            if (!buckets.count) return nullptr;

            const usize index = key_to_index(key);

            Pair* found = layout.data[index];
            if (!found) return nullptr;

            if (found->key == key) return &found->value;

            while (found) {
                found = found->next;

                if (found && found->key == key) return &found->value;
            }

            return nullptr;
        }

		bool remove(const Key& key) {
			for (usize i = 0; i < buckets.count; i++) {
				const Pair& p = buckets[i];
				if (p.key == key) {
					buckets.remove(i);
					refresh_layout();
					return true;
				}
			}

			return false;
		}

		void remove_by_index(usize index) {
			assert(index < buckets.count);

			buckets.remove(index);
			refresh_layout();
		}

		bool contains(const Key& key) {
			return find(key) != nullptr;
		}
	};
}