/* reload1_stress_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Opaque functions to prevent optimization */
NOINLINE int func1(int a, int b, int c, int d, int e, int f);
NOINLINE double func2(double a, double b, double c, double d, double e);
NOINLINE long func3(long a, long b, long c, long d, long* addr);
NOINLINE float func4(float a, float b, float c, float* addr1, float* addr2);

/* Implementation of opaque functions */
NOINLINE int func1(int a, int b, int c, int d, int e, int f) {
    VOLATILE_VAR int sink = a + b - c + d - e + f;
    return sink & 0xFF;
}

NOINLINE double func2(double a, double b, double c, double d, double e) {
    VOLATILE_VAR double sink = a * 0.5 + b * 0.3 + c * 0.1 + d * 0.05 + e * 0.05;
    return sink;
}

NOINLINE long func3(long a, long b, long c, long d, long* addr) {
    VOLATILE_VAR long sink = a ^ b ^ c ^ d ^ (*addr);
    *addr = sink;
    return sink;
}

NOINLINE float func4(float a, float b, float c, float* addr1, float* addr2) {
    VOLATILE_VAR float sink = a + b + c + *addr1 + *addr2;
    *addr1 = sink * 0.5f;
    *addr2 = sink * 0.5f;
    return sink;
}

/* Main stress function */
NOINLINE uint64_t stress_reload(int* arr_int, double* arr_dbl, 
                                float* arr_flt, long* arr_lng, 
                                int size) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Initialize with values from arrays */
    v1 = arr_int[0]; v2 = arr_int[1]; v3 = arr_int[2]; v4 = arr_int[3];
    v5 = arr_int[4]; v6 = arr_int[5]; v7 = arr_int[6]; v8 = arr_int[7];
    v9 = arr_int[8]; v10 = arr_int[9];
    v11 = arr_int[10]; v12 = arr_int[11]; v13 = arr_int[12]; v14 = arr_int[13];
    v15 = arr_int[14]; v16 = arr_int[15]; v17 = arr_int[16]; v18 = arr_int[17];
    v19 = arr_int[18]; v20 = arr_int[19];
    
    d1 = arr_dbl[0]; d2 = arr_dbl[1]; d3 = arr_dbl[2]; d4 = arr_dbl[3];
    d5 = arr_dbl[4]; d6 = arr_dbl[5]; d7 = arr_dbl[6]; d8 = arr_dbl[7];
    d9 = arr_dbl[8]; d10 = arr_dbl[9];
    
    f1 = arr_flt[0]; f2 = arr_flt[1]; f3 = arr_flt[2]; f4 = arr_flt[3];
    f5 = arr_flt[4]; f6 = arr_flt[5]; f7 = arr_flt[6]; f8 = arr_flt[7];
    f9 = arr_flt[8]; f10 = arr_flt[9];
    
    l1 = arr_lng[0]; l2 = arr_lng[1]; l3 = arr_lng[2]; l4 = arr_lng[3];
    l5 = arr_lng[4]; l6 = arr_lng[5]; l7 = arr_lng[6]; l8 = arr_lng[7];
    l9 = arr_lng[8]; l10 = arr_lng[9];
    
    VOLATILE_VAR uint64_t checksum = 0;
    
    /* Complex loop with extreme register pressure */
    for (int i = 0; i < 1000; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % size;
        int idx2 = (i * 11 + v3 * 13 + v4 * 17) % size;
        int idx3 = (i * 19 + v5 * 23 + v6 * 29) % size;
        int idx4 = (i * 31 + v7 * 37 + v8 * 41) % size;
        int idx5 = (i * 43 + v9 * 47 + v10 * 53) % size;
        
        /* Take addresses of locals to force stack-based reloads */
        int* ptr1 = &v11;
        double* ptr2 = &d1;
        float* ptr3 = &f1;
        long* ptr4 = &l1;
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Complex addressing in one branch */
                int* addr1 = &arr_int[(idx1 * 3 + idx2 * 7) % size];
                double* addr2 = &arr_dbl[(idx3 * 5 + idx4 * 11) % size];
                
                /* Inline assembly with conflicting constraints */
                int temp1, temp2;
                asm volatile (
                    "mov %[val1], %[res1]\n\t"
                    "add %[val2], %[res1]\n\t"
                    "mov %[val3], %[res2]\n\t"
                    : [res1] "=r" (temp1), [res2] "=r" (temp2)
                    : [val1] "r" (*addr1), [val2] "r" (v1), [val3] "m" (*addr2)
                    : "cc"
                );
                
                /* Use computed addresses in function calls */
                v11 = func1(v1, v2, temp1, temp2, *addr1, idx1);
                d1 = func2(d1, d2, d3, *addr2, arr_dbl[idx2]);
                break;
            }
            
            case 1: {
                /* Different addressing pattern */
                float* addr3 = &arr_flt[(idx2 * 13 + idx3 * 17) % size];
                long* addr4 = &arr_lng[(idx4 * 19 + idx5 * 23) % size];
                
                /* More inline assembly */
                long temp3;
                asm volatile (
                    "mov %[src], %[dst]\n\t"
                    "shl $3, %[dst]\n\t"
                    : [dst] "=r" (temp3)
                    : [src] "m" (*addr4)
                    : "cc"
                );
                
                f1 = func4(f1, f2, f3, addr3, &arr_flt[idx1]);
                l1 = func3(l1, l2, temp3, *addr4, addr4);
                break;
            }
            
            case 2:
            case 3: {
                /* Mixed operand types with address computations */
                int* addr5 = &arr_int[(v1 * v2 + i * 3) % size];
                double* addr6 = &arr_dbl[(v3 * v4 + i * 5) % size];
                
                /* Force address register pressure */
                asm volatile (
                    "mov %[in1], %%rax\n\t"
                    "add %[in2], %%rax\n\t"
                    "mov %%rax, %[out]\n\t"
                    : [out] "=m" (*addr5)
                    : [in1] "r" (v5), [in2] "r" (v6)
                    : "rax", "cc"
                );
                
                d2 = func2(d2, *addr6, d3, d4, d5);
                break;
            }
            
            case 4:
            case 5:
            case 6: {
                /* Nested addressing calculations */
                int complex_idx = (v7 * v8 + v9 * v10 + i * 7) % size;
                int* addr7 = &arr_int[complex_idx];
                int* addr8 = &arr_int[(complex_idx * 2 + 1) % size];
                
                /* Multiple memory constraints */
                int temp4, temp5;
                asm volatile (
                    "mov (%[addr7]), %[t4]\n\t"
                    "mov (%[addr8]), %[t5]\n\t"
                    "add %[t4], %[t5]\n\t"
                    : [t4] "=r" (temp4), [t5] "=r" (temp5)
                    : [addr7] "r" (addr7), [addr8] "r" (addr8)
                    : "memory"
                );
                
                v12 = func1(temp4, temp5, v11, v12, v13, v14);
                break;
            }
            
            default: {
                /* Use all variable types together */
                float* addr9 = &arr_flt[(l1 + l2 + i) % size];
                long* addr10 = &arr_lng[(l3 + l4 + i * 2) % size];
                
                /* Complex inline assembly with many operands */
                double temp6;
                asm volatile (
                    "cvtsi2sd %[intval], %[dblout]\n\t"
                    "addsd %[dblin], %[dblout]\n\t"
                    : [dblout] "=x" (temp6)
                    : [intval] "r" (v15), [dblin] "x" (d6)
                    : 
                );
                
                f2 = func4(f2, f3, f4, addr9, &f5);
                l2 = func3(l2, l3, l4, *addr10, addr10);
                d3 = temp6;
                break;
            }
        }
        
        /* Update most variables to keep them live */
        v1 = v1 ^ arr_int[idx1];
        v2 = v2 + arr_int[idx2];
        v3 = v3 - arr_int[idx3];
        v4 = v4 * (arr_int[idx4] | 1);
        v5 = v5 ^ (v6 + i);
        v6 = v6 + (v7 * 3);
        v7 = v7 - (v8 / 2);
        v8 = v8 ^ (v9 * 5);
        v9 = v9 + (v10 * 7);
        v10 = v10 - (v11 * 11);
        
        d1 = d1 * 1.01 + arr_dbl[idx1];
        d2 = d2 * 0.99 + arr_dbl[idx2];
        d3 = d3 * 1.02 + arr_dbl[idx3];
        d4 = d4 * 0.98 + arr_dbl[idx4];
        
        f1 = f1 + arr_flt[idx1] * 0.5f;
        f2 = f2 + arr_flt[idx2] * 0.3f;
        f3 = f3 + arr_flt[idx3] * 0.7f;
        
        l1 = l1 ^ arr_lng[idx1];
        l2 = l2 + arr_lng[idx2];
        l3 = l3 - arr_lng[idx3];
        
        /* Update checksum */
        checksum += (uint64_t)v1 + (uint64_t)v2 + (uint64_t)(d1 * 1000) + 
                   (uint64_t)l1 + (uint64_t)(f1 * 1000);
    }
    
    return checksum;
}

int main() {
    const int SIZE = 1000;
    
    /* Allocate and initialize arrays */
    int* arr_int = (int*)malloc(SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(SIZE * sizeof(double));
    float* arr_flt = (float*)malloc(SIZE * sizeof(float));
    long* arr_lng = (long*)malloc(SIZE * sizeof(long));
    
    if (!arr_int || !arr_dbl || !arr_flt || !arr_lng) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = i * 3 + 1;
        arr_dbl[i] = i * 0.5 + 1.0;
        arr_flt[i] = i * 0.3f + 0.5f;
        arr_lng[i] = i * 7L + 3L;
    }
    
    /* Call stress function */
    uint64_t result = stress_reload(arr_int, arr_dbl, arr_flt, arr_lng, SIZE);
    
    printf("Checksum: %llu\n", (unsigned long long)result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_flt);
    free(arr_lng);
    
    return 0;
}
