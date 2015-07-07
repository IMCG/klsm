/*
 *  Copyright 2015 Jakob Gruber
 *
 *  This file is part of kpqueue.
 *
 *  kpqueue is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  kpqueue is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with kpqueue.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <limits>

template <class K, class V>
block_array<K, V>::block_array() :
    m_size(0)
{
}

template <class K, class V>
block_array<K, V>::~block_array()
{
}

template <class K, class V>
void
block_array<K, V>::insert(block<K, V> *new_block)
{
    if (m_size == 0) {
        assert(m_blocks.empty());
        m_blocks.push_back(new_block);
    } else {
        size_t i;
        for (i = 0; i < m_size; i++) {
            auto b = m_blocks[i];
            if (b->capacity() < new_block->capacity()) {
                break;
            }
        }

        assert(i < MAX_BLOCKS);

        /* Merge with equal capacity blocks until strictly descending invariant is preserved.*/

        auto insert_block = new_block;
        for (; i > 0; i--) {
            auto other_block = m_blocks[i - 1];
            if (other_block == nullptr) {
                continue;
            } else if (other_block->capacity() > insert_block->capacity()) {
                break;
            } else {
                assert(other_block->capacity() == insert_block->capacity());
                auto merged_block = new block<K, V>(insert_block->power_of_2() + 1);
                merged_block->set_used();
                merged_block->merge(insert_block, other_block);

                insert_block = merged_block;
                m_blocks[i - 1] = nullptr;  // TODO: Don't lose reference.
            }
        }
        m_blocks.insert(m_blocks.begin() + i, insert_block);
    }

    m_size++;
    compact();
}

template <class K, class V>
void
block_array<K, V>::compact()
{
    remove_null_blocks();

    /* Shrink blocks. */

    for (size_t i = 0; i < m_size; i++) {
        auto b = m_blocks[i];

        if (b == nullptr) {
            continue;
        }

        if (b->size() < b->capacity() / 2) {
           int shrunk_power_of_2 = b->power_of_2() - 1;
            while (shrunk_power_of_2 > 0 && (1 << (shrunk_power_of_2 - 1)) > (int)b->size()) {
                shrunk_power_of_2--;
            }

            auto shrunk = new block<K, V>(shrunk_power_of_2);
            shrunk->set_used();  // TODO: Remove all of these manual set_used() calls.
            shrunk->copy(b);
            b = m_blocks[i] = shrunk;
        }
    }

    /* Merge blocks. */

    for (int i = m_size - 2; i >= 0; i--) {
        auto big_block = m_blocks[i];
        auto small_block = m_blocks[i + 1];

        size_t big_pow = big_block->power_of_2();
        size_t small_pow = small_block->power_of_2();

        if (big_pow > small_pow) {
            continue;
        }

        int merge_pow = std::max(big_pow, small_pow);
        if ((int)(big_block->size() + small_block->size()) > (1 << merge_pow)) {
            merge_pow++;
        }

        auto merge_block = new block<K, V>(merge_pow);
        merge_block->set_used();
        merge_block->merge(big_block, small_block);

        /* TODO: Don't lose reference to old blocks. */
        m_blocks[i + 1] = nullptr;
        m_blocks[i] = merge_block;
    }

    remove_null_blocks();
}

template <class K, class V>
void
block_array<K, V>::remove_null_blocks() {
    size_t prev_capacity = std::numeric_limits<size_t>::max();
    size_t dst = 0;
    for (size_t src = 0; src < m_size; src++) {
        auto b = m_blocks[src];
        if (b == nullptr) {
            continue;
        }
        m_blocks[dst++] = b;

        /* Debugging. */
        assert(b->capacity() < prev_capacity);
        prev_capacity = b->capacity();
    }

    m_size = dst;
}

template <class K, class V>
bool
block_array<K, V>::delete_min(V &val)
{
    // TODO: Uniformly random relaxed deletion using pivots.

    typename block<K, V>::peek_t best;
    for (size_t i = 0; i < m_size; i++) {
        auto b = m_blocks[i];
        auto candidate  = b->peek();

        if (best.m_item == nullptr ||
                (candidate.m_item != nullptr && candidate.m_key < best.m_key)) {
            best = candidate;
        }
    }

    if (best.m_item == nullptr) {
        return false; /* We did our best, give up. */
    }

    return best.m_item->take(best.m_version, val);
}

template <class K, class V>
block_array<K, V> *
block_array<K, V>::copy()
{
    auto new_array = new block_array<K, V>();

    size_t i;
    for (i = 0; i < m_size; i++) {
        new_array->m_blocks.push_back(m_blocks[i]);
    }
    new_array->m_size = i;

    return new_array;
}
