/* reload_stress_test.c
 * Designed to trigger various reload types in reload1.cc
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-move-loop-invariants reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Opaque noinline functions to prevent optimization */
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
    volatile void* sink = (void*)((uintptr_t)a + (uintptr_t)b - (uintptr_t)c);
    return sink;
}

__attribute__((noinline)) float use_float(float a, float b, float c, float d, float e, float f, float g) {
    volatile float sink = a * b + c * d - e * f + g;
    return sink;
}

/* Main stress function */
__attribute__((noinline)) 
unsigned long stress_reload(volatile int* arr_int, volatile double* arr_double, 
                           volatile long* arr_long, volatile float* arr_float) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    
    /* Additional variables for address computations */
    int* ptr1, *ptr2, *ptr3;
    double* dptr1, *dptr2;
    long* lptr1, *lptr2;
    
    /* Initialize with array values to create dependencies */
    v1 = arr_int[0]; v2 = arr_int[1]; v3 = arr_int[2]; v4 = arr_int[3];
    v5 = arr_int[4]; v6 = arr_int[5]; v7 = arr_int[6]; v8 = arr_int[7];
    v9 = arr_int[8]; v10 = arr_int[9];
    
    d1 = arr_double[0]; d2 = arr_double[1]; d3 = arr_double[2];
    d4 = arr_double[3]; d5 = arr_double[4];
    
    l1 = arr_long[0]; l2 = arr_long[1]; l3 = arr_long[2]; l4 = arr_long[3];
    
    f1 = arr_float[0]; f2 = arr_float[1]; f3 = arr_float[2]; f4 = arr_float[3];
    
    /* Take addresses of locals to force stack-based reloads */
    ptr1 = &v1; ptr2 = &v2; ptr3 = &v3;
    dptr1 = &d1; dptr2 = &d2;
    lptr1 = &l1; lptr2 = &l2;
    
    unsigned long checksum = 0;
    
    /* Main loop with extreme register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2) % ARRAY_SIZE;
        int idx2 = (i * 11 + v3 * 5 + v4 * 2) % ARRAY_SIZE;
        int idx3 = (i * 13 + v5 * 7 + v6 * 3) % ARRAY_SIZE;
        int idx4 = (i * 17 + v7 * 11 + v8 * 5) % ARRAY_SIZE;
        int idx5 = (i * 19 + v9 * 13 + v10 * 7) % ARRAY_SIZE;
        
        /* More complex indices for different arrays */
        int idx_d1 = (idx1 * 3 + idx2 * 5 + i) % ARRAY_SIZE;
        int idx_d2 = (idx3 * 7 + idx4 * 11 + i * 2) % ARRAY_SIZE;
        int idx_l1 = (idx1 + idx2 * 2 + idx3 * 3) % ARRAY_SIZE;
        int idx_f1 = (idx4 * 5 + idx5 * 7 + i * 3) % ARRAY_SIZE;
        
        /* Use inline assembly with conflicting constraints to force reloads */
        int temp1, temp2, temp3;
        double dtemp1, dtemp2;
        long ltemp1, ltemp2;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "mov %[val1], %[res1]\n\t"
            "add %[val2], %[res1]\n\t"
            "mov %[addr1], %[res2]\n\t"
            "mov (%[addr2]), %[res3]"
            : [res1] "=r" (temp1), [res2] "=r" (temp2), [res3] "=r" (temp3)
            : [val1] "r" (arr_int[idx1]), 
              [val2] "r" (arr_int[idx2]),
              [addr1] "r" (&arr_int[idx3]),
              [addr2] "r" (&arr_int[idx4])
            : "memory"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS */
        volatile int output_var;
        asm volatile (
            "lea (%[base], %[index], 4), %[out]\n\t"
            "movl $42, (%[out])"
            : [out] "=m" (output_var)
            : [base] "r" (arr_int), [index] "r" (idx1)
            : "memory"
        );
        
        /* Control flow to split live ranges */
        switch (i % 8) {
            case 0: {
                /* Block 0: Complex address computation */
                int* complex_addr = &arr_int[(idx1 * v1 + idx2 * v2) % ARRAY_SIZE];
                volatile int* volatile_addr = complex_addr;
                
                /* Use in another computation */
                int val_at_addr = *volatile_addr;
                v1 = val_at_addr + v3;
                
                /* Force RELOAD_FOR_OPERAND_ADDRESS */
                asm volatile (
                    "mov (%[addr]), %%eax\n\t"
                    "add %%eax, %[out]"
                    : [out] "+r" (v2)
                    : [addr] "r" (complex_addr)
                    : "%eax", "memory"
                );
                break;
            }
            case 1: {
                /* Block 1: Different address pattern */
                double* daddr = &arr_double[(idx_d1 * 3 + i) % ARRAY_SIZE];
                d1 = *daddr + d2;
                
                /* RELOAD_FOR_INPADDR_ADDRESS */
                asm volatile (
                    "movsd (%[addr]), %%xmm0\n\t"
                    "addsd %%xmm0, %%xmm1"
                    : 
                    : [addr] "r" (daddr)
                    : "%xmm0", "%xmm1", "memory"
                );
                break;
            }
            case 2: {
                /* Block 2: More address computations */
                long* laddr = &arr_long[(idx_l1 + v4) % ARRAY_SIZE];
                l1 = *laddr ^ l2;
                
                /* RELOAD_FOR_OUTADDR_ADDRESS */
                volatile long* out_addr = laddr;
                asm volatile (
                    "mov %[val], (%[addr])"
                    : 
                    : [val] "r" (l3), [addr] "r" (out_addr)
                    : "memory"
                );
                break;
            }
            case 3: {
                /* Block 3: Mixed types */
                float* faddr = &arr_float[(idx_f1 * 2 + v5) % ARRAY_SIZE];
                f1 = *faddr * f2;
                
                /* RELOAD_FOR_OPADDR_ADDR */
                asm volatile (
                    "movss (%[addr]), %%xmm2\n\t"
                    "mulss %%xmm2, %%xmm3"
                    : 
                    : [addr] "r" (faddr)
                    : "%xmm2", "%xmm3", "memory"
                );
                break;
            }
            case 4: {
                /* Block 4: Pointer arithmetic */
                int* addr1 = &arr_int[idx1];
                int* addr2 = &arr_int[idx2];
                int* addr3 = &arr_int[idx3];
                
                /* RELOAD_FOR_OTHER_ADDRESS */
                void* result = use_address(addr1, addr2, addr3);
                checksum += (uintptr_t)result;
                break;
            }
            default: {
                /* Default: Use all variables to keep them live */
                v1 = arr_int[idx1] + v2;
                v3 = arr_int[idx2] - v4;
                v5 = arr_int[idx3] * v6;
                v7 = arr_int[idx4] / (v8 ? v8 : 1);
                v9 = arr_int[idx5] ^ v10;
                break;
            }
        }
        
        /* Call noinline functions with many arguments to force register shuffling */
        v11 = use_int(v1, v2, v3, v4, v5, v6);
        d6 = use_double(d1, d2, d3, d4);
        l5 = use_long(l1, l2, l3, l4, l5);
        f5 = use_float(f1, f2, f3, f4, f5, f6, f7);
        
        /* Update most variables to keep them live across iterations */
        v1 += arr_int[(i + 1) % ARRAY_SIZE];
        v2 -= arr_int[(i + 2) % ARRAY_SIZE];
        v3 ^= arr_int[(i + 3) % ARRAY_SIZE];
        v4 |= arr_int[(i + 4) % ARRAY_SIZE];
        v5 &= arr_int[(i + 5) % ARRAY_SIZE];
        
        d1 += arr_double[(i + 6) % ARRAY_SIZE];
        d2 *= arr_double[(i + 7) % ARRAY_SIZE];
        
        l1 ^= arr_long[(i + 8) % ARRAY_SIZE];
        l2 += arr_long[(i + 9) % ARRAY_SIZE];
        
        f1 *= arr_float[(i + 10) % ARRAY_SIZE];
        f2 -= arr_float[(i + 11) % ARRAY_SIZE];
        
        /* Update checksum */
        checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        checksum += (unsigned long)(d1 + d2 + d3 + d4 + d5);
        checksum += l1 + l2 + l3 + l4 + l5;
        checksum += (unsigned long)(f1 + f2 + f3 + f4 + f5);
        
        /* Use computed goto for non-trivial control flow (occasionally) */
        if ((i % 64) == 0) {
            static void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
            goto *labels[i % 4];
            
        label1:
            v1 = arr_int[(v1 + i) % ARRAY_SIZE];
            goto after_labels;
        label2:
            v2 = arr_int[(v2 + i) % ARRAY_SIZE];
            goto after_labels;
        label3:
            v3 = arr_int[(v3 + i) % ARRAY_SIZE];
            goto after_labels;
        label4:
            v4 = arr_int[(v4 + i) % ARRAY_SIZE];
            goto after_labels;
        after_labels:
            /* Continue */
        }
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays with pattern data */
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
        arr_int[i] = i * 3 + 1;
        arr_double[i] = i * 1.5 + 0.5;
        arr_long[i] = i * 7L + 3L;
        arr_float[i] = i * 0.7f + 0.3f;
    }
    
    printf("Starting reload stress test...\n");
    
    /* Call the stress function */
    unsigned long result = stress_reload(arr_int, arr_double, arr_long, arr_float);
    
    printf("Checksum result: %lu\n", result);
    
    /* Cleanup */
    free((void*)arr_int);
    free((void*)arr_double);
    free((void*)arr_long);
    free((void*)arr_float);
    
    return 0;
}
