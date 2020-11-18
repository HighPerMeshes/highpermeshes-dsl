#ifndef FORWARDEULERSTRIPPED_HPP
#define FORWARDEULERSTRIPPED_HPP
#include <HighPerMeshes.hpp>

#include <Grid.hpp>
#include <HighPerMeshes/drts/UsingOpenCL.hpp>
#include <HighPerMeshes/auxiliary/HelperFunctions.hpp>
#include "help.hpp"
#include <vector>

using namespace HPM;


template <typename Mesh>
auto ForwardEulerStripped(const Mesh &mesh, size_t iteration_mod)
{

    const auto AllNodes{
        mesh.template GetEntityRange<0>()};

    constexpr auto Dofs = dof::MakeDofs<1, 0, 0, 0, 0>();

    auto buffers = MakeTuple<2>(
        [&](auto) {
            return std::vector<double>(mesh.template GetNumEntities<0>() * Dofs.At<0>());
        });

    auto kernel = [&]() {

        #pragma omp parallel for
        for (size_t i = 0; i < mesh.template GetNumEntities<0>(); i++)
        {
            auto &u = std::get<0>(buffers);
            auto &u_d = std::get<0>(buffers);

            auto tau = 0.2;

            u[i] += tau * u_d[i];
        }
    };

    return HPM::auxiliary::MeasureTime([&]() {
               for (size_t time = 0; time < iteration_mod; ++time)
               {
                   kernel();
               }
           })
        .count();
}
#endif /* FORWARDEULER_HPP */
