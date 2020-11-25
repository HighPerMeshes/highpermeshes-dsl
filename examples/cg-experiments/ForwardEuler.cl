typedef struct Vector_double_3{
 double values[3];
} Vector_double_3;
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


struct Vector_double_3 function_2(struct Vector_double_3 global * lhs, struct Vector_double_3 const rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
(*lhs).values[i]+=rhs.values[i];
}
;
return (*lhs);
}


void function_3(unsigned long const omitted_0, struct Vector_double_3 global * lvs_0, struct Vector_double_3 global * lvs_1, unsigned long hpm_offset_0, unsigned long hpm_offset_1)
{
size_t const dof_1_0_0_0_0[5] = {1, 0, 0, 0, 0};
struct Vector_double_3 global * u = lvs_0+get_global_id(0)*1+hpm_offset_0;
struct Vector_double_3 global * u_d = lvs_1+get_global_id(0)*1+hpm_offset_1;
double tau = 0.2;
function_2((&u[0]), function_1(tau, u_d[0]));
}


void kernel function_4(unsigned long const iter, struct Vector_double_3 global * localVectors_0, struct Vector_double_3 global * localVectors_1, unsigned long hpm_offset_0, unsigned long hpm_offset_1)
{
function_3(iter, localVectors_0, localVectors_1, hpm_offset_0, hpm_offset_1);
}


