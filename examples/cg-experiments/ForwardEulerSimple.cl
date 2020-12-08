
void kernel function_1(double global * localVectors_0, double global * localVectors_1)
{
    double tau = 0.2;
    
    int id = get_global_id(0);
    localVectors_0[id]+=tau*localVectors_1[id];

}


