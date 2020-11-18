int const constant numVolNodes = 20;

double constant rk4[5][2] = {{0, 1432997174477/9575080441755}, {-567301805773/1357537059087, 5161836677717/13612068292357}, {-2404267990393/2016746695238, 1720146321549/2090206949498}, {-3550918686646/2091501179385, 3134564353537/4481467310338}, {-1275806237668/842570457699, 2277821191437/14882151754819}};

double4 rk_function_0(double4 lhs, double rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]*=rhs;
}
;
return lhs;
}


double4 rk_function_1(double lhs, double4 const rhs)
{
double4 copy = rhs;
rk_function_0(copy, lhs);
return copy;
}


double4 rk_function_2(double4 lhs, double4 const rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]+=rhs[i];
}
;
return lhs;
}


double4 rk_function_3(double4 const lhs, double4 const rhs)
{
double4 copy = lhs;
rk_function_2(copy, rhs);
return copy;
}


double4 rk_function_4(double4 lhs, double rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]*=rhs;
}
;
return lhs;
}


double4 rk_function_5(double lhs, double4 const rhs)
{
double4 copy = rhs;
rk_function_4(copy, lhs);
return copy;
}


double4 rk_function_6(double4 lhs, double4 const rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]+=rhs[i];
}
;
return lhs;
}


double4 rk_function_7(double4 const lhs, double4 const rhs)
{
double4 copy = lhs;
rk_function_6(copy, rhs);
return copy;
}


double4 rk_function_8(double4 lhs, double rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]*=rhs;
}
;
return lhs;
}


double4 rk_function_9(double lhs, double4 const rhs)
{
double4 copy = rhs;
rk_function_8(copy, lhs);
return copy;
}


double4 rk_function_10(double4 lhs, double4 const rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]+=rhs[i];
}
;
return lhs;
}


double4 rk_function_11(double4 lhs, double rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]*=rhs;
}
;
return lhs;
}


double4 rk_function_12(double lhs, double4 const rhs)
{
double4 copy = rhs;
rk_function_11(copy, lhs);
return copy;
}


double4 rk_function_13(double4 lhs, double4 const rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]+=rhs[i];
}
;
return lhs;
}


void rk_function_14(double4 vec, double new_value)
{
for(unsigned long i = 0; i<3; ++i)
{
vec[i]=new_value;
}
;
}


void rk_function_15(double4 vec, double new_value)
{
for(unsigned long i = 0; i<3; ++i)
{
vec[i]=new_value;
}
;
}


void rk_function_16(unsigned long const iter, double4 global * lvs_0, double4 global * lvs_1, double4 global * lvs_2, double4 global * lvs_3, double4 global * lvs_4, double4 global * lvs_5, unsigned long hpm_offset_0, unsigned long hpm_offset_1, unsigned long hpm_offset_2, unsigned long hpm_offset_3, unsigned long hpm_offset_4, unsigned long hpm_offset_5)
{
size_t const dof_0_0_0_20_0[5] = {0, 0, 0, 20, 0};
double constant * RKstage = rk4[iter%5];
double4 global * fieldH = lvs_0+get_global_id(0)*20+hpm_offset_0;
double4 global * fieldE = lvs_1+get_global_id(0)*20+hpm_offset_1;
double4 global * rhsH = lvs_2+get_global_id(0)*20+hpm_offset_2;
double4 global * rhsE = lvs_3+get_global_id(0)*20+hpm_offset_3;
double4 global * resH = lvs_4+get_global_id(0)*20+hpm_offset_4;
double4 global * resE = lvs_5+get_global_id(0)*20+hpm_offset_5;
for(unsigned long n = 0; n<numVolNodes; ++n)
{
resH[n]=rk_function_3(rk_function_1(RKstage[0], resH[n]), rhsH[n]);;
resE[n]=rk_function_7(rk_function_5(RKstage[0], resE[n]), rhsE[n]);;
rk_function_10(fieldH[n], rk_function_9(RKstage[1], resH[n]));
rk_function_13(fieldE[n], rk_function_12(RKstage[1], resE[n]));
rk_function_14(rhsH[n], 0);
rk_function_15(rhsE[n], 0);
}
;
}


void kernel rk_function_17(unsigned long const iter, double4 global * localVectors_0, double4 global * localVectors_1, double4 global * localVectors_2, double4 global * localVectors_3, double4 global * localVectors_4, double4 global * localVectors_5, unsigned long hpm_offset_0, unsigned long hpm_offset_1, unsigned long hpm_offset_2, unsigned long hpm_offset_3, unsigned long hpm_offset_4, unsigned long hpm_offset_5)
{
rk_function_16(iter, localVectors_0, localVectors_1, localVectors_2, localVectors_3, localVectors_4, localVectors_5, hpm_offset_0, hpm_offset_1, hpm_offset_2, hpm_offset_3, hpm_offset_4, hpm_offset_5);
}


