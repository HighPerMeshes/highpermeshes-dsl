#ifndef WRITELOOP_HPP
#define WRITELOOP_HPP

#include <mutex>
#include <fstream>
#include <HighPerMeshes.hpp>

namespace HPM {

//! prints values to a stream
//! \{
template<typename Stream, typename T> void print(Stream&& stream, T&& value) {
    stream << std::forward<T>(value);
}

template<typename Stream, typename T, size_t Dim> void print(Stream&& stream, const HPM::dataType::Vec<T, Dim>& value) {
    for(auto& val : value.data) {
        stream << val << " ";
    }
}

template<typename Stream, typename T, size_t Dim> void print(Stream&& stream, HPM::dataType::Vec<T, Dim>& value) {
    for(auto& val : value.data) {
        stream << val << " ";
    }
}
// \}

//! pre-condition for printing. Always prints
auto Always() { return [](const auto& /* not used */, const auto& /*time_step*/) { return true; }; }

//! pre-condition for printing. Only prints every nth time-step
template<typename T>
auto EveryNthStep(T val, T offset = T{}) {
    return [val, offset](const auto& /* not used */, const auto& time_step) {
        return (time_step + offset) % val == 0;
    };
}

//! WriteLoop provides a loop that writes the dof entries of a specific range to a file
//! \param file the file to write to
//! \param range The range to iterate over
//! \param buffer The entries of this buffer are written to file
//! \param condition A condition that determines if the entry should be written to the file, i.e., entries are only written to the file if condition(entity, time_step) == true.
//! \return A MeshLoop that is usable by the rts that writes result back to file
template<size_t Dimension, typename Mesh, typename Buffer, typename Condition = decltype(Always())>
auto WriteLoop(std::mutex& mutex, std::ofstream& file, HPM::mesh::Range<Dimension, Mesh>& range, Buffer& buffer, Condition condition = Always()) {

    using namespace HPM;

    return ForEachEntity(
        range,
        std::tuple( RequestDim<Dimension>(buffer) ),
        [&, condition](const auto &entity, auto&& time_step, auto lvs) {
            auto &field = std::get<0>(lvs);

            if(!condition(entity, time_step)) return;

            ForEach(std::decay_t<Buffer>::DofT::Get()[Dimension], [&](auto e) {
                
                std::lock_guard guard { mutex };

                std::stringstream ss;
                ss  << "{" << "\n"
                    << "\t" << "index: " << entity.GetTopology().GetIndex() << "\n"
                    << "\t" << "time_step: " << time_step << "\n"
                    << "\t" << "Dof: " << e << "\n"
                    << "\t" << "Value: "; 
                print(ss, field[e]);
                ss << "\n}" << "\n";

                file << ss.str();

            });
        });
}

}

#endif /* WRITELOOP_HPP */
