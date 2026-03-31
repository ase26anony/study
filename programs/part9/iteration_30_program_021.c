/* reload_stress.c - Extreme register pressure test for GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Opaque helper functions to prevent optimization */
NOINLINE int helper1(int a, int b, int c, int d, int e, int f);
NOINLINE float helper2(float a, float b, float c, float d, float* addr);
NOINLINE double helper3(double a, double b, double* addr1, double* addr2);
NOINLINE long helper4(long a, long b, long c, long* addr, int offset);
NOINLINE void helper5(int* addr1, double* addr2, long* addr3, float* addr4);

/* Implementation of helpers (prevents them from being inlined) */
NOINLINE int helper1(int a, int b, int c, int d, int e, int f) {
    VOLATILE_VAR int sink = 0;
    sink = a + b - c + d - e + f;
    return sink;
}

NOINLINE float helper2(float a, float b, float c, float d, float* addr) {
    VOLATILE_VAR float sink = 0.0f;
    sink = a * b + c / d + *addr;
    return sink;
}

NOINLINE double helper3(double a, double b, double* addr1, double* addr2) {
    VOLATILE_VAR double sink = 0.0;
    sink = a + b + *addr1 + *addr2;
    return sink;
}

NOINLINE long helper4(long a, long b, long c, long* addr, int offset) {
    VOLATILE_VAR long sink = 0;
    sink = a * b + c + addr[offset];
    return sink;
}

NOINLINE void helper5(int* addr1, double* addr2, long* addr3, float* addr4) {
    VOLATILE_VAR int sink1 = *addr1;
    VOLATILE_VAR double sink2 = *addr2;
    VOLATILE_VAR long sink3 = *addr3;
    VOLATILE_VAR float sink4 = *addr4;
    (void)sink1; (void)sink2; (void)sink3; (void)sink4;
}

