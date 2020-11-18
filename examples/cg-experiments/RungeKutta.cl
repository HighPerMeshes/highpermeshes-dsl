struct Vector_double_3{
 double values[3];
};
int const constant numVolNodes = 20;

double constant rk4[5][2] = {{0.0, 1432997174477.0/9575080441755.0}, {-567301805773.0/1357537059087.0, 5161836677717.0/13612068292357.0}, {-2404267990393.0/2016746695238.0, 1720146321549.0/2090206949498.0}, {-3550918686646.0/2091501179385.0, 3134564353537.0/4481467310338.0}, {-1275806237668.0/842570457699.0, 2277821191437.0/14882151754819.0}};

struct Vector_double_3 function_0(struct Vector_double_3 * lhs, double rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
(*lhs).values[i]*=rhs;
}
;
return (*lhs);
}


struct Vector_double_3 function_1(double lhs, struct Vector_double_3 const rhs)
{
struct Vector_double_3 copy = rhs;
function_0((&copy), lhs);
return copy;
}


struct Vector_double_3 function_2(struct Vector_double_3 * lhs, struct Vector_double_3 const rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
(*lhs).values[i]+=rhs.values[i];
}
;
return (*lhs);
}


struct Vector_double_3 function_3(struct Vector_double_3 const lhs, struct Vector_double_3 const rhs)
{
struct Vector_double_3 copy = lhs;
function_2((&copy), rhs);
return copy;
}


struct Vector_double_3 function_4(struct Vector_double_3 * lhs, double rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
(*lhs).values[i]*=rhs;
}
;
return (*lhs);
}


struct Vector_double_3 function_5(double lhs, struct Vector_double_3 const rhs)
{
struct Vector_double_3 copy = rhs;
function_4((&copy), lhs);
return copy;
}


struct Vector_double_3 function_6(struct Vector_double_3 * lhs, struct Vector_double_3 const rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
(*lhs).values[i]+=rhs.values[i];
}
;
return (*lhs);
}


struct Vector_double_3 function_7(struct Vector_double_3 const lhs, struct Vector_double_3 const rhs)
{
struct Vector_double_3 copy = lhs;
function_6((&copy), rhs);
return copy;
}


struct Vector_double_3 function_8(struct Vector_double_3 * lhs, double rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
(*lhs).values[i]*=rhs;
}
;
return (*lhs);
}


struct Vector_double_3 function_9(double lhs, struct Vector_double_3 const rhs)
{
struct Vector_double_3 copy = rhs;
function_8((&copy), lhs);
return copy;
}


struct Vector_double_3 function_10(struct Vector_double_3 global * lhs, struct Vector_double_3 const rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
(*lhs).values[i]+=rhs.values[i];
}
;
return (*lhs);
}


struct Vector_double_3 function_11(struct Vector_double_3 * lhs, double rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
(*lhs).values[i]*=rhs;
}
;
return (*lhs);
}


struct Vector_double_3 function_12(double lhs, struct Vector_double_3 const rhs)
{
struct Vector_double_3 copy = rhs;
function_11((&copy), lhs);
return copy;
}


struct Vector_double_3 function_13(struct Vector_double_3 global * lhs, struct Vector_double_3 const rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
(*lhs).values[i]+=rhs.values[i];
}
;
return (*lhs);
}


void function_14(struct Vector_double_3 global * vec, double new_value)
{
for(unsigned long i = 0; i<3; ++i)
{
(*vec).values[i]=new_value;
}
;
}


void function_15(struct Vector_double_3 global * vec, double new_value)
{
for(unsigned long i = 0; i<3; ++i)
{
(*vec).values[i]=new_value;
}
;
}


void function_16(unsigned long const iter, struct Vector_double_3 global * lvs_0, struct Vector_double_3 global * lvs_1, struct Vector_double_3 global * lvs_2, struct Vector_double_3 global * lvs_3, struct Vector_double_3 global * lvs_4, struct Vector_double_3 global * lvs_5, unsigned long hpm_offset_0, unsigned long hpm_offset_1, unsigned long hpm_offset_2, unsigned long hpm_offset_3, unsigned long hpm_offset_4, unsigned long hpm_offset_5)
{
size_t const dof_0_0_0_20_0[5] = {0, 0, 0, 20, 0};
double constant * RKstage = rk4[iter%5];
struct Vector_double_3 global * fieldH = lvs_0+get_global_id(0)*20+hpm_offset_0;
struct Vector_double_3 global * fieldE = lvs_1+get_global_id(0)*20+hpm_offset_1;
struct Vector_double_3 global * rhsH = lvs_2+get_global_id(0)*20+hpm_offset_2;
struct Vector_double_3 global * rhsE = lvs_3+get_global_id(0)*20+hpm_offset_3;
struct Vector_double_3 global * resH = lvs_4+get_global_id(0)*20+hpm_offset_4;
struct Vector_double_3 global * resE = lvs_5+get_global_id(0)*20+hpm_offset_5;
for(unsigned long n = 0; n<numVolNodes; ++n)
{
resH[n]=function_3(function_1(RKstage[0], resH[n]), rhsH[n]);;
resE[n]=function_7(function_5(RKstage[0], resE[n]), rhsE[n]);;
function_10((&fieldH[n]), function_9(RKstage[1], resH[n]));
function_13((&fieldE[n]), function_12(RKstage[1], resE[n]));
function_14((&rhsH[n]), 0.0);
function_15((&rhsE[n]), 0.0);
}
;
}


void kernel function_17(unsigned long const iter, struct Vector_double_3 global * localVectors_0, struct Vector_double_3 global * localVectors_1, struct Vector_double_3 global * localVectors_2, struct Vector_double_3 global * localVectors_3, struct Vector_double_3 global * localVectors_4, struct Vector_double_3 global * localVectors_5, unsigned long hpm_offset_0, unsigned long hpm_offset_1, unsigned long hpm_offset_2, unsigned long hpm_offset_3, unsigned long hpm_offset_4, unsigned long hpm_offset_5)
{
function_16(iter, localVectors_0, localVectors_1, localVectors_2, localVectors_3, localVectors_4, localVectors_5, hpm_offset_0, hpm_offset_1, hpm_offset_2, hpm_offset_3, hpm_offset_4, hpm_offset_5);
}


