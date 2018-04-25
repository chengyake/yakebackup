



#define NOD     (60)
#define NOI     (1120)
#define NOH     (1120/10)
#define NOF     (1024)
#define NOT     (1480-NOD-1)




__kernel void matrix(__global float *iat, __global float *pa, __global float *od) {


    int idx = get_global_id(0)*32 + get_global_id(1);

    float iatt[NOI];
    float ha[NOH];
    float oa;

    int i, j;

    for(i=0; i<NOI; i++) {
        iatt[i] = iat[idx*(NOI+1)+i];
    }


    oa=0.0;
    for(i=0; i<NOH; i++) {
        ha[i]=0.0;
        for(j=0; j<NOI; j++) {
            ha[i]+=iatt[j]*(pa[i*NOI+j]);
        }
        ha[i]/=NOI;
        ha[i]+=(pa[NOI*NOH+i]/10);
        oa += ha[i]*pa[NOI*NOH+NOH+i];
    }
    oa/=NOH;
    oa+=(pa[NOI*NOH+NOH+NOH]/10);


    od[idx] = 1/(pow(2.71828, fabs(iat[idx*(NOI+1)+NOI]-oa)*100));
}











/*



    table_o[i] = (table_i[i]+(int4)(3))*(table_q[i]+(int4)(3))*(table_i[i]+(int4)(2))*(table_q[i]+(int4)(2))*(table_i[i]+(int4)(1))*(table_q[i]+(int4)(1)) * (table_i[i])*(table_q[i])*(table_i[i]+(int4)(3))*(table_q[i]+(int4)(3))*(table_i[i]+(int4)(2))*(table_q[i]+(int4)(2))*(table_i[i]+(int4)(1))*(table_q[i]+(int4)(1)) * (table_i[i])*(table_q[i]);
    table_o[i] += convert_int4(sin(((float4)(100.0))/convert_float4((table_i[i])+(table_q[i])))*1000);
    table_o[i] += convert_int4(sqrt(convert_float4(table_i[i])) + sqrt(convert_float4(table_q[i])) + sqrt(convert_float4(table_i[i]+table_q[i])));


    //float4 f = cos(convert_float4(tmp1/tmp2));

    //table_o[i] = tmp1*tmp2*table_i[i]*table_q[i]/1000*(tmp1/tmp2)*convert_int4(f*1000.0)/1000;


  
    //f = cos(f);
*/


