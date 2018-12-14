#include <iostream>
#include <sstream>
#include <fstream>
#include <CL/cl.h>

using namespace std;
#define ARRAY_SIZE 100

void clean(cl_context,cl_command_queue,cl_program,cl_kernel,cl_mem[]);

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

    err=clGetPlatformIDs(1, &platform, &num);
    if(err!=CL_SUCCESS||num<=0||platform==NULL){
        cout<<"no platform."<<endl;
        return -1;
    }
    err=clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &num);
    if(err!=CL_SUCCESS||num<=0||device==NULL){
        cout<<"no device."<<endl;
        return -1;
    }
    cl_context_properties properties[]={
        CL_CONTEXT_PLATFORM,(cl_context_properties)platform,0
    };
    context=clCreateContextFromType(properties,CL_DEVICE_TYPE_GPU,NULL,NULL,&err);
    if(err!=CL_SUCCESS||context==NULL){
        cout<<"no context."<<endl;
        return -1;
    }
    commandQueue=clCreateCommandQueue(context, device, 0, &err);
    if(err!=CL_SUCCESS||commandQueue==NULL){
        cout<<"no commandQueue."<<endl;
        clean(context,commandQueue,program,kernel,memObjects);
        return -1;
    }
    ifstream kernelFile("test.cl",ios::in);
    if(!kernelFile.is_open()){
        cout<<"kernel file open failed."<<endl;
        return -1;
    }
    ostringstream oss;
    oss<<kernelFile.rdbuf();
    string src=oss.str();
    const char *srcStr=src.c_str();
    program=clCreateProgramWithSource(context, 1, (const char **)&srcStr, NULL, &err);
    if(err!=CL_SUCCESS||program==NULL){
        cout<<"no program."<<endl;
        clean(context,commandQueue,program,kernel,memObjects);
        return -1;
    }
    err=clBuildProgram(program, 0, NULL, NULL,NULL,NULL);
    if(err!=CL_SUCCESS){
        cout<<"can not build program."<<endl;
        char buildLog[16384];
        clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG,sizeof(buildLog),
            buildLog,NULL);
        cerr<<buildLog;
        clean(context,commandQueue,program,kernel,memObjects);
        return -1;
    }
    kernel=clCreateKernel(program, "add", &err);
    if(err!=CL_SUCCESS||kernel==NULL){
        cout<<"no kernel."<<endl;
        clean(context,commandQueue,program,kernel,memObjects);
        return -1;
    }
    float a[ARRAY_SIZE],b[ARRAY_SIZE],result[ARRAY_SIZE];
    for(int i=0;i<ARRAY_SIZE;i++){
        a[i]=i;b[i]=i+1;
    }
    memObjects[0]=clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, 
        sizeof(float)*ARRAY_SIZE, a, NULL);
    memObjects[1]=clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, 
        sizeof(float)*ARRAY_SIZE, b, NULL);
    memObjects[2]=clCreateBuffer(context, CL_MEM_READ_WRITE, 
        sizeof(float)*ARRAY_SIZE, NULL, NULL);
    if(memObjects[0]==NULL||memObjects[1]==NULL||memObjects[2]==NULL){
        cout<<"mem err."<<endl;
        clean(context,commandQueue,program,kernel,memObjects);
        return -1;
    }
    err=clSetKernelArg(kernel, 0, sizeof(cl_mem), &memObjects[0]);
    err|=clSetKernelArg(kernel, 1, sizeof(cl_mem), &memObjects[1]);
    err|=clSetKernelArg(kernel, 2, sizeof(cl_mem), &memObjects[2]);
    if(err!=CL_SUCCESS){
        cout<<"no kernel arg set."<<endl;
        clean(context,commandQueue,program,kernel,memObjects);
        return -1;
    }
    size_t globalWorkSize[1]={ARRAY_SIZE};
    size_t localWorkSize[1]={1};
    err=clEnqueueNDRangeKernel(commandQueue,kernel,1,NULL,globalWorkSize,localWorkSize,0,NULL,NULL);
    if(err!=CL_SUCCESS){
        cout<<"no enqueue."<<endl;
        clean(context,commandQueue,program,kernel,memObjects);
        return -1;
    }
    err=clEnqueueReadBuffer(commandQueue,memObjects[2],CL_TRUE,0,sizeof(float)*ARRAY_SIZE,
        result,0,NULL,NULL);
    if(err!=CL_SUCCESS){
        cout<<"no read buffer."<<endl;
        clean(context,commandQueue,program,kernel,memObjects);
        return -1;
    }
    for(int i=0;i<ARRAY_SIZE;i++){
        cout<<result[i]<<" ";
    }
    cout<<endl<<"success..."<<endl;
    clean(context,commandQueue,program,kernel,memObjects);
    return 0;
}

void clean(cl_context context,cl_command_queue commandQueue,cl_program program,
    cl_kernel kernel,cl_mem memObjects[3]){
    for(int i=0;i<3;i++){
        if(memObjects[i]!=0)
            clReleaseMemObject(memObjects[i]);
    }
    if(commandQueue!=0)
        clReleaseCommandQueue(commandQueue);
    if(kernel!=0)
        clReleaseKernel(kernel);
    if(program!=0)
        clReleaseProgram(program);
    if(context!=0)
        clReleaseContext(context);
}