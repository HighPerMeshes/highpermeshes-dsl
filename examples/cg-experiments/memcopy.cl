void kernel function_0(double global * lvs_0, double global * lvs_1)
{
    size_t index = get_global_id(0);
    lvs_0[index] = lvs_1[index];

}



