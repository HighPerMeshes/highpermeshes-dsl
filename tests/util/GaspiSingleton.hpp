// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef TEST_GASPI_SINGLETON_HPP
#define TEST_GASPI_SINGLETON_HPP

#include <HighPerMeshes/drts/UsingGaspi.hpp>

class GaspiSingleton
{
  public:
    static auto& instance()
    {
        static HPM::UsingGaspi gaspi;
        return gaspi;
    }

  private:
    GaspiSingleton() = default;
};

#endif
