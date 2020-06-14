// reference:   https://www.fixstars.com/en/opencl/book/OpenCLProgrammingBook/first-opencl-program/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <CL/cl.h>

#ifdef AOCL
#include "CL/opencl.h"
#include "AOCLUtils/aocl_utils.h"

using namespace aocl_utils;
void cleanup();
#endif

// #define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x100000)

#define DEVICE_NAME_LEN 128 //MATRIX
static char dev_name[DEVICE_NAME_LEN]; // MATRIX

//MATRIX
static float A[8] = {
  1.0f,  1.0f,  1.0f,  1.0f,
  1.0f,  1.0f,  1.0f,  1.0f };

//MATRIX
static float B[24] = {
  2.0f,  2.0f,  2.0f,  2.0f, 2.0f, 2.0f,
  2.0f,  2.0f,  2.0f,  2.0f, 2.0f, 2.0f,
  2.0f,  2.0f,  2.0f,  2.0f, 2.0f, 2.0f,
  2.0f,  2.0f,  2.0f,  2.0f, 2.0f, 2.0f };

int main()
{
    printf("MatrixMulti.\n");

    // MATRIX
    cl_uint platformCount;
    cl_platform_id* platforms; //expecting an array of platforms from devcloud

    cl_device_id device_id = NULL;
    cl_uint ret_num_devices;
    cl_int ret;
    cl_context context = NULL;
    cl_command_queue command_queue = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;

    // from HELLO
    // cl_mem memobj = NULL; // likely unecessary
    cl_platform_id platform_id = NULL;
    cl_uint ret_num_platforms;

    // char string[MEM_SIZE];

    FILE* fp;
    char fileName[] = "mykernel.cl";  // used to have ./
    char* source_str;
    size_t source_size;

    // MATRIX
    printf("Creating Height / Width ints.\n");
    int wA = 4;
    int hA = 2;
    int wB = 6;
    int hB = 4;
    int wC = wB;
    int hC = hA;
    printf("Created Height / Width ints.\n\n");
    // end MATRIX

    // MATRIX
    #ifdef AOCL  // Altera FPGA
    // get all platforms
        clGetPlatformIDs(0, NULL, &platformCount);
        platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * platformCount);
        // Get the OpenCL platform.
        platforms[0] = findPlatform("Intel(R) FPGA");
        if (platforms[0] == NULL) {
            printf("ERROR: Unable to find Intel(R) FPGA OpenCL platform.\n");
            return false;
        }
        // Query the available OpenCL device.
        getDevices(platforms[0], CL_DEVICE_TYPE_ALL, &ret_num_devices);
        printf("Platform: %s\n", getPlatformName(platforms[0]).c_str());
        printf("Using one out of %d device(s)\n", ret_num_devices);
        ret = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
        printf("device name=  %s\n", getDeviceName(device_id).c_str());
    
    #else // for local machine use
        // Get Platform and Device Info   HELLO
        ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
        ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);

    #endif // end MATRIX


    // MATRIX
    printf("Creating OpenCL context.\n");
    // Create OpenCL context
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    printf("Created OpenCL context.\n\n");
    //system("pause");

    printf("Create Command queue.\n");
    // Create Command Queue
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
    printf("Created Command queue.\n\n");
    //system("pause");
    // end MATRIX

// MATRIX
#ifdef AOCL  /* on FPGA we need to create kernel from binary */
    /* Create Kernel Program from the binary */
    std::string binary_file = getBoardBinaryFile("mykernel", device_id);
    printf("Using AOCX: %s\n", binary_file.c_str());
    program = createProgramFromBinary(context, binary_file.c_str(), &device_id, 1);
#else
    // end MATRIX

    // Load the source code containing kernel MATRIX (APPLE)
    fp = fopen(fileName, "r");

    if (!fp) {
        fprintf(stderr, "Failed to load source code containing kernel.\n");
        exit(1);
    }
    else {
        printf("loaded source code contanining kernel.\n\n");
    }

    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    // Create Kernel Program from the source
    program = clCreateProgramWithSource(context, 1, (const char**)&source_str,
        (const size_t*)&source_size, &ret);
    if (ret != CL_SUCCESS) {
        printf("Failed to create program from source.\n");
        exit(1);
    } // end MATRIX (APPLE)
    else {
        printf("Created program from source.\n\n");
    }
