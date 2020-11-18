struct Vector3 {     double values[3]; }; struct Vector3 make_Vector3(double arg_1, double arg_2, double arg_3) {     struct Vector3 vec = { .values = { arg_1, arg_2, arg_3 } };     return vec; } void add_assign_Vector3(struct Vector3 * lhs, struct Vector3 rhs) {     for(int i = 0; i < 3; ++i) {         lhs->values[i] += rhs.values[i];     } } void add_scalar_assign_Vector3(struct Vector3 * lhs, double rhs) {     for(int i = 0; i < 3; ++i) {         lhs->values[i] += rhs;     } } void times_assign_Vector3(struct Vector3 * lhs, struct Vector3 rhs) {     for(int i = 0; i < 3; ++i) {         lhs->values[i] *= rhs.values[i];     } } void times_scalar_assign_Vector3(struct Vector3 * lhs, double rhs) {     for(int i = 0; i < 3; ++i) {         lhs->values[i] *= rhs;     } } void divides_assign_Vector3(struct Vector3 * lhs, struct Vector3 rhs) {     for(int i = 0; i < 3; ++i) {         lhs->values[i] /= rhs.values[i];     } } void minus_assign_Vector3(struct Vector3 * lhs, struct Vector3 rhs) {     for(int i = 0; i < 3; ++i) {         lhs->values[i] -= rhs.values[i];     } } struct Vector3 add_Vector3(struct Vector3 lhs, struct Vector3 rhs) {     add_assign_Vector3(&lhs, rhs);     return lhs; } struct Vector3 times_Vector3(struct Vector3 lhs, struct Vector3 rhs) {     times_assign_Vector3(&lhs, rhs);     return lhs; } struct Vector3 add_scalar_Vector3(struct Vector3 lhs, double rhs) {     add_scalar_assign_Vector3(&lhs, rhs);     return lhs; } struct Vector3 times_scalar_Vector3(struct Vector3 lhs, double rhs) {     times_scalar_assign_Vector3(&lhs, rhs);     return lhs; } struct Vector3 divides_Vector3(struct Vector3 lhs, struct Vector3 rhs) {     divides_assign_Vector3(&lhs, rhs);     return lhs; } struct Vector3 minus_Vector3(struct Vector3 lhs, struct Vector3 rhs) {     minus_assign_Vector3(&lhs, rhs);     return lhs; }double4 function_0(double arg0, double arg1, double arg2)
{
double4 result_float = {arg0, arg1, arg2, 0};
return result_float;
}


void function_1(unsigned long const omitted_0, double4 global * lvs_0, double4 global * lvs_1, double4 global * lvs_2, double4 global * lvs_3, unsigned long hpm_offset_0, unsigned long hpm_offset_1, unsigned long hpm_offset_2, unsigned long hpm_offset_3)
{
size_t const dof_0_0_0_20_0[5] = {0, 0, 0, 20, 0};
double4 global * rhsH = lvs_2+get_global_id(0)*20+hpm_offset_2;
rhsH[0]=function_0(0, 0, 0);;
}


void kernel function_2(unsigned long const iter, double4 global * localVectors_0, double4 global * localVectors_1, double4 global * localVectors_2, double4 global * localVectors_3, unsigned long hpm_offset_0, unsigned long hpm_offset_1, unsigned long hpm_offset_2, unsigned long hpm_offset_3)
{
function_1(iter, localVectors_0, localVectors_1, localVectors_2, localVectors_3, hpm_offset_0, hpm_offset_1, hpm_offset_2, hpm_offset_3);
}


