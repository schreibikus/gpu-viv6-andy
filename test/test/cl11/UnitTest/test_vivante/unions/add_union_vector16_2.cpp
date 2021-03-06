/****************************************************************************
*
*    Copyright 2012 - 2017 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


#include <iostream>
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <math.h>

#include "types8_2.h"
#include "tests.h"

const char *kernel_add_union_vector16_2 =
"#ifndef _TYPES8_H_                                                                                                                 \n"
"#define _TYPES8_H_                                                                                                                   \n"
"                                                                                                                                   \n"
"#ifdef __OPENCL_VERSION__                                                                                                           \n"
"#define ALIGNED_STRUCT(structureType, alignBytes) structureType __attribute__ ((aligned(alignBytes)))                               \n"
"#else // __OPENCL_VERSION__                                                                                                       \n"
"#define ALIGNED_STRUCT(structureType, alignBytes) __declspec(align(alignBytes)) structureType                                       \n"
"#endif // __OPENCL_VERSION__                                                                                                       \n"
"                                                                                                                                   \n"
"ALIGNED_STRUCT(union, 32) InputA {                                                                                                   \n"
"    uint16 a;                                                                                                                       \n"
"    float16 h;                                                                                                                   \n"
"};                                                                                                                                \n"
"                                                                                                                                  \n"
"ALIGNED_STRUCT(union, 32) InputB {                                                                                                \n"
"    uint16 k;                                                                                                                     \n"
"    float16 b;                                                                                                                   \n"
"};                                                                                                                                \n"
"                                                                                                                                  \n"
"ALIGNED_STRUCT(union, 32) Result {                                                                                                \n"
"    uint16 z;                                                                                                                     \n"
"    float16 r;                                                                                                                   \n"
"};                                                                                                                                \n"
"                                                                                                                                  \n"
"#endif // _TYPES_H_                                                                                                               \n"
"__kernel void sum(__global const union InputA* a, __global const union InputB* b, __global union Result* c, int iNumElements)\n"
"{                                                                                                                                 \n"
"    // get index into global data array                                                                                           \n"
"    int tid = get_global_id(0);                                                                                                   \n"
"                                                                                                                                  \n"
"    // bound check (equivalent to the limit on a 'for' loop for standard/serial C code                                            \n"
"                                                                                                                                  \n"
"                                                                                                                                   \n"
"        c[tid].r = convert_float16(a[tid].a) + b[tid].b;                                                                                            \n"
"                                                                                                                                  \n"
"}                                                                                                                                 \n";

class TestCase {
public:
    TestCase(cl::Program program_, cl::Context context, const std::vector<cl::Device>& devices)
        : _program(program_)
        , _context(context)
        , _devices(devices)
    {
    }

    virtual ~TestCase() {}

    virtual void SetUp() {}
    virtual bool Execute() = 0;
    virtual void TearDown() {}

protected:
    cl::Program _program;
    cl::Context _context;
    const std::vector<cl::Device>& _devices;
};

class TestCase_add_union_vector16_2 : public TestCase  {
public:
    TestCase_add_union_vector16_2(int numElements, cl::Program program_, cl::Context context, const std::vector<cl::Device>& devices)
        : TestCase(program_, context, devices)
        , _numElements(numElements)
    {
    }

    virtual void SetUp()
    {
        // Find the relevant kernel
        cl_int err;
        _kernel = cl::Kernel(_program, "sum", &err);

        _localWorkSize = 256;
        _globalWorkSize = (size_t)(_localWorkSize * ceil((float)_numElements / _localWorkSize));  // rounded up to the nearest multiple of the LocalWorkSize

        // Allocate the arrays
        _inputA = new InputA[_globalWorkSize];
        _inputB = new InputB[_globalWorkSize];
        _result = new Result[_globalWorkSize];
        _goldStandard = new Result[_numElements];

        _deviceInputA = cl::Buffer(_context, CL_MEM_READ_ONLY, sizeof(InputA) * _globalWorkSize);
        _deviceInputB = cl::Buffer(_context, CL_MEM_READ_ONLY, sizeof(InputB) * _globalWorkSize);
        _deviceResult = cl::Buffer(_context, CL_MEM_WRITE_ONLY, sizeof(Result) * _globalWorkSize);

        _kernel.setArg(0,sizeof (_deviceInputA), &_deviceInputA);
        _kernel.setArg(1,sizeof (_deviceInputB), &_deviceInputB);
        _kernel.setArg(2,sizeof (_deviceResult), &_deviceResult);
        _kernel.setArg(3,sizeof (_numElements), &_numElements);
    }

    virtual bool Execute()
    {
        _fillData();
        _computeGoldStandard();
        _computeDeviceResult();
        return _compare();
    }

    virtual void TearDown()
    {
        delete [] _inputA;
        delete [] _inputB;
        delete [] _result;
        delete [] _goldStandard;
    }
private:
    void _fillData()
    {
        for (int i = 0; i < _numElements; ++i) {
            _inputA[i].a.s[0] = rand();
            _inputA[i].h.s[0] = (float)rand();
            _inputB[i].k.s[0] = rand();
            _inputB[i].b.s[0] = (float)rand();
            _inputA[i].a.s[1] = rand();
            _inputA[i].h.s[1] = (float)rand();
            _inputB[i].k.s[1] = rand();
            _inputB[i].b.s[1] = (float)rand();
            _inputA[i].a.s[2] = rand();
            _inputA[i].h.s[2] = (float)rand();
            _inputB[i].k.s[2] = rand();
            _inputB[i].b.s[2] = (float)rand();
            _inputA[i].a.s[3] = rand();
            _inputA[i].h.s[3] = (float)rand();
            _inputB[i].k.s[3] = rand();
            _inputB[i].b.s[3] = (float)rand();
            /////////////////////////////////
            _inputA[i].a.s[4] = rand();
            _inputA[i].h.s[4] = (float)rand();
            _inputB[i].k.s[4] = rand();
            _inputB[i].b.s[4] = (float)rand();
            _inputA[i].a.s[5] = rand();
            _inputA[i].h.s[5] = (float)rand();
            _inputB[i].k.s[5] = rand();
            _inputB[i].b.s[5] = (float)rand();
            _inputA[i].a.s[6] = rand();
            _inputA[i].h.s[6] = (float)rand();
            _inputB[i].k.s[6] = rand();
            _inputB[i].b.s[6] = (float)rand();
            _inputA[i].a.s[7] = rand();
            _inputA[i].h.s[7] = (float)rand();
            _inputB[i].k.s[7] = rand();
            _inputB[i].b.s[7] = (float)rand();
            ///////////////////////////////////
            _inputA[i].a.s[8] = rand();
            _inputA[i].h.s[8] = (float)rand();
            _inputB[i].k.s[8] = rand();
            _inputB[i].b.s[8] = (float)rand();
            _inputA[i].a.s[9] = rand();
            _inputA[i].h.s[9] = (float)rand();
            _inputB[i].k.s[9] = rand();
            _inputB[i].b.s[9] = (float)rand();
            _inputA[i].a.s[10] = rand();
            _inputA[i].h.s[10] = (float)rand();
            _inputB[i].k.s[10] = rand();
            _inputB[i].b.s[10] = (float)rand();
            _inputA[i].a.s[11] = rand();
            _inputA[i].h.s[11] = (float)rand();
            _inputB[i].k.s[11] = rand();
            _inputB[i].b.s[11] = (float)rand();
            /////////////////////////////////
            _inputA[i].a.s[12] = rand();
            _inputA[i].h.s[12] = (float)rand();
            _inputB[i].k.s[12] = rand();
            _inputB[i].b.s[12] = (float)rand();
            _inputA[i].a.s[13] = rand();
            _inputA[i].h.s[13] = (float)rand();
            _inputB[i].k.s[13] = rand();
            _inputB[i].b.s[13] = (float)rand();
            _inputA[i].a.s[14] = rand();
            _inputA[i].h.s[14] = (float)rand();
            _inputB[i].k.s[14] = rand();
            _inputB[i].b.s[14] = (float)rand();
            _inputA[i].a.s[15] = rand();
            _inputA[i].h.s[15] = (float)rand();
            _inputB[i].k.s[15] = rand();
            _inputB[i].b.s[15] = (float)rand();
        }
    }

    void _computeGoldStandard()
    {
        for (int i = 0; i < _numElements; i++) {
            _goldStandard[i].r.s[0] = _inputA[i].a.s[0] + (ISSUBNORM(_inputB[i].b.s[0]) ? 0 : _inputB[i].b.s[0]);
            _goldStandard[i].r.s[1] = _inputA[i].a.s[1] + (ISSUBNORM(_inputB[i].b.s[1]) ? 0 : _inputB[i].b.s[1]);
            _goldStandard[i].r.s[2] = _inputA[i].a.s[2] + (ISSUBNORM(_inputB[i].b.s[2]) ? 0 : _inputB[i].b.s[2]);
            _goldStandard[i].r.s[3] = _inputA[i].a.s[3] + (ISSUBNORM(_inputB[i].b.s[3]) ? 0 : _inputB[i].b.s[3]);
            /////////////////////////////////////////////////////////////////
            _goldStandard[i].r.s[4] = _inputA[i].a.s[4] + (ISSUBNORM(_inputB[i].b.s[4]) ? 0 : _inputB[i].b.s[4]);
            _goldStandard[i].r.s[5] = _inputA[i].a.s[5] + (ISSUBNORM(_inputB[i].b.s[5]) ? 0 : _inputB[i].b.s[5]);
            _goldStandard[i].r.s[6] = _inputA[i].a.s[6] + (ISSUBNORM(_inputB[i].b.s[6]) ? 0 : _inputB[i].b.s[6]);
            _goldStandard[i].r.s[7] = _inputA[i].a.s[7] + (ISSUBNORM(_inputB[i].b.s[7]) ? 0 : _inputB[i].b.s[7]);
            /////////////////////////////////////////////////////////////////
            _goldStandard[i].r.s[8] = _inputA[i].a.s[8] + (ISSUBNORM(_inputB[i].b.s[8]) ? 0 : _inputB[i].b.s[8]);
            _goldStandard[i].r.s[9] = _inputA[i].a.s[9] + (ISSUBNORM(_inputB[i].b.s[9]) ? 0 : _inputB[i].b.s[9]);
            _goldStandard[i].r.s[10] = _inputA[i].a.s[10] + (ISSUBNORM(_inputB[i].b.s[10]) ? 0 : _inputB[i].b.s[10]);
            _goldStandard[i].r.s[11] = _inputA[i].a.s[11] + (ISSUBNORM(_inputB[i].b.s[11]) ? 0 : _inputB[i].b.s[11]);
            /////////////////////////////////////////////////////////////////
            _goldStandard[i].r.s[12] = _inputA[i].a.s[12] + (ISSUBNORM(_inputB[i].b.s[12]) ? 0 : _inputB[i].b.s[12]);
            _goldStandard[i].r.s[13] = _inputA[i].a.s[13] + (ISSUBNORM(_inputB[i].b.s[13]) ? 0 : _inputB[i].b.s[13]);
            _goldStandard[i].r.s[14] = _inputA[i].a.s[14] + (ISSUBNORM(_inputB[i].b.s[14]) ? 0 : _inputB[i].b.s[14]);
            _goldStandard[i].r.s[15] = _inputA[i].a.s[15] + (ISSUBNORM(_inputB[i].b.s[15]) ? 0 : _inputB[i].b.s[15]);

        }
    }

    void _computeDeviceResult()
    {
        int err;
        cl::CommandQueue queue(_context, _devices[0], 0, &err);

        queue.enqueueWriteBuffer(_deviceInputA, false, 0, sizeof(InputA) * _globalWorkSize, _inputA);
        queue.enqueueWriteBuffer(_deviceInputB, false, 0, sizeof(InputB) * _globalWorkSize, _inputB);
        queue.enqueueNDRangeKernel(_kernel, cl::NullRange, cl::NDRange(_globalWorkSize), cl::NullRange);
        queue.enqueueReadBuffer(_deviceResult, true, 0, sizeof(Result) * _globalWorkSize, _result);
    }

    bool _compare()
    {
        unsigned int checkResult1;
        unsigned int checkResult2;
        float checkResult3;
        float checkResult4;

        for (int i = 0; i < _numElements; ++i) {
            for (int j = 0; j < 16; ++j) {
                checkResult1 = (*(unsigned int *)&(_goldStandard[i].r.s[j]) + 2);
                checkResult2 = (*(unsigned int *)&(_goldStandard[i].r.s[j]) - 2);
                checkResult3 = fabsf(*( float *) &checkResult1);
                checkResult4 = fabsf(*( float *) &checkResult2);
                if ((fabsf(_result[i].r.s[j]) > checkResult3 || fabsf(_result[i].r.s[j]) < checkResult4) && !isnan(_goldStandard[i].r.s[j]) && !isinf(_goldStandard[i].r.s[j])){
                    std::cout << "  c:" << _goldStandard[i].r.s[j] << " ocl:" << _result[i].r.s[j] << std::endl;
                    return false;
                }
            }
        }
        return true;
    }

private:
    int _numElements;

    InputA* _inputA;
    InputB* _inputB;
    Result* _result;
    Result* _goldStandard;

    cl::Kernel _kernel;
    cl::Buffer _deviceInputA;
    cl::Buffer _deviceInputB;
    cl::Buffer _deviceResult;

    size_t _globalWorkSize;
    size_t _localWorkSize;
};



int add_union_vector16_2(void)
{
    cl_int err = CL_SUCCESS;
    int  cnt = 1;

    try {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (platforms.size() == 0) {
            std::cerr << "Failed to find any platforms." << std::endl;
            return 0;
        }

        cl_context_properties properties[] =
        { CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(), 0};
        cl::Context context(CL_DEVICE_TYPE_GPU, properties);

        std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

        
        const char* clProgramSource = kernel_add_union_vector16_2;//oclLoadProgSource("multiply.cl", "", &szKernelLength);
        if (clProgramSource == 0) {
            std::cerr << "OpenCL program not found." << std::endl;
            return 0;
        }

        cl::Program::Sources source(1, std::make_pair(clProgramSource,strlen(clProgramSource)));
        cl::Program program_ = cl::Program(context, source);
        try {
            program_.build(devices);
        }
        catch (...) {
            for (size_t i = 0; i < devices.size(); ++i) {
                cl::STRING_CLASS  log;
                program_.getBuildInfo(devices[i], CL_PROGRAM_BUILD_LOG, &log);
                std::cerr << log.c_str() << std::endl << std::endl;
            }
            throw;
        }

        std::cout << "Running test add_union_vector16_2..." << std::endl;
        TestCase_add_union_vector16_2 add_union_vector16_2(10, program_, context, devices);

        bool control = true;
        add_union_vector16_2.SetUp();
        for (int i = 0; i < 10; ++i) {
            if(!add_union_vector16_2.Execute()){
                control = false;
                cnt = 0;
            }
            std::cout << "RUN " << i + 1<< ": " << (control ? "PASSED" : "FAILED!") << std::endl;
        }
        add_union_vector16_2.TearDown();
    }
    catch (cl::Error err) {
        std::cerr
            << "ERROR: "
            << err.what()
            << "("
            << err.err()
            << ")"
            << std::endl;
    }

    return cnt;
}