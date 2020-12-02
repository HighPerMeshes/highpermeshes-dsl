typedef struct Vector_double_3{
 double values[3];
} Vector_double_3;
int const constant NumEulerDofs = 20;

void function_0(unsigned long omitted_0, double global * lvs_0, double global * lvs_1, unsigned long hpm_offset_0, unsigned long hpm_offset_1)
{
size_t const dof_20_0_0_0_0[5] = {20, 0, 0, 0, 0};
double global * u = lvs_0+get_global_id(0)*20+hpm_offset_0;
double global * u_d = lvs_1+get_global_id(0)*20+hpm_offset_1;
double tau = 0.2;
for(unsigned long i = 0; i<NumEulerDofs; ++i)
{
u[i]+=tau*u_d[i];
}
;
}


void kernel function_1(unsigned long const iter, double global * localVectors_0, double global * localVectors_1, unsigned long hpm_offset_0, unsigned long hpm_offset_1)
{
function_0(iter, localVectors_0, localVectors_1, hpm_offset_0, hpm_offset_1);
}


