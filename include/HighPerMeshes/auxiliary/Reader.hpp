// Copyright (c) 2017-2020
//
// Distributed under the MIT Software License
// (See accompanying file LICENSE)

#ifndef AUXILIARY_READER_HPP
#define AUXILIARY_READER_HPP

#include <cctype>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

#include <HighPerMeshes/auxiliary/ArrayOperations.hpp>

namespace HPM::auxiliary
{
    //!
    //! \brief MeshFileReader base class.
    //!
    //! CRTP is used to combine polymorphism and template meta programming.
    //! Only derived classes can instantiate the base class.
    //!
    template <typename CoordinateT, typename ElementNodeIndexT, typename ReaderT>
    class MeshFileReader
    {
        protected:
        MeshFileReader() {}

        public:
        //!
        //! \brief Read node and element sets from a mesh file.
        //!
        //! Nodes and elements are expected to be stored in the mesh file as separate blocks of lines each.
        //! Lines contain a sequence of numbers according to the node dimensionality and the element type.
        //!
        //! Example: for a 3D mesh consisting of tetrahedra, we can use the `Vec<float,3>` type for the
        //! node representation, and `std::array<std::size_t,4>` for the elements.
        //! ```
        //!     std::vector<Vec<float,3>> nodes;
        //!     std::vector<std::array<std::size_t,4> elements;
        //!     reader.ReadNodesAndElements(nodes, elements, "mesh_file");
        //! ```
        //! This function redirects to its derived classes for the actual implementation.
        //!
        //! \param nodes a container to put in the node coordinates
        //! \param elements a container to put in the node index sets representing the elements
        //! \param filename file to read from
        //! \return `true` in case of sucess, otherwise `false`
        //!
        auto ReadNodesAndElements(std::vector<CoordinateT>& nodes, std::vector<ElementNodeIndexT>& elements, const std::string& filename) const
        {
            return static_cast<const ReaderT&>(*this).ReadNodesAndElementsImplementation(nodes, elements, filename);
        }

        //!
        //! \brief Read node and element sets from a mesh file.
        //!
        //! This function redirects to its derived classes for the actual implementation.
        //!
        //! \param filename file to read from
        //! \return a tuple of containers holding the node coordinates and node index sets representing the elements
        //!
        auto ReadNodesAndElements(const std::string& filename) const -> std::tuple<std::vector<CoordinateT>, std::vector<ElementNodeIndexT>>
        {
            return static_cast<const ReaderT&>(*this).ReadNodesAndElementsImplementation(filename);
        }

        //!
        //! \brief Read a group from a mesh file.
        //!
        //! This function redirects to its derived classes for the actual implementation.
        //!
        //! \tparam GroupT the type for the group representation of the data in each line of the mesh file
        //! \param groupname the name of the group (tag)
        //! \param data a container to put in the data
        //! \param filename file to read from
        //! \return `true` in case of sucess, otherwise `false`
        //!
        template <typename GroupT>
        auto ReadGroup(const std::string& groupname, std::vector<GroupT>& data, const std::string& filename) const
        {
            return static_cast<const ReaderT&>(*this).template ReadGroupImplementation<GroupT>(groupname, data, filename);
        }

        //!
        //! \brief Read a set of groups from a mesh file.
        //!
        //! This function redirects to its derived classes for the actual implementation.
        //!
        //! \tparam GroupT_1 the type for the 1st group representation of the data in each line of the mesh file
        //! \tparam GroupT_2 the type for the 2nd group representation of the data in each line of the mesh file
        //! \param filename file to read from
        //! \param groupname_1 the name of the 1st group (tag)
        //! \param groupname_2 the name of the 2st group (tag)
        //! \param index_shift_1 index shifting in the 1st group (ID numbering from 'n' to 'n-1' if true, otherwise it starts at the mesh given numbering)
        //! \param index_shift_2 index shifting in the 2nd group (ID numbering from 'n' to 'n-1' if true, otherwise it starts at the mesh given numbering)
        //! \return a tuple of containers holding the data of the groups
        //!
        template <typename GroupT_1, typename GroupT_2>
        auto ReadGroups(const std::string& filename, const std::string& groupname_1, const std::string& groupname_2, const bool index_shift_1 = false, const bool index_shift_2 = false) const
            -> std::tuple<std::vector<GroupT_1>, std::vector<GroupT_2>>
        {
            return static_cast<const ReaderT&>(*this).template ReadGroupsImplementation<GroupT_1, GroupT_2>(filename, groupname_1, groupname_2, index_shift_1, index_shift_2);
        }
    };

    template <typename CoordinateT, typename ElementNodeIndexT>
    class GambitMeshFileReader : public MeshFileReader<CoordinateT, ElementNodeIndexT, GambitMeshFileReader<CoordinateT, ElementNodeIndexT>>
    {
        using Self = GambitMeshFileReader<CoordinateT, ElementNodeIndexT>;
        using Base = MeshFileReader<CoordinateT, ElementNodeIndexT, Self>;

