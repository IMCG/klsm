/*
 *  Copyright 2014 Jakob Gruber
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

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <thread>

#include "clsm/block_array.h"

using namespace kpq;

TEST(BlockArrayTest, SanityCheck)
{
    block_array<uint32_t, uint32_t> bs;
}

TEST(BlockArrayTest, Insert)
{
    block_array<uint32_t, uint32_t> bs;

    auto b = new block<uint32_t, uint32_t>(1);

    auto i = new item<uint32_t, uint32_t>();
    i->initialize(42, 42);

    b->set_used();
    b->insert(i, i->version());

    bs.insert(b);

    delete i;
    delete b;
}

TEST(BlockArrayTest, Copy)
{
    block_array<uint32_t, uint32_t> bs;

    auto b = new block<uint32_t, uint32_t>(1);

    auto i = new item<uint32_t, uint32_t>();
    i->initialize(42, 42);

    b->set_used();
    b->insert(i, i->version());

    bs.insert(b);

    auto cs = bs.copy();

    delete cs;
    delete i;
    delete b;
}

TEST(BlockArrayTest, DeleteMinEmpty)
{
    block_array<uint32_t, uint32_t> bs;

    uint32_t x;
    ASSERT_FALSE(bs.delete_min(x));
}

TEST(BlockArrayTest, DeleteMin)
{
    block_array<uint32_t, uint32_t> bs;

    auto b = new block<uint32_t, uint32_t>(1);

    auto i = new item<uint32_t, uint32_t>();
    i->initialize(42, 42);

    b->set_used();
    b->insert(i, i->version());

    bs.insert(b);

    uint32_t x;
    ASSERT_TRUE(bs.delete_min(x));
    ASSERT_EQ(42, x);

    delete i;
    delete b;
}

int
main(int argc,
     char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
