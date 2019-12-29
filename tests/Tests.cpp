// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
    // Google Test will remove its own parameters from argv, so run it first!
    ::testing::InitGoogleTest(&argc, argv);

    int result = RUN_ALL_TESTS();

    return result;
}