double4 constant derivative[20][20] = {{{-3, -3, -3, 0}, {4.0450849718747302, -3.5802794839105902E-16, 1.3424148928403901E-15, 0}, {-1.5450849718747299, 4.0639432448880901E-16, -5.0237840281546102E-16, 0}, {0.499999999999999, -1.1033485125543401E-16, 2.3498457460162499E-16, 0}, {-6.4601027964871695E-16, 4.0450849718747302, -3.6579136239513901E-16, 0}, {8.6151368970716802E-17, 1.2550331560364E-15, -2.9067657372004098E-16, 0}, {1.4588034090373301E-16, -1.95078747853951E-16, -8.8232214685273503E-17, 0}, {3.22178354602495E-16, -1.5450849718747399, -7.8126440576178098E-16, 0}, {1.2171460603952801E-16, -4.9397890959476905E-16, 2.2581893010046601E-16, 0}, {-4.3327171208775298E-17, 0.5, 0, 0}, {-7.5328497009953904E-16, -4.3888168193844901E-16, 4.0450849718747399, 0}, {4.3745938038242298E-16, -2.9515854864301598E-16, 1.9981689672582201E-15, 0}, {-4.76638916594989E-17, 8.3926346559672101E-17, -9.9885204982266197E-16, 0}, {-8.2782092304419702E-16, -6.4983141276633403E-16, 4.5916879626377996E-16, 0}, {1.1402056847584601E-15, -6.7339367032739001E-16, -6.3656695825111102E-16, 0}, {2.0202318328845501E-16, -7.8005789219742104E-16, 0, 0}, {2.71865995752453E-16, -1.9826186921551801E-16, -1.5450849718747399, 0}, {-1.5006500680069801E-16, 2.50039820997204E-16, -1.7824310097090399E-16, 0}, {-1.2180098895175499E-16, 2.5426583017538301E-16, 7.5009719291724797E-16, 0}, {3.3073998916241201E-32, -3.3100455376630302E-17, 0.499999999999999, 0}}, {{-0.80901699437494801, -0.70901699437494803, -0.70901699437494803, 0}, {5.7331670465990098E-16, -2, -2, 0}, {1.11803398874989, -0.19098300562505299, -0.19098300562505299, 0}, {-0.30901699437494701, 0.10000000000000001, 0.10000000000000001, 0}, {-1.5980058898801799E-17, 1.6180339887499, -1.1625435901742601E-16, 0}, {-3.6251347186248899E-18, 2.7000000000000002, -1.01736800802015E-15, 0}, {-1.5543208479165401E-16, -0.19098300562505299, 3.9983697007543301E-16, 0}, {9.4415458310242108E-18, -1.3090169943749499, -1.3021073429363E-16, 0}, {1.1463486834866301E-16, -0.61803398874989501, 6.66741074405521E-16, 0}, {-8.9387978485436406E-17, 0.59999999999999998, -2.9098800439454998E-16, 0}, {-9.66852401770597E-17, 4.8644451055870699E-17, 1.6180339887499, 0}, {5.1354932792395798E-17, -1.52139481515912E-17, 2.7000000000000002, 0}, {-4.2681144476213502E-18, 7.3773011330069198E-17, -0.19098300562505299, 0}, {-4.06374652955946E-17, -4.0003241302191501E-18, -8.7397053157864101E-16, 0}, {-6.7082817987480105E-17, -8.9853143185963404E-16, -1.7505591351905501E-15, 0}, {7.8766046524307502E-17, -4.9325603350346497E-16, 0, 0}, {-9.8839304549985604E-17, -1.6577184218839599E-16, -1.3090169943749499, 0}, {1.1212619672659E-16, 3.0890053568389798E-16, -0.61803398874989501, 0}, {6.64344608830194E-18, 1.62915088461567E-16, 3.7504859645862399E-16, 0}, {-1.7366985820036001E-17, -3.3100455376630302E-17, 0.59999999999999898, 0}}, {{0.30901699437494801, 0.40901699437494698, 0.40901699437494798, 0}, {-1.11803398874989, -1.3090169943749499, -1.3090169943749499, 0}, {4.4408920985006202E-16, -2, -2, 0}, {0.809016994374947, 0.10000000000000001, 0.099999999999999298, 0}, {-1.3506932994608899E-17, -0.19098300562505199, -1.0250188168659999E-16, 0}, {-5.1787795214922999E-17, 2.7000000000000002, -1.74405944232025E-15, 0}, {-2.34542617506609E-17, 1.61803398874989, 2.9280427934384099E-16, 0}, {-6.2566824883851403E-17, -0.61803398874989401, 3.2552683573407502E-16, 0}, {1.28100001521894E-16, -1.3090169943749499, 2.1366316333487699E-16, 0}, {-1.5529649999523699E-18, 0.59999999999999998, -2.49018580683798E-16, 0}, {1.1306592720424399E-16, 4.7681987519053997E-17, -0.19098300562505299, 0}, {-1.5415227778452599E-16, -6.1861391694006199E-16, 2.7000000000000002, 0}, {-6.1846395013963903E-17, 1.2042953173511601E-16, 1.6180339887499, 0}, {7.0548585348897096E-18, -5.3994899252115697E-16, -1.3507569003574599E-15, 0}, {-6.8640381290805899E-17, 2.8000649351226999E-16, -2.5502114584256798E-16, 0}, {1.2459359919773301E-16, 9.8988467492971094E-18, 9.7140438210515906E-17, 0}, {1.99313946873364E-17, -8.3082311087632703E-16, -0.61803398874989501, 0}, {7.2672770006480196E-17, 4.34969896783124E-16, -1.3090169943749499, 0}, {6.2671149000019297E-19, 3.9585321409320402E-16, 7.5009719291724797E-16, 0}, {3.5550729771792799E-17, -1.0289688551719501E-31, 0.59999999999999898, 0}}, {{-0.5, -0.500000000000001, -0.500000000000001, 0}, {1.5450849718747399, 1.5450849718747399, 1.5450849718747399, 0}, {-4.0450849718747302, -4.0450849718747302, -4.0450849718747302, 0}, {3, -4.4133940502173701E-16, -1.54468791757608E-15, 0}, {-1.5057967739440501E-16, -8.0783671256458104E-17, -8.2376504969534298E-16, 0}, {-1.6529523414955399E-16, -2.5166188846482E-15, 2.20987933239075E-16, 0}, {2.3384398302303001E-16, 4.0450849718747399, -7.2226065670491696E-16, 0}, {-1.6048204015329101E-17, 0, 1.2028184645752299E-15, 0}, {1.3342524297862301E-18, -1.5450849718747399, -7.5550825855310704E-18, 0}, {-1.8091761897899999E-17, 0.5, 3.00586798890633E-16, 0}, {7.2959839565973104E-16, 1.0951633118766999E-15, -7.7626572923240104E-16, 0}, {-1.8701692597972702E-15, 6.0062589688284E-16, -3.4647605886036E-16, 0}, {8.9202078498821297E-17, 8.4103145058659196E-16, 4.0450849718747399, 0}, {1.3290500638130601E-16, -2.0053004044358201E-15, -1.29593977951486E-15, 0}, {3.4775440696013601E-16, -1.0109997833215699E-15, 8.9553172951491904E-16, 0}, {5.4495902169621302E-16, -5.7294378251379797E-16, -1.0685448203156699E-15, 0}, {4.6910779433668902E-16, -1.84741263216816E-16, 8.4386200510629999E-16, 0}, {4.7176791783539197E-17, -2.1111195087638799E-16, -1.5450849718747399, 0}, {-2.8996215722278298E-16, 3.9585321409320402E-16, 1.5001943858345001E-15, 0}, {3.6367487903513701E-17, -7.3497775369424895E-32, 0.499999999999999, 0}}, {{-0.70901699437494703, -0.80901699437494801, -0.70901699437494803, 0}, {1.61803398874989, 3.8289858764701599E-16, 7.87902954874898E-17, 0}, {-1.3090169943749499, 6.8745120708123303E-19, -3.0342084095244502E-16, 0}, {0.59999999999999998, 1.1033485125543401E-16, 3.0342084095244399E-16, 0}, {-2, 2.8189478873908898E-16, -2, 0}, {2.7000000000000002, -9.2349429430861899E-16, -2.8234943086296501E-15, 0}, {-0.61803398874989501, 0, 1.4630906089046301E-16, 0}, {-0.19098300562505199, 1.1180339887499, -0.19098300562505299, 0}, {-0.19098300562505299, 3.6695576141325701E-16, 1.41136831312791E-16, 0}, {0.10000000000000001, -0.30901699437494801, 0.10000000000000001, 0}, {-3.9102877588384502E-16, 1.2478070882168401E-16, 1.6180339887499, 0}, {-7.8275247521481402E-17, -7.17446420753144E-17, 0, 0}, {1.7040347967183599E-16, -2.2454655145381602E-16, -5.7990729458313699E-16, 0}, {2.7021480992586401E-16, -2.5879943997078398E-16, 2.7000000000000002, 0}, {1.2062172385764601E-15, 2.9661779368991802E-16, -1.8500227224172898E-15, 0}, {6.9840151206983306E-17, 3.3449909452584098E-16, -0.19098300562505299, 0}, {1.5863454503442601E-16, -4.4517530974243897E-17, -1.3090169943749499, 0}, {1.5863454503442601E-16, 6.0965219664043695E-17, -9.1350741713816301E-17, 0}, {-2.4066938393207101E-17, -1.28802268225824E-16, -0.61803398874989401, 0}, {2.1167086236731499E-16, 2.3085010670977699E-17, 0.59999999999999898, 0}}, {{0.33333333333333398, 0.33333333333333298, 0.38888888888888901, 0}, {-0.62112999374994105, -0.72723166354163804, -0.72723166354163704, 0}, {0.62112999374994105, -0.106101669791696, -0.106101669791695, 0}, {-0.33333333333333298, 1.37918564069293E-16, 0.055555555555555199, 0}, {-0.72723166354163704, -0.62112999374994105, -0.72723166354163704, 0}, {-1.3911344246271301E-15, -1.1054577863706699E-15, -1.5, 0}, {0.72723166354163804, 0.106101669791696, 7.5593014793405899E-16, 0}, {-0.106101669791696, 0.62112999374994105, -0.106101669791696, 0}, {0.106101669791696, 0.72723166354163804, 1.2702314818151199E-16, 0}, {-1.9171369943972798E-17, -0.33333333333333298, 0.055555555555555497, 0}, {8.7092049233659103E-17, 3.2871912768519901E-17, -0.106101669791696, 0}, {1.16998167122197E-16, -1.5564248597125501E-16, 1.5, 0}, {-7.00711415472103E-18, -1.2216040407611401E-16, -0.106101669791695, 0}, {-3.8713149945490301E-17, -4.1096059118786297E-17, 1.5, 0}, {1.4297682400275401E-16, -9.6761644578150202E-17, 1.5, 0}, {-4.4543269007873701E-17, 2.9308560691054198E-16, -0.106101669791696, 0}, {-6.7563020504027197E-18, 1.7323084431853299E-16, -0.72723166354163704, 0}, {9.8726448587884797E-17, 6.7748093680245999E-17, -0.72723166354163804, 0}, {-2.25279830166212E-17, -1.07887385285151E-16, -0.72723166354163704, 0}, {-1.9217210201444499E-17, 8.1675813172088497E-19, 0.66666666666666596, 0}}, {{-0.59999999999999998, -0.59999999999999898, -0.59999999999999998, 0}, {1.3090169943749499, 1.3090169943749499, 1.3090169943749499, 0}, {-1.61803398874989, -1.61803398874989, -1.61803398874989, 0}, {0.70901699437494603, -0.10000000000000001, -8.6888695363654599E-16, 0}, {0.61803398874989302, 0.61803398874989401, 0.61803398874989401, 0}, {-2.7000000000000002, -2.7000000000000002, -2.7000000000000002, 0}, {2, 2, 4.3892718267138898E-16, 0}, {0.19098300562505299, 0.19098300562505199, 0.19098300562505299, 0}, {0.19098300562505299, 1.3090169943749499, -5.3527498704726199E-16, 0}, {-0.10000000000000001, -0.40901699437494698, 1.09378444506581E-16, 0}, {1.7333825474569E-16, 2.5211577461159202E-16, -2.9946283923529098E-16, 0}, {9.6336033270400894E-18, 3.3047617982430899E-16, 0, 0}, {-4.4366275644302002E-17, -1.93819811359277E-16, 1.6180339887499, 0}, {-1.9875969666725201E-16, -3.20759876036015E-16, 4.3829430180595602E-16, 0}, {-2.0444008279435E-16, 3.0533172658538501E-16, 2.7000000000000002, 0}, {1.4135782182357299E-16, -1.1784665647751699E-16, -0.19098300562505299, 0}, {-1.5766612160156699E-16, 2.17659012590381E-16, 4.2193100255315E-16, 0}, {5.3299379675007998E-17, 2.4402970024995299E-16, -1.3090169943749499, 0}, {-5.21833709632796E-17, 9.3818243849442398E-17, -0.61803398874989401, 0}, {1.2911590965328499E-16, -7.5292782729138902E-17, 0.59999999999999898, 0}}, {{0.40901699437494798, 0.30901699437494801, 0.40901699437494798, 0}, {-0.19098300562505199, 5.51674256277172E-17, -3.4589115349284399E-16, 0}, {-0.61803398874989401, 5.51674256277172E-17, 3.6319982947090598E-17, 0}, {0.59999999999999998, -5.51674256277172E-17, -1.1033485125543401E-16, 0}, {-1.3090169943749499, -1.11803398874989, -1.3090169943749499, 0}, {2.7000000000000002, -8.7202972116012502E-16, -2.6160891634803699E-15, 0}, {-1.3090169943749499, -2.05027885052434E-17, 1.95078747853951E-16, 0}, {-2, 4.8026403647612599E-16, -2, 0}, {1.61803398874989, 1.2702314818151199E-16, -5.16913644683098E-16, 0}, {0.10000000000000001, 0.809016994374947, 0.10000000000000001, 0}, {3.2136787275216301E-16, 4.6923727816666702E-17, -0.19098300562505199, 0}, {-5.26929988159447E-17, -4.1030824431274899E-16, -3.27065210268917E-16, 0}, {-1.70885343198498E-16, 4.2590628073983099E-17, 3.8660486305542501E-16, 0}, {-1.6657475356230201E-16, -4.5517752999611997E-17, 2.7000000000000002, 0}, {5.36505748345712E-18, -1.92649567901636E-16, -1.5914173956277801E-15, 0}, {-6.5610845868922498E-16, -4.2683286798663703E-17, 1.61803398874989, 0}, {4.02777932928136E-16, 2.3909841007266801E-16, -0.61803398874989501, 0}, {-1.91530696250145E-17, -5.0979154182623002E-17, 3.6540296685526501E-16, 0}, {1.91812431651561E-16, 2.3108574051574701E-16, -1.3090169943749499, 0}, {-9.2659768549174801E-17, -1.4916711424873399E-17, 0.59999999999999898, 0}}, {{-0.59999999999999998, -0.59999999999999998, -0.59999999999999998, 0}, {0.61803398874989501, 0.61803398874989501, 0.61803398874989501, 0}, {0.19098300562505299, 0.19098300562505299, 0.19098300562505399, 0}, {-0.40901699437494698, -0.10000000000000001, -3.3100455376630302E-16, 0}, {1.3090169943749499, 1.3090169943749499, 1.3090169943749499, 0}, {-2.7000000000000002, -2.7000000000000002, -2.7000000000000002, 0}, {1.3090169943749499, 0.19098300562505299, 6.8277561748882798E-16, 0}, {-1.6180339887499, -1.61803398874989, -1.61803398874989, 0}, {2, 2, -7.7872795205668897E-16, 0}, {-0.099999999999999797, 0.70901699437494703, 1.3704142075239901E-16, 0}, {4.9088785086304397E-16, 2.4335327255336499E-17, 4.8732246759304895E-16, 0}, {-1.5616510450613501E-16, 5.7464926516065499E-16, -2.1804347351261201E-16, 0}, {-4.8685262129553397E-16, 2.8536541236814301E-16, -0.19098300562505199, 0}, {-2.0701643706960201E-16, 1.9805059448242099E-16, -2.1914715090297802E-15, 0}, {-7.8296211624252901E-16, -1.5232914148212401E-15, 2.7000000000000002, 0}, {-5.5608854282362604E-16, -8.7076185334371596E-16, 1.61803398874989, 0}, {-6.0909054433954002E-17, 1.2695690478184701E-16, 1.05482750638288E-16, 0}, {3.61021948119196E-16, 3.1155171839885E-16, -0.61803398874989501, 0}, {5.3829709912634002E-16, 1.7357894073344E-16, -1.3090169943749499, 0}, {-2.9833422849746902E-17, -6.6200910753260505E-17, 0.59999999999999998, 0}}, {{-0.500000000000001, -0.5, -0.5, 0}, {1.2102047282546501E-16, 7.6442227287986896E-16, 9.41637508622875E-17, 0}, {1.5543122344752199E-15, 0, -6.7025852201758103E-16, 0}, {0.499999999999999, 0, -6.6200910753260603E-16, 0}, {1.5450849718747399, 1.5450849718747399, 1.5450849718747399, 0}, {1.22992979240474E-17, 1.7637171600464699E-15, 1.72452074187426E-16, 0}, {-1.5450849718747399, 0, 3.90157495707902E-16, 0}, {-4.0450849718747399, -4.0450849718747399, -4.0450849718747399, 0}, {4.0450849718747399, -1.41136831312791E-15, -5.4870120761681097E-16, 0}, {1.4702669139819701E-16, 3, 2.20174645220459E-16, 0}, {-1.9095206750509599E-16, -5.8484393191044003E-17, 1.8673697732338201E-16, 0}, {-4.0384939064166001E-16, 1.18619071475742E-16, 8.7217389405044597E-16, 0}, {1.5660917718109999E-16, 4.2505546918519501E-16, 1.10877834396043E-15, 0}, {-3.6999979096586402E-16, -2.8473860123848201E-16, -1.82900147613175E-15, 0}, {-1.51167539895853E-15, -2.1347006689523301E-16, -9.5485043737666598E-16, 0}, {-1.1121770856472499E-15, -1.05078492817669E-15, 4.0450849718747399, 0}, {-1.21818108867908E-16, -4.47281619581441E-16, -1.68640049254222E-16, 0}, {7.2204389623839298E-16, 1.0200706312815401E-18, -1.82701483427633E-16, 0}, {1.07659419825268E-15, -8.6104661904355099E-17, -1.5450849718747399, 0}, {-5.9666845699493804E-17, -3.2670325268834501E-18, 0.5, 0}}, {{-0.70901699437494803, -0.70901699437494803, -0.80901699437494901, 0}, {1.61803398874989, 1.1331217120074199E-15, 3.2512517428166301E-16, 0}, {-1.3090169943749499, 3.6869943912755599E-16, 4.1717526624120102E-16, 0}, {0.59999999999999998, -2.20669702510869E-16, -1.02691506077379E-16, 0}, {3.2945162591403501E-17, 1.6180339887499, -8.1584124673710904E-17, 0}, {-8.7202972116012502E-16, -1.31864686688094E-15, 2.5054016844745902E-16, 0}, {4.2962002342968699E-16, -9.7539373926975403E-17, 3.2039385244111598E-16, 0}, {7.8126440576178098E-16, -1.3090169943749499, 2.8382060050662998E-16, 0}, {2.84807204469788E-16, 3.1050102888814101E-16, -1.6291464744890401E-17, 0}, {-4.02906467623223E-16, 0.59999999999999998, -1.4018912212719001E-16, 0}, {-2, -2, 1.22374290764717E-15, 0}, {2.7000000000000002, -8.7217389405044597E-16, -9.7329074487427603E-16, 0}, {-0.61803398874989401, 5.7990729458313699E-16, 5.4331124581345997E-16, 0}, {-1.46685443073347E-15, 2.7000000000000002, -6.8680160376295196E-16, 0}, {-1.0833469166532601E-15, -1.0344213071580601E-15, 1.8978699984896099E-16, 0}, {2.9142131463154799E-16, -0.61803398874989501, 2.9142131463154799E-16, 0}, {-0.19098300562505199, -0.19098300562505299, 1.11803398874989, 0}, {-0.19098300562505299, 9.1350741713816301E-17, -1.99922061086367E-16, 0}, {3.2816752190129601E-16, -0.19098300562505199, -1.8752429822931199E-16, 0}, {0.099999999999999894, 0.099999999999999894, -0.30901699437494701, 0}}, {{0.33333333333333298, 0.38888888888888901, 0.33333333333333398, 0}, {-0.62112999374994204, -0.72723166354163804, -0.72723166354163704, 0}, {0.62112999374994105, -0.106101669791696, -0.106101669791696, 0}, {-0.33333333333333298, 0.055555555555555601, -4.8271497424252499E-17, 0}, {-1.7054765120929501E-16, -0.106101669791696, -6.1310498873456005E-17, 0}, {-2.9067657372004202E-16, 1.5, -1.09003715145016E-16, 0}, {-2.7968106978839502E-17, -0.106101669791696, -1.1796863878579399E-16, 0}, {1.3021073429363E-16, -0.72723166354163704, 5.2084293717452098E-16, 0}, {-1.76853251836587E-16, -0.72723166354163704, 3.7421377574435E-16, 0}, {-5.5959231614336498E-17, 0.66666666666666596, -1.56685848520142E-16, 0}, {-0.72723166354163704, -0.72723166354163704, -0.62112999374994204, 0}, {1.8448149077006301E-16, -1.5, 7.8535887818221402E-17, 0}, {0.72723166354163704, 0, 0.106101669791696, 0}, {-1.6114247091010501E-17, 1.5, -4.3928020816126598E-16, 0}, {9.3459328360478596E-17, 1.5, -9.8590635531003302E-19, 0}, {2.1325975227729901E-17, -0.72723166354163704, 2.4285109552629002E-16, 0}, {-0.106101669791696, -0.106101669791696, 0.62112999374994105, 0}, {0.106101669791696, 6.8513056285362201E-17, 0.72723166354163704, 0}, {1.35544261408003E-17, -0.106101669791696, -1.8752429822931199E-16, 0}, {-5.0032178999068501E-17, 0.055555555555555497, -0.33333333333333298, 0}}, {{-0.59999999999999998, -0.60000000000000098, -0.59999999999999898, 0}, {1.3090169943749499, 1.3090169943749499, 1.3090169943749399, 0}, {-1.61803398874989, -1.61803398874989, -1.61803398874989, 0}, {0.70901699437494703, -1.1033485125543401E-16, -0.10000000000000001, 0}, {1.77389565478437E-16, 6.66593810281595E-16, -6.1000980808580196E-16, 0}, {5.8135314744008295E-16, -1.64613601604337E-15, -1.4533828686002099E-15, 0}, {-2.9038423102594797E-17, 1.6180339887499, 7.21865830084784E-16, 0}, {3.9063220288089E-16, 7.8126440576178098E-16, 1.3021073429363E-16, 0}, {-7.6703466717672195E-16, -1.3090169943749499, -7.5078978773214397E-16, 0}, {2.7979615807168299E-16, 0.59999999999999998, 1.90261387488744E-16, 0}, {0.61803398874989302, 0.61803398874989401, 0.61803398874989302, 0}, {-2.7000000000000002, -2.7000000000000002, -2.7000000000000002, 0}, {2, -2.6845320276049402E-16, 2, 0}, {1.42118574318105E-15, -1.3148829054178699E-15, 4.2393365123370499E-16, 0}, {1.2568253800038201E-15, 2.7000000000000002, 1.08137510394264E-15, 0}, {9.7140438210515906E-17, -0.61803398874989501, 1.9428087642103201E-16, 0}, {0.19098300562505299, 0.19098300562505299, 0.19098300562505299, 0}, {0.19098300562505299, 7.0796824828207598E-16, 1.3090169943749499, 0}, {-9.3762149114655996E-17, -0.19098300562505199, 1.8752429822931199E-16, 0}, {-0.099999999999999894, -9.1026252285733095E-17, -0.40901699437494698, 0}}, {{0.38888888888888901, 0.33333333333333398, 0.33333333333333398, 0}, {-0.106101669791696, -9.7963098136445696E-17, -1.39316066920344E-16, 0}, {-0.72723166354163704, 4.7535360652345603E-16, 1.4734228540960599E-16, 0}, {0.66666666666666596, -1.79294133290081E-16, -1.1033485125543401E-16, 0}, {-0.72723166354163704, -0.62112999374994105, -0.72723166354163704, 0}, {1.5, -5.61498481347017E-16, 3.7327305019658502E-16, 0}, {-0.72723166354163704, 1.95078747853951E-16, -6.0962108704359603E-18, 0}, {-0.106101669791696, 0.62112999374994204, -0.106101669791696, 0}, {-0.106101669791696, 1.83477880706629E-16, 1.5075708188732799E-16, 0}, {0.0555555555555554, -0.33333333333333298, -3.3575538968601902E-17, 0}, {-0.72723166354163604, -0.72723166354163704, -0.62112999374994105, 0}, {1.5, -4.3608694702522299E-16, 0, 0}, {-0.72723166354163704, 2.0945146911001399E-16, 1.82528505078179E-16, 0}, {-1.5, -1.3470116883034199E-16, 4.5249375944965799E-16, 0}, {1.5, 1.59141739562778E-16, 4.8737157741100701E-16, 0}, {7.2855328657886899E-17, 0.72723166354163704, 0.106101669791696, 0}, {-0.106101669791696, -0.106101669791696, 0.62112999374994105, 0}, {-0.106101669791696, -2.28376854284541E-17, 0, 0}, {2.1096483550797601E-16, 0.106101669791696, 0.72723166354163704, 0}, {0.055555555555555497, 4.5513126142866597E-17, -0.33333333333333298, 0}}, {{-0.66666666666666696, -0.66666666666666696, -0.66666666666666596, 0}, {0.72723166354163704, 0.72723166354163704, 0.72723166354163704, 0}, {0.106101669791696, 0.106101669791696, 0.106101669791696, 0}, {-0.38888888888888901, -0.055555555555555497, -0.0555555555555554, 0}, {0.72723166354163604, 0.72723166354163704, 0.72723166354163704, 0}, {-1.5, -1.5, -1.5, 0}, {0.72723166354163704, 0.106101669791696, 3.5967644135572201E-16, 0}, {0.106101669791696, 0.106101669791696, 0.106101669791696, 0}, {0.106101669791696, 0.72723166354163704, -4.05768390024275E-16, 0}, {-0.0555555555555554, -0.38888888888888801, -0.055555555555555497, 0}, {0.72723166354163704, 0.72723166354163704, 0.72723166354163604, 0}, {-1.5, -1.5, -1.5, 0}, {0.72723166354163704, -1.9330243152771201E-16, 0.106101669791695, 0}, {-1.5, -1.5, -1.5, 0}, {1.5, 1.5, 1.5, 0}, {-8.4997883434201402E-17, 0.72723166354163704, 0.106101669791696, 0}, {0.106101669791696, 0.106101669791696, 0.106101669791696, 0}, {0.106101669791696, -4.5675370856908101E-17, 0.72723166354163704, 0}, {-9.3762149114655996E-17, 0.106101669791696, 0.72723166354163704, 0}, {-0.055555555555555497, -0.055555555555555497, -0.38888888888888901, 0}}, {{-0.59999999999999998, -0.60000000000000098, -0.59999999999999998, 0}, {3.8644922436892399E-16, 9.3672559929298691E-16, 7.9403160804454803E-16, 0}, {1.0362081563168101E-15, -5.9211894646675002E-16, -3.5260180127525501E-16, 0}, {0.59999999999999898, 0, -1.65502276883152E-16, 0}, {1.3090169943749499, 1.3090169943749499, 1.3090169943749499, 0}, {2.9317943373009499E-16, 1.2868400114586701E-15, 1.1778362963136499E-15, 0}, {-1.3090169943749499, -1.95078747853951E-16, 1.95078747853951E-16, 0}, {-1.61803398874989, -1.61803398874989, -1.61803398874989, 0}, {1.61803398874989, -5.2220627585732796E-16, -5.6807574603398497E-16, 0}, {-6.7151077937203903E-17, 0.70901699437494703, -0.099999999999999894, 0}, {0.61803398874989401, 0.61803398874989501, 0.61803398874989501, 0}, {-6.1107851829159099E-16, 1.74434778810089E-15, 1.74434778810089E-15, 0}, {-0.61803398874989401, -1.9330243152771201E-16, 3.8660486305542501E-16, 0}, {-2.7000000000000002, -2.7000000000000002, -2.7000000000000002, 0}, {2.7000000000000002, -3.1828347912555501E-16, 1.59141739562778E-16, 0}, {1.9730792124268499E-16, 2, 2, 0}, {0.19098300562505299, 0.19098300562505199, 0.19098300562505199, 0}, {-0.19098300562505199, 1.5986379799917901E-16, -3.6540296685526501E-16, 0}, {-4.94020932076666E-17, 0.19098300562505199, 1.3090169943749499, 0}, {-7.5239016465664398E-17, -0.099999999999999894, -0.40901699437494698, 0}}, {{0.40901699437494798, 0.40901699437494798, 0.30901699437494801, 0}, {-0.19098300562505299, -1.0352560810353E-15, -3.6190146217476301E-16, 0}, {-0.61803398874989401, -3.1951847880474598E-17, 2.63202911692858E-17, 0}, {0.59999999999999998, 6.8959282034646499E-17, -1.5111982935196099E-18, 0}, {-4.4139780516897498E-16, -0.19098300562505199, -1.3894926241878801E-16, 0}, {-2.9067657372004202E-16, -2.12045702815167E-17, -4.3601486058006202E-16, 0}, {2.9261812178092602E-16, 3.4138780874441399E-16, -1.8539927348641899E-16, 0}, {0, -0.61803398874989501, -1.3021073429363E-16, 0}, {5.0463489116126398E-16, 1.4113683131279101E-17, -2.0358280724689801E-16, 0}, {-4.4767385291469198E-16, 0.59999999999999998, -4.4767385291469197E-17, 0}, {-1.3090169943749499, -1.3090169943749499, -1.11803398874989, 0}, {2.7000000000000002, -8.7217389405044597E-16, -9.0128390131488597E-17, 0}, {-1.3090169943749499, -1.9330243152771201E-16, -4.8824363702964798E-17, 0}, {-4.9341062552540902E-16, 2.7000000000000002, 3.20214644140311E-16, 0}, {-1.36999922913732E-15, -3.1828347912555501E-16, 5.2455263897017402E-17, 0}, {3.39991533736806E-16, -1.3090169943749499, 8.0545990907591798E-17, 0}, {-2, -2, 5.7587025691724703E-16, 0}, {1.61803398874989, 5.48104450282898E-16, -1.9340963702042001E-16, 0}, {-2.3440537278664E-16, 1.61803398874989, 3.7504859645862399E-16, 0}, {0.099999999999999797, 0.099999999999999505, 0.80901699437494601, 0}}, {{-0.60000000000000098, -0.60000000000000098, -0.59999999999999998, 0}, {0.61803398874989501, 0.61803398874989601, 0.61803398874989501, 0}, {0.19098300562505199, 0.19098300562505199, 0.19098300562505199, 0}, {-0.40901699437494698, 3.8617197939401999E-16, -0.099999999999999298, 0}, {5.4747878803662795E-16, 1.26050067352214E-15, 3.4214978099263599E-16, 0}, {-8.7202972116012403E-16, 1.35700629487863E-15, -5.8135314744008295E-16, 0}, {-3.90157495707902E-16, -0.19098300562505199, -3.9462527721785802E-17, 0}, {0, -7.8126440576178098E-16, -7.8126440576178098E-16, 0}, {-4.33664129973962E-16, -0.61803398874989601, -4.4990900941854002E-16, 0}, {1.34302155874408E-16, 0.59999999999999898, 2.2383692645734599E-16, 0}, {1.3090169943749499, 1.3090169943749499, 1.3090169943749499, 0}, {-2.7000000000000002, -2.7000000000000002, -2.7000000000000002, 0}, {1.3090169943749499, -1.13476433208868E-15, 0.19098300562505099, 0}, {-9.4893499924480495E-17, 4.3829430180595602E-16, 1.37261730117059E-15, 0}, {-9.4893499924480606E-17, 2.7000000000000002, -3.8055990605323199E-16, 0}, {-1.4571065731577399E-16, -1.3090169943749499, -3.39991533736806E-16, 0}, {-1.61803398874989, -1.61803398874989, -1.61803398874989, 0}, {2, 0, 2, 0}, {-9.3762149114655996E-17, 1.61803398874989, 3.7504859645862399E-16, 0}, {-0.10000000000000001, -3.9720546451956298E-16, 0.70901699437494603, 0}}, {{-0.60000000000000098, -0.60000000000000098, -0.60000000000000098, 0}, {1.4336000766649001E-15, 9.5543296242633803E-16, 3.27432637289536E-16, 0}, {-5.9211894646675002E-16, -7.6451715155336599E-16, -6.280952038103E-16, 0}, {0.59999999999999998, 1.7239820508661599E-16, 1.65502276883152E-16, 0}, {0.61803398874989501, 0.61803398874989401, 0.61803398874989401, 0}, {-3.9920228513767801E-16, -3.7394800229080898E-16, -5.5562086086583597E-16, 0}, {-0.61803398874989501, -7.6812256967493101E-16, -2.1946359133569501E-16, 0}, {0.19098300562505199, 0.19098300562505299, 0.19098300562505199, 0}, {-0.19098300562505199, 3.3167155358505899E-16, -2.9991576653968099E-16, 0}, {-8.9534770582938504E-17, -0.40901699437494698, -0.099999999999999894, 0}, {1.3090169943749499, 1.3090169943749499, 1.3090169943749499, 0}, {2.6980407361011398E-16, 8.7217389405044597E-16, 8.7217389405044597E-16, 0}, {-1.3090169943749499, -5.7990729458313699E-16, -7.7320972611085001E-16, 0}, {-2.7000000000000002, -2.7000000000000002, -2.7000000000000002, 0}, {2.7000000000000002, 0, -1.7903445700812499E-16, 0}, {4.4800875891200499E-16, 1.3090169943749499, 0.19098300562505199, 0}, {-1.6180339887499, -1.6180339887499, -1.61803398874989, 0}, {1.6180339887499, -3.6540296685526501E-16, 3.6540296685526501E-16, 0}, {-1.2430783491175601E-16, 2, 2, 0}, {-1.42202919087171E-16, -0.10000000000000001, 0.70901699437494703, 0}}, {{-0.500000000000001, -0.500000000000001, -0.500000000000001, 0}, {5.1433081888359199E-16, 1.4231625976716099E-15, 7.3658958664189898E-16, 0}, {-6.6613381477509402E-16, -1.05057253290261E-16, -1.84149270121201E-15, 0}, {0.5, 5.8615389729449499E-16, 3.28347597218496E-17, 0}, {-5.0749368267349301E-16, 2.2805320781159502E-16, -2.2177534256180299E-16, 0}, {-5.0868400401007204E-16, 5.8135314744008404E-16, -3.4881188846405001E-15, 0}, {-3.1020875339430799E-16, -4.0207157973445101E-17, -1.5172585057406299E-15, 0}, {-9.1147514005541103E-16, -2.08337174869808E-15, -9.1147514005541103E-16, 0}, {2.2708570102256898E-16, -1.3549135806028001E-15, -1.6037733246186399E-16, 0}, {-2.0145323381161199E-16, 0.5, 6.7151077937203903E-17, 0}, {1.5450849718747399, 1.5450849718747399, 1.5450849718747399, 0}, {1.4721620161449999E-15, 1.0338067162700699E-15, 3.8616384275892899E-16, 0}, {-1.5450849718747399, -1.3977884078995201E-15, -8.1775215913001395E-16, 0}, {3.0138862162076798E-15, 4.7290846606975501E-15, 1.9801711648134799E-15, 0}, {-2.24564540546379E-15, 1.5914173956277801E-15, -1.52618324963417E-15, 0}, {4.6161108564519496E-16, -1.5450849718747399, -3.8856175284206298E-16, 0}, {-4.0450849718747399, -4.0450849718747399, -4.0450849718747399, 0}, {4.0450849718747399, -7.3080593371053001E-16, -8.1980319421046397E-16, 0}, {-6.0352647705985398E-16, 4.0450849718747399, 0, 0}, {-2.9093990322810902E-16, -1.1916163935586901E-15, 3, 0}}};

