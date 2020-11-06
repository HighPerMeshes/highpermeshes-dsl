#include <gtest/gtest.h>

#include <HighPerMeshes.hpp>

#include "../../util/UnitCube.hpp"

using namespace HPM;

// Ensures that the time steps are correctly passed to the lambda in the dispatcher
TEST(SequentialDispatcherTest, TimeSteps)
{

  static constexpr size_t CellDimension = 3;

  UnitCube cube;
  const auto &mesh{cube.mesh};
  SequentialDispatcher dispatcher;
  auto AllCells{mesh.GetEntityRange<CellDimension>()};

  auto set_data = [](auto &buffer) {
    const std::size_t buffer_size = buffer.GetSize();
    for (std::size_t i = 0; i < buffer_size; ++i)
    {
      buffer[i] = -1;
    }
  };

  auto execute_kernel = [&](auto &buffer, const std::size_t steps) {
    dispatcher.Execute(
        iterator::Range{steps},
        ForEachEntity(
            AllCells,
            std::tuple(Write(Cell(buffer))),
            [&](const auto &, auto step, auto local_view) {
              auto &bufferAccess = std::get<0>(local_view);
              bufferAccess[step] = step;
            }));
  };

  auto check_results = [](auto &buffer, const std::size_t steps) {
    for (std::size_t i = 0; i < steps; ++i)
    {
      EXPECT_EQ(buffer[i], i);
    }
  };

  //Works for one time step
  {
    Buffer<int, std::decay_t<decltype(mesh)>, dataType::ConstexprArray<std::size_t, 0, 0, 0, 1, 0>, std::allocator<int>> buffer{mesh, {}, {}};
    set_data(buffer);
    execute_kernel(buffer, 1);
    check_results(buffer, 1);
  }

  //Works for multiple time step
  {
    Buffer<int, std::decay_t<decltype(mesh)>, dataType::ConstexprArray<std::size_t, 0, 0, 0, 5, 0>, std::allocator<int>> buffer{mesh, {}, {}};
    set_data(buffer);
    execute_kernel(buffer, 5);
    check_results(buffer, 5);
  }

  //Works for zero time steps
  {
    Buffer<int, std::decay_t<decltype(mesh)>, dataType::ConstexprArray<std::size_t, 0, 0, 0, 1, 0>, std::allocator<int>> buffer{mesh, {}, {}};
    set_data(buffer);
    execute_kernel(buffer, 0);
    EXPECT_EQ(buffer[0], -1);
  }

  auto execute_multiple_kernels = [&](auto &buffer, auto steps) {
    dispatcher.Execute(
        iterator::Range{steps},
        ForEachEntity(
            AllCells,
            std::tuple(Write(Cell(buffer))),
            [&](const auto &, auto step, auto local_view) {
              auto &bufferAccess = std::get<0>(local_view);
              bufferAccess[step] = -1;
            }),
        ForEachEntity(
            AllCells,
            std::tuple(Write(Cell(buffer))),
            [&](const auto &, auto step, auto local_view) {
              auto &bufferAccess = std::get<0>(local_view);
              bufferAccess[step] = step;
            }));
  };

  //Works for multiple kernels / one time step
  {
    Buffer<int, std::decay_t<decltype(mesh)>, dataType::ConstexprArray<std::size_t, 0, 0, 0, 1, 0>, std::allocator<int>> buffer{mesh, {}, {}};
    set_data(buffer);
    execute_multiple_kernels(buffer, 1);
    check_results(buffer, 1);
  }

  //Works for multiple kernels / multiple time steps
  {
    Buffer<int, std::decay_t<decltype(mesh)>, dataType::ConstexprArray<std::size_t, 0, 0, 0, 5, 0>, std::allocator<int>> buffer{mesh, {}, {}};
    set_data(buffer);
    execute_multiple_kernels(buffer, 5);
    check_results(buffer, 5);
  }

  //Works for multiple kernels / zero time steps
  {
    Buffer<int, std::decay_t<decltype(mesh)>, dataType::ConstexprArray<std::size_t, 0, 0, 0, 1, 0>, std::allocator<int>> buffer{mesh, {}, {}};
    set_data(buffer);
    execute_multiple_kernels(buffer, 0);
    EXPECT_EQ(buffer[0], -1);
  }


}