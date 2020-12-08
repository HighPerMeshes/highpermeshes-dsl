typedef struct Vector_double_3{
 double values[3];
} Vector_double_3;

double constant rk4[5][2] = {{0.0, 0.1496590219992291}, {-0.4178904744998520, 0.3792103129996273}, {-1.1921516946426769, 0.8229550293869817}, {-1.6977846924715278, 0.6994504559491221}, {-1.5141834442571558, 0.1530572479681520}};

void kernel RK(unsigned long const iter, float3 global * localVectors_0, struct Vector_double_3 global * localVectors_1, struct Vector_double_3 global * localVectors_2, struct Vector_double_3 global * localVectors_3, struct Vector_double_3 global * localVectors_4, struct Vector_double_3 global * localVectors_5,
unsigned long off_0,unsigned long off_1,unsigned long off_2,unsigned long off_3,unsigned long off_4,unsigned long off_5 )
{

    double constant * RKstage = rk4[iter%5];
    struct Vector_double_3 global * fieldH = localVectors_0+get_global_id(0);
    struct Vector_double_3 global * fieldE = localVectors_1+get_global_id(0);
    struct Vector_double_3 global * rhsH = localVectors_2+get_global_id(0);
    struct Vector_double_3 global * rhsE = localVectors_3+get_global_id(0);
    struct Vector_double_3 global * resH = localVectors_4+get_global_id(0);
    struct Vector_double_3 global * resE = localVectors_5+get_global_id(0);
    
    resH->values[0] = RKstage[0] * resH->values[0] + rhsH->values[0];
    resH->values[1] = RKstage[0] * resH->values[1] + rhsH->values[1];
    resH->values[2] = RKstage[0] * resH->values[2] + rhsH->values[2];

    resE->values[0] = RKstage[0] * resE->values[0] + rhsE->values[0];
    resE->values[1] = RKstage[0] * resE->values[1] + rhsE->values[1];
    resE->values[2] = RKstage[0] * resE->values[2] + rhsE->values[2];

    fieldH->values[0] += RKstage[1] * resH->values[0];
    fieldH->values[1] += RKstage[1] * resH->values[1];
    fieldH->values[2] += RKstage[1] * resH->values[2];

    fieldE->values[0] += RKstage[1] * resE->values[0];
    fieldE->values[1] += RKstage[1] * resE->values[1];
    fieldE->values[2] += RKstage[1] * resE->values[2];

    rhsH->values[0] = 0.0;
    rhsH->values[1] = 0.0;
    rhsH->values[2] = 0.0;

    rhsE->values[0] = 0.0;
    rhsE->values[1] = 0.0;
    rhsE->values[2] = 0.0;
    
}