unsigned long const constant HPM__dataType__Matrix_double__3__3___M = 3;

unsigned long const constant HPM__dataType__Matrix_double__3__3___N = 3;

double16 function_0(double16 * ocl_code_trafo_this, double const x)
{
for(unsigned long i = 0; i<HPM__dataType__Matrix_double__3__3___M; ++i)
{
for(unsigned long j = 0; j<HPM__dataType__Matrix_double__3__3___N; ++j)
{
(*ocl_code_trafo_this)[i+j*3]*=x;
}
;
}
;
return *ocl_code_trafo_this;
}


double16 function_1(double16 const m, double const x)
{
double16 matrix = m;
function_0(&(matrix), x);
return matrix;
}


double16 function_2(double4 const v_1, double4 const v_2)
{
double16 matrix;
for(unsigned long j = 0; j<3; ++j)
{
for(unsigned long i = 0; i<3; ++i)
{
matrix[j+i*3]=v_2[j]*v_1[i];
}
;
}
;
return matrix;
}


double16 function_3(double16 * ocl_code_trafo_this, double16 const m)
{
for(unsigned long i = 0; i<HPM__dataType__Matrix_double__3__3___M; ++i)
{
for(unsigned long j = 0; j<HPM__dataType__Matrix_double__3__3___N; ++j)
{
(*ocl_code_trafo_this)[i+j*3]+=m[i+j*3];
}
;
}
;
return *ocl_code_trafo_this;
}


