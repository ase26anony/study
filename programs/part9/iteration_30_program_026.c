/* reload_stress.c - Extreme register pressure test for GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Opaque functions to prevent optimization */
__attribute__((noinline)) int use_int(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

__attribute__((noinline)) double use_double(double a, double b, double c, 
                                           double d, double e, double f) {
    volatile double sink = a * b / c + d - e * f;
    return sink;
}

__attribute__((noinline)) void* use_address(void* a, void* b, void* c, 
                                           int offset1, int offset2) {
    volatile char* p1 = (char*)a + offset1;
    volatile char* p2 = (char*)b + offset2;
    volatile char* p3 = (char*)c;
    return (void*)(p1 + (p2 - p3));
}

__attribute__((noinline)) long long use_long_long(long long a, long long b,
                                                 long long c, long long d,
                                                 long long e, long long f,
                                                 long long g, long long h) {
    volatile long long sink = a * b + c * d - e * f + g * h;
    return sink;
}

__attribute__((noinline)) float use_float(float a, float b, float c, float d,
                                         float e, float f, float g, float h,
                                         float i, float j) {
    volatile float sink = a + b * c - d / e + f * g - h / i + j;
    return sink;
}

/* Main stress function */
__attribute__((noinline)) 
unsigned long long stress_reload(volatile int* int_arr, 
                                 volatile double* dbl_arr,
                                 volatile long long* ll_arr,
                                 volatile float* flt_arr) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    int v31, v32, v33, v34, v35, v36, v37, v38, v39, v40;
    
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    
    long long ll1, ll2, ll3, ll4, ll5, ll6, ll7, ll8, ll9, ll10;
    
    /* Take addresses of some locals to force stack addressing */
    int* pv1 = &v1;
    int* pv2 = &v2;
    double* pd1 = &d1;
    float* pf1 = &f1;
    
    unsigned long long checksum = 0;
    
    /* Initialize with array values */
    v1 = int_arr[0]; v2 = int_arr[1]; v3 = int_arr[2]; v4 = int_arr[3];
    v5 = int_arr[4]; v6 = int_arr[5]; v7 = int_arr[6]; v8 = int_arr[7];
    v9 = int_arr[8]; v10 = int_arr[9];
    
    d1 = dbl_arr[0]; d2 = dbl_arr[1]; d3 = dbl_arr[2]; d4 = dbl_arr[3];
    d5 = dbl_arr[4]; d6 = dbl_arr[5]; d7 = dbl_arr[6]; d8 = dbl_arr[7];
    
    f1 = flt_arr[0]; f2 = flt_arr[1]; f3 = flt_arr[2]; f4 = flt_arr[3];
    f5 = flt_arr[4]; f6 = flt_arr[5]; f7 = flt_arr[6]; f8 = flt_arr[7];
    
    ll1 = ll_arr[0]; ll2 = ll_arr[1]; ll3 = ll_arr[2]; ll4 = ll_arr[3];
    ll5 = ll_arr[4]; ll6 = ll_arr[5]; ll7 = ll_arr[6]; ll8 = ll_arr[7];
    
    /* Complex loop with extreme register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % ARRAY_SIZE;
        int idx2 = (i * 11 + v3 * 13 + v4 * 17) % ARRAY_SIZE;
        int idx3 = (i * 19 + v5 * 23 + v6 * 29) % ARRAY_SIZE;
        int idx4 = (i * 31 + v7 * 37 + v8 * 41) % ARRAY_SIZE;
        
        /* Force address computations into registers */
        volatile int* addr1 = &int_arr[idx1];
        volatile int* addr2 = &int_arr[idx2];
        volatile double* addr3 = &dbl_arr[idx3];
        volatile float* addr4 = &flt_arr[idx4];
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Complex addressing in this branch */
                int complex_idx = (idx1 * 3 + idx2 * 5 + idx3 * 7) % ARRAY_SIZE;
                volatile int* complex_addr = &int_arr[complex_idx];
                
                /* Inline assembly with conflicting constraints */
                int temp1, temp2;
                asm volatile (
                    "mov %[val1], %[tmp1]\n\t"
                    "add %[val2], %[tmp1]\n\t"
                    "mov %[tmp1], %[out1]\n\t"
                    : [out1] "=r" (temp1), [out2] "=r" (temp2)
                    : [val1] "r" (v1), [val2] "r" (v2)
                    : "memory"
                );
                
                /* Use computed address */
                v11 = *complex_addr + temp1;
                break;
            }
            case 1: {
                /* Different addressing pattern */
                double* dbl_addr = (double*)((char*)dbl_arr + idx1 * sizeof(double));
                
                /* Inline assembly with memory constraint */
                double dbl_temp;
                asm volatile (
                    "fldl %[mem]\n\t"
                    "fstpl %[out]\n\t"
                    : [out] "=m" (dbl_temp)
                    : [mem] "m" (*dbl_addr)
                    : "memory"
                );
                
                d9 = dbl_temp + d1;
                break;
            }
            case 2: {
                /* Mixed operand types */
                long long offset = (ll1 * i + ll2 * v1) % 256;
                char* byte_ptr = (char*)ll_arr + offset;
                
                /* Force address reload */
                asm volatile (
                    "movq (%[ptr]), %%rax\n\t"
                    "addq %%rax, %[sum]\n\t"
                    : [sum] "+r" (checksum)
                    : [ptr] "r" (byte_ptr)
                    : "rax", "memory"
                );
                break;
            }
            case 3:
            case 4:
            case 5: {
                /* Multiple address computations */
                int* ptr1 = &int_arr[(v1 + v2 * 2) % ARRAY_SIZE];
                int* ptr2 = &int_arr[(v3 + v4 * 3) % ARRAY_SIZE];
                int* ptr3 = &int_arr[(v5 + v6 * 4) % ARRAY_SIZE];
                
                /* Use addresses in function calls */
                void* result = use_address(ptr1, ptr2, ptr3, v7, v8);
                checksum += (uintptr_t)result;
                break;
            }
            case 6: {
                /* Output address reload */
                int output_val;
                int* output_ptr = &int_arr[idx4];
                
                asm volatile (
                    "movl %[val], (%[ptr])\n\t"
                    : 
                    : [val] "r" (v9), [ptr] "r" (output_ptr)
                    : "memory"
                );
                
                output_val = *output_ptr;
                v10 = output_val + v1;
                break;
            }
            case 7: {
                /* Operand address reload */
                int base_idx = (v1 * v2 + v3 * v4) % ARRAY_SIZE;
                volatile int* base_ptr = &int_arr[base_idx];
                
                asm volatile (
                    "movl (%[base], %[index], 4), %[out]\n\t"
                    : [out] "=r" (v12)
                    : [base] "r" (base_ptr), [index] "r" (v5)
                    : "memory"
                );
                break;
            }
        }
        
        /* Call multiple non-inline functions with many arguments */
        v13 = use_int(v1, v2, v3, v4, v5, v6);
        d10 = use_double(d1, d2, d3, d4, d5, d6);
        f9 = use_float(f1, f2, f3, f4, f5, f6, f7, f8, f9, f10);
        ll9 = use_long_long(ll1, ll2, ll3, ll4, ll5, ll6, ll7, ll8);
        
        /* Update most variables to keep them live */
        v1 = v2 + int_arr[(i + 1) % ARRAY_SIZE];
        v2 = v3 + int_arr[(i + 2) % ARRAY_SIZE];
        v3 = v4 + int_arr[(i + 3) % ARRAY_SIZE];
        v4 = v5 + int_arr[(i + 4) % ARRAY_SIZE];
        v5 = v6 + int_arr[(i + 5) % ARRAY_SIZE];
        
        d1 = d2 * dbl_arr[(i + 6) % ARRAY_SIZE];
        d2 = d3 / dbl_arr[(i + 7) % ARRAY_SIZE];
        d3 = d4 + dbl_arr[(i + 8) % ARRAY_SIZE];
        
        f1 = f2 + flt_arr[(i + 9) % ARRAY_SIZE];
        f2 = f3 * flt_arr[(i + 10) % ARRAY_SIZE];
        
        ll1 = ll2 + ll_arr[(i + 11) % ARRAY_SIZE];
        ll2 = ll3 - ll_arr[(i + 12) % ARRAY_SIZE];
        
        /* Complex index update */
        int update_idx = (v1 * 47 + v2 * 53 + i * 59) % ARRAY_SIZE;
        v6 = int_arr[update_idx];
        
        /* Volatile sink to prevent elimination */
        volatile int sink_int = v1 + v2 + v3 + v4 + v5 + v6;
        volatile double sink_dbl = d1 + d2 + d3;
        checksum += sink_int + (unsigned long long)sink_dbl;
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays */
    volatile int* int_arr = malloc(ARRAY_SIZE * sizeof(int));
    volatile double* dbl_arr = malloc(ARRAY_SIZE * sizeof(double));
    volatile long long* ll_arr = malloc(ARRAY_SIZE * sizeof(long long));
    volatile float* flt_arr = malloc(ARRAY_SIZE * sizeof(float));
    
    if (!int_arr || !dbl_arr || !ll_arr || !flt_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = i * 3 + 1;
        dbl_arr[i] = i * 1.5 + 0.5;
        ll_arr[i] = i * 7LL + 3LL;
        flt_arr[i] = i * 0.7f + 0.3f;
    }
    
    /* Call stress function */
    unsigned long long result = stress_reload(int_arr, dbl_arr, ll_arr, flt_arr);
    
    printf("Reload stress test completed. Checksum: %llu\n", result);
    
    /* Cleanup */
    free((void*)int_arr);
    free((void*)dbl_arr);
    free((void*)ll_arr);
    free((void*)flt_arr);
    
    return 0;
}
