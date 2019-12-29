# pragma once

#include <array>

template <typename real_t>
struct RungeKuttaCoeff
{
public:
  using Value = std::array<real_t, 2>;
  using Container = std::array<Value, 5>;
  static Container const rk4;
};

/** \brief low storage RK coefficients*/
template <typename real_t>
typename RungeKuttaCoeff<real_t>::Container const RungeKuttaCoeff<real_t>::rk4
  {{Value{real_t{0.0},                                real_t{1432997174477.0 / 9575080441755.0}},
    Value{real_t{-567301805773.0 / 1357537059087.0},  real_t{5161836677717.0 / 13612068292357.0}},
    Value{real_t{-2404267990393.0 / 2016746695238.0}, real_t{1720146321549.0 / 2090206949498.0}},
    Value{real_t{-3550918686646.0 / 2091501179385.0}, real_t{3134564353537.0 / 4481467310338.0}},
    Value{real_t{-1275806237668.0 / 842570457699.0},  real_t{2277821191437.0 / 14882151754819.0}}}};
