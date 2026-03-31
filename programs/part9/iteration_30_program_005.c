/* reload1_stress_test.c
 * Designed to trigger multiple reload types in GCC's reload pass:
 * - RELOAD_OTHER
 * - RELOAD_FOR_INPUT
 * - RELOAD_FOR_INPUT_ADDRESS
 * - RELOAD_FOR_INPADDR_ADDRESS
 * - RELOAD_FOR_OUTPUT_ADDRESS
 * - RELOAD_FOR_OUTADDR_ADDRESS
 * - RELOAD_FOR_OPERAND_ADDRESS
 * - RELOAD_FOR_OPADDR_ADDR
 * - RELOAD_FOR_OTHER_ADDRESS
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Opaque functions to prevent optimization */
__attribute__((noinline)) int use_int(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

__attribute__((noinline)) double use_double(double a, double b, double c, double d) {
    volatile double sink = a * b + c - d;
    return sink;
}

__attribute__((noinline)) long use_long(long a, long b, long c, long d, long e) {
    volatile long sink = (a ^ b) | (c & d) + e;
    return sink;
}

__attribute__((noinline)) void* use_address(void* a, void* b, void* c) {
    volatile void* sink = (void*)((long)a ^ (long)b ^ (long)c);
    return sink;
}

__attribute__((noinline)) float use_float(float a, float b, float c, float d, float e, float f, float g) {
    volatile float sink = a + b * c - d / e + f - g;
    return sink;
}

/* Main stress function with extreme register pressure */
__attribute__((noinline)) unsigned long stress_reload(int* arr_int, double* arr_dbl, 
                                                     long* arr_long, float* arr_flt) {
    /* Declare many local variables to exhaust registers */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile int* p1, *p2, *p3, *p4, *p5;
    volatile double* dp1, *dp2, *dp3;
    volatile long* lp1, *lp2, *lp3;
    
    /* Initialize with complex expressions */
    v1 = arr_int[0] * 3 + 1;
    v2 = arr_int[1] * 7 - 2;
    v3 = arr_int[2] / 2 + 3;
    v4 = arr_int[3] * 5 - 4;
    v5 = arr_int[4] + arr_int[5] - 1;
    v6 = arr_int[6] * 2 + arr_int[7];
    v7 = arr_int[8] - arr_int[9] * 3;
    v8 = arr_int[10] + arr_int[11] / 2;
    v9 = arr_int[12] * 4 - arr_int[13];
    v10 = arr_int[14] + arr_int[15] * 2;
    
    d1 = arr_dbl[0] * 1.5;
    d2 = arr_dbl[1] / 2.0 + 3.14;
    d3 = arr_dbl[2] * 2.71 - 1.0;
    d4 = arr_dbl[3] + arr_dbl[4] * 0.5;
    d5 = arr_dbl[5] - arr_dbl[6] / 3.0;
    d6 = arr_dbl[7] * 4.2 + arr_dbl[8];
    d7 = arr_dbl[9] - arr_dbl[10] * 1.1;
    d8 = arr_dbl[11] + arr_dbl[12] / 5.0;
    d9 = arr_dbl[13] * 3.3 - arr_dbl[14];
    d10 = arr_dbl[15] + arr_dbl[16] * 2.2;
    
    l1 = arr_long[0] ^ 0x12345678;
    l2 = arr_long[1] | 0x87654321;
    l3 = arr_long[2] & 0xF0F0F0F0;
    l4 = arr_long[3] + arr_long[4] * 2;
    l5 = arr_long[5] - arr_long[6] / 3;
    l6 = arr_long[7] | arr_long[8];
    l7 = arr_long[9] & arr_long[10];
    l8 = arr_long[11] ^ arr_long[12];
    l9 = arr_long[13] + arr_long[14] - 1;
    l10 = arr_long[15] * 3 + arr_long[16];
    
    f1 = arr_flt[0] * 1.1f;
    f2 = arr_flt[1] / 2.2f + 3.3f;
    f3 = arr_flt[2] * 4.4f - 5.5f;
    f4 = arr_flt[3] + arr_flt[4] * 0.5f;
    f5 = arr_flt[5] - arr_flt[6] / 3.0f;
    f6 = arr_flt[7] * 2.2f + arr_flt[8];
    f7 = arr_flt[9] - arr_flt[10] * 1.5f;
    f8 = arr_flt[11] + arr_flt[12] / 4.0f;
    f9 = arr_flt[13] * 3.3f - arr_flt[14];
    f10 = arr_flt[15] + arr_flt[16] * 2.5f;
    
    /* Take addresses of locals to force stack-based reloads */
    p1 = &v1; p2 = &v2; p3 = &v3; p4 = &v4; p5 = &v5;
    dp1 = &d1; dp2 = &d2; dp3 = &d3;
    lp1 = &l1; lp2 = &l2; lp3 = &l3;
    
    unsigned long checksum = 0;
    
    /* Complex loop with extreme register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2) % ARRAY_SIZE;
        int idx2 = (i * 11 + v3 * 5 + v4 * 2) % ARRAY_SIZE;
        int idx3 = (i * 13 + v5 * 7 + v6 * 3) % ARRAY_SIZE;
        int idx4 = (i * 17 + v7 * 11 + v8 * 5) % ARRAY_SIZE;
        int idx5 = (i * 19 + v9 * 13 + v10 * 7) % ARRAY_SIZE;
        
        double d_idx1 = (i * 3.14159 + d1 * 2.71) % ARRAY_SIZE;
        double d_idx2 = (i * 2.71828 + d2 * 3.14) % ARRAY_SIZE;
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2, temp3;
        double dtemp1, dtemp2;
        long ltemp1, ltemp2;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "mov %[val1], %[res1]\n\t"
            "add %[val2], %[res1]\n\t"
            "mov %[addr1], %%rax\n\t"
            "add (%%rax), %[res1]"
            : [res1] "=r" (temp1), "=r" (temp2)
            : [val1] "r" (arr_int[idx1]), 
              [val2] "r" (arr_int[idx2]),
              [addr1] "r" (&arr_int[idx3])
            : "rax", "memory"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        asm volatile (
            "lea (%[base], %[index], 4), %[out]\n\t"
            "mov %[val], (%[out])"
            : [out] "=r" (temp3)
            : [base] "r" (arr_int), 
              [index] "r" (idx4),
              [val] "r" (temp1)
            : "memory"
        );
        
        /* Mixed type assembly with double */
        asm volatile (
            "movsd %[dval1], %%xmm0\n\t"
            "addsd %[dval2], %%xmm0\n\t"
            "movsd %%xmm0, %[dres]"
            : [dres] "=m" (dtemp1)
            : [dval1] "m" (arr_dbl[(int)d_idx1]),
              [dval2] "m" (arr_dbl[(int)d_idx2])
            : "xmm0"
        );
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Address computation in one branch */
                int* addr = &arr_int[(i + v1 * 2 + v3) % ARRAY_SIZE];
                /* Use in another context - forces RELOAD_FOR_OTHER_ADDRESS */
                temp2 = *addr + v2;
                use_int(temp2, v4, v6, v8, v10, idx1);
                break;
            }
            case 1: {
                /* Complex addressing with multiple computations */
                long complex_idx = (l1 * i + l2 * 3 + l3 * 7) % ARRAY_SIZE;
                double* daddr = &arr_dbl[complex_idx % (ARRAY_SIZE/2)];
                dtemp2 = *daddr * d1 + d3 - d5;
                use_double(dtemp2, d2, d4, d6);
                break;
            }
            case 2: {
                /* Pointer arithmetic forcing operand address reloads */
                int offset = (v5 * 3 + v7 * 5) % 100;
                int* ptr = arr_int + offset + idx2;
                /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
                asm volatile (
                    "mov (%[ptr]), %[res]\n\t"
                    "imul %[mul], %[res]"
                    : [res] "=r" (temp3)
                    : [ptr] "r" (ptr),
                      [mul] "r" (v9)
                    : "memory"
                );
                break;
            }
            case 3: {
                /* Multiple address computations in sequence */
                float* fptr1 = &arr_flt[(i * 5 + v2) % ARRAY_SIZE];
                float* fptr2 = &arr_flt[(i * 7 + v4) % ARRAY_SIZE];
                float* fptr3 = &arr_flt[(i * 11 + v6) % ARRAY_SIZE];
                use_float(*fptr1, *fptr2, *fptr3, f1, f3, f5, f7);
                break;
            }
            case 4: {
                /* Long computations with address taking */
                long* lptr = &arr_long[(l4 * i + l6) % ARRAY_SIZE];
                ltemp1 = *lptr ^ l2 | l8;
                use_long(ltemp1, l3, l5, l7, l9);
                break;
            }
            case 5: {
                /* Nested addressing */
                int** pptr = (int**)&p1;
                int val = **pptr + arr_int[(**pptr + i) % ARRAY_SIZE];
                temp1 = use_int(val, v3, v5, v7, v9, idx3);
                break;
            }
            case 6: {
                /* Computed goto to create complex control flow */
                void* labels[] = { &&label1, &&label2, &&label3 };
                goto *labels[i % 3];
                
                label1:
                    temp1 = arr_int[idx1] + arr_int[idx2];
                    goto end_switch;
                label2:
                    temp1 = arr_int[idx3] * arr_int[idx4];
                    goto end_switch;
                label3:
                    temp1 = arr_int[idx5] - arr_int[idx1];
                    goto end_switch;
                end_switch:
                    break;
            }
            case 7: {
                /* Multiple volatile memory accesses */
                volatile int* volatile_ptr = &arr_int[idx1];
                volatile double* volatile_dptr = &arr_dbl[idx2 % (ARRAY_SIZE/2)];
                temp1 = *volatile_ptr;
                dtemp1 = *volatile_dptr;
                use_address(volatile_ptr, volatile_dptr, &temp1);
                break;
            }
        }
        
        /* Update most variables to keep them live */
        v1 = v1 + arr_int[idx1] - i;
        v2 = v2 * 3 + arr_int[idx2];
        v3 = v3 / 2 + arr_int[idx3];
        v4 = v4 - arr_int[idx4] + i;
        v5 = v5 ^ arr_int[idx5];
        
        d1 = d1 + arr_dbl[(int)d_idx1] * 0.5;
        d2 = d2 - arr_dbl[(int)d_idx2] / 3.0;
        d3 = d3 * 1.1 + arr_dbl[idx1 % (ARRAY_SIZE/2)];
        
        l1 = l1 | arr_long[idx2 % (ARRAY_SIZE/2)];
        l2 = l2 ^ arr_long[idx3 % (ARRAY_SIZE/2)];
        l3 = l3 + arr_long[idx4 % (ARRAY_SIZE/2)] * 2;
        
        f1 = f1 + arr_flt[idx1 % ARRAY_SIZE] * 1.5f;
        f2 = f2 - arr_flt[idx2 % ARRAY_SIZE] / 2.0f;
        f3 = f3 * 2.0f + arr_flt[idx3 % ARRAY_SIZE];
        
        /* Accumulate checksum */
        checksum += temp1 + temp2 + temp3 + (unsigned long)dtemp1 + 
                   (unsigned long)dtemp2 + ltemp1 + ltemp2 +
                   (unsigned long)v1 + (unsigned long)v2 + 
                   (unsigned long)d1 + (unsigned long)l1;
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays with pattern data */
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
        arr_int[i] = i * 3 - 7;
        arr_dbl[i] = i * 1.5 - 3.14;
        arr_long[i] = i * 5L - 11L;
        arr_flt[i] = i * 2.2f - 5.5f;
    }
    
    printf("Starting reload stress test...\n");
    
    /* Call the stress function */
    unsigned long result = stress_reload(arr_int, arr_dbl, arr_long, arr_flt);
    
    printf("Checksum result: %lu\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_long);
    free(arr_flt);
    
    return 0;
}
