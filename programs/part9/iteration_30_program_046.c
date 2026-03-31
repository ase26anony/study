/* reload1_stress_test.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-move-loop-invariants reload1_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Opaque functions to prevent optimization */
__attribute__((noinline)) int helper1(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

__attribute__((noinline)) double helper2(double a, double b, double c, double* addr) {
    volatile double sink = a * b + c + *addr;
    return sink;
}

__attribute__((noinline)) long helper3(long a, long b, long* addr1, long* addr2) {
    volatile long sink = a ^ b ^ *addr1 ^ *addr2;
    return sink;
}

__attribute__((noinline)) float helper4(float a, float b, float c, float d, float e, float f, float g, float h) {
    volatile float sink = a + b - c + d - e + f - g + h;
    return sink;
}

__attribute__((noinline)) void* helper5(void* p1, void* p2, void* p3, int offset) {
    volatile char* sink = (char*)p1 + (char*)p2 - (char*)p3 + offset;
    return (void*)sink;
}

/* Main stress function */
__attribute__((noinline)) int stress_reload(int* arr_int, double* arr_dbl, long* arr_long, float* arr_flt) {
    /* Declare many local variables to exhaust registers */
    volatile int v1 = arr_int[0];
    volatile int v2 = arr_int[1];
    volatile int v3 = arr_int[2];
    volatile int v4 = arr_int[3];
    volatile int v5 = arr_int[4];
    volatile int v6 = arr_int[5];
    volatile int v7 = arr_int[6];
    volatile int v8 = arr_int[7];
    volatile int v9 = arr_int[8];
    volatile int v10 = arr_int[9];
    
    volatile double d1 = arr_dbl[0];
    volatile double d2 = arr_dbl[1];
    volatile double d3 = arr_dbl[2];
    volatile double d4 = arr_dbl[3];
    volatile double d5 = arr_dbl[4];
    volatile double d6 = arr_dbl[5];
    volatile double d7 = arr_dbl[6];
    volatile double d8 = arr_dbl[7];
    volatile double d9 = arr_dbl[8];
    volatile double d10 = arr_dbl[9];
    
    volatile long l1 = arr_long[0];
    volatile long l2 = arr_long[1];
    volatile long l3 = arr_long[2];
    volatile long l4 = arr_long[3];
    volatile long l5 = arr_long[4];
    volatile long l6 = arr_long[5];
    volatile long l7 = arr_long[6];
    volatile long l8 = arr_long[7];
    volatile long l9 = arr_long[8];
    volatile long l10 = arr_long[9];
    
    volatile float f1 = arr_flt[0];
    volatile float f2 = arr_flt[1];
    volatile float f3 = arr_flt[2];
    volatile float f4 = arr_flt[3];
    volatile float f5 = arr_flt[4];
    volatile float f6 = arr_flt[5];
    volatile float f7 = arr_flt[6];
    volatile float f8 = arr_flt[7];
    volatile float f9 = arr_flt[8];
    volatile float f10 = arr_flt[9];
    
    /* Additional variables for address computations */
    volatile int idx1, idx2, idx3, idx4;
    volatile double* dbl_ptr;
    volatile long* long_ptr;
    volatile float* flt_ptr;
    volatile int* int_ptr;
    
    volatile int result = 0;
    
    /* Complex loop with register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        idx1 = (i * 7 + v1 * 3 + v2) % ARRAY_SIZE;
        idx2 = (i * 11 + v3 * 5 + v4 * 2) % ARRAY_SIZE;
        idx3 = (i * 13 + v5 * 7 + v6 * 3) % ARRAY_SIZE;
        idx4 = (i * 17 + v7 * 11 + v8 * 5) % ARRAY_SIZE;
        
        /* Multiple address computations */
        int_ptr = &arr_int[(idx1 * 3 + idx2 * 7 + idx3) % ARRAY_SIZE];
        dbl_ptr = &arr_dbl[(idx2 * 5 + idx3 * 11 + idx4) % ARRAY_SIZE];
        long_ptr = &arr_long[(idx3 * 7 + idx4 * 13 + idx1) % ARRAY_SIZE];
        flt_ptr = &arr_flt[(idx4 * 11 + idx1 * 17 + idx2) % ARRAY_SIZE];
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2;
        asm volatile (
            "mov %[val1], %[tmp1]\n\t"
            "add %[val2], %[tmp1]\n\t"
            "mov %[tmp1], %[addr]\n\t"
            : [tmp1] "=&r" (temp1), [addr] "=m" (*int_ptr)
            : [val1] "r" (v1), [val2] "r" (v2)
            : "memory"
        );
        
        double temp_dbl;
        asm volatile (
            "movq %[dbl1], %%xmm0\n\t"
            "addsd %[dbl2], %%xmm0\n\t"
            "movq %%xmm0, %[out]\n\t"
            : [out] "=m" (*dbl_ptr)
            : [dbl1] "r" (d1), [dbl2] "r" (d2)
            : "xmm0", "memory"
        );
        
        /* Switch statement to create complex control flow */
        switch (i % 8) {
            case 0:
                /* Use computed addresses in one branch */
                result += helper1(v1, v2, v3, *int_ptr, v4, v5);
                result += (int)helper2(d1, d2, d3, dbl_ptr);
                break;
            case 1:
                /* Different address computations */
                long_ptr = &arr_long[(i * 19 + v9 * 13) % ARRAY_SIZE];
                result += helper3(l1, l2, long_ptr, &arr_long[idx1]);
                break;
            case 2:
                /* More address variations */
                flt_ptr = &arr_flt[(i * 23 + v10 * 17) % ARRAY_SIZE];
                result += (int)helper4(f1, f2, f3, f4, f5, f6, f7, f8);
                break;
            case 3:
                /* Pointer arithmetic forcing address reloads */
                void* ptr1 = (void*)&arr_int[idx1];
                void* ptr2 = (void*)&arr_int[idx2];
                void* ptr3 = (void*)&arr_int[idx3];
                helper5(ptr1, ptr2, ptr3, v6);
                break;
            case 4:
                /* Mixed operations */
                asm volatile (
                    "mov %[val], (%[addr])\n\t"
                    : 
                    : [val] "r" (v7), [addr] "r" (int_ptr)
                    : "memory"
                );
                break;
            case 5:
                /* Another addressing mode */
                result += arr_int[(v1 * v2 + v3 * v4) % ARRAY_SIZE];
                break;
            case 6:
                /* Complex expression with multiple address uses */
                result += arr_int[(i * 29 + result * 3) % ARRAY_SIZE] +
                         arr_long[(i * 31 + result * 5) % ARRAY_SIZE];
                break;
            case 7:
                /* All variables used together */
                result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
                result += (int)(d1 + d2 + d3 + d4 + d5);
                result += (int)(f1 + f2 + f3 + f4 + f5);
                break;
        }
        
        /* Update most variables to keep them live */
        v1 = v1 * 3 + arr_int[idx1];
        v2 = v2 * 5 + arr_int[idx2];
        v3 = v3 * 7 + arr_int[idx3];
        v4 = v4 * 11 + arr_int[idx4];
        v5 = v5 * 13 + v1;
        v6 = v6 * 17 + v2;
        v7 = v7 * 19 + v3;
        v8 = v8 * 23 + v4;
        v9 = v9 * 29 + v5;
        v10 = v10 * 31 + v6;
        
        d1 = d1 * 1.1 + arr_dbl[idx1];
        d2 = d2 * 1.2 + arr_dbl[idx2];
        d3 = d3 * 1.3 + arr_dbl[idx3];
        d4 = d4 * 1.4 + arr_dbl[idx4];
        d5 = d5 * 1.5 + d1;
        
        l1 = l1 ^ arr_long[idx1];
        l2 = l2 ^ arr_long[idx2];
        l3 = l3 ^ arr_long[idx3];
        l4 = l4 ^ arr_long[idx4];
        
        f1 = f1 * 1.1f + arr_flt[idx1];
        f2 = f2 * 1.2f + arr_flt[idx2];
        f3 = f3 * 1.3f + arr_flt[idx3];
        f4 = f4 * 1.4f + arr_flt[idx4];
    }
    
    /* Final computation using all variables */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10);
    result += (int)(l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10);
    result += (int)(f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10);
    
    return result;
}

int main() {
    /* Allocate and initialize arrays */
    int* arr_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(ARRAY_SIZE * sizeof(double));
    long* arr_long = (long*)malloc(ARRAY_SIZE * sizeof(long));
    float* arr_flt = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!arr_int || !arr_dbl || !arr_long || !arr_flt) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = i * 3 + 1;
        arr_dbl[i] = i * 1.5 + 0.5;
        arr_long[i] = i * 7L + 3L;
        arr_flt[i] = i * 2.3f + 1.1f;
    }
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_long, arr_flt);
    
    printf("Result: %d\n", result);
    printf("Reload stress test completed.\n");
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_long);
    free(arr_flt);
    
    return 0;
}
