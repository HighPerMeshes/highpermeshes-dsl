#ifndef AUXILIARY_MEASURE_TIME_HPP
#define AUXILIARY_MEASURE_TIME_HPP

#include <chrono>

namespace HPM::auxiliary 
{
  //! \param op The function to be measured
  //! \param args If op takes arguments, they can be passed with args
  //! \return the time needed to execute op in nanoseconds
  template<typename Op, typename... Args> auto MeasureTime(Op&& op, Args&&... args) -> std::chrono::nanoseconds {
    auto start = std::chrono::steady_clock::now();
    std::forward<Op>(op)(std::forward<Args>(args)...);
    auto end = std::chrono::steady_clock::now();
    return end - start;
  }


}

#endif /* MEASURE_HPP */