double16 function_4(double4 const v_1, double4 const v_2)
{
double16 matrix;
for(unsigned long j = 0; j<3; ++j)
{
for(unsigned long i = 0; i<3; ++i)
{
matrix[j+i*3]=v_2[j]*v_1[i];
}
;
}
;
return matrix;
}


double16 function_5(double16 * ocl_code_trafo_this, double16 const m)
{
for(unsigned long i = 0; i<HPM__dataType__Matrix_double__3__3___M; ++i)
{
for(unsigned long j = 0; j<HPM__dataType__Matrix_double__3__3___N; ++j)
{
(*ocl_code_trafo_this)[i+j*3]+=m[i+j*3];
}
;
}
;
return *ocl_code_trafo_this;
}


double16 function_6(double16 const * ocl_code_trafo_this)
{
double16 matrix;
for(unsigned long i = 0; i<HPM__dataType__Matrix_double__3__3___M; ++i)
{
for(unsigned long j = 0; j<HPM__dataType__Matrix_double__3__3___N; ++j)
{
matrix[j+i*3]=(*ocl_code_trafo_this)[i+j*3];
}
;
}
;
return matrix;
}


double function_7(double4 const v_1, double4 const v_2)
{
double dot_product = 0;
for(unsigned long i = 0; i<3; ++i)
{
dot_product+=(v_1[i]*v_2[i]);
}
;
return dot_product;
}


