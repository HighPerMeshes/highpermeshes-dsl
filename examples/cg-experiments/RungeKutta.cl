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
int const constant numVolNodes = 20;

double constant rk4[5][2] = {{0, 1432997174477/9575080441755}, {-567301805773/1357537059087, 5161836677717/13612068292357}, {-2404267990393/2016746695238, 1720146321549/2090206949498}, {-3550918686646/2091501179385, 3134564353537/4481467310338}, {-1275806237668/842570457699, 2277821191437/14882151754819}};

double4 function_0(double4 lhs, double rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]*=rhs;
}
;
return lhs;
}


double4 function_1(double lhs, double4 const rhs)
{
double4 copy = rhs;
function_0(copy, lhs);
return copy;
}


double4 function_2(double4 lhs, double4 const rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]+=rhs[i];
}
;
return lhs;
}


double4 function_3(double4 const lhs, double4 const rhs)
{
double4 copy = lhs;
function_2(copy, rhs);
return copy;
}


double4 function_4(double4 lhs, double rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]*=rhs;
}
;
return lhs;
}


double4 function_5(double lhs, double4 const rhs)
{
double4 copy = rhs;
function_4(copy, lhs);
return copy;
}


double4 function_6(double4 lhs, double4 const rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]+=rhs[i];
}
;
return lhs;
}


double4 function_7(double4 const lhs, double4 const rhs)
{
double4 copy = lhs;
function_6(copy, rhs);
return copy;
}


double4 function_8(double4 lhs, double rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]*=rhs;
}
;
return lhs;
}


double4 function_9(double lhs, double4 const rhs)
{
double4 copy = rhs;
function_8(copy, lhs);
return copy;
}


double4 function_10(double4 lhs, double4 const rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]+=rhs[i];
}
;
return lhs;
}


double4 function_11(double4 lhs, double rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]*=rhs;
}
;
return lhs;
}


double4 function_12(double lhs, double4 const rhs)
{
double4 copy = rhs;
function_11(copy, lhs);
return copy;
}


double4 function_13(double4 lhs, double4 const rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]+=rhs[i];
}
;
return lhs;
}


void function_14(double4 vec, double new_value)
{
for(unsigned long i = 0; i<3; ++i)
{
vec[i]=new_value;
}
;
}


void function_15(double4 vec, double new_value)
{
for(unsigned long i = 0; i<3; ++i)
{
vec[i]=new_value;
}
;
}


void function_16(int global const * mesh_info, int mesh_info_size, unsigned long const iter, double4 global * lvs_0, double4 global * lvs_1, double4 global * lvs_2, double4 global * lvs_3, double4 global * lvs_4, double4 global * lvs_5)
{
size_t const dof_0_0_0_20_0[5] = {0, 0, 0, 20, 0};
double constant * RKstage = rk4[iter%5];
double4 global * fieldH = lvs_0+get_global_id(0)*dof_0_0_0_20_0[3]+GetOffset(mesh_info_size, mesh_info, 3, dof_0_0_0_20_0);
double4 global * fieldE = lvs_1+get_global_id(0)*dof_0_0_0_20_0[3]+GetOffset(mesh_info_size, mesh_info, 3, dof_0_0_0_20_0);
double4 global * rhsH = lvs_2+get_global_id(0)*dof_0_0_0_20_0[3]+GetOffset(mesh_info_size, mesh_info, 3, dof_0_0_0_20_0);
double4 global * rhsE = lvs_3+get_global_id(0)*dof_0_0_0_20_0[3]+GetOffset(mesh_info_size, mesh_info, 3, dof_0_0_0_20_0);
double4 global * resH = lvs_4+get_global_id(0)*dof_0_0_0_20_0[3]+GetOffset(mesh_info_size, mesh_info, 3, dof_0_0_0_20_0);
double4 global * resE = lvs_5+get_global_id(0)*dof_0_0_0_20_0[3]+GetOffset(mesh_info_size, mesh_info, 3, dof_0_0_0_20_0);
for(unsigned long n = 0; n<numVolNodes; ++n)
{
resH[n]=function_3(function_1(RKstage[0], resH[n]), rhsH[n]);;
resE[n]=function_7(function_5(RKstage[0], resE[n]), rhsE[n]);;
function_10(fieldH[n], function_9(RKstage[1], resH[n]));
function_13(fieldE[n], function_12(RKstage[1], resE[n]));
function_14(rhsH[n], 0);
function_15(rhsE[n], 0);
}
;
}


void kernel function_17(int global const * mesh_info, int mesh_info_size, unsigned long const iter, double4 global * localVectors_0, double4 global * localVectors_1, double4 global * localVectors_2, double4 global * localVectors_3, double4 global * localVectors_4, double4 global * localVectors_5)
{
function_16(mesh_info, mesh_info_size, iter, localVectors_0, localVectors_1, localVectors_2, localVectors_3, localVectors_4, localVectors_5);
}


