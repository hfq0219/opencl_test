#include "tool.h"


/**创建平台、设备、上下文、命令队列、程序对象,对大部分 OpenCL 程序相同。
 */
int CreateTool(cl_platform_id &platform,cl_device_id &device,cl_context &context,
                cl_command_queue &commandQueue,cl_program &program,const char *fileName){
    cl_int err;
    cl_uint num;
    //获得第一个可用平台
    err=clGetPlatformIDs(1, &platform, &num);
    if(err!=CL_SUCCESS||num<=0||platform==NULL){
        cout<<"no platform."<<endl;
        return -1;
    }
    //获得第一个可用设备
    err=clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, &num);
    if(err!=CL_SUCCESS||num<=0||device==NULL){
        cout<<"no device."<<endl;
        return -1;
    }
    //获得一个上下文
    cl_context_properties properties[]={
        CL_CONTEXT_PLATFORM,(cl_context_properties)platform,0
    };
    context=clCreateContextFromType(properties,CL_DEVICE_TYPE_GPU,NULL,NULL,&err);
    if(err!=CL_SUCCESS||context==NULL){
        cout<<"no context."<<endl;
        return -1;
    }
    //通过上下文对指定设备构建命令队列
    commandQueue=clCreateCommandQueue(context, device, 0, &err);
    if(err!=CL_SUCCESS||commandQueue==NULL){
        cout<<"no commandQueue."<<endl;
        return -1;
    }
    //读取内核文件并转换为字符串
    ifstream kernelFile(fileName,ios::in);
    if(!kernelFile.is_open()){
        cout<<"kernel file open failed."<<endl;
        return -1;
    }
    ostringstream oss;
    oss<<kernelFile.rdbuf();
    string src=oss.str();
    const char *srcStr=src.c_str();
    //在上下文环境下编译指定内核文件的程序对象
    program=clCreateProgramWithSource(context, 1, (const char **)&srcStr, NULL, &err);
    if(err!=CL_SUCCESS||program==NULL){
        cout<<"no program."<<endl;
        return -1;
    }
    err=clBuildProgram(program, 0, NULL, NULL,NULL,NULL);
    if(err!=CL_SUCCESS){
        cout<<"can not build program."<<endl;
        char buildLog[16384];
        clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG,sizeof(buildLog),
            buildLog,NULL);
        cerr<<buildLog;
        return -1;
    }
    return 0;
}
//资源释放
void clean(cl_context &context,cl_command_queue &commandQueue,cl_program &program,cl_kernel &kernel){
    if(commandQueue!=0)
        clReleaseCommandQueue(commandQueue);
    if(kernel!=0)
        clReleaseKernel(kernel);
    if(program!=0)
        clReleaseProgram(program);
    if(context!=0)
        clReleaseContext(context);
}