double function_8(double4 const v_1, double4 const v_2)
{
double dot_product = 0;
for(unsigned long i = 0; i<3; ++i)
{
dot_product+=(v_1[i]*v_2[i]);
}
;
return dot_product;
}


double function_9(double4 const v_1, double4 const v_2)
{
double dot_product = 0;
for(unsigned long i = 0; i<3; ++i)
{
dot_product+=(v_1[i]*v_2[i]);
}
;
return dot_product;
}


double function_10(double4 const v_1, double4 const v_2)
{
double dot_product = 0;
for(unsigned long i = 0; i<3; ++i)
{
dot_product+=(v_1[i]*v_2[i]);
}
;
return dot_product;
}


double function_11(double4 const v_1, double4 const v_2)
{
double dot_product = 0;
for(unsigned long i = 0; i<3; ++i)
{
dot_product+=(v_1[i]*v_2[i]);
}
;
return dot_product;
}


double function_12(double4 const v_1, double4 const v_2)
{
double dot_product = 0;
for(unsigned long i = 0; i<3; ++i)
{
dot_product+=(v_1[i]*v_2[i]);
}
;
return dot_product;
}


double4 function_13(double arg0, double arg1, double arg2)
{
double4 result_float = {arg0, arg1, arg2, 0};
return result_float;
}


