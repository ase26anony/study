#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to force calls */
extern void foo(void);
extern void bar(void);
extern void baz(void);

/* Vector types for SSE/AVX register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Force non-inline, register-clobbering operations */
#define CLOBBER_CALL() \
    asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", \
                  "rsi", "rdi", "r8", "r9", "r10", "r11", \
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", \
                  "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", \
                  "xmm12", "xmm13", "xmm14", "xmm15")

#define CLOBBER_INT() \
    asm volatile ("" ::: "rax", "rbx", "rcx", "rdx", "rsi", "rdi", \
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15")

#define CLOBBER_VEC() \
    asm volatile ("" ::: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", \
                  "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", \
                  "xmm12", "xmm13", "xmm14", "xmm15", "ymm0", "ymm1", \
                  "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7")

int main(int argc, char *argv[]) {
    /* Force argc check for conditional control flow */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations <= 0) iterations = 3;
    
    /* Declare MANY local variables to create register pressure */
    
    /* Integer variables - 8 bytes each */
    volatile long long int1 = 0x123456789ABCDEF0LL;
    volatile long long int2 = 0xFEDCBA9876543210LL;
    volatile long long int3 = 0xAAAAAAAAAAAAAAAALL;
    volatile long long int4 = 0x5555555555555555LL;
    volatile long long int5 = 0x3333333333333333LL;
    volatile long long int6 = 0xCCCCCCCCCCCCCCCCLL;
    volatile long long int7 = 0x7777777777777777LL;
    volatile long long int8 = 0x8888888888888888LL;
    volatile long long int9 = 0x9999999999999999LL;
    volatile long long int10 = 0xBBBBBBBBBBBBBBBBLL;
    
    /* Floating point variables */
    volatile double dbl1 = 3.141592653589793;
    volatile double dbl2 = 2.718281828459045;
    volatile double dbl3 = 1.414213562373095;
    volatile double dbl4 = 1.618033988749895;
    volatile double dbl5 = 0.577215664901532;
    volatile float flt1 = 1.234567f;
    volatile float flt2 = 9.876543f;
    volatile float flt3 = 4.567890f;
    volatile float flt4 = 7.654321f;
    
    /* Pointer variables */
    volatile char *ptr1 = (char*)&int1;
    volatile char *ptr2 = (char*)&int2;
    volatile char *ptr3 = (char*)&int3;
    volatile char *ptr4 = (char*)&int4;
    
    /* Vector variables - use all vector registers */
    volatile v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    volatile v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    volatile v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    volatile v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    volatile v2df vecd1 = {1.234, 5.678};
    volatile v2df vecd2 = {9.012, 3.456};
    volatile v4si veci1 = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
    volatile v4si veci2 = {0x55555555, 0x66666666, 0x77777777, 0x88888888};
    
    /* Additional variables to ensure spill */
    volatile long long spill1 = 0x1111111111111111LL;
    volatile long long spill2 = 0x2222222222222222LL;
    volatile long long spill3 = 0x3333333333333333LL;
    volatile long long spill4 = 0x4444444444444444LL;
    
    /* Result accumulator */
    volatile double result = 0.0;
    
    /* Complex loop with conditional control flow */
    for (volatile int i = 0; i < iterations; i++) {
        /* Pre-call computations - keep all variables live */
        int1 = int2 + int3 - int4;
        int2 = int5 * int6 / (int7 + 1);
        int3 = int8 ^ int9 | int10;
        
        dbl1 = dbl2 * dbl3 + dbl4 - dbl5;
        flt1 = flt2 / flt3 * flt4;
        
        /* Pointer arithmetic */
        ptr1 = ptr2 + (int)(dbl1 * 10);
        ptr3 = ptr4 - (int)(flt1 * 20);
        
        /* Vector operations - use all vector registers */
        vec1 = vec2 + vec3 * vec4;
        vec2 = vec1 - vec3 / vec4;
        vec3 = vec4 * vec1 + vec2;
        vecd1 = vecd2 * 2.5 - vecd1;
        veci1 = veci2 | veci1;
        veci2 = veci1 & veci2;
        
        /* Spill variable computations */
        spill1 = spill2 + spill3 - spill4;
        spill2 = spill3 * spill4 / (spill1 + 1);
        
        /* First clobber - force save of integer registers */
        CLOBBER_INT();
        
        /* First external call - forces caller-save */
        foo();
        
        /* Second clobber - force save of vector registers */
        CLOBBER_VEC();
        
        /* More computations between calls */
        int4 = int1 * int2 + int3;
        int5 = int6 - int7 * int8;
        dbl2 = dbl3 / dbl4 + dbl5;
        flt2 = flt3 * flt4 - flt1;
        
        vec4 = vec1 + vec2 - vec3;
        vecd2 = vecd1 * 3.14 + vecd2;
        
        /* Third clobber - mixed registers */
        CLOBBER_CALL();
        
        /* Second external call */
        bar();
        
        /* Post-call computations */
        int6 = int7 + int8 - int9;
        int7 = int8 * int9 / (int10 + 1);
        dbl3 = dbl4 * dbl5 - dbl1;
        
        /* Another clobber */
        asm volatile ("" ::: "memory", "rax", "rbx", "xmm0", "xmm1", "xmm2");
        
        /* Third external call */
        baz();
        
        /* Final computations in the iteration */
        int8 = int9 ^ int10 | int1;
        int9 = int10 + int1 - int2;
        dbl4 = dbl5 / dbl1 + dbl2;
        
        vec1 = vec2 * vec3 + vec4;
        vecd1 = vecd2 - vecd1 * 0.5;
        
        /* Accumulate result to prevent elimination */
        result += dbl1 + dbl2 + dbl3 + dbl4 + dbl5;
        result += flt1 + flt2 + flt3 + flt4;
        result += (double)(int1 + int2 + int3 + int4 + int5);
        result += (double)(int6 + int7 + int8 + int9 + int10);
        
        /* Conditional branch inside loop - creates complex CFG */
        if (i % 2 == 0) {
            /* Extra clobber in branch */
            asm volatile ("" ::: "xmm3", "xmm4", "xmm5", "xmm6");
            int10 = int1 * 2 + int2 / 3;
            dbl5 = dbl1 * 1.5 - dbl2;
        } else {
            /* Different clobber in else branch */
            asm volatile ("" ::: "rax", "rdx", "xmm7", "xmm8");
            int10 = int3 * 3 - int4 / 2;
            dbl5 = dbl3 * 2.5 + dbl4;
        }
        
        /* More vector operations */
        vec2 = vec3 + vec4 * vec1;
        vec3 = vec4 - vec1 / vec2;
        
        /* Force another spill */
        spill3 = spill4 + spill1 - spill2;
        spill4 = spill1 * spill2 / (spill3 + 1);
    }
    
    /* Final aggregation and output */
    double final_result = result;
    final_result += (double)(spill1 + spill2 + spill3 + spill4);
    
    /* Use vector elements in final result */
    float vec_sum = vec1[0] + vec1[1] + vec1[2] + vec1[3];
    final_result += vec_sum;
    
    printf("Result: %f\n", final_result);
    
    return (int)final_result % 256;
}
