#include <stdio.h>
#include <CL/cl.h>
#include <stdlib.h>
#include <fcntl.h>

#define BUF_SIZE 2048
#define ERROR -1

#define END 0

char *KernelSource = "";

char* read_file(char* filename, int* len) {
    int ip_file, rd_cnt, tot_cnt = 0;
    int buf_size = BUF_SIZE;
    char * tmp_buf = malloc(buf_size * sizeof(char));
    char * offset = tmp_buf;
    ip_file = open(filename, O_RDONLY);
    if ( ip_file == ERROR ) {
        perror("Error accessing file.");
        return (char*)-1;
    }
    do {
        rd_cnt = read(ip_file,offset,BUF_SIZE);
        offset += rd_cnt;
        tot_cnt += rd_cnt;
    }while(rd_cnt != END);
    (*len) = tot_cnt;
    return tmp_buf;
}

const char *getErrorString(cl_int error)
{
switch(error){
    // run-time and JIT compiler errors
    case 0: return "CL_SUCCESS";
    case -1: return "CL_DEVICE_NOT_FOUND";
    case -2: return "CL_DEVICE_NOT_AVAILABLE";
    case -3: return "CL_COMPILER_NOT_AVAILABLE";
    case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case -5: return "CL_OUT_OF_RESOURCES";
    case -6: return "CL_OUT_OF_HOST_MEMORY";
    case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case -8: return "CL_MEM_COPY_OVERLAP";
    case -9: return "CL_IMAGE_FORMAT_MISMATCH";
    case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case -11: return "CL_BUILD_PROGRAM_FAILURE";
    case -12: return "CL_MAP_FAILURE";
    case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case -15: return "CL_COMPILE_PROGRAM_FAILURE";
    case -16: return "CL_LINKER_NOT_AVAILABLE";
    case -17: return "CL_LINK_PROGRAM_FAILURE";
    case -18: return "CL_DEVICE_PARTITION_FAILED";
    case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

    // compile-time errors
    case -30: return "CL_INVALID_VALUE";
    case -31: return "CL_INVALID_DEVICE_TYPE";
    case -32: return "CL_INVALID_PLATFORM";
    case -33: return "CL_INVALID_DEVICE";
    case -34: return "CL_INVALID_CONTEXT";
    case -35: return "CL_INVALID_QUEUE_PROPERTIES";
    case -36: return "CL_INVALID_COMMAND_QUEUE";
    case -37: return "CL_INVALID_HOST_PTR";
    case -38: return "CL_INVALID_MEM_OBJECT";
    case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40: return "CL_INVALID_IMAGE_SIZE";
    case -41: return "CL_INVALID_SAMPLER";
    case -42: return "CL_INVALID_BINARY";
    case -43: return "CL_INVALID_BUILD_OPTIONS";
    case -44: return "CL_INVALID_PROGRAM";
    case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46: return "CL_INVALID_KERNEL_NAME";
    case -47: return "CL_INVALID_KERNEL_DEFINITION";
    case -48: return "CL_INVALID_KERNEL";
    case -49: return "CL_INVALID_ARG_INDEX";
    case -50: return "CL_INVALID_ARG_VALUE";
    case -51: return "CL_INVALID_ARG_SIZE";
    case -52: return "CL_INVALID_KERNEL_ARGS";
    case -53: return "CL_INVALID_WORK_DIMENSION";
    case -54: return "CL_INVALID_WORK_GROUP_SIZE";
    case -55: return "CL_INVALID_WORK_ITEM_SIZE";
    case -56: return "CL_INVALID_GLOBAL_OFFSET";
    case -57: return "CL_INVALID_EVENT_WAIT_LIST";
    case -58: return "CL_INVALID_EVENT";
    case -59: return "CL_INVALID_OPERATION";
    case -60: return "CL_INVALID_GL_OBJECT";
    case -61: return "CL_INVALID_BUFFER_SIZE";
    case -62: return "CL_INVALID_MIP_LEVEL";
    case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64: return "CL_INVALID_PROPERTY";
    case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66: return "CL_INVALID_COMPILER_OPTIONS";
    case -67: return "CL_INVALID_LINKER_OPTIONS";
    case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

    // extension errors
    case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
    case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
    case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
    case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
    case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
    default: return "Unknown OpenCL error";
    }
}

