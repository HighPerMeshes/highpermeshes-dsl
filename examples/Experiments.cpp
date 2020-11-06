// remove all DUNE dependencies from Jakob Schenk's gridIteration.hh implementation
// performance results are no longer hurt
// merge version created from midg_cpp_modified and gridIteration.hh by Ayesha Afzal

#include <array>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <chrono>
#include <numeric>

#include <HighPerMeshes.hpp>
#include <Grid.hpp>

#include "MIDG2_DSL/data3dN03.hpp" //!< application-dependent discontinuous Galerkin's cubic order node information
#include "MIDG2_DSL/RKCoeff.hpp"   //!< application-dependent Runge-Kutta coefficients

using CoordinateType = HPM::dataType::Vec<double,3>;
using RealType = HPM::dataType::Real;
using Vec3D = HPM::dataType::Vec<double,3>;
using Mat3D = HPM::dataType::Matrix<double,3,3>;

using namespace HPM;
using namespace HPM::internal;

auto GetGradientsDSL()
{
  HPM::dataType::Matrix<double, 4, 3> gradientsDSL;
  gradientsDSL[0][0] = -1;
  gradientsDSL[0][1] = -1;
  gradientsDSL[0][2] = -1;
  gradientsDSL[1][0] = 1;
  gradientsDSL[1][1] = 0;
  gradientsDSL[1][2] = 0;
  gradientsDSL[2][0] = 0;
  gradientsDSL[2][1] = 1;
  gradientsDSL[2][2] = 0;
  gradientsDSL[3][0] = 0;
  gradientsDSL[3][1] = 0;
  gradientsDSL[3][2] = 1;
  return gradientsDSL;
}