#endif

    /*
    // MATRIX // exists above on 113
    #ifdef AOCL  // on FPGA we need to create kernel from binary
        // Create Kernel Program from the binary
        std::string binary_file = getBoardBinaryFile("mykernel", device_id);
        printf("Using AOCX: %s\n", binary_file.c_str());
        program = createProgramFromBinary(context, binary_file.c_str(), &device_id, 1);
    #endif
    //end MATRIX
    */

    // Build Kernel Program    in hello & MATRIX
    printf("build kernel program.\n");
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    if (ret != CL_SUCCESS) {
        printf("Failed to build kernel program.\n");
        // exit(1);
    }
    else {
        printf("built kernel program.\n");
    }
    //system("pause");

    // Create OpenCL Kernel     in hellow & MATRIX
    kernel = clCreateKernel(program, "simpleMultiply", &ret);
    if (ret != CL_SUCCESS) {
        printf("Failed to create kernel.\n");
        //exit(1);
    }
    else {
        printf("created OpenCL kernel.\n");
    }
    //system("pause");

    // MATRIX  cretaes an array of size height by width of c with floats and sets them to zeros in local memory
    float* C = (float*)calloc(hC * wC, sizeof(float));
    for (int i = 0; i < wC * hC; i++) {
        printf("%f ", C[i]); // prints the array of float zeroes
    }
    printf("\n");
    // end MATRIX

    // MATRIX
    // PREVIOUSLY     // Create Memory Buffer memobj = clCreateBuffer(context, CL_MEM_READ_WRITE, MEM_SIZE * sizeof(char), NULL, &ret);

    // We assume A, B, C are float arrays which have been declared and initialized
    // allocate space for MATRIX A on the device
    cl_mem bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY,
        wA * hA * sizeof(float), NULL, &ret);

    // copy MATRIX A to the device
    clEnqueueWriteBuffer(command_queue, bufferA, CL_TRUE, 0,
        wA * hA * sizeof(float), (void*)A, 0, NULL, NULL);

    // allocate space for MATRIX B on the device
    cl_mem bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY,
        wB * hB * sizeof(float), NULL, &ret);

    // copy MATRIX B to the device
    clEnqueueWriteBuffer(command_queue, bufferB, CL_TRUE, 0,
        wB * hB * sizeof(float), (void*)B, 0, NULL, NULL);

    // allocate space for MATRIX C on the device
    cl_mem bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
        wC * hC * sizeof(float), NULL, &ret);
    // end MATRIX

    // Set the kernel arguments JOHN
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&bufferC);
    clSetKernelArg(kernel, 1, sizeof(cl_int), (void*)&wA);
    clSetKernelArg(kernel, 2, sizeof(cl_int), (void*)&hA);
    clSetKernelArg(kernel, 3, sizeof(cl_int), (void*)&wB);
    clSetKernelArg(kernel, 4, sizeof(cl_int), (void*)&hB);
    clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*)&bufferA);
    clSetKernelArg(kernel, 6, sizeof(cl_mem), (void*)&bufferB); // end john

    // Create Kernel Program from the source
    // program = clCreateProgramWithSource(context, 1, (const char**)&source_str,
    //    (const size_t*)&source_size, &ret);

    // Set OpenCL Kernel Parameters   HELLO
    // ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&memobj);

    // Execute OpenCL Kernel   HELLO
    // ret = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);

    // Execute the kernel MATRIX
    size_t globalws[2] = { wC, hC };
    size_t localws[2] = { 2, 2 };
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL,
        globalws, localws, 0, NULL, NULL);
    // it is important to check the return value. 
    // e.g. enqueueNDRangeKernel may fail when Work group size does not divide evenly into global work size
    if (ret != CL_SUCCESS) {
        printf("Failed to enqueueNDRangeKernel.\n");
        //exit(1);
    } // end MATRIX
    else {
        printf("executed kernel & enqueueNDRangeKernel.\n");
    }
    // system("pause");

    // Copy the output data back to the host MATRIX
    clEnqueueReadBuffer(command_queue, bufferC, CL_TRUE, 0, wC * hC * sizeof(float),
        (void*)C, 0, NULL, NULL); // end MATRIX

    // Copy results from the memory buffer HELLO
    //ret = clEnqueueReadBuffer(command_queue, memobj, CL_TRUE, 0,
    //    MEM_SIZE * sizeof(char), string, 0, NULL, NULL);

    // Verify result MATRIX
    for (int i = 0; i < wC * hC; i++) {
        printf("%f ", C[i]);
    }
    printf("\n"); // end MATRIX

    // Display Result
    //puts(string); // HELLO

    // system("pause"); // last chance to look

    // free(source_str); // HELLO

    // free resources
    free(C);

    // MATRIX    // Finalization HELLO
    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferB);
    clReleaseMemObject(bufferC);   // ret = clReleaseMemObject(memobj);
    clReleaseCommandQueue(command_queue);   // ret = clReleaseCommandQueue(command_queue);
    clReleaseKernel(kernel);   // ret = clReleaseKernel(kernel);
    clReleaseProgram(program);   // ret = clReleaseProgram(program);
    clReleaseContext(context);   // ret = clReleaseContext(context);
    // end MATRIX

    // Finalization HELLO
    // ret = clFlush(command_queue);
    // ret = clFinish(command_queue);
    
    return 0;
}

#ifdef AOCL
// Altera OpenCL needs this callback function implemented in main.c
// Free the resources allocated during initialization
void cleanup() {
}
#endif
