


__kernel void process_iq(__global void *in_i, __global void *in_q, __global void *output) {


    int i = get_global_id(0)*256 + get_global_id(1);

    __global int4 *table_i = in_i;
    __global int4 *table_q = in_q;
    __global int4 *table_o = output;

    table_o[i] = (table_i[i]+(int4)(3))*(table_q[i]+(int4)(3))*(table_i[i]+(int4)(2))*(table_q[i]+(int4)(2))*(table_i[i]+(int4)(1))*(table_q[i]+(int4)(1)) * (table_i[i])*(table_q[i])*(table_i[i]+(int4)(3))*(table_q[i]+(int4)(3))*(table_i[i]+(int4)(2))*(table_q[i]+(int4)(2))*(table_i[i]+(int4)(1))*(table_q[i]+(int4)(1)) * (table_i[i])*(table_q[i]);
    table_o[i] += convert_int4(sin(((float4)(100.0))/convert_float4((table_i[i])+(table_q[i])))*1000);
    table_o[i] += convert_int4(sqrt(convert_float4(table_i[i])) + sqrt(convert_float4(table_q[i])) + sqrt(convert_float4(table_i[i]+table_q[i])));


    //float4 f = cos(convert_float4(tmp1/tmp2));

    //table_o[i] = tmp1*tmp2*table_i[i]*table_q[i]/1000*(tmp1/tmp2)*convert_int4(f*1000.0)/1000;


  
    //f = cos(f);




}


