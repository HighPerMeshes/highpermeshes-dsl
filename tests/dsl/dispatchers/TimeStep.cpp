#include <gtest/gtest.h>

#include <HighPerMeshes.hpp>

#include "../../util/UnitCube.hpp"

using namespace HPM;

// Ensures that the time steps are correctly passed to the lambda in the dispatcher
TEST(SequentialDispatcherTest, TimeSteps) {

    static constexpr size_t CellDimension = 3;

    UnitCube cube;
    const auto& mesh{cube.mesh};
    SequentialDispatcher dispatcher;
    auto AllCells { mesh.GetEntityRange<CellDimension>() } ;

    auto set_data = [](auto& buffer) {
      auto * data = buffer.Data();
      for(size_t i = 0; i < buffer.GetSize(); ++i) {
        data[i] = -1;
      }
    };

    auto execute_kernel = [&](auto& buffer, auto steps) {
      dispatcher.Execute(
        iterator::Range { steps },
        ForEachEntity(
            AllCells,
            std::tuple(Write(Cell(buffer))),
            [&](const auto &cell, auto step, auto local_view) {
              auto& bufferAccess = dof::GetDofs<CellDimension>(std::get<0>(local_view));
              bufferAccess[step] = step;
            })
      );
    };

    auto check_results = [](auto& buffer, auto steps) {
      auto * data = buffer.Data();
      for(size_t i = 0; i < steps; ++i) {
        EXPECT_EQ(data[i], i);
      }
    };
    
    //Works for one time step
    {
      Buffer<int, std::decay_t<decltype(mesh)>, dataType::ConstexprArray<std::size_t, 0, 0, 0, 1>, std::allocator<int>> buffer {mesh, {}, {}};
      set_data(buffer);
      execute_kernel(buffer, 1);
      check_results(buffer, 1);      
    }
    
    //Works for multiple time step
    { 
      Buffer<int, std::decay_t<decltype(mesh)>, dataType::ConstexprArray<std::size_t, 0, 0, 0, 5>, std::allocator<int>> buffer {mesh, {}, {}};
      set_data(buffer);
      execute_kernel(buffer, 5);
      check_results(buffer, 5);
    }

    //Works for zero time steps
    { 
      Buffer<int, std::decay_t<decltype(mesh)>, dataType::ConstexprArray<std::size_t, 0, 0, 0, 1>, std::allocator<int>> buffer {mesh, {}, {}};
      set_data(buffer);
      execute_kernel(buffer, 0);
      auto * data = buffer.Data();    
      EXPECT_EQ(data[0], -1);
    }

}