double4 function_14(double16 const m_1, double16 const m_2)
{
double16 const m_1t = function_6(&(m_1));
return function_13((function_7(m_1t[1], m_2[2]))-(function_8(m_1t[2], m_2[1])), (function_9(m_1t[2], m_2[0]))-(function_10(m_1t[0], m_2[2])), (function_11(m_1t[0], m_2[1]))-(function_12(m_1t[1], m_2[0])));
}


double4 function_15(double4 lhs, double rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]*=rhs;
}
;
return lhs;
}


double4 function_16(double4 const lhs, double rhs)
{
double4 copy = lhs;
function_15(copy, rhs);
return copy;
}


double4 function_17(double4 const vec)
{
double4 copy = vec;
return function_16(copy, (double)-1);
}


double4 function_18(double4 lhs, double4 const rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]+=rhs[i];
}
;
return lhs;
}


double16 function_19(double16 const * ocl_code_trafo_this)
{
double16 matrix;
for(unsigned long i = 0; i<HPM__dataType__Matrix_double__3__3___M; ++i)
{
for(unsigned long j = 0; j<HPM__dataType__Matrix_double__3__3___N; ++j)
{
matrix[j+i*3]=(*ocl_code_trafo_this)[i+j*3];
}
;
}
;
return matrix;
}


