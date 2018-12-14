#include <iostream>
#include <CL/cl.h>

#include "tool.h"

using namespace std;
#define ARRAY_SIZE 10000

int main(){
    cl_int err;
    cl_uint num;

    cl_platform_id platform=0;
    cl_device_id device=0;
    cl_context context=0;
    cl_command_queue commandQueue=0;
    cl_program program=0;
    cl_kernel kernel=0;
    cl_mem memObjects[3]={0,0,0};
    //获得平台、设备、上下文、命令队列、程序对象
    int r=CreateTool(platform,device,context,commandQueue,program,"kernel.cl");
    if(r==-1){
        clean(context,commandQueue,program,kernel);
        return -1;
    }
    //构建核函数
    kernel=clCreateKernel(program, "add", &err);
    if(err!=CL_SUCCESS||kernel==NULL){
        cout<<"no kernel."<<endl;
        clean(context,commandQueue,program,kernel);
        return -1;
    }
    float a[ARRAY_SIZE],b[ARRAY_SIZE],result[ARRAY_SIZE];
    for(int i=0;i<ARRAY_SIZE;i++){
        a[i]=i;b[i]=i+1;
    }
    //构建内存对象，分配内存，并初始化，即从主机拷贝数据到 device buffer
    memObjects[0]=clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, 
        sizeof(float)*ARRAY_SIZE, a, NULL);
    memObjects[1]=clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, 
        sizeof(float)*ARRAY_SIZE, b, NULL);
    memObjects[2]=clCreateBuffer(context, CL_MEM_READ_WRITE, 
        sizeof(float)*ARRAY_SIZE, NULL, NULL);
    if(memObjects[0]==NULL||memObjects[1]==NULL||memObjects[2]==NULL){
        cout<<"mem err."<<endl;
        clean(context,commandQueue,program,kernel);
        for(int i=0;i<3;i++){
            if(memObjects[i]!=0)
                clReleaseMemObject(memObjects[i]);
        }
        return -1;
    }
    //设置核函数参数
    err=clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
    err|=clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);
    err|=clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);
    if(err!=CL_SUCCESS){
        cout<<"no kernel arg set."<<endl;
        clean(context,commandQueue,program,kernel);
        for(int i=0;i<3;i++){
            if(memObjects[i]!=0)
                clReleaseMemObject(memObjects[i]);
        }
        return -1;
    }
    //设置工作项维数和每个维度的大小
    size_t globalWorkSize[1]={ARRAY_SIZE};
    size_t localWorkSize[1]={1};
    //核函数入内核队列排队等待执行
    err=clEnqueueNDRangeKernel(commandQueue,kernel,1,NULL,globalWorkSize,localWorkSize,0,NULL,NULL);
    if(err!=CL_SUCCESS){
        cout<<"no enqueue."<<endl;
        clean(context,commandQueue,program,kernel);
        for(int i=0;i<3;i++){
            if(memObjects[i]!=0)
                clReleaseMemObject(memObjects[i]);
        }
        return -1;
    }
    //从 device 读取计算结果返回主机内存
    err=clEnqueueReadBuffer(commandQueue,memObjects[2],CL_TRUE,0,sizeof(float)*ARRAY_SIZE,
        result,0,NULL,NULL);
    if(err!=CL_SUCCESS){
        cout<<"no read buffer."<<endl;
        clean(context,commandQueue,program,kernel);
        for(int i=0;i<3;i++){
            if(memObjects[i]!=0)
                clReleaseMemObject(memObjects[i]);
        }
        return -1;
    }
    for(int i=0;i<ARRAY_SIZE;i++){
        cout<<result[i]<<" ";
    }
    cout<<endl<<endl<<"success..."<<endl;
    //释放 device 资源
    clean(context,commandQueue,program,kernel);
    for(int i=0;i<3;i++){
        if(memObjects[i]!=0)
            clReleaseMemObject(memObjects[i]);
    }
    return 0;
}