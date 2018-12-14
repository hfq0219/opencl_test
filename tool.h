#ifndef TOOL_H
#define TOOL_H

#include <CL/cl.h>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

int CreateTool(cl_platform_id &platform,cl_device_id &device,cl_context &context,
                cl_command_queue &commandQueue,cl_program &program,const char *fileName);

void clean(cl_context &context,cl_command_queue &commandQueue,cl_program &program,cl_kernel &kernel);

#endif