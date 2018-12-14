

__kernel void add(__global float *a,__global float *b,__global float *result){
    int gid=get_global_id(0);
    if(gid<100){
        result[gid]=a[gid]+b[gid];
    }
}