        friend Base;

        //!
        //! \brief Implementation for reading node and element sets from a 'GAMBIT neutral' mesh file.
        //!
        //! \param nodes a container to put in the nodes
        //! \param elements a container to put in the elements
        //! \param filename file to read from
        //! \return returns 'true', if the arrays are filled otherwise 'false'
        //!
        auto ReadNodesAndElementsImplementation(std::vector<CoordinateT>& nodes, std::vector<ElementNodeIndexT>& elements, const std::string& filename) const
        {
            std::ifstream file(filename);
            std::size_t num_nodes;
            std::size_t num_elements;
            std::size_t dummy;
            std::string line;

            file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            while (std::getline(file, line) && (line.find("NUMNP") == std::string::npos) && (line.find("NELEM") == std::string::npos))
            {
            }
            file >> num_nodes >> num_elements;

            nodes.resize(num_nodes);
            elements.resize(num_elements);

            while (std::getline(file, line) && (line.find("NODAL COORDINATES") == std::string::npos))
            {
            }
            for (auto& node : nodes)
                file >> dummy >> node.x >> node.y >> node.z; // Reading x,y,z coordinates of vertices

            while (std::getline(file, line) && (line.find("ELEMENTS/CELLS") == std::string::npos))
            {
            }
            for (auto& etov : elements)
            {
                //!< ignore first three columns
                file >> dummy >> dummy >> dummy >> etov;
                // std::sort(etov.begin(), etov.end(), std::less<std::size_t>());
                for (auto& v : etov)
                    --v; //!< correct to 0-index, the .neu file provides 1-index
            }

            // std::sort(elements.begin(), elements.end(), std::less<ElementNodeIndexT>());

            return true;
        }

        //!
        //! \brief Read node and element sets from a 'GAMBIT neutral' mesh file.
        //!
        //! \param nodes a container to put in the nodes
        //! \param elements a container to put in the elements
        //! \param filename file to read from
        //! \return returns 'true', if the arrays are filled otherwise 'false'
        //!
        auto ReadNodesAndElementsImplementation(const std::string& filename) const -> std::tuple<std::vector<CoordinateT>, std::vector<ElementNodeIndexT>>
        {
            std::vector<CoordinateT> nodes;
            std::vector<ElementNodeIndexT> elements;

            if (!ReadNodesAndElementsImplementation(nodes, elements, filename))
            {
                std::cerr << "error from GambitMeshFileReader::read_nodes_and_elements() : error while reading from file " << filename << std::endl << std::flush;
            }

            return {nodes, elements};
        }
    };

    template <typename CoordinateT, typename ElementNodeIndexT>
    class AmiraMeshFileReader : public MeshFileReader<CoordinateT, ElementNodeIndexT, AmiraMeshFileReader<CoordinateT, ElementNodeIndexT>>
    {
        using Self = AmiraMeshFileReader<CoordinateT, ElementNodeIndexT>;
        using Base = MeshFileReader<CoordinateT, ElementNodeIndexT, Self>;

        friend Base;

        //!
        //! \brief Implementation for reading node and element sets from a Amira mesh file.
        //!
        //! \param nodes a container to put in the nodes
        //! \param elements a container to put in the elements
        //! \param filename file to read from
        //! \return returns 'true', if the arrays are filled otherwise 'false'
        //!
        auto ReadNodesAndElementsImplementation(std::vector<CoordinateT>& nodes, std::vector<ElementNodeIndexT>& elements, const std::string& filename) const
        {
            // get information about element counts etc.
            std::ifstream file(filename);
            std::size_t num_nodes = 0;
            std::size_t num_elements = 0;
            std::string line;

            while (std::getline(file, line))
            {
                // find position of nodes in the mesh file
                if (num_nodes == 0)
                {
                    const std::size_t pos = line.find("nNodes");
                    if (pos != std::string::npos)
                    {
                        num_nodes = atoi(line.substr(pos + 7).c_str());
                        continue;
                    }
                }

                // find position of elements in the mesh file
                if (num_elements == 0)
                {
                    const std::size_t pos = line.find("nTetrahedra");
                    if (pos != std::string::npos)
                    {
                        num_elements = atoi(line.substr(pos + 12).c_str());
                        continue;
                    }
                }

                // found positions
                if (num_nodes && num_elements)
                    break;
            }

            // did not find nodes and elements
            if (!(num_nodes && num_elements))
                return false;

            // reset the inputstream
            file.seekg(file.beg);

            // read nodes coordinates and element to node mapping
            nodes.resize(num_nodes);
            elements.resize(num_elements);

            // go to @1 section: vertices
            while (std::getline(file, line))
            {
                std::size_t pos = line.find("@1");
                if (pos > 0 || pos == std::string::npos)
                    continue;

                // found
                break;
            }

            for (auto& node : nodes)
                file >> node;

            // go to @3 section: thetrahedra
            while (std::getline(file, line))
            {
                std::size_t pos = line.find("@3");
                if (pos > 0 || pos == std::string::npos)
                    continue;

                // found
                break;
            }

            // index shift for node indices
            for (auto& element : elements)
            {
                file >> element;
                for (auto& node_index : element)
                    --node_index;
            }

            return true;
        }

