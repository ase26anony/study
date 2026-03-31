/* reload_stress_test.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-move-loop-invariants reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Opaque functions that compiler cannot analyze */
__attribute__((noinline)) int use_int(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

__attribute__((noinline)) double use_double(double a, double b, double c, double d) {
    volatile double sink = a * b - c / d;
    return sink;
}

__attribute__((noinline)) long use_long(long a, long b, long c, long d, long e) {
    volatile long sink = (a ^ b) | (c & d) + e;
    return sink;
}

__attribute__((noinline)) void* use_address(void* a, void* b, int offset) {
    volatile char* sink = (char*)a + (intptr_t)b + offset;
    return (void*)sink;
}

__attribute__((noinline)) float use_float(float a, float b, float c, float d, float e, float f, float g) {
    volatile float sink = a + b * c - d / e + f - g;
    return sink;
}

/* Main stress function */
__attribute__((noinline)) 
int stress_reload(volatile int* arr_int, volatile double* arr_double, 
                  volatile long* arr_long, volatile float* arr_float) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    
    /* Initialize with values from arrays */
    v1 = arr_int[0]; v2 = arr_int[1]; v3 = arr_int[2]; v4 = arr_int[3];
    v5 = arr_int[4]; v6 = arr_int[5]; v7 = arr_int[6]; v8 = arr_int[7];
    v9 = arr_int[8]; v10 = arr_int[9]; v11 = arr_int[10]; v12 = arr_int[11];
    v13 = arr_int[12]; v14 = arr_int[13]; v15 = arr_int[14]; v16 = arr_int[15];
    v17 = arr_int[16]; v18 = arr_int[17]; v19 = arr_int[18]; v20 = arr_int[19];
    v21 = arr_int[20]; v22 = arr_int[21]; v23 = arr_int[22]; v24 = arr_int[23];
    v25 = arr_int[24]; v26 = arr_int[25]; v27 = arr_int[26]; v28 = arr_int[27];
    v29 = arr_int[28]; v30 = arr_int[29];
    
    d1 = arr_double[0]; d2 = arr_double[1]; d3 = arr_double[2];
    d4 = arr_double[3]; d5 = arr_double[4]; d6 = arr_double[5];
    d7 = arr_double[6]; d8 = arr_double[7]; d9 = arr_double[8];
    d10 = arr_double[9];
    
    l1 = arr_long[0]; l2 = arr_long[1]; l3 = arr_long[2]; l4 = arr_long[3];
    l5 = arr_long[4]; l6 = arr_long[5]; l7 = arr_long[6]; l8 = arr_long[7];
    l9 = arr_long[8]; l10 = arr_long[9];
    
    f1 = arr_float[0]; f2 = arr_float[1]; f3 = arr_float[2]; f4 = arr_float[3];
    f5 = arr_float[4]; f6 = arr_float[5]; f7 = arr_float[6]; f8 = arr_float[7];
    f9 = arr_float[8]; f10 = arr_float[9];
    
    volatile int result = 0;
    
    /* Complex loop with multiple addressing modes */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2) % ARRAY_SIZE;
        int idx2 = (i * 11 + v3 * 5 + v4) % ARRAY_SIZE;
        int idx3 = (i * 13 + v5 * 7 + v6) % ARRAY_SIZE;
        int idx4 = (i * 17 + v7 * 11 + v8) % ARRAY_SIZE;
        int idx5 = (i * 19 + v9 * 13 + v10) % ARRAY_SIZE;
        
        /* Take addresses of array elements with complex computations */
        volatile int* addr1 = &arr_int[idx1];
        volatile int* addr2 = &arr_int[idx2];
        volatile double* addr3 = &arr_double[idx3];
        volatile long* addr4 = &arr_long[idx4];
        volatile float* addr5 = &arr_float[idx5];
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2;
        asm volatile (
            "mov %[val1], %[tmp1]\n\t"
            "add %[val2], %[tmp1]\n\t"
            "mov %[tmp1], %[tmp2]\n\t"
            : [tmp1] "=&r" (temp1), [tmp2] "=r" (temp2)
            : [val1] "r" (v1), [val2] "r" (v2)
            : "cc"
        );
        
        /* More inline assembly with memory constraints */
        int temp3;
        asm volatile (
            "mov (%[addr]), %[tmp]\n\t"
            "add %[val], %[tmp]\n\t"
            : [tmp] "=r" (temp3)
            : [addr] "m" (*addr1), [val] "r" (v3)
            : "cc"
        );
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* RELOAD_FOR_INPUT_ADDRESS pattern */
                int complex_idx = (v11 * v12 + v13 * v14) % ARRAY_SIZE;
                volatile int* complex_addr = &arr_int[complex_idx];
                
                /* Use in inline assembly with address constraint */
                int temp4;
                asm volatile (
                    "mov (%[addr]), %[tmp]\n\t"
                    "imul %[val1], %[tmp]\n\t"
                    "add %[val2], %[tmp]\n\t"
                    : [tmp] "=r" (temp4)
                    : [addr] "r" (complex_addr), [val1] "r" (v15), [val2] "r" (v16)
                    : "cc"
                );
                v17 = temp4;
                break;
            }
            case 1: {
                /* RELOAD_FOR_OUTPUT_ADDRESS pattern */
                double* output_addr;
                asm volatile (
                    "lea (%[base], %[index], 8), %[out]\n\t"
                    : [out] "=r" (output_addr)
                    : [base] "r" (arr_double), [index] "r" (idx2)
                    : "cc"
                );
                *output_addr = d1 + d2;
                break;
            }
            case 2: {
                /* RELOAD_FOR_OPERAND_ADDRESS pattern */
                long* op_addr;
                asm volatile (
                    "mov %[base], %[out]\n\t"
                    "add %[offset], %[out]\n\t"
                    : [out] "=r" (op_addr)
                    : [base] "r" (arr_long), [offset] "r" (idx3 * sizeof(long))
                    : "cc"
                );
                l1 = *op_addr + l2;
                break;
            }
            case 3: {
                /* RELOAD_FOR_OTHER_ADDRESS pattern */
                /* Complex addressing across basic blocks */
                int* other_addr = (int*)use_address((void*)arr_int, (void*)(intptr_t)idx4, v18 * 4);
                v19 = *other_addr;
                break;
            }
            case 4: {
                /* Mix of different reload types */
                float* faddr1 = &arr_float[idx1];
                float* faddr2 = &arr_float[idx2];
                
                /* Force address computation into register */
                float temp_float;
                asm volatile (
                    "movss (%[addr1]), %[tmp]\n\t"
                    "addss (%[addr2]), %[tmp]\n\t"
                    : [tmp] "=x" (temp_float)
                    : [addr1] "r" (faddr1), [addr2] "r" (faddr2)
                    : "memory"
                );
                f1 = temp_float;
                break;
            }
            case 5: {
                /* Nested addressing */
                int nested_idx = (idx1 + idx2 * 3) % ARRAY_SIZE;
                volatile int* nested_addr = &arr_int[nested_idx];
                
                /* Use as both data and address operand */
                int data = *nested_addr;
                volatile int* new_addr = &arr_int[data % ARRAY_SIZE];
                v20 = *new_addr;
                break;
            }
            case 6: {
                /* Multiple address computations in sequence */
                int* a1 = &arr_int[(v21 * 2 + v22) % ARRAY_SIZE];
                int* a2 = &arr_int[(v23 * 3 + v24) % ARRAY_SIZE];
                int* a3 = &arr_int[(v25 * 5 + v26) % ARRAY_SIZE];
                
                /* Chain of operations forcing register pressure */
                v27 = *a1 + *a2;
                v28 = *a2 - *a3;
                v29 = *a3 * *a1;
                break;
            }
            case 7: {
                /* Computed goto to create complex control flow */
                static void* labels[] = { &&label1, &&label2, &&label3 };
                goto *labels[i % 3];
                
                label1:
                    v30 = arr_int[idx1] + arr_int[idx2];
                    goto end_case;
                label2:
                    v30 = arr_int[idx3] - arr_int[idx4];
                    goto end_case;
                label3:
                    v30 = arr_int[idx5] * arr_int[idx1];
                    goto end_case;
                end_case:
                    break;
            }
        }
        
        /* Call multiple non-inline functions with many arguments */
        v1 = use_int(v1, v2, v3, v4, v5, v6);
        d1 = use_double(d1, d2, d3, d4);
        l1 = use_long(l1, l2, l3, l4, l5);
        f1 = use_float(f1, f2, f3, f4, f5, f6, f7);
        
        /* Update many variables to keep them live */
        v2 += arr_int[idx1];
        v3 -= arr_int[idx2];
        v4 *= arr_int[idx3];
        v5 ^= arr_int[idx4];
        v6 |= arr_int[idx5];
        
        v7 = (v7 << 3) | (v8 >> 2);
        v8 = (v8 << 5) ^ (v9 >> 1);
        v9 = (v9 << 7) & (v10 >> 3);
        
        d2 += arr_double[idx1 % (ARRAY_SIZE/2)];
        d3 -= arr_double[idx2 % (ARRAY_SIZE/2)];
        d4 *= arr_double[idx3 % (ARRAY_SIZE/2)];
        
        l2 += arr_long[idx1 % (ARRAY_SIZE/2)];
        l3 ^= arr_long[idx2 % (ARRAY_SIZE/2)];
        l4 |= arr_long[idx3 % (ARRAY_SIZE/2)];
        
        f2 += arr_float[idx1 % (ARRAY_SIZE/2)];
        f3 -= arr_float[idx2 % (ARRAY_SIZE/2)];
        f4 *= arr_float[idx3 % (ARRAY_SIZE/2)];
        
        /* Accumulate result to prevent optimization */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                  (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 +
                  (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5 +
                  (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    }
    
    return result;
}

int main() {
    /* Allocate and initialize large arrays */
    volatile int* arr_int = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile double* arr_double = (volatile double*)malloc(ARRAY_SIZE * sizeof(double));
    volatile long* arr_long = (volatile long*)malloc(ARRAY_SIZE * sizeof(long));
    volatile float* arr_float = (volatile float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!arr_int || !arr_double || !arr_long || !arr_float) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = i * 3 + 7;
        arr_double[i] = i * 1.5 + 2.7;
        arr_long[i] = i * 5L + 11L;
        arr_float[i] = i * 0.7f + 1.3f;
    }
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_double, arr_long, arr_float);
    
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free((void*)arr_int);
    free((void*)arr_double);
    free((void*)arr_long);
    free((void*)arr_float);
    
    return 0;
}
