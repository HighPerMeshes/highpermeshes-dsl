#ifndef DATA_HPP
#define DATA_HPP

#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <vector>

struct entry {
    size_t index;    
    size_t time_step;
    size_t Dof;
    std::vector<double> values; 
};

BOOST_FUSION_ADAPT_STRUCT(
    entry,
    (size_t, index)    
    (size_t, time_step)
    (size_t, Dof)
    (std::vector<double>, values)
)

#endif /* DATA_HPP */