void printMatrix(int *arr, int n, int m) {
  int i, j;
  for (i = 0 ; i < n ; ++i ) {
    for (j = 0 ; j < m ; ++j ) {
        printf("%d ", arr[i*n+j]);
    }
    printf("\n" );
  }
  printf("\n" );
}

int main(int argc, char const *argv[]) {
  cl_platform_id platform_id;
  cl_uint num_of_platforms;
  cl_device_id device_id;
  cl_uint num_of_devices;
  cl_context context;
  cl_context_properties properties[3];
  cl_kernel kernel;
  cl_command_queue command_queue;
  cl_int err;
  cl_program program;
  cl_mem inputA, inputB, output;
  size_t local[2], global[2];
  int len;
  int res;
  int n = 3;
  int inp_len = sizeof(int)*n*n, op_len = sizeof(int)*n*n;
  int matA[] = {1,2,5,4,2,5,5,2,6}, matB[] = {5,2,7,3,6,2,6,2,6};
  int matC[9];

  printMatrix(matA,n,n);
  printMatrix(matB,n,n);

  KernelSource = read_file("mat_mul.cl",&len);

  if (clGetPlatformIDs(1,&platform_id,&num_of_platforms) != CL_SUCCESS) {
    printf("Platform ID Error\n");
    return -1;
  }

  if (clGetDeviceIDs(platform_id,CL_DEVICE_TYPE_ALL,1,&device_id,&num_of_devices)) {
    printf("Device ID error\n");
    return -1;
  }


  properties[0] = CL_CONTEXT_PLATFORM;
  properties[1] = (cl_context_properties) platform_id;
  properties[2] = 0;

  context = clCreateContext(properties,1,&device_id,NULL,NULL,&err);

  command_queue = clCreateCommandQueue(context,device_id,0,&err);

  inputA = clCreateBuffer(context,CL_MEM_READ_ONLY,inp_len,NULL,&err);
  if (err < 0)
    printf("Create buffer Error: %s\n",getErrorString(err) );
  inputB = clCreateBuffer(context,CL_MEM_READ_ONLY,inp_len,NULL,&err);
  if (err < 0)
    printf("Create buffer Error: %s\n",getErrorString(err) );
  output = clCreateBuffer(context,CL_MEM_WRITE_ONLY,op_len,NULL,&err);
  if (err < 0)
    printf("Create buffer Error: %s\n",getErrorString(err) );

  clEnqueueWriteBuffer(command_queue,inputA,CL_TRUE,0,inp_len,matA,0,NULL,NULL);
  clEnqueueWriteBuffer(command_queue,inputB,CL_TRUE,0,inp_len,matB,0,NULL,NULL);

  program = clCreateProgramWithSource(context,1,(const char**) &KernelSource, NULL, &err);

  if (res = clBuildProgram(program,1,&device_id,NULL,NULL,NULL) != CL_SUCCESS) {
    printf("Error building program: %s\n",getErrorString(res));
    return -1;
  }

  kernel = clCreateKernel(program,"mat_mul",&err);
  if (err < 0)
    printf("Create buffer Error: %s\n",getErrorString(err) );

  if (clSetKernelArg(kernel,0,sizeof(cl_mem),&inputA) != CL_SUCCESS) {
    printf("Kernell setting error\n");
  }
  if (clSetKernelArg(kernel,1,sizeof(cl_mem),&inputB) != CL_SUCCESS) {
    printf("Kernell setting error\n");
  }
  if (clSetKernelArg(kernel,2,sizeof(cl_mem),&output) != CL_SUCCESS) {
    printf("Kernell setting error\n");
  }

  global[0] = 3;
  global[1] = 3;

  if(clEnqueueNDRangeKernel(command_queue,kernel,2,NULL,global,NULL,0,NULL,NULL) != CL_SUCCESS ) {
    printf("Enque error\n");
    return -1;
  }

  if(clEnqueueReadBuffer(command_queue,output,CL_TRUE,0,op_len,matC,0,NULL,NULL) != CL_SUCCESS ) {
    printf("Buffer read error\n");
    return -1;
  }

  printMatrix(matC,n,n);

  return 0;
}
