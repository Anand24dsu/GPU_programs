#include <CL/cl.h>
#include <stdio.h>

#define MAX_SOURCE_SIZE (0x100000)

int main() {
    // Input data
    int a = 5, b = 10;

    // Load the kernel source code
    FILE *kernelFile = fopen("add_numbers.cl", "r");
    if (!kernelFile) {
        fprintf(stderr, "Failed to open kernel file.\n");
        return 1;
    }

    char *kernelSource = (char *)malloc(MAX_SOURCE_SIZE);
    size_t sourceSize = fread(kernelSource, 1, MAX_SOURCE_SIZE, kernelFile);
    fclose(kernelFile);

    // Initialize OpenCL
    cl_platform_id platformId;
    clGetPlatformIDs(1, &platformId, NULL);

    cl_device_id deviceId;
    clGetDeviceIDs(platformId, CL_DEVICE_TYPE_GPU, 1, &deviceId, NULL);

    cl_context context = clCreateContext(NULL, 1, &deviceId, NULL, NULL, NULL);
    cl_command_queue commandQueue = clCreateCommandQueue(context, deviceId, 0, NULL);

    // Create and build the program
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, &sourceSize, NULL);
    clBuildProgram(program, 1, &deviceId, NULL, NULL, NULL);

    // Create the kernel
    cl_kernel kernel = clCreateKernel(program, "add_numbers", NULL);

    // Set the kernel arguments
    clSetKernelArg(kernel, 0, sizeof(int), &a);
    clSetKernelArg(kernel, 1, sizeof(int), &b);

    // Execute the kernel
    size_t globalWorkSize = 1;
    clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &globalWorkSize, NULL, 0, NULL, NULL);

    // Read the result
    int result;
    clEnqueueReadBuffer(commandQueue, aBuffer, CL_TRUE, 0, sizeof(int), &result, 0, NULL, NULL);

    // Print the result
    printf("Result: %d\n", result);

    // Clean up
    clReleaseMemObject(aBuffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(context);

    free(kernelSource);

    return 0;
}