double function_20(double4 const v_1, double4 const v_2)
{
double dot_product = 0;
for(unsigned long i = 0; i<3; ++i)
{
dot_product+=(v_1[i]*v_2[i]);
}
;
return dot_product;
}


double function_21(double4 const v_1, double4 const v_2)
{
double dot_product = 0;
for(unsigned long i = 0; i<3; ++i)
{
dot_product+=(v_1[i]*v_2[i]);
}
;
return dot_product;
}


double function_22(double4 const v_1, double4 const v_2)
{
double dot_product = 0;
for(unsigned long i = 0; i<3; ++i)
{
dot_product+=(v_1[i]*v_2[i]);
}
;
return dot_product;
}


double function_23(double4 const v_1, double4 const v_2)
{
double dot_product = 0;
for(unsigned long i = 0; i<3; ++i)
{
dot_product+=(v_1[i]*v_2[i]);
}
;
return dot_product;
}


double function_24(double4 const v_1, double4 const v_2)
{
double dot_product = 0;
for(unsigned long i = 0; i<3; ++i)
{
dot_product+=(v_1[i]*v_2[i]);
}
;
return dot_product;
}


double function_25(double4 const v_1, double4 const v_2)
{
double dot_product = 0;
for(unsigned long i = 0; i<3; ++i)
{
dot_product+=(v_1[i]*v_2[i]);
}
;
return dot_product;
}


