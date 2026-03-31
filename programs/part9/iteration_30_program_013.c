/* reload_stress_test.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-move-loop-invariants reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Opaque functions that prevent optimization */
__attribute__((noinline)) int use_int(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

__attribute__((noinline)) double use_double(double a, double b, double c, double d) {
    volatile double sink = a * b + c - d;
    return sink;
}

__attribute__((noinline)) long use_long(long a, long b, long c, long d, long e) {
    volatile long sink = (a ^ b) | (c & d) ^ e;
    return sink;
}

__attribute__((noinline)) void* use_address(void* a, void* b, void* c) {
    volatile void* sink = (void*)((uintptr_t)a + (uintptr_t)b - (uintptr_t)c);
    return sink;
}

__attribute__((noinline)) float use_float(float a, float b, float c, float d, float e, float f, float g) {
    volatile float sink = a * b + c * d - e * f + g;
    return sink;
}

/* Main stress function */
__attribute__((noinline)) int stress_reload(int* arr_int, double* arr_dbl, 
                                          long* arr_long, float* arr_flt) {
    /* Declare many local variables to exhaust registers */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile int* ptr1, *ptr2, *ptr3, *ptr4, *ptr5;
    volatile double* dptr1, *dptr2, *dptr3;
    volatile long* lptr1, *lptr2;
    volatile float* fptr1, *fptr2;
    
    int result = 0;
    
    /* Initialize with complex expressions */
    v1 = arr_int[0] * 3;
    v2 = arr_int[1] + 7;
    v3 = arr_int[2] - 5;
    v4 = arr_int[3] ^ 0x55;
    v5 = arr_int[4] | 0xFF;
    v6 = arr_int[5] & 0x7F;
    v7 = arr_int[6] << 2;
    v8 = arr_int[7] >> 1;
    v9 = arr_int[8] + arr_int[9];
    v10 = arr_int[10] - arr_int[11];
    
    d1 = arr_dbl[0] * 2.5;
    d2 = arr_dbl[1] + 3.14;
    d3 = arr_dbl[2] - 1.618;
    d4 = arr_dbl[3] / 2.0;
    d5 = arr_dbl[4] * arr_dbl[5];
    d6 = arr_dbl[6] + arr_dbl[7];
    d7 = arr_dbl[8] - arr_dbl[9];
    d8 = arr_dbl[10] * 0.5;
    d9 = arr_dbl[11] + 2.718;
    d10 = arr_dbl[12] - 0.577;
    
    l1 = arr_long[0] * 3L;
    l2 = arr_long[1] + 7L;
    l3 = arr_long[2] - 5L;
    l4 = arr_long[3] ^ 0xAAAAAAAA;
    l5 = arr_long[4] | 0x55555555;
    l6 = arr_long[5] & 0xFFFFFFFF;
    l7 = arr_long[6] << 3;
    l8 = arr_long[7] >> 2;
    l9 = arr_long[8] + arr_long[9];
    l10 = arr_long[10] - arr_long[11];
    
    f1 = arr_flt[0] * 1.5f;
    f2 = arr_flt[1] + 2.5f;
    f3 = arr_flt[2] - 1.5f;
    f4 = arr_flt[3] / 3.0f;
    f5 = arr_flt[4] * arr_flt[5];
    f6 = arr_flt[6] + arr_flt[7];
    f7 = arr_flt[8] - arr_flt[9];
    f8 = arr_flt[10] * 0.25f;
    f9 = arr_flt[11] + 1.333f;
    f10 = arr_flt[12] - 0.666f;
    
    /* Take addresses of locals to force stack addressing */
    ptr1 = &v1; ptr2 = &v2; ptr3 = &v3; ptr4 = &v4; ptr5 = &v5;
    dptr1 = &d1; dptr2 = &d2; dptr3 = &d3;
    lptr1 = &l1; lptr2 = &l2;
    fptr1 = &f1; fptr2 = &f2;
    
    /* Main loop with extreme register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2) % ARRAY_SIZE;
        int idx2 = (i * 11 + v3 * 5 + v4) % ARRAY_SIZE;
        int idx3 = (i * 13 + v5 * 7 + v6) % ARRAY_SIZE;
        int idx4 = (i * 17 + v7 * 11 + v8) % ARRAY_SIZE;
        int idx5 = (i * 19 + v9 * 13 + v10) % ARRAY_SIZE;
        
        double idx_d1 = (i * 3.14159 + d1 * 2.71828) % ARRAY_SIZE;
        double idx_d2 = (i * 1.61803 + d2 * 0.57721) % ARRAY_SIZE;
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2, temp3;
        double dtemp1, dtemp2;
        void* addr1, *addr2;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "mov %[val1], %[res1]\n\t"
            "add %[val2], %[res1]\n\t"
            "mov %[addr1], %[res2]\n\t"
            "mov (%[res2]), %[res3]"
            : [res1] "=r" (temp1), [res2] "=r" (addr1), [res3] "=r" (temp2)
            : [val1] "r" (arr_int[idx1]), [val2] "r" (arr_int[idx2]),
              [addr1] "r" (&arr_int[idx3])
            : "memory"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS */
        asm volatile (
            "lea (%[base], %[index], 4), %[out_addr]\n\t"
            "mov %[value], (%[out_addr])"
            : [out_addr] "=r" (addr2)
            : [base] "r" (arr_int), [index] "r" (idx4),
              [value] "r" (temp1)
            : "memory"
        );
        
        /* Mixed types with double */
        asm volatile (
            "movsd %[dval1], %[dres1]\n\t"
            "addsd %[dval2], %[dres1]\n\t"
            "mov %[daddr], %[dres2]"
            : [dres1] "=x" (dtemp1), [dres2] "=r" (addr1)
            : [dval1] "xm" (arr_dbl[(int)idx_d1]), 
              [dval2] "xm" (arr_dbl[(int)idx_d2]),
              [daddr] "r" (&arr_dbl[idx1])
            : "memory"
        );
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Address computation in one branch */
                int* complex_addr = &arr_int[(i * v1 + v2 * v3) % ARRAY_SIZE];
                /* Use in another context - forces address to be live across */
                temp3 = *complex_addr + v4;
                result += use_int(temp3, v5, v6, v7, v8, v9);
                break;
            }
            case 1: {
                /* Different address pattern */
                double* daddr = &arr_dbl[(i * 3 + v2 * 7) % ARRAY_SIZE];
                dtemp2 = *daddr * d1;
                result += (int)use_double(dtemp2, d2, d3, d4);
                break;
            }
            case 2: {
                /* RELOAD_FOR_OPERAND_ADDRESS pattern */
                long* laddr = &arr_long[(i * 5 + v3 * 11) % ARRAY_SIZE];
                long ltemp = *laddr ^ l1;
                result += (int)use_long(ltemp, l2, l3, l4, l5);
                break;
            }
            case 3: {
                /* RELOAD_FOR_OTHER_ADDRESS pattern */
                float* faddr = &arr_flt[(i * 7 + v4 * 13) % ARRAY_SIZE];
                float ftemp = *faddr * f1;
                result += (int)use_float(ftemp, f2, f3, f4, f5, f6, f7);
                break;
            }
            case 4: {
                /* Multiple address computations */
                void* addr3 = use_address(&arr_int[idx1], &arr_int[idx2], &arr_int[idx3]);
                void* addr4 = use_address(&arr_long[idx2], &arr_long[idx3], &arr_long[idx4]);
                /* Force both addresses to be used */
                temp3 = *(int*)addr3 + *(int*)addr4;
                result += temp3;
                break;
            }
            case 5: {
                /* Complex expression with many operands */
                temp3 = arr_int[idx1] * v1 + arr_int[idx2] * v2 - 
                        arr_int[idx3] * v3 + arr_int[idx4] * v4;
                result += use_int(temp3, v6, v7, v8, v9, v10);
                break;
            }
            case 6: {
                /* Mixed addressing modes */
                dtemp2 = arr_dbl[idx1] * d1 + arr_dbl[idx2] * d2 -
                         arr_dbl[idx3] * d3 + arr_dbl[idx4] * d4;
                result += (int)use_double(dtemp2, d5, d6, d7);
                break;
            }
            case 7: {
                /* All types together */
                temp3 = arr_int[idx5];
                dtemp2 = arr_dbl[idx1];
                long ltemp = arr_long[idx2];
                float ftemp = arr_flt[idx3];
                
                result += use_int(temp3, (int)dtemp2, (int)ltemp, (int)ftemp, v1, v2);
                result += (int)use_double(dtemp2, (double)ltemp, (double)temp3, (double)ftemp);
                break;
            }
        }
        
        /* Update most variables to keep them live */
        v1 = v1 * 3 + arr_int[idx1];
        v2 = v2 / 2 + arr_int[idx2];
        v3 = v3 ^ arr_int[idx3];
        v4 = v4 | arr_int[idx4];
        v5 = v5 & arr_int[idx5];
        v6 = v6 + i;
        v7 = v7 - arr_int[(i * 23) % ARRAY_SIZE];
        v8 = v8 * 2;
        v9 = v9 ^ 0xAA;
        v10 = v10 | 0x55;
        
        d1 = d1 * 1.01 + arr_dbl[idx1];
        d2 = d2 / 1.01 - arr_dbl[idx2];
        d3 = d3 + arr_dbl[idx3] * 0.5;
        d4 = d4 - arr_dbl[idx4] * 0.25;
        d5 = d5 * arr_dbl[(i * 29) % ARRAY_SIZE];
        d6 = d6 + i * 0.1;
        d7 = d7 - i * 0.01;
        d8 = d8 * 0.99;
        d9 = d9 + 0.001;
        d10 = d10 - 0.0001;
        
        l1 = l1 << 1;
        l2 = l2 >> 1;
        l3 = l3 ^ arr_long[idx1];
        l4 = l4 | arr_long[idx2];
        l5 = l5 & arr_long[idx3];
        l6 = l6 + i * 1000L;
        l7 = l7 - arr_long[idx4];
        l8 = l8 * 3L;
        l9 = l9 ^ 0xCCCCCCCC;
        l10 = l10 | 0x33333333;
        
        /* Force address register pressure */
        ptr1 = &arr_int[(i * 31 + v1) % ARRAY_SIZE];
        ptr2 = &arr_int[(i * 37 + v2) % ARRAY_SIZE];
        dptr1 = &arr_dbl[(i * 41 + v3) % ARRAY_SIZE];
        dptr2 = &arr_dbl[(i * 43 + v4) % ARRAY_SIZE];
        lptr1 = &arr_long[(i * 47 + v5) % ARRAY_SIZE];
        fptr1 = &arr_flt[(i * 53 + v6) % ARRAY_SIZE];
        
        /* Use the pointers to prevent optimization */
        if (ptr1 != ptr2) {
            temp3 = *ptr1 + *ptr2;
            result += temp3;
        }
    }
    
    /* Final mixing of all values */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10);
    result += (int)(l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10);
    result += (int)(f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10);
    
    return result;
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
        arr_long[i] = i * 1000L - 5000L;
        arr_flt[i] = i * 0.75f - 1.5f;
    }
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_long, arr_flt);
    
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_long);
    free(arr_flt);
    
    return 0;
}
