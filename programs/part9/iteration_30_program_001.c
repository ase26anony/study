/* reload1_stress_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define NUM_LOCALS 40
#define ITERATIONS 1000

/* Opaque noinline functions to prevent optimization */
int __attribute__((noinline)) helper1(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b + c + d + e + f;
    return sink & 0xFF;
}

double __attribute__((noinline)) helper2(double a, double b, double c, 
                                         double d, double e, double f) {
    volatile double sink = a + b + c + d + e + f;
    return sink;
}

void __attribute__((noinline)) helper3(int* addr1, int* addr2, double* addr3, 
                                       double* addr4, long* addr5) {
    volatile int sink1 = *addr1 + *addr2;
    volatile double sink2 = *addr3 + *addr4;
    volatile long sink3 = *addr5;
    (void)sink1; (void)sink2; (void)sink3;
}

void __attribute__((noinline)) helper4(long long a, long long b, 
                                       float c, float d, 
                                       int* addr, double* daddr) {
    volatile long long sink1 = a + b;
    volatile float sink2 = c + d;
    volatile int sink3 = *addr;
    volatile double sink4 = *daddr;
    (void)sink1; (void)sink2; (void)sink3; (void)sink4;
}

/* Main stress function */
int __attribute__((noinline)) stress_reload(int* arr_int, double* arr_double, 
                                           long* arr_long, float* arr_float) {
    /* Declare many local variables to exhaust registers */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
    long l0, l1, l2, l3, l4, l5, l6, l7, l8, l9;
    long long ll0, ll1, ll2, ll3, ll4;
    
    /* Initialize with values from arrays to create dependencies */
    v0 = arr_int[0]; v1 = arr_int[1]; v2 = arr_int[2]; v3 = arr_int[3];
    v4 = arr_int[4]; v5 = arr_int[5]; v6 = arr_int[6]; v7 = arr_int[7];
    v8 = arr_int[8]; v9 = arr_int[9]; v10 = arr_int[10]; v11 = arr_int[11];
    v12 = arr_int[12]; v13 = arr_int[13]; v14 = arr_int[14]; v15 = arr_int[15];
    v16 = arr_int[16]; v17 = arr_int[17]; v18 = arr_int[18]; v19 = arr_int[19];
    
    d0 = arr_double[0]; d1 = arr_double[1]; d2 = arr_double[2]; d3 = arr_double[3];
    d4 = arr_double[4]; d5 = arr_double[5]; d6 = arr_double[6]; d7 = arr_double[7];
    d8 = arr_double[8]; d9 = arr_double[9];
    
    f0 = arr_float[0]; f1 = arr_float[1]; f2 = arr_float[2]; f3 = arr_float[3];
    f4 = arr_float[4]; f5 = arr_float[5]; f6 = arr_float[6]; f7 = arr_float[7];
    f8 = arr_float[8]; f9 = arr_float[9];
    
    l0 = arr_long[0]; l1 = arr_long[1]; l2 = arr_long[2]; l3 = arr_long[3];
    l4 = arr_long[4]; l5 = arr_long[5]; l6 = arr_long[6]; l7 = arr_long[7];
    l8 = arr_long[8]; l9 = arr_long[9];
    
    ll0 = (long long)v0 + v1; ll1 = (long long)v2 + v3;
    ll2 = (long long)v4 + v5; ll3 = (long long)v6 + v7;
    ll4 = (long long)v8 + v9;
    
    volatile int checksum = 0;
    
    /* Complex loop with extreme register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v0 * 3 + v1 * 5) % ARRAY_SIZE;
        int idx2 = (i * 11 + v2 * 13 + v3 * 17) % ARRAY_SIZE;
        int idx3 = (i * 19 + v4 * 23 + v5 * 29) % ARRAY_SIZE;
        int idx4 = (i * 31 + v6 * 37 + v7 * 41) % ARRAY_SIZE;
        int idx5 = (i * 43 + v8 * 47 + v9 * 53) % ARRAY_SIZE;
        
        /* Address computations that need registers */
        int* addr1 = &arr_int[idx1];
        int* addr2 = &arr_int[idx2];
        double* addr3 = &arr_double[idx3];
        double* addr4 = &arr_double[idx4];
        long* addr5 = &arr_long[idx5];
        float* addr6 = &arr_float[(idx1 + idx2) % ARRAY_SIZE];
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2;
        double dtemp1, dtemp2;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS patterns */
        asm volatile (
            "mov %[val1], %[tmp1]\n\t"
            "add %[val2], %[tmp1]\n\t"
            "mov %[tmp1], %[out1]\n\t"
            : [out1] "=r" (temp1)
            : [val1] "r" (v0), [val2] "r" (v1), [tmp1] "r" (0)
            : "memory"
        );
        
        /* More complex addressing in assembly */
        asm volatile (
            "mov (%[addr]), %[tmp]\n\t"
            "add %[val], %[tmp]\n\t"
            "mov %[tmp], (%[addr2])\n\t"
            : 
            : [addr] "r" (addr1), [val] "r" (v2), 
              [addr2] "r" (addr2), [tmp] "r" (temp1)
            : "memory"
        );
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Block with address computations */
                int* complex_addr = &arr_int[(v10 * i + v11 * 3) % ARRAY_SIZE];
                double* dcomplex_addr = &arr_double[(v12 * i + v13 * 5) % ARRAY_SIZE];
                
                /* Use computed addresses in another block */
                v10 = *complex_addr + v14;
                d0 = *dcomplex_addr + d1;
                
                /* Force RELOAD_FOR_OTHER_ADDRESS */
                helper3(complex_addr, &arr_int[idx1], dcomplex_addr, 
                       &arr_double[idx2], &arr_long[idx3]);
                break;
            }
            case 1:
            case 2: {
                /* Different addressing pattern */
                long* laddr = &arr_long[(v15 * 7 + i * 11) % ARRAY_SIZE];
                float* faddr = &arr_float[(v16 * 13 + i * 17) % ARRAY_SIZE];
                
                /* Inline assembly with memory constraint */
                long ltemp;
                asm volatile (
                    "mov (%[addr]), %[out]\n\t"
                    : [out] "=r" (ltemp)
                    : [addr] "m" (*laddr)
                    : "memory"
                );
                
                v11 = (int)ltemp + v17;
                f0 = *faddr + f1;
                break;
            }
            case 3:
            case 4: {
                /* Mixed operand types */
                int idx6 = (v18 * 19 + v19 * 23 + i * 29) % ARRAY_SIZE;
                int* addr7 = &arr_int[idx6];
                double* addr8 = &arr_double[(idx6 * 31) % ARRAY_SIZE];
                
                /* Force RELOAD_FOR_OUTPUT_ADDRESS */
                asm volatile (
                    "mov %[val], (%[addr])\n\t"
                    : 
                    : [val] "r" (v18), [addr] "r" (addr7)
                    : "memory"
                );
                
                helper4(ll0, ll1, f2, f3, addr7, addr8);
                break;
            }
            case 5:
            case 6: {
                /* More complex control flow with computed goto */
                static void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
                goto *labels[i % 4];
                
            label1:
                v12 = arr_int[(v13 * i + v14) % ARRAY_SIZE];
                goto after_labels;
            label2:
                v13 = arr_int[(v14 * i + v15) % ARRAY_SIZE];
                goto after_labels;
            label3:
                v14 = arr_int[(v15 * i + v16) % ARRAY_SIZE];
                goto after_labels;
            label4:
                v15 = arr_int[(v16 * i + v17) % ARRAY_SIZE];
                goto after_labels;
            after_labels:
                break;
            }
            case 7: {
                /* RELOAD_FOR_OPERAND_ADDRESS pattern */
                int* opaddr = &arr_int[(v0 * v1 + v2 * v3) % ARRAY_SIZE];
                int* opaddr2 = &arr_int[(v4 * v5 + v6 * v7) % ARRAY_SIZE];
                
                /* Multiple uses of same value as data and address */
                int temp_val = v8 + v9;
                *opaddr = temp_val;
                v16 = *opaddr2 + temp_val;
                
                /* Force operand address reloads */
                asm volatile (
                    "mov (%[addr1]), %[tmp1]\n\t"
                    "add (%[addr2]), %[tmp1]\n\t"
                    "mov %[tmp1], %[out]\n\t"
                    : [out] "=r" (v17)
                    : [addr1] "r" (opaddr), [addr2] "r" (opaddr2), [tmp1] "r" (0)
                    : "memory"
                );
                break;
            }
        }
        
        /* Update most variables to keep them live */
        v0 = v1 + arr_int[idx1];
        v1 = v2 + arr_int[idx2];
        v2 = v3 + arr_int[idx3];
        v3 = v4 + temp1;
        v4 = v5 + v0;
        v5 = v6 + v1;
        v6 = v7 + v2;
        v7 = v8 + v3;
        v8 = v9 + v4;
        v9 = v10 + v5;
        v10 = v11 + v6;
        v11 = v12 + v7;
        v12 = v13 + v8;
        v13 = v14 + v9;
        v14 = v15 + v10;
        v15 = v16 + v11;
        v16 = v17 + v12;
        v17 = v18 + v13;
        v18 = v19 + v14;
        v19 = v0 + v15;
        
        d0 = d1 + arr_double[idx3];
        d1 = d2 + arr_double[idx4];
        d2 = d3 + d0;
        d3 = d4 + d1;
        d4 = d5 + d2;
        d5 = d6 + d3;
        d6 = d7 + d4;
        d7 = d8 + d5;
        d8 = d9 + d6;
        d9 = d0 + d7;
        
        f0 = f1 + arr_float[idx1];
        f1 = f2 + arr_float[idx2];
        f2 = f3 + f0;
        f3 = f4 + f1;
        f4 = f5 + f2;
        f5 = f6 + f3;
        f6 = f7 + f4;
        f7 = f8 + f5;
        f8 = f9 + f6;
        f9 = f0 + f7;
        
        l0 = l1 + arr_long[idx5];
        l1 = l2 + l0;
        l2 = l3 + l1;
        l3 = l4 + l2;
        l4 = l5 + l3;
        l5 = l6 + l4;
        l6 = l7 + l5;
        l7 = l8 + l6;
        l8 = l9 + l7;
        l9 = l0 + l8;
        
        ll0 = ll1 + v0;
        ll1 = ll2 + v1;
        ll2 = ll3 + v2;
        ll3 = ll4 + v3;
        ll4 = ll0 + v4;
        
        /* Call helper functions with different argument combinations */
        if (i % 3 == 0) {
            v0 = helper1(v0, v1, v2, v3, v4, v5);
        }
        if (i % 5 == 0) {
            d0 = helper2(d0, d1, d2, d3, d4, d5);
        }
        if (i % 7 == 0) {
            helper3(&arr_int[idx1], &arr_int[idx2], 
                   &arr_double[idx3], &arr_double[idx4],
                   &arr_long[idx5]);
        }
        if (i % 11 == 0) {
            helper4(ll0, ll1, f0, f1, &arr_int[idx1], &arr_double[idx2]);
        }
        
        checksum += v0 + v10 + v19 + (int)d0 + (int)f0 + (int)l0;
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays with pattern data */
    int* arr_int = malloc(ARRAY_SIZE * sizeof(int));
    double* arr_double = malloc(ARRAY_SIZE * sizeof(double));
    long* arr_long = malloc(ARRAY_SIZE * sizeof(long));
    float* arr_float = malloc(ARRAY_SIZE * sizeof(float));
    
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
    
    printf("Starting stress test...\n");
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_double, arr_long, arr_float);
    
    printf("Result checksum: %d\n", result);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(arr_int);
    free(arr_double);
    free(arr_long);
    free(arr_float);
    
    return 0;
}
