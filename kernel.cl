


__kernel void add(__global float *a,__global float *b,__global float *result){
    int i=get_global_id(0);
    if(i<10000){
        result[i]=a[i]/4*7+b[i]*23/5+1;
    }
}