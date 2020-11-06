void function_0(unsigned long const omitted_0, double global * lvs_0, double global * lvs_1, unsigned long hpm_offset_0, unsigned long hpm_offset_1)
{
size_t const dof_1_0_0_0_0[5] = {1, 0, 0, 0, 0};
double global * u = lvs_0+get_global_id(0)*1+hpm_offset_0;
double global * u_d = lvs_1+get_global_id(0)*1+hpm_offset_1;
double tau = 0.20000000000000001;
u[0]+=tau*u_d[0];
}


void kernel function_1(unsigned long const iter, double global * localVectors_0, double global * localVectors_1, unsigned long hpm_offset_0, unsigned long hpm_offset_1)
{
function_0(iter, localVectors_0, localVectors_1, hpm_offset_0, hpm_offset_1);
}


