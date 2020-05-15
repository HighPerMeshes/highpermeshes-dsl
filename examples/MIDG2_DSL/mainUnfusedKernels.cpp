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
#include <cstring>

#include <omp.h>

#include <HighPerMeshes.hpp>

#include "data3dN03.hpp" 					                  //!< application-dependent discontinuous Galerkin's cubic order node information
#include "RKCoeff.hpp" 						                  //!< application-dependent Runge-Kutta coefficients

using namespace ::HPM;
using namespace ::HPM::dataType;

template <typename T>
class DEBUG;

int main(int, char**)
{
    HPM::drts::Runtime hpm { HPM::GetBuffer{} };

    using CoordinateType = HPM::dataType::Coord3D;
    using RealType = HPM::dataType::Real;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                         Reading Configuration, DG and Mesh Files                                     //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto maint1 = std::chrono::high_resolution_clock::now();

    /** \brief read configuration file */
    HPM::auxiliary::ConfigParser CFG("config.cfg");
    const RealType startTime = CFG.GetValue<RealType>("StartTime"); 		//!< get the value of a user-specific starting time
    const RealType finalTime = CFG.GetValue<RealType>("FinalTime"); 		//!< get the value of a user-specific stop time

    /** \brief read mesh file */
    const std::string meshFile = CFG.GetValue<std::string>("MeshFile"); 	//!< get the name of a user-specific mesh file
    using Mesh = HPM::mesh::Mesh<CoordinateType, HPM::entity::Simplex>;
    const Mesh mesh = Mesh::template CreateFromFile<HPM::auxiliary::GambitMeshFileReader>(meshFile);

    /** \brief read application-dependent discontinuous Galerkin's stuff */
    constexpr std::size_t order = 3;
    using DG = DgNodes<RealType, Vec3D, order>;
    HPM::DG::DgNodesMap<DG, Mesh> DgNodeMap(mesh);
    
    // //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // //    All three kernels (Maxwell's Surface Kernel, Maxwell's Volume Kernel, Runge-Kutta kernel)         //
    // //////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    auto AllCells { mesh.GetEntityRange<Mesh::CellDimension>() } ;

#if !defined(MIDG_CODE)
    constexpr auto Dofs = ::HPM::dof::MakeDofs<0, 0, 0, DG::numVolNodes, 0>();

    /** \brief load initial conditions for fields */
    auto fieldH { hpm.GetBuffer<CoordinateType>(mesh, Dofs) }; 
    auto fieldE { hpm.GetBuffer<CoordinateType>(mesh, Dofs) };

    HPM::SequentialDispatcher body;

    body.Execute(
        HPM::ForEachEntity(
            AllCells, 
            std::tuple(Write(Cell(fieldE))),
            [&] (const auto& cell, auto&&, auto& lvs)
            {
                const auto& nodes = cell.GetTopology().GetNodes();

                HPM::ForEach(DG::numVolNodes, [&](const auto n) 
                {  
                    const auto& nodeCoords = DG::LocalToGlobal(DG::referenceCoords[n], nodes);
                    auto& fieldE = dof::GetDofs<dof::Name::Cell>(std::get<0>(lvs));

                    fieldE[n].y = std::sin(M_PI * nodeCoords.x) * std::sin(M_PI * nodeCoords.z); 	//!< initial conditions for y component of electric field
                });
            })
    );

    /** \brief create storage for intermediate fields*/
    auto resH { hpm.GetBuffer<CoordinateType>(mesh, Dofs) };
    auto resE { hpm.GetBuffer<CoordinateType>(mesh, Dofs) };
    auto rhsH { hpm.GetBuffer<CoordinateType>(mesh, Dofs) };
    auto rhsE { hpm.GetBuffer<CoordinateType>(mesh, Dofs) };
    
    auto maint2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> setup_duration = maint2 - maint1;
    std::cout << "Setup time in seconds: " << setup_duration.count() << std::endl;
    double aggregate_time1 = 0.0, aggregate_time2 = 0.0, aggregate_time3 = 0.0;

    /** \brief outer time step loop, Runge-Kutta loop, maxwell's kernels (surface and volume) and Runge-Kutta kernel  */

    /** \brief determine time step size (polynomial order-based and algorithmic-specific) */
    RealType timeStep = 1.0e6;

    HPM::ForEachEntity(
        AllCells, 
        [&] (const auto& cell)
        {
            const RealType face_normal_scaling_factor = 2.0 / cell.GetGeometry().GetAbsJacobianDeterminant();

            HPM::ForEachSubEntity(cell, [&](const auto& face)
            {
                timeStep = std::min(timeStep, 1.0 / (face.GetGeometry().GetNormal() * face_normal_scaling_factor).Norm());
            }); 
        });

    timeStep = finalTime / floor(finalTime * (order + 1) * (order + 1)/(.5 * timeStep) );
    std::cout << "time step: " << timeStep << std::endl;

    {
        auto t1 = std::chrono::high_resolution_clock::now();

        /** \brief Maxwell's surface kernel */
        Vec<Matrix<RealType, DG::numVolNodes, DG::NumSurfaceNodes>, Mesh::NumFacesPerCell> lift;
        
        for (std::size_t face_index = 0; face_index < Mesh::NumFacesPerCell; ++face_index)
        {
            for (std::size_t m = 0; m < DG::NumSurfaceNodes; ++m)
            {
                for (std::size_t n = 0; n < DG::numVolNodes; ++n)
                {
                    lift[face_index][n][m] = DG::LIFT[face_index][m][n];
                }
            }
        }

        auto surfaceKernelLoop = HPM::ForEachIncidence<2>(
            AllCells,
            std::tuple(
                Read(ContainingMeshElement(fieldH)),
                Read(ContainingMeshElement(fieldE)),
                Read(NeighboringMeshElementOrSelf(fieldH)),
                Read(NeighboringMeshElementOrSelf(fieldE)),
                Write(ContainingMeshElement(rhsH)),
                Write(ContainingMeshElement(rhsE))),
            [&](const auto &element, const auto &face, const auto&, auto &lvs) {
                /* ORIGINAL CODE */
                /*
                const std::size_t face_index = face.GetTopology().GetLocalIndex();
                const RealType face_normal_scaling_factor = 2.0 / element.GetGeometry().GetAbsJacobianDeterminant();

                const Vec3D &face_normal = face.GetGeometry().GetNormal() * face_normal_scaling_factor; //!< get all normal coordinates for each face of an element
                const RealType Edg = face_normal.Norm() * 0.5;                                          //!< get edge length for each face
                const Vec3D &face_unit_normal = face.GetGeometry().GetUnitNormal();
                const auto &localMap{DgNodeMap.Get(element, face)};

                const auto &fieldH = dof::GetDofs<dof::Name::Cell>(std::get<0>(lvs));
                const auto &fieldE = dof::GetDofs<dof::Name::Cell>(std::get<1>(lvs));
                auto &NeighboringFieldH = dof::GetDofs<dof::Name::Cell>(std::get<2>(lvs));
                auto &NeighboringFieldE = dof::GetDofs<dof::Name::Cell>(std::get<3>(lvs));
                auto &rhsH = dof::GetDofs<dof::Name::Cell>(std::get<4>(lvs));
                auto &rhsE = dof::GetDofs<dof::Name::Cell>(std::get<5>(lvs));

                HPM::ForEach(DG::NumSurfaceNodes, [&](const std::size_t m) {
                    const Vec3D &dH = Edg * HPM::DG::Delta(fieldH, NeighboringFieldH, m, localMap); //!< fields differences
                    const Vec3D &dE = Edg * HPM::DG::DirectionalDelta(fieldE, NeighboringFieldE, face, m, localMap);

                    const Vec3D &flux_H = (dH - (dH*face_unit_normal) * face_unit_normal - CrossProduct(face_unit_normal, dE)); //!< fields fluxes
                    const Vec3D &flux_E = (dE - (dE*face_unit_normal) * face_unit_normal + CrossProduct(face_unit_normal, dH));

                    HPM::ForEach(DG::numVolNodes, [&](const auto n) {
                        rhsH[n] += DG::LIFT[face_index][m][n] * flux_H;
                        rhsE[n] += DG::LIFT[face_index][m][n] * flux_E;
                    });
                });
                */
                const std::size_t face_index = face.GetTopology().GetLocalIndex();
                const RealType face_normal_scaling_factor = 2.0 / element.GetGeometry().GetAbsJacobianDeterminant();

                const Vec3D &face_normal = face.GetGeometry().GetNormal() * face_normal_scaling_factor; //!< get all normal coordinates for each face of an element
                const RealType Edg = face_normal.Norm() * 0.5;                                          //!< get edge length for each face
                const Vec3D &face_unit_normal = face.GetGeometry().GetUnitNormal();
                const auto &localMap{DgNodeMap.Get(element, face)};

                const auto &fieldH = dof::GetDofs<dof::Name::Cell>(std::get<0>(lvs));
                const auto &fieldE = dof::GetDofs<dof::Name::Cell>(std::get<1>(lvs));
                auto &NeighboringFieldH = dof::GetDofs<dof::Name::Cell>(std::get<2>(lvs));
                auto &NeighboringFieldE = dof::GetDofs<dof::Name::Cell>(std::get<3>(lvs));
                auto &rhsH = dof::GetDofs<dof::Name::Cell>(std::get<4>(lvs));
                auto &rhsE = dof::GetDofs<dof::Name::Cell>(std::get<5>(lvs));
                Matrix<RealType, 3, DG::NumSurfaceNodes> dE, dH, flux_E, flux_H;

                for (std::size_t m = 0; m < DG::NumSurfaceNodes; ++m)
                {
                    const auto& tmp_1 = Edg * HPM::DG::Delta(fieldH, NeighboringFieldH, m, localMap); //!< fields differences
                    const auto& tmp_2 = Edg * HPM::DG::DirectionalDelta(fieldE, NeighboringFieldE, face, m, localMap);

                    dH[0][m] = tmp_1[0];
                    dH[1][m] = tmp_1[1];
                    dH[2][m] = tmp_1[2];
                    dE[0][m] = tmp_2[0];
                    dE[1][m] = tmp_2[1];
                    dE[2][m] = tmp_2[2];   
                }

                #pragma omp simd
                for (std::size_t m = 0; m < DG::NumSurfaceNodes; ++m)
                {
                    const auto sp_1 = dH[0][m] * face_unit_normal[0] + dH[1][m] * face_unit_normal[1] + dH[2][m] * face_unit_normal[2];
                    flux_H[0][m] = (dH[0][m] - sp_1 * face_unit_normal[0] - (face_unit_normal[1] * dE[2][m] - face_unit_normal[2] * dE[1][m]));
                    flux_H[1][m] = (dH[1][m] - sp_1 * face_unit_normal[1] - (face_unit_normal[2] * dE[0][m] - face_unit_normal[0] * dE[2][m]));
                    flux_H[2][m] = (dH[2][m] - sp_1 * face_unit_normal[2] - (face_unit_normal[0] * dE[1][m] - face_unit_normal[1] * dE[0][m]));
                    
                    const auto sp_2 = dE[0][m] * face_unit_normal[0] + dE[1][m] * face_unit_normal[1] + dE[2][m] * face_unit_normal[2];
                    flux_E[0][m] = (dE[0][m] - sp_2 * face_unit_normal[0] + (face_unit_normal[1] * dH[2][m] - face_unit_normal[2] * dH[1][m]));
                    flux_E[1][m] = (dE[1][m] - sp_2 * face_unit_normal[1] + (face_unit_normal[2] * dH[0][m] - face_unit_normal[0] * dH[2][m]));
                    flux_E[2][m] = (dE[2][m] - sp_2 * face_unit_normal[2] + (face_unit_normal[0] * dH[1][m] - face_unit_normal[1] * dH[0][m]));
                }
                
                #pragma omp simd
                for (std::size_t n = 0; n < DG::numVolNodes; ++n)
                {
                    RealType rhsH_0 = rhsH[n][0], rhsH_1 = rhsH[n][1], rhsH_2 = rhsH[n][2];
                    RealType rhsE_0 = rhsE[n][0], rhsE_1 = rhsE[n][1], rhsE_2 = rhsE[n][2];
                    
                    for (std::size_t m = 0; m < DG::NumSurfaceNodes; ++m)
                    {
                        
                        rhsH_0 += lift[face_index][n][m] * flux_H[0][m];
                        rhsH_1 += lift[face_index][n][m] * flux_H[1][m];
                        rhsH_2 += lift[face_index][n][m] * flux_H[2][m];
                        rhsE_0 += lift[face_index][n][m] * flux_E[0][m];
                        rhsE_1 += lift[face_index][n][m] * flux_E[1][m];
                        rhsE_2 += lift[face_index][n][m] * flux_E[2][m];
                    }

                    rhsH[n] = Vec3D(rhsH_0, rhsH_1, rhsH_2);
                    rhsE[n] = Vec3D(rhsE_0, rhsE_1, rhsE_2);
                }
            });

        auto t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = t2 - t1;
        aggregate_time1 += duration.count();
        t1 = std::chrono::high_resolution_clock::now();

#if defined(SIMD_EXECUTION_POLICY)
        /** \brief Maxwell's volume kernel */
        auto volumeKernelLoop = HPM::ForEachEntity<ExecutionPolicy::SIMD>(
            AllCells,
            std::tuple(
                Read(Cell(fieldH)),
                Read(Cell(fieldE)),
                Cell(rhsH),
                Cell(rhsE)),
            [&](const auto &cell, const auto&, auto &lvs) 
            {
                const std::size_t num_cells = lvs.second;

#if defined(SIMD_SOA)
                RealType D[3][3][num_cells];
                RealType derivative_E[3][3][num_cells], derivative_H[3][3][num_cells]; //!< derivative of fields w.r.t reference coordinates
                RealType f_e[DG::numVolNodes][3][num_cells], f_h[DG::numVolNodes][3][num_cells];
                RealType rhs_e[DG::numVolNodes][3][num_cells], rhs_h[DG::numVolNodes][3][num_cells];

                for (std::size_t id = 0; id < num_cells; ++id)
                {
                    const auto& d = cell[id].GetGeometry().GetInverseJacobian() * 2.0;
                    
                    for (std::size_t j = 0; j < 3; ++j)
                    {
                        for (std::size_t i = 0; i < 3; ++i)
                        {
                            D[j][i][id] = d[j][i];
                        }
                    }
                }

                for (std::size_t id = 0; id < num_cells; ++id)
                {
                    const auto &fieldH = dof::GetDofs<dof::Name::Cell>(std::get<0>(lvs.first[id]));
                    const auto &fieldE = dof::GetDofs<dof::Name::Cell>(std::get<1>(lvs.first[id]));

                    for (std::size_t n = 0; n < DG::numVolNodes; ++n)
                    {
                        for (std::size_t i = 0; i < 3; ++i)
                        {
                            f_h[n][i][id] = fieldH[n][i];
                            f_e[n][i][id] = fieldE[n][i];
                            rhs_h[n][i][id] = 0;
                            rhs_e[n][i][id] = 0;
                        }
                    }
                }

                for (std::size_t n = 0; n < DG::numVolNodes; ++n)
                {
                    for (std::size_t j = 0; j < 3; ++j)
                    {
                        for (std::size_t i = 0; i < 3; ++i)
                        {
                            #pragma omp simd
                            for (std::size_t id = 0; id < num_cells; ++id)
                            {
                                derivative_H[j][i][id] = f_h[0][j][id] * DG::derivative[n][0][i];
                                derivative_E[j][i][id] = f_e[0][j][id] * DG::derivative[n][0][i];
                            }
                        }
                    }
                    
                    for (std::size_t m = 1; m < DG::numVolNodes; ++m)
                    {
                        for (std::size_t j = 0; j < 3; ++j)
                        {
                            for (std::size_t i = 0; i < 3; ++i)
                            {
                                #pragma omp simd
                                for (std::size_t id = 0; id < num_cells; ++id)
                                {
                                    derivative_H[j][i][id] += f_h[m][j][id] * DG::derivative[n][m][i];
                                    derivative_E[j][i][id] += f_e[m][j][id] * DG::derivative[n][m][i];
                                }
                            }
                        }
                    }
                
                    for (std::size_t i = 0; i < 3; ++i)
                    {
                        #pragma omp simd
                        for (std::size_t id = 0; id < num_cells; ++id)
                        {
                            rhs_h[n][0][id] += D[i][1][id] * derivative_E[2][i][id] - D[i][2][id] * derivative_E[1][i][id];
                            rhs_h[n][1][id] += D[i][2][id] * derivative_E[0][i][id] - D[i][0][id] * derivative_E[2][i][id];
                            rhs_h[n][2][id] += D[i][0][id] * derivative_E[1][i][id] - D[i][1][id] * derivative_E[0][i][id];

                            rhs_e[n][0][id] += D[i][1][id] * derivative_H[2][i][id] - D[i][2][id] * derivative_H[1][i][id];
                            rhs_e[n][1][id] += D[i][2][id] * derivative_H[0][i][id] - D[i][0][id] * derivative_H[2][i][id];
                            rhs_e[n][2][id] += D[i][0][id] * derivative_H[1][i][id] - D[i][1][id] * derivative_H[0][i][id];
                        }
                    }
                }

                for (std::size_t id = 0; id < num_cells; ++id)
                {
                    auto &rhsH = dof::GetDofs<dof::Name::Cell>(std::get<2>(lvs.first[id]));
                    auto &rhsE = dof::GetDofs<dof::Name::Cell>(std::get<3>(lvs.first[id]));

                    for (std::size_t n = 0; n < DG::numVolNodes; ++n)
                    {
                        rhsH[n] -= Vec3D(rhs_h[n][0][id], rhs_h[n][1][id], rhs_h[n][2][id]);
                        rhsE[n] += Vec3D(rhs_e[n][0][id], rhs_e[n][1][id], rhs_e[n][2][id]);
                    }
                }
#else // SIMD_SOA
                Mat3D D[num_cells];
                Mat3D derivative_E[num_cells], derivative_H[num_cells]; //!< derivative of fields w.r.t reference coordinates
                Matrix<RealType, DG::numVolNodes, 3> f_e[num_cells], f_h[num_cells], rhs_e[num_cells], rhs_h[num_cells];

                for (std::size_t id = 0; id < num_cells; ++id)
                {
                    const auto& d = cell[id].GetGeometry().GetInverseJacobian() * 2.0;
                    
                    for (std::size_t j = 0; j < 3; ++j)
                    {
                        for (std::size_t i = 0; i < 3; ++i)
                        {
                            D[id][j][i] = d[j][i];
                        }
                    }
                }

                for (std::size_t id = 0; id < num_cells; ++id)
                {
                    const auto &fieldH = dof::GetDofs<dof::Name::Cell>(std::get<0>(lvs.first[id]));
                    const auto &fieldE = dof::GetDofs<dof::Name::Cell>(std::get<1>(lvs.first[id]));

                    for (std::size_t n = 0; n < DG::numVolNodes; ++n)
                    {
                        for (std::size_t i = 0; i < 3; ++i)
                        {
                            f_h[id][n][i] = fieldH[n][i];
                            f_e[id][n][i] = fieldE[n][i];
                            rhs_h[id][n][i] = 0;
                            rhs_e[id][n][i] = 0;
                        }
                    }
                }

                for (std::size_t id = 0; id < num_cells; ++id)
                {
                    for (std::size_t n = 0; n < DG::numVolNodes; ++n)
                    {
                        for (std::size_t j = 0; j < 3; ++j)
                        {
                            for (std::size_t i = 0; i < 3; ++i)
                            {
                                derivative_H[id][j][i] = f_h[id][0][j] * DG::derivative[n][0][i];
                                derivative_E[id][j][i] = f_e[id][0][j] * DG::derivative[n][0][i];
                            }
                        }
                        
                        for (std::size_t m = 1; m < DG::numVolNodes; ++m)
                        {
                            for (std::size_t j = 0; j < 3; ++j)
                            {
                                for (std::size_t i = 0; i < 3; ++i)
                                {
                                    derivative_H[id][j][i] += f_h[id][m][j] * DG::derivative[n][m][i];
                                    derivative_E[id][j][i] += f_e[id][m][j] * DG::derivative[n][m][i];
                                }
                            }
                        }

                        for (std::size_t i = 0; i < 3; ++i)
                        {
                            rhs_h[id][n][0] += D[id][i][1] * derivative_E[id][2][i] - D[id][i][2] * derivative_E[id][1][i];
                            rhs_h[id][n][1] += D[id][i][2] * derivative_E[id][0][i] - D[id][i][0] * derivative_E[id][2][i];
                            rhs_h[id][n][2] += D[id][i][0] * derivative_E[id][1][i] - D[id][i][1] * derivative_E[id][0][i];

                            rhs_e[id][n][0] += D[id][i][1] * derivative_H[id][2][i] - D[id][i][2] * derivative_H[id][1][i];
                            rhs_e[id][n][1] += D[id][i][2] * derivative_H[id][0][i] - D[id][i][0] * derivative_H[id][2][i];
                            rhs_e[id][n][2] += D[id][i][0] * derivative_H[id][1][i] - D[id][i][1] * derivative_H[id][0][i];
                        }
                    }
                }

                for (std::size_t id = 0; id < num_cells; ++id)
                {
                    auto &rhsH = dof::GetDofs<dof::Name::Cell>(std::get<2>(lvs.first[id]));
                    auto &rhsE = dof::GetDofs<dof::Name::Cell>(std::get<3>(lvs.first[id]));    

                    for (std::size_t n = 0; n < DG::numVolNodes; ++n)
                    {
                        rhsH[n] -= Vec3D(rhs_h[id][n][0], rhs_h[id][n][1], rhs_h[id][n][2]);
                        rhsE[n] += Vec3D(rhs_e[id][n][0], rhs_e[id][n][1], rhs_e[id][n][2]);
                    }
                }
#endif
            });
#else // SIMD_EXECUTION_POLICY
        /** \brief Maxwell's volume kernel */
        RealType dg_derivative[3][DG::numVolNodes][DG::numVolNodes];

        for (std::size_t i = 0; i < 3; ++i)
        {
            for (std::size_t m = 0; m < DG::numVolNodes; ++m)
            {
                for (std::size_t n = 0; n < DG::numVolNodes; ++n)
                {
                    dg_derivative[i][m][n] = DG::derivative[n][m][i];
                }
            }
        }

        auto volumeKernelLoop = HPM::ForEachEntity(
            AllCells,
            std::tuple(
                Read(Cell(fieldH)),
                Read(Cell(fieldE)),
                Cell(rhsH),
                Cell(rhsE)),
            [&](const auto &element, auto&&, auto &lvs) 
            {
                const Mat3D &D = element.GetGeometry().GetInverseJacobian() * 2.0;
                const auto &fieldH = dof::GetDofs<dof::Name::Cell>(std::get<0>(lvs));
                const auto &fieldE = dof::GetDofs<dof::Name::Cell>(std::get<1>(lvs));
                auto &rhsH = dof::GetDofs<dof::Name::Cell>(std::get<2>(lvs));
                auto &rhsE = dof::GetDofs<dof::Name::Cell>(std::get<3>(lvs));

                /* ORIGINAL CODE */
                /*
                HPM::ForEach(DG::numVolNodes, [&](const auto n) {
                    Mat3D derivative_E, derivative_H; //!< derivative of fields w.r.t reference coordinates

                    HPM::ForEach(DG::numVolNodes, [&](const auto m) {
                        derivative_H += DyadicProduct(DG::derivative[n][m], fieldH[m]);
                        derivative_E += DyadicProduct(DG::derivative[n][m], fieldE[m]);
                    });

                    rhsH[n] -= Curl(D, derivative_E); //!< first half of right-hand-side of fields
                    rhsE[n] += Curl(D, derivative_H);
                });
                */
                
                Matrix<RealType, 3, DG::numVolNodes> derivative_E, derivative_H;

                for (std::size_t i = 0; i < 3; ++i)
                {
                    for (std::size_t j = 0; j < 3; ++j)
                    {
                        #pragma omp simd
                        for (std::size_t n = 0; n < DG::numVolNodes; ++n)
                        {
                            derivative_E[j][n] = fieldH[0][j] * dg_derivative[i][0][n];
                            derivative_H[j][n] = fieldE[0][j] * dg_derivative[i][0][n];
                        }
                    }

                    for (std::size_t m = 1; m < DG::numVolNodes; ++m)
                    {
                        for (std::size_t j = 0; j < 3; ++j)
                        {
                            #pragma omp simd
                            for (std::size_t n = 0; n < DG::numVolNodes; ++n)
                            {
                                derivative_E[j][n] += fieldH[m][j] * dg_derivative[i][m][n];
                                derivative_H[j][n] += fieldE[m][j] * dg_derivative[i][m][n];
                            }
                        }
                    }
                
                    #pragma omp simd
                    for (std::size_t n = 0; n < DG::numVolNodes; ++n)
                    {
                        rhsE[n] += Vec3D(
                            D[i][1] * derivative_E[2][n] - D[i][2] * derivative_E[1][n],
                            D[i][2] * derivative_E[0][n] - D[i][0] * derivative_E[2][n],
                            D[i][0] * derivative_E[1][n] - D[i][1] * derivative_E[0][n]);

                        rhsH[n] -= Vec3D(
                            D[i][1] * derivative_H[2][n] - D[i][2] * derivative_H[1][n],
                            D[i][2] * derivative_H[0][n] - D[i][0] * derivative_H[2][n],
                            D[i][0] * derivative_H[1][n] - D[i][1] * derivative_H[0][n]);
                    }
                }
            });
#endif

        t2 = std::chrono::high_resolution_clock::now();
        duration = t2 - t1;
        aggregate_time2 += duration.count();
        t1 = std::chrono::high_resolution_clock::now();

        /** \brief Runge-Kutta integrtion kernel */
        auto rungeKuttaLoop = HPM::ForEachEntity(
            AllCells,
            std::tuple(
                Write(Cell(fieldH)),
                Write(Cell(fieldE)),
                Cell(rhsH),
                Cell(rhsE),
                Cell(resH),
                Cell(resE)),
            [&](const auto &, const auto &iter, auto &lvs) {
                const auto &RKstage = RungeKuttaCoeff<RealType>::rk4[iter % 5];
                auto &fieldH = dof::GetDofs<dof::Name::Cell>(std::get<0>(lvs));
                auto &fieldE = dof::GetDofs<dof::Name::Cell>(std::get<1>(lvs));
                auto &rhsH = dof::GetDofs<dof::Name::Cell>(std::get<2>(lvs));
                auto &rhsE = dof::GetDofs<dof::Name::Cell>(std::get<3>(lvs));
                auto &resH = dof::GetDofs<dof::Name::Cell>(std::get<4>(lvs));
                auto &resE = dof::GetDofs<dof::Name::Cell>(std::get<5>(lvs));

                HPM::ForEach(DG::numVolNodes, [&](const auto n) {
                    resH[n] = RKstage[0] * resH[n] + timeStep * rhsH[n]; //!< residual fields
                    resE[n] = RKstage[0] * resE[n] + timeStep * rhsE[n];
                    fieldH[n] += RKstage[1] * resH[n]; //!< updated fields
                    fieldE[n] += RKstage[1] * resE[n];
                    rhsH[n] = 0.0; //TODO
                    rhsE[n] = 0.0;
                });
            });

        body.Execute( HPM::iterator::Range<size_t> { static_cast<std::size_t>(((finalTime - startTime) / timeStep) * 5) },
                        surfaceKernelLoop, volumeKernelLoop, rungeKuttaLoop);

        t2 = std::chrono::high_resolution_clock::now();
        duration = t2 - t1;
        aggregate_time3 += duration.count();
    }

    std::cout << "Aggregate execution time for Surface kernel       = " << aggregate_time1 * 1000 << " ms" << std::endl;
    std::cout << "Aggregate execution time for Volume kernel        = " << aggregate_time2 * 1000 << " ms" << std::endl;
    std::cout << "Aggregate execution time for RK kernel            = " << aggregate_time3 * 1000 << " ms" << std::endl;
    std::cout << "Aggregate all kernel execution time               = " << (aggregate_time1 + aggregate_time2 + aggregate_time3) * 1000 << " ms" << std::endl;
    std::cout << "Individual Execution time of Surface kernel       = " << (aggregate_time1 * 1000) / (finalTime / timeStep * 5) << " ms" << std::endl;
    std::cout << "Individual Execution time of Volume kernel        = " << (aggregate_time2 * 1000) / (finalTime / timeStep * 5) << " ms" << std::endl;
    std::cout << "Individual Execution time of RK kernel            = " << (aggregate_time3 * 1000) / (finalTime / timeStep * 5) << " ms" << std::endl;
    std::cout << "Individual all kernel execution time              = " << ((aggregate_time1 + aggregate_time2 + aggregate_time3) * 1000) / (finalTime / timeStep * 5) << " ms" << std::endl;
    
    /** \brief find maximum & minimum values for Ey*/
    double maxErrorEy = 0;
    double minEy = std::numeric_limits<RealType>::max();
    double maxEy = std::numeric_limits<RealType>::lowest();
    
    body.Execute(
        HPM::ForEachEntity(
        AllCells,
        std::tuple( Read(Cell(fieldE)) ),
        [&](const auto& element, auto&&, auto lvs)
        {    
            const auto& fieldE = dof::GetDofs<dof::Name::Cell>(std::get<0>(lvs));

            HPM::ForEach(DG::numVolNodes, [&] (const std::size_t n)
            {
                const auto& nodeCoords = DG::LocalToGlobal(DG::referenceCoords[n], element.GetTopology().GetNodes()); 		                         //!< reference to global nodal coordinates
                const RealType exactEy = sin(M_PI * nodeCoords.x) * sin(M_PI * nodeCoords.z) * cos(sqrt(2.) * M_PI * finalTime); 	                            //!< exact analytical electrical field value in y direction
                maxErrorEy = std::max(maxErrorEy, std::abs(exactEy - fieldE[n].y));   //!< maximum error in electrical field value in y direction
                minEy = std::min(minEy, fieldE[n].y);                                 //!< minimum electric field value in y direction
                maxEy = std::max(maxEy, fieldE[n].y);  
            });
        })
    );

    std::cout << "\nt=" << finalTime
        << " Ey in [ " << minEy
        << ", " << maxEy
        << " ] with max nodal error " << maxErrorEy
        << std::endl;
#else
    constexpr auto Dofs = ::HPM::dof::MakeDofs<0, 0, 0, 1, 0>();

    /** \brief load initial conditions for fields */
    auto field { hpm.GetBuffer<double>(mesh, Dofs) }; 

    HPM::SequentialDispatcher body;

    double start_time = omp_get_wtime();
    
    body.Execute(
        HPM::ForEachEntity<ExecutionPolicy::SIMD>(
            AllCells, 
            std::tuple(ReadWrite(Cell(field))),
            [&] (const auto& cell, auto&&, auto&& lvs)
            {
                const std::size_t num_entries = lvs.second;
                double field[num_entries];

                for (std::size_t i = 0; i < num_entries; ++i)
                {
                    field[i] = dof::GetDofs<dof::Name::Cell>(std::get<0>(lvs.first[i]))[0];
                }

                #pragma omp simd
                for (std::size_t i = 0; i < num_entries; ++i)
                {
                    const double tmp = std::exp(M_PI * field[i]);
                    field[i] = std::log(tmp + 0.4);
                }

                for (std::size_t i = 0; i < num_entries; ++i)
                {
                    dof::GetDofs<dof::Name::Cell>(std::get<0>(lvs.first[i]))[0] = field[i];
                }
            }
        )
    );
    /*
    body.Execute(
        HPM::ForEachEntity(
            AllCells, 
            std::tuple(ReadWrite(Cell(field))),
            [&] (const auto& cell, auto&&, auto&& lvs)
            {
                auto& field = dof::GetDofs<dof::Name::Cell>(std::get<0>(lvs));

                const double tmp = std::exp(M_PI * field[0]);
                field[0] = std::log(tmp + 0.4);
            }
        )
    );
    */
    
    double end_time = omp_get_wtime();
    std::cout << "time: " << (end_time - start_time) * 1.0E3 << std::endl;
#endif
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                         Shutdown of the runtime system                                               //
    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    return EXIT_SUCCESS;
}