/* Main stress function with extreme register pressure */
NOINLINE int stress_reload(int* arr_int, double* arr_dbl, 
                          long* arr_long, float* arr_float, 
                          int size) {
    /* Declare MANY local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    VOLATILE_VAR int result = 0;
    VOLATILE_VAR int* volatile_ptr1 = arr_int;
    VOLATILE_VAR double* volatile_ptr2 = arr_dbl;
    VOLATILE_VAR long* volatile_ptr3 = arr_long;
    VOLATILE_VAR float* volatile_ptr4 = arr_float;
    
    /* Initialize locals with array values */
    v1 = arr_int[0]; v2 = arr_int[1]; v3 = arr_int[2]; v4 = arr_int[3];
    v5 = arr_int[4]; v6 = arr_int[5]; v7 = arr_int[6]; v8 = arr_int[7];
    v9 = arr_int[8]; v10 = arr_int[9]; v11 = arr_int[10]; v12 = arr_int[11];
    v13 = arr_int[12]; v14 = arr_int[13]; v15 = arr_int[14]; v16 = arr_int[15];
    v17 = arr_int[16]; v18 = arr_int[17]; v19 = arr_int[18]; v20 = arr_int[19];
    
    f1 = arr_float[0]; f2 = arr_float[1]; f3 = arr_float[2]; f4 = arr_float[3];
    f5 = arr_float[4]; f6 = arr_float[5]; f7 = arr_float[6]; f8 = arr_float[7];
    f9 = arr_float[8]; f10 = arr_float[9];
    
    d1 = arr_dbl[0]; d2 = arr_dbl[1]; d3 = arr_dbl[2]; d4 = arr_dbl[3];
    d5 = arr_dbl[4]; d6 = arr_dbl[5]; d7 = arr_dbl[6]; d8 = arr_dbl[7];
    d9 = arr_dbl[8]; d10 = arr_dbl[9];
    
    l1 = arr_long[0]; l2 = arr_long[1]; l3 = arr_long[2]; l4 = arr_long[3];
    l5 = arr_long[4]; l6 = arr_long[5]; l7 = arr_long[6]; l8 = arr_long[7];
    l9 = arr_long[8]; l10 = arr_long[9];
    
    /* Complex loop with extreme register pressure */
    for (int i = 0; i < 1000; i++) {
        /* Complex addressing computations - will require address reloads */
        int idx1 = (i * 7 + v1 * 3 + v2) % size;
        int idx2 = (i * 11 + v3 * 5 + v4) % size;
        int idx3 = (i * 13 + v5 * 7 + v6) % size;
        int idx4 = (i * 17 + v7 * 11 + v8) % size;
        int idx5 = (i * 19 + v9 * 13 + v10) % size;
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
                int* addr1 = &arr_int[idx1 * 3 + idx2];
                double* addr2 = &arr_dbl[idx3 * 2 + idx4];
                
                /* Inline assembly with conflicting constraints */
                int temp1, temp2;
                asm volatile (
                    "mov %[val1], %[tmp1]\n\t"
                    "add %[val2], %[tmp1]\n\t"
                    "mov %[tmp1], %[out1]\n\t"
                    : [out1] "=r" (temp1), [tmp1] "=&r" (temp2)
                    : [val1] "r" (v1), [val2] "r" (v2)
                    : "cc"
                );
                
                /* Use computed addresses */
                v11 = *addr1 + temp1;
                d1 = *addr2 + d1;
                
                /* Call with address arguments */
                helper5(addr1, addr2, &arr_long[idx5], &arr_float[idx1]);
                break;
            }
            case 1: {
                /* RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
                long* addr3 = &arr_long[(idx1 + idx2 * 2) % size];
                float* addr4 = &arr_float[(idx3 + idx4 * 3) % size];
                
                /* Complex expression requiring multiple registers */
                int complex_idx = (v11 * v12 + v13 * v14 - v15 * v16) % size;
                if (complex_idx < 0) complex_idx = -complex_idx;
                
                /* Inline assembly with memory constraint */
                int mem_val;
                asm volatile (
                    "movl %[mem], %[out]\n\t"
                    "addl $1, %[out]\n\t"
                    : [out] "=r" (mem_val)
                    : [mem] "m" (arr_int[complex_idx])
                    : "cc"
                );
                
                v12 = mem_val + v17;
                *addr3 = l1 + l2 + l3;
                *addr4 = f1 * f2 - f3;
                
                /* Force address computation to be live across call */
                helper4(l4, l5, l6, addr3, idx2);
                break;
            }
            case 2: {
                /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
                int offset = (v18 * 7 + v19 * 11) % 32;
                int* base_addr = arr_int + offset;
                
                /* Multiple address computations in same expression */
                int* addr5 = base_addr + (v20 * 3) / 2;
                int* addr6 = addr5 + (v1 * 5) % 16;
                
                /* Use both as data and address */
                int val_at_addr5 = *addr5;
                int val_at_addr6 = *addr6;
                
                /* Inline assembly with '+' constraint */
                int sum;
                asm volatile (
                    "add %[a], %[b]\n\t"
                    "mov %[b], %[s]\n\t"
                    : [s] "=r" (sum), [b] "+r" (val_at_addr5)
                    : [a] "r" (val_at_addr6)
                    : "cc"
                );
                
                v13 = sum + v2;
                
                /* Nested addressing */
                helper1(v3, v4, v5, *addr5, *addr6, v6);
                break;
            }
            case 3: {
                /* RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER */
                /* Take address of local variable - forces stack addressing */
                int* local_addr1 = &v14;
                int* local_addr2 = &v15;
                float* local_addr3 = &f4;
                double* local_addr4 = &d5;
                
                /* Complex control flow with phi nodes */
                int branch_sel = i % 3;
                int* selected_addr = NULL;
                
                if (branch_sel == 0) {
                    selected_addr = local_addr1;
                } else if (branch_sel == 1) {
                    selected_addr = local_addr2;
                } else {
                    selected_addr = &v16;
                }
                
                /* Use selected address in another basic block */
                int temp = *selected_addr;
                
                /* Computed goto to create complex control flow */
                static void* labels[] = { &&label1, &&label2, &&label3 };
                goto *labels[i % 3];
                
                label1:
                    v17 = temp + v7;
                    helper2(f5, f6, f7, f8, local_addr3);
                    goto after_labels;
                label2:
                    v18 = temp * v8;
                    helper3(d6, d7, local_addr4, &d8);
                    goto after_labels;
                label3:
                    v19 = temp - v9;
                    /* Mixed addressing modes */
                    asm volatile (
                        "mov %[addr], %%rsi\n\t"
                        "mov (%%rsi), %[out]\n\t"
                        : [out] "=r" (v20)
                        : [addr] "r" (selected_addr)
                        : "rsi", "memory"
                    );
                    goto after_labels;
                    
                after_labels:
                    /* Continue with more operations */
                    break;
            }
            default: {
                /* Mix of all reload types */
                /* Multiple array accesses with complex indices */
                int idx6 = (v10 * 17 + v11 * 13 + v12 * 11) % size;
                int idx7 = (v13 * 19 + v14 * 17 + v15 * 13) % size;
                int idx8 = (v16 * 23 + v17 * 19 + v18 * 17) % size;
                
                /* Chain of address computations */
                int* chain_addr1 = &arr_int[idx6];
                int* chain_addr2 = chain_addr1 + idx7;
                int* chain_addr3 = chain_addr2 + idx8;
                
                /* Use all local variables to keep them live */
                int sum_all = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                             v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
                
                /* Force spilling with many function calls */
                helper1(*chain_addr1, *chain_addr2, *chain_addr3, v1, v2, v3);
                helper2(f1, f2, f3, f4, &arr_float[idx6]);
                helper3(d1, d2, &arr_dbl[idx7], &arr_dbl[idx8]);
                helper4(l1, l2, l3, &arr_long[idx6], idx7);
                
                /* Update many variables to keep them live */
                v1 = v2 + *chain_addr1;
                v2 = v3 + *chain_addr2;
                v3 = v4 + *chain_addr3;
                v4 = v5 + idx6;
                v5 = v6 + idx7;
                v6 = v7 + idx8;
                v7 = v8 + sum_all % 256;
                v8 = v9 + i;
                v9 = v10 + (v1 ^ v2);
                v10 = v11 + (v3 | v4);
                
                f1 = f2 * 1.1f;
                f2 = f3 / 1.2f;
                f3 = f4 + f1;
                f4 = f5 - f2;
                
                d1 = d2 * 1.01;
                d2 = d3 / 1.02;
                d3 = d4 + d1;
                d4 = d5 - d2;
                
                l1 = l2 + l3;
                l2 = l3 + l4;
                l3 = l4 + l5;
                l4 = l5 + l6;
                
                break;
            }
        }
        
        /* Update result to prevent dead code elimination */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 +
                 (int)d1 + (int)d2 + (int)d3 + (int)d4 +
                 (int)l1 + (int)l2 + (int)l3 + (int)l4;
        
        /* Force periodic spilling with volatile accesses */
        if (i % 100 == 0) {
            *volatile_ptr1 = result;
            *volatile_ptr2 = d1;
            *volatile_ptr3 = l1;
            *volatile_ptr4 = f1;
        }
    }
    
    return result;
}

int main() {
    const int SIZE = 1024;
    
    /* Allocate and initialize arrays with pattern data */
    int* arr_int = (int*)malloc(SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(SIZE * sizeof(double));
    long* arr_long = (long*)malloc(SIZE * sizeof(long));
    float* arr_float = (float*)malloc(SIZE * sizeof(float));
    
    if (!arr_int || !arr_dbl || !arr_long || !arr_float) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = i * 3 + 1;
        arr_dbl[i] = i * 1.5 + 0.5;
        arr_long[i] = i * 7L + 3L;
        arr_float[i] = i * 0.7f + 0.3f;
    }
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_long, arr_float, SIZE);
    
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_long);
    free(arr_float);
    
    return 0;
}