int main(int argc, char **argv)
{
  constexpr auto Dofs = ::HPM::dof::MakeDofs<0, 0, 0, 20, 0>();
  constexpr std::size_t order = 3;

  HPM::drts::Runtime<HPM::GetBuffer<>> hpm {{}};

  HPM::auxiliary::ConfigParser CFG("config.cfg");

  Grid<3> grid {{ 100 , 10, 10 }};
  const auto& mesh = grid.mesh;
  using Mesh = std::decay_t<decltype(mesh)>;

  using DG = DgNodes<double, Vec3D, order>;
  HPM::DG::DgNodesMap<DG, Mesh> DgNodeMap(mesh);

  auto AllCells{mesh.GetEntityRange<Mesh::CellDimension>()};

  std::cout << "cells: " << AllCells.GetIndices().size() << "\n";

  auto fieldH = hpm.GetBuffer<Vec3D>(mesh, Dofs);
  auto fieldE = hpm.GetBuffer<Vec3D>(mesh, Dofs);
  auto rhsH = hpm.GetBuffer<Vec3D>(mesh, Dofs);
  auto rhsE = hpm.GetBuffer<Vec3D>(mesh, Dofs);

  HPM::SequentialDispatcher dispatcher;

  auto MeasureKernel = [&dispatcher](auto &&kernel) {
    return HPM::auxiliary::MeasureTime(
               [&]() {
                 dispatcher.Execute(
                    iterator::Range { 10 },
                    std::forward<decltype(kernel)>(kernel));
               })
        .count();
  };

  auto volume_kernel = HPM::ForEachEntity(
      AllCells,
      std::tuple(
          Read(Cell(fieldH)),
          Read(Cell(fieldE)),
          ReadWrite(Cell(rhsH)),
          ReadWrite(Cell(rhsE))),
      [&](const auto &element, const auto &, auto &lvs) {
        const Mat3D &D = element.GetGeometry().GetInverseJacobian() * 2.0;

        HPM::ForEach(DG::numVolNodes, [&](const std::size_t n) {
          Mat3D derivative_E, derivative_H; //!< derivative of fields w.r.t reference coordinates

          const auto &fieldH = std::get<0>(lvs);
          const auto &fieldE = std::get<1>(lvs);

          HPM::ForEach(DG::numVolNodes, [&](const std::size_t m) {
            derivative_H += DyadicProduct(DG::derivative[n][m], fieldH[m]);
            derivative_E += DyadicProduct(DG::derivative[n][m], fieldE[m]);
          });

          auto &rhsH = std::get<2>(lvs);
          auto &rhsE = std::get<3>(lvs);

          rhsH[n] += -Curl(D, derivative_E); //!< first half of right-hand-side of fields
          rhsE[n] += Curl(D, derivative_H);
        });
      });

  auto volume_kernel_openmp =
      HPM::MeshLoop{
          AllCells,
          volume_kernel.access_definitions,
          HPM::internal::OpenMP_ForEachEntity<3>{},
          volume_kernel.loop_body};

  std::cout << "Without OpenMP: \n"
            << MeasureKernel(volume_kernel)
            << "\n";

  std::cout << "With OpenMP: \n"
            << MeasureKernel(volume_kernel_openmp)
            << "\n";

  auto surface_kernel = HPM::ForEachIncidence<2>(
      AllCells,
      std::tuple(
          Read(ContainingMeshElement(fieldH)),
          Read(ContainingMeshElement(fieldE)),
          Read(NeighboringMeshElementOrSelf(fieldH)),
          Read(NeighboringMeshElementOrSelf(fieldE)),
          ReadWrite(ContainingMeshElement(rhsH)),
          ReadWrite(ContainingMeshElement(rhsE))),
      [&](const auto &element, const auto &face, const auto &, auto &lvs) {
        const std::size_t face_index = face.GetTopology().GetLocalIndex();
        const RealType face_normal_scaling_factor = 2.0 / element.GetGeometry().GetAbsJacobianDeterminant();
        const Vec3D &face_normal = face.GetGeometry().GetNormal() * face_normal_scaling_factor; //!< get all normal coordinates for each face of an element
        const RealType Edg = face_normal.Norm() * 0.5;                                          //!< get edge length for each face
        const Vec3D &face_unit_normal = face.GetGeometry().GetUnitNormal();
        const auto &localMap{DgNodeMap.Get(element, face)};

        HPM::ForEach(DG::NumSurfaceNodes, [&](const std::size_t m) {
          const auto &fieldH = std::get<0>(lvs);
          const auto &fieldE = std::get<1>(lvs);

          auto &NeighboringFieldH = std::get<2>(lvs);
          auto &NeighboringFieldE = std::get<3>(lvs);

          const Vec3D &dH = Edg * HPM::DG::Delta(fieldH, NeighboringFieldH, m, localMap); //!< fields differences
          const Vec3D &dE = Edg * HPM::DG::DirectionalDelta(fieldE, NeighboringFieldE, face, m, localMap);

          const Vec3D &flux_H = (dH - (dH * face_unit_normal) * face_unit_normal - CrossProduct(face_unit_normal, dE)); //!< fields fluxes
          const Vec3D &flux_E = (dE - (dE * face_unit_normal) * face_unit_normal + CrossProduct(face_unit_normal, dH));

          auto &rhsH = std::get<4>(lvs);
          auto &rhsE = std::get<5>(lvs);

          HPM::ForEach(DG::numVolNodes, [&](const std::size_t n) {
            rhsH[n] += DG::LIFT[face_index][m][n] * flux_H;
            rhsE[n] += DG::LIFT[face_index][m][n] * flux_E;
          });
        });
      });

  auto surface_kernel_openmp =
      HPM::MeshLoop{
          AllCells,
          surface_kernel.access_definitions,
          HPM::internal::OpenMP_ForEachIncidence<3, 2>{},
          surface_kernel.loop_body};

  std::cout << "Without OpenMP: \n"
            << MeasureKernel(surface_kernel)
            << "\n";

  std::cout << "With OpenMP: \n"
            << MeasureKernel(surface_kernel_openmp)
            << "\n";

  constexpr int dim = Mesh::CellDimension;
  HPM::dataType::Vec<double, 8> d;

  auto buffer = hpm.GetBuffer<double>(mesh, ::HPM::dof::MakeDofs<1, 0, 0, 0, 0>());

  auto matrix_vec_product =
      HPM::ForEachEntity(
          AllCells,
          std::tuple(ReadWrite(Node(buffer))),
          [&](auto const &cell, const auto &iter, auto &lvs) {
            auto &sBuffer = std::get<0>(lvs);

            const int nrows = dim + 1;
            const int ncols = dim + 1;
            const auto gradients = GetGradientsDSL();
            const auto &nodeIdSet = std::array { 0, 1, 2, 3 };

            auto tmp = cell.GetGeometry().GetJacobian();
            double detJ = tmp.Determinant();
            detJ = std::abs(detJ);
            auto inv = tmp.Invert();
            auto invJT = inv.Transpose();

            double val = 0;

            // Material information is integrate as diffusion tensor D = sigma * I with sigma as random scalar value.
            // For example: sigma = 2
            double sigma = 2;

            for (int col = 0; col < ncols; ++col)
            {
              auto gc = invJT * gradients[col] * sigma * (detJ / 6);
              for (int row = 0; row < nrows; ++row)
              {
                // add mass (matrix) term
                if (col == row)
                  val = detJ / 60;
                else
                  val = detJ / 120;

                auto gr = invJT * gradients[row];
                sBuffer[nodeIdSet[col]][0] += ((gc * gr) + val) * d[nodeIdSet[row]];
              }
            }
          });

  auto matrix_vec_product_openmp =
      HPM::MeshLoop{
          AllCells,
          matrix_vec_product.access_definitions,
          HPM::internal::OpenMP_ForEachEntity<3>{},
          matrix_vec_product.loop_body};

  std::cout << "Without OpenMP: \n"
            << MeasureKernel(matrix_vec_product)
            << "\n";

  std::cout << "With OpenMP: \n"
            << MeasureKernel(matrix_vec_product_openmp)
            << "\n";

  

  return EXIT_SUCCESS;
}