double4 function_26(double arg0, double arg1, double arg2)
{
double4 result_float = {arg0, arg1, arg2, 0};
return result_float;
}


double4 function_27(double16 const m_1, double16 const m_2)
{
double16 const m_1t = function_19(&(m_1));
return function_26((function_20(m_1t[1], m_2[2]))-(function_21(m_1t[2], m_2[1])), (function_22(m_1t[2], m_2[0]))-(function_23(m_1t[0], m_2[2])), (function_24(m_1t[0], m_2[1]))-(function_25(m_1t[1], m_2[0])));
}


double4 function_28(double4 lhs, double4 const rhs)
{
for(unsigned long i = 0; i<3; ++i)
{
lhs[i]+=rhs[i];
}
;
return lhs;
}


void function_29(unsigned long const omitted_0, double4 global * lvs_0, double4 global * lvs_1, double4 global * lvs_2, double4 global * lvs_3, unsigned long hpm_offset_0, unsigned long hpm_offset_1, unsigned long hpm_offset_2, unsigned long hpm_offset_3, double16 global * GetInverseJacobian)
{
size_t const dof_0_0_0_20_0[5] = {0, 0, 0, 20, 0};
double16 const D = function_1(GetInverseJacobian[get_global_id(0)], 2);
for(unsigned long n = 0; n<numVolNodes; ++n)
{
double16 derivative_E;
double16 derivative_H;
double4 global * fieldH = lvs_0+get_global_id(0)*20+hpm_offset_0;
double4 global * fieldE = lvs_1+get_global_id(0)*20+hpm_offset_1;
for(unsigned long m = 0; m<numVolNodes; ++m)
{
function_3(&(derivative_H), function_2(derivative[n][m], fieldH[m]));
function_5(&(derivative_E), function_4(derivative[n][m], fieldE[m]));
}
;
double4 global * rhsH = lvs_2+get_global_id(0)*20+hpm_offset_2;
double4 global * rhsE = lvs_3+get_global_id(0)*20+hpm_offset_3;
function_18(rhsH[n], function_17(function_14(D, derivative_E)));
function_28(rhsE[n], function_27(D, derivative_H));
}
;
}


void kernel function_30(unsigned long const iter, double4 global * localVectors_0, double4 global * localVectors_1, double4 global * localVectors_2, double4 global * localVectors_3, unsigned long hpm_offset_0, unsigned long hpm_offset_1, unsigned long hpm_offset_2, unsigned long hpm_offset_3, double16 global * GetInverseJacobian)
{
function_29(iter, localVectors_0, localVectors_1, localVectors_2, localVectors_3, hpm_offset_0, hpm_offset_1, hpm_offset_2, hpm_offset_3, GetInverseJacobian);
}

