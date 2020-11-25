void kernel foo(int global * localVectors_0)
{
    printf("%d", get_global_id(0));   
    int id = get_global_id(0); //(get_global_id(0)*get_local_size(0)+get_local_id(0));
    localVectors_0[id] = id;
}


