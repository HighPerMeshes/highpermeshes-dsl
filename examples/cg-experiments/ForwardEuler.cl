int GetOffset(
	size_t mesh_info_size,
	size_t global const * mesh_info,
	int dimensionality,
	size_t const * dofs
) {
	int res = 0;
	for(int dimension = mesh_info_size-1; dimension > dimensionality; --dimension) {
		    res += mesh_info[dimension] * dofs[dimension];
	}
	return res;
}
void function_0(int global const * mesh_info, int mesh_info_size, unsigned long const omitted_0, double global * lvs_0, double global * lvs_1)
{
size_t const dof_1_0_0_0_0[5] = {1, 0, 0, 0, 0};
double global * u = lvs_0+get_global_id(0)*dof_1_0_0_0_0[0]+GetOffset(mesh_info_size, mesh_info, 0, dof_1_0_0_0_0);
double global * u_d = lvs_1+get_global_id(0)*dof_1_0_0_0_0[0]+GetOffset(mesh_info_size, mesh_info, 0, dof_1_0_0_0_0);
double tau = 0.20000000000000001;
u[0]+=tau*u_d[0];
}


void kernel function_1(int global const * mesh_info, int mesh_info_size, unsigned long const iter, double global * localVectors_0, double global * localVectors_1)
{
function_0(mesh_info, mesh_info_size, iter, localVectors_0, localVectors_1);
}