        //!
        //! \brief Read node and element sets from a mesh file.
        //!
        //! \param filename file to read from
        //! \return a tuple of containers holding the node coordinates and node indices
        //!
        auto ReadNodesAndElementsImplementation(const std::string& filename) const -> std::tuple<std::vector<CoordinateT>, std::vector<ElementNodeIndexT>>
        {
            std::vector<CoordinateT> nodes;
            std::vector<ElementNodeIndexT> elements;

            if (!ReadNodesAndElementsImplementation(nodes, elements, filename))
            {
                std::cerr << "error from AmiraMeshFileReader::read_nodes_and_elements() : error while reading from file " << filename << std::endl << std::flush;
            }

            return {nodes, elements};
        }

        //!
        //! \brief Implementation for reading a group from a mesh file.
        //!
        //! \tparam GroupT the type for the group representation of the data in each line of the mesh file
        //! \param groupname the name of the group (tag)
        //! \param data a container to put in the data
        //! \param filename file to read from
        //! \param index_shift index shifting (ID numbering from 'n' to 'n-1' if true, otherwise it starts at the mesh given numbering)
        //! \return `true` in case of sucess, otherwise `false`
        //!
        template <typename GroupT>
        auto ReadGroupImplementation(const std::string& groupname, std::vector<GroupT>& data, const std::string& filename, const bool index_shift = false) const
        {
            std::ifstream file(filename);
            std::string line;

            // find group position (e.g. boundary elements)
            std::streampos pos_group_elements;
            while (std::getline(file, line))
            {
                std::size_t pos = line.find(groupname);
                if (pos > 0 || pos == std::string::npos)
                    continue;

                pos_group_elements = file.tellg();
                break;
            }

            // get number of group-elements(e.g. number of boundary elements)
            std::size_t num_group_elements = 0;
            while (std::getline(file, line))
            {
                if (line.size() == 0 || !std::isdigit(line[0]))
                    break;
                ++num_group_elements;
            }

            // did not find group-elements
            if (num_group_elements == 0)
                return false;

            data.resize(num_group_elements);

            // fill data with group-elements
            if constexpr (!std::is_void_v<GroupT>)
            {
                for (auto& element : data)
                {
                    GroupT element_index_set;
                    file.seekg(pos_group_elements);
                    file >> element_index_set;
                    pos_group_elements = file.tellg();

                    // index shift if needed (user option)
                    if (index_shift)
                    {
                        for (auto& element_index : element_index_set)
                        {
                            if (element_index != 0)
                                --element_index;
                            else
                                std::cerr << "error from AmiraMeshFileReader::ReadGroupImplementation() : error while shifting element indices from file " << filename << std::endl << std::flush;
                        }
                    }

                    element = {element_index_set};
                }
            }

            return true;
        }

        //!
        //! \brief Read a set of groups from a mesh file.
        //!
        //! \tparam GroupT_1 the type for the 1st group representation of the data in each line of the mesh file
        //! \tparam GroupT_2 the type for the 2nd group representation of the data in each line of the mesh file
        //! \param filename file to read from
        //! \param groupname_1 the name of the 1st group (tag)
        //! \param groupname_2 the name of the 2st group (tag)
        //! \param index_shift_1 index shifting in the 1st group (ID numbering from 'n' to 'n-1' if true, otherwise it starts at the mesh given numbering)
        //! \param index_shift_2 index shifting in the 2nd group (ID numbering from 'n' to 'n-1' if true, otherwise it starts at the mesh given numbering)
        //! \return a tuple of containers holding the data of the groups
        //!
        template <typename GroupT_1, typename GroupT_2>
        auto ReadGroupsImplementation(const std::string& filename, const std::string& groupname_1, const std::string& groupname_2, const bool index_shift_1 = false, const bool index_shift_2 = false) const
            -> std::tuple<std::vector<GroupT_1>, std::vector<GroupT_2>>
        {
            // read 1st group
            std::vector<GroupT_1> group_1;
            ReadGroupImplementation(groupname_1, group_1, filename, index_shift_1);

            // read 2nd group
            std::vector<GroupT_2> group_2;
            ReadGroupImplementation(groupname_2, group_2, filename, index_shift_2);

            // create tuple
            return {group_1, group_2};
        }
    };
} // namespace HPM::auxiliary

#endif