#ifndef FORWARDEULER_HPP
#define FORWARDEULER_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"

using namespace HPM;

using EulerType = double;
constexpr auto NumEulerDofs = 20;
constexpr auto EulerDofs = dof::MakeDofs<NumEulerDofs, 0, 0, 0, 0>();

template <typename Range, typename Buffers>
auto ForwardEulerKernel(const Range &range, Buffers &buffers)
{

    return ForEachEntity(
                range,
                std::tuple(Node(std::get<0>(buffers)), Read(Node(std::get<1>(buffers)))),
                [](const auto&, auto, auto lvs) {
                    
                    auto &u = std::get<0>(lvs);
                    const auto &u_d = std::get<1>(lvs);

                    auto tau = 0.2;

                    for (size_t i = 0; i < NumEulerDofs; ++i)
                    {
                        u[i] += tau * u_d[i];
                    }

                }
                , HPM::internal::OpenMP_ForEachEntity<0>{}
            );
}

template <typename Mesh, typename Buffers>
auto ForwardEuler(const Mesh &mesh, size_t iteration_mod, Buffers &buffers)
{

    SequentialDispatcher dispatcher;

    auto range = mesh.template GetEntityRange<0>();  

    return HPM::auxiliary::MeasureTime(
        [&]() {
            dispatcher.Execute(
                HPM::iterator::Range{iteration_mod},
                ForwardEulerKernel(range, buffers));
        }).count();
}
#endif /* FORWARDEULER_HPP */
