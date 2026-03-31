/* reload_stress_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))
#define VOLATILE_DEREF(ptr) (*(volatile typeof(*(ptr))*)(ptr))

/* Opaque helper functions that force register shuffling */
NOINLINE int helper1(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

NOINLINE double helper2(double a, double b, double c, 
                        double d, double e, double f) {
    volatile double sink = a * b + c * d - e * f;
    return sink;
}

NOINLINE long helper3(long a, long b, long c, long d,
                      long e, long f, long g, long h) {
    volatile long sink = (a * b) + (c * d) - (e * f) + (g * h);
    return sink;
}

NOINLINE void* helper4(void* p1, void* p2, void* p3, 
                       int i1, int i2, int i3) {
    volatile char* sink = (char*)p1 + i1;
    sink = (char*)p2 + i2;
    sink = (char*)p3 + i3;
    return (void*)sink;
}

/* Main stress function with extreme register pressure */
NOINLINE uint64_t stress_reload(int* arr_int, double* arr_dbl, 
                                long* arr_long, int size) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Initialize with complex expressions */
    v1 = arr_int[0] * 3;
    v2 = arr_int[1] + 7;
    v3 = arr_int[2] - 5;
    v4 = arr_int[3] * 2;
    v5 = arr_int[4] / 3;
    v6 = arr_int[5] + 11;
    v7 = arr_int[6] - 13;
    v8 = arr_int[7] * 7;
    v9 = arr_int[8] + 17;
    v10 = arr_int[9] - 19;
    
    v11 = v1 * v2 + v3;
    v12 = v4 - v5 * v6;
    v13 = v7 + v8 - v9;
    v14 = v10 * 3 + v1;
    v15 = v2 - v3 * 4;
    v16 = v4 + v5 - v6;
    v17 = v7 * v8 + v9;
    v18 = v10 - v1 * v2;
    v19 = v3 + v4 - v5;
    v20 = v6 * v7 + v8;
    
    v21 = v9 - v10 + v11;
    v22 = v12 * v13 - v14;
    v23 = v15 + v16 * v17;
    v24 = v18 - v19 + v20;
    v25 = v21 * v22 + v23;
    v26 = v24 - v25 * v26;
    v27 = arr_int[10] + v1 * 2;
    v28 = arr_int[11] - v2 * 3;
    v29 = arr_int[12] + v3 * 4;
    v30 = arr_int[13] - v4 * 5;
    
    d1 = arr_dbl[0] * 1.5;
    d2 = arr_dbl[1] + 2.5;
    d3 = arr_dbl[2] - 3.5;
    d4 = arr_dbl[3] * 4.5;
    d5 = arr_dbl[4] / 5.5;
    d6 = d1 + d2 - d3;
    d7 = d4 * d5 + d6;
    d8 = d1 - d2 * d3;
    d9 = d4 + d5 - d6;
    d10 = d7 * d8 - d9;
    
    l1 = arr_long[0] * 3L;
    l2 = arr_long[1] + 7L;
    l3 = arr_long[2] - 5L;
    l4 = arr_long[3] * 2L;
    l5 = arr_long[4] / 3L;
    l6 = l1 + l2 * l3;
    l7 = l4 - l5 + l6;
    l8 = l1 * l2 - l3;
    l9 = l4 + l5 * l6;
    l10 = l7 - l8 + l9;
    
    volatile int checksum = 0;
    volatile double dchecksum = 0.0;
    volatile long lchecksum = 0L;
    
    /* Complex loop with multiple addressing modes */
    for (int i = 0; i < 1000; i++) {
        /* Complex array indexing requiring address computations */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % size;
        int idx2 = (i * 11 + v3 * 2 + v4 * 7) % size;
        int idx3 = (i * 13 + v5 * 3 + v6 * 11) % size;
        int idx4 = (i * 17 + v7 * 5 + v8 * 13) % size;
        int idx5 = (i * 19 + v9 * 7 + v10 * 17) % size;
        
        /* Multiple volatile memory accesses with address computations */
        int* addr1 = &arr_int[idx1];
        int* addr2 = &arr_int[idx2];
        int* addr3 = &arr_int[idx3];
        double* addr4 = &arr_dbl[idx4 % (size/2)];
        long* addr5 = &arr_long[idx5 % (size/2)];
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2, temp3;
        double dtemp1, dtemp2;
        long ltemp1, ltemp2;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "mov %[val1], %[tmp1]\n\t"
            "add %[val2], %[tmp1]\n\t"
            "mov %[tmp1], %[out1]"
            : [out1] "=r" (temp1), [tmp1] "=&r" (temp2)
            : [val1] "rm" (VOLATILE_DEREF(addr1)), 
              [val2] "rm" (VOLATILE_DEREF(addr2))
            : "cc"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS */
        asm volatile (
            "lea (%[base], %[index], 4), %[addr]\n\t"
            "mov (%[addr]), %[val]"
            : [addr] "=&r" (addr1), [val] "=r" (temp3)
            : [base] "r" (arr_int), [index] "r" (idx1)
            : "cc"
        );
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* RELOAD_FOR_INPADDR_ADDRESS */
                int* addr6 = &arr_int[(idx1 + idx2) % size];
                asm volatile (
                    "mov (%[addr]), %%eax\n\t"
                    "add %%eax, %[sum]"
                    : [sum] "+r" (checksum)
                    : [addr] "r" (addr6)
                    : "eax", "cc"
                );
                v1 = VOLATILE_DEREF(addr6) + v2;
                break;
            }
            case 1: {
                /* RELOAD_FOR_OUTADDR_ADDRESS */
                long* addr7 = &arr_long[(idx3 * 2) % (size/2)];
                asm volatile (
                    "mov (%[addr]), %%rax\n\t"
                    "add %%rax, %[sum]"
                    : [sum] "+r" (lchecksum)
                    : [addr] "r" (addr7)
                    : "rax", "cc"
                );
                l1 = VOLATILE_DEREF(addr7) * l2;
                break;
            }
            case 2: {
                /* RELOAD_FOR_OPERAND_ADDRESS */
                double* addr8 = &arr_dbl[(idx4 + idx5) % (size/2)];
                dtemp1 = VOLATILE_DEREF(addr8);
                asm volatile (
                    "addsd %[val], %[sum]"
                    : [sum] "+x" (dchecksum)
                    : [val] "xm" (dtemp1)
                );
                d1 = dtemp1 * d2;
                break;
            }
            case 3: {
                /* RELOAD_FOR_OPADDR_ADDR */
                int offset = (v1 * v2 + v3) % size;
                asm volatile (
                    "mov (%[base], %[off], 4), %%eax\n\t"
                    "imul %%eax, %[val]"
                    : [val] "+r" (v4)
                    : [base] "r" (arr_int), [off] "r" (offset)
                    : "eax", "cc"
                );
                break;
            }
            case 4: {
                /* RELOAD_FOR_OTHER_ADDRESS */
                void* addr9 = (char*)arr_int + idx1 * sizeof(int);
                void* addr10 = (char*)arr_dbl + idx2 * sizeof(double);
                helper4(addr9, addr10, arr_long, v5, v6, v7);
                break;
            }
            default: {
                /* RELOAD_OTHER - mixed operations */
                v10 = helper1(v1, v2, v3, v4, v5, v6);
                d10 = helper2(d1, d2, d3, d4, d5, d6);
                l10 = helper3(l1, l2, l3, l4, l5, l6, l7, l8);
                break;
            }
        }
        
        /* Update many variables to keep them live */
        v1 = v1 + VOLATILE_DEREF(&arr_int[idx1]);
        v2 = v2 - VOLATILE_DEREF(&arr_int[idx2]);
        v3 = v3 * VOLATILE_DEREF(&arr_int[idx3]);
        v4 = v4 / (VOLATILE_DEREF(&arr_int[idx4 % size]) + 1);
        v5 = v5 + VOLATILE_DEREF(&arr_int[idx5 % size]);
        
        v6 = v6 - v7 * v8;
        v7 = v7 + v9 / (v10 + 1);
        v8 = v8 * v1 - v2;
        v9 = v9 + v3 * v4;
        v10 = v10 - v5 / (v6 + 1);
        
        v11 = v11 + v7;
        v12 = v12 - v8;
        v13 = v13 * v9;
        v14 = v14 / (v10 + 1);
        v15 = v15 + v11;
        
        d1 = d1 + arr_dbl[idx1 % (size/2)];
        d2 = d2 - arr_dbl[idx2 % (size/2)];
        d3 = d3 * arr_dbl[idx3 % (size/2)];
        d4 = d4 / (arr_dbl[idx4 % (size/2)] + 1.0);
        d5 = d5 + arr_dbl[idx5 % (size/2)];
        
        l1 = l1 + arr_long[idx1 % (size/2)];
        l2 = l2 - arr_long[idx2 % (size/2)];
        l3 = l3 * arr_long[idx3 % (size/2)];
        l4 = l4 / (arr_long[idx4 % (size/2)] + 1L);
        l5 = l5 + arr_long[idx5 % (size/2)];
        
        /* Force address computations to remain */
        volatile int* volatile_ptr = &arr_int[(i * 23 + v1) % size];
        checksum += *volatile_ptr;
        
        volatile double* volatile_dbl_ptr = &arr_dbl[(i * 29 + v2) % (size/2)];
        dchecksum += *volatile_dbl_ptr;
        
        volatile long* volatile_long_ptr = &arr_long[(i * 31 + v3) % (size/2)];
        lchecksum += *volatile_long_ptr;
    }
    
    /* Final computation using all variables */
    uint64_t result = (uint64_t)checksum;
    result += (uint64_t)dchecksum;
    result += (uint64_t)lchecksum;
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    result += v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    result += (uint64_t)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10);
    result += l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10;
    
    return result;
}

int main() {
    const int SIZE = 1024;
    
    /* Allocate and initialize arrays with pattern data */
    int* arr_int = (int*)malloc(SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(SIZE * sizeof(double));
    long* arr_long = (long*)malloc(SIZE * sizeof(long));
    
    if (!arr_int || !arr_dbl || !arr_long) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = i * 3 - 7;
        arr_dbl[i] = i * 1.5 - 3.14;
        arr_long[i] = i * 5L - 11L;
    }
    
    /* Call the stress function */
    uint64_t result = stress_reload(arr_int, arr_dbl, arr_long, SIZE);
    
    printf("Result: %lu\n", (unsigned long)result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_long);
    
    return 0;
}
