/* reload_stress_test.c - Extreme register pressure test for GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define NUM_LOCALS 40
#define ITERATIONS 1000

/* Opaque noinline functions to prevent optimization */
int __attribute__((noinline)) helper1(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

double __attribute__((noinline)) helper2(double a, double b, double c, 
                                        double d, double e, double f) {
    volatile double sink = a * b + c - d * e + f;
    return sink;
}

void __attribute__((noinline)) helper3(int* addr1, int* addr2, double* addr3, 
                                      volatile int* result) {
    *result = *addr1 + *addr2 + (int)(*addr3);
}

void __attribute__((noinline)) helper4(long long a, long long b, 
                                      volatile long long* out1,
                                      volatile long long* out2) {
    *out1 = a ^ b;
    *out2 = a & b;
}

/* Main stress function with extreme register pressure */
int __attribute__((noinline)) stress_reload(int* arr_int, double* arr_dbl, 
                                           long long* arr_ll, float* arr_flt) {
    /* Declare many local variables to exhaust registers */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    float f0, f1, f2, f3, f4;
    long long ll0, ll1, ll2, ll3, ll4;
    
    /* Volatile sinks to prevent elimination */
    volatile int vsink_int = 0;
    volatile double vsink_dbl = 0.0;
    volatile long long vsink_ll = 0;
    
    /* Initialize locals with complex expressions */
    v0 = arr_int[0] * 3;
    v1 = arr_int[1] + arr_int[2];
    v2 = arr_int[3] - arr_int[4];
    v3 = arr_int[5] * arr_int[6];
    v4 = arr_int[7] / (arr_int[8] + 1);
    
    d0 = arr_dbl[0] * 2.5;
    d1 = arr_dbl[1] + arr_dbl[2];
    d2 = arr_dbl[3] - arr_dbl[4];
    d3 = arr_dbl[5] * arr_dbl[6];
    d4 = arr_dbl[7] / (arr_dbl[8] + 1.0);
    
    /* More initializations */
    for (int i = 0; i < 10; i++) {
        v5 = arr_int[i * 7 % ARRAY_SIZE];
        v6 = arr_int[i * 11 % ARRAY_SIZE];
        v7 = v5 + v6 * 3;
        v8 = v7 - arr_int[i * 13 % ARRAY_SIZE];
    }
    
    /* Main stress loop */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (iter * 7 + v0 * 3 + v1 * 5) % ARRAY_SIZE;
        int idx2 = (iter * 11 + v2 * 13 + v3 * 17) % ARRAY_SIZE;
        int idx3 = (iter * 19 + v4 * 23 + v5 * 29) % ARRAY_SIZE;
        int idx4 = (iter * 31 + v6 * 37 + v7 * 41) % ARRAY_SIZE;
        
        double idx5 = (iter * 1.5 + d0 * 2.3 + d1 * 3.7);
        int idx6 = ((int)idx5 * 43 + v8 * 47) % ARRAY_SIZE;
        
        /* Force address computations into registers */
        int* addr1 = &arr_int[idx1];
        int* addr2 = &arr_int[idx2];
        double* addr3 = &arr_dbl[idx3];
        float* addr4 = &arr_flt[idx4];
        long long* addr5 = &arr_ll[idx6];
        
        /* Use inline assembly with conflicting constraints */
        int temp1, temp2;
        double temp_dbl;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "mov %[val1], %[res1]\n\t"
            "add %[val2], %[res1]\n\t"
            : [res1] "=r" (temp1)
            : [val1] "r" (*addr1), [val2] "r" (*addr2)
            : "cc"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS */
        long long ll_temp;
        asm volatile (
            "movq %[addr], %%rax\n\t"
            "movq (%%rax), %[out]\n\t"
            : [out] "=r" (ll_temp)
            : [addr] "r" (addr5)
            : "rax", "memory"
        );
        
        /* Control flow that splits live ranges */
        switch (iter % 8) {
            case 0: {
                /* Block with address computations */
                int* complex_addr = &arr_int[(idx1 * 3 + idx2 * 7) % ARRAY_SIZE];
                double* dbl_complex_addr = &arr_dbl[(idx3 * 5 + idx4 * 11) % ARRAY_SIZE];
                
                /* Use computed addresses in another basic block */
                if (iter & 1) {
                    temp2 = *complex_addr + v0;
                    temp_dbl = *dbl_complex_addr + d0;
                } else {
                    temp2 = *complex_addr - v0;
                    temp_dbl = *dbl_complex_addr - d0;
                }
                break;
            }
            case 1:
            case 2: {
                /* Different address pattern */
                float* flt_addr = &arr_flt[(iter * 53 + v1 * 59) % ARRAY_SIZE];
                vsink_int = (int)(*flt_addr * 100.0f);
                break;
            }
            case 3:
            case 4: {
                /* Force RELOAD_FOR_OPERAND_ADDRESS */
                int offset = (v2 * 61 + v3 * 67) % 100;
                int* offset_addr = addr1 + offset;
                vsink_int = *offset_addr;
                break;
            }
            default: {
                /* Mixed operations */
                ll0 = arr_ll[idx1] ^ arr_ll[idx2];
                ll1 = arr_ll[idx3] & arr_ll[idx4];
                vsink_ll = ll0 + ll1;
                break;
            }
        }
        
        /* Call helper functions with many arguments - forces register shuffling */
        v9 = helper1(v0, v1, v2, v3, v4, temp1);
        d5 = helper2(d0, d1, d2, d3, d4, temp_dbl);
        
        /* More complex addressing */
        int idx7 = (iter * 71 + v9 * 73) % ARRAY_SIZE;
        int idx8 = (iter * 79 + (int)d5 * 83) % ARRAY_SIZE;
        
        /* RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        int* inpaddr = &arr_int[idx7];
        int* outaddr = &arr_int[idx8];
        
        volatile int addr_sink;
        helper3(inpaddr, outaddr, &arr_dbl[idx6], &addr_sink);
        
        /* Update many local variables to keep them live */
        v0 = v0 + temp1 - v9;
        v1 = v1 * 3 - temp2;
        v2 = v2 + arr_int[idx1] - arr_int[idx2];
        v3 = v3 ^ arr_int[idx3];
        v4 = v4 | arr_int[idx4];
        
        d0 = d0 + d5 * 0.5;
        d1 = d1 - arr_dbl[idx1] + arr_dbl[idx2];
        d2 = d2 * arr_dbl[idx3];
        d3 = d3 / (arr_dbl[idx4] + 1.0);
        d4 = d4 + temp_dbl;
        
        /* Use computed goto for non-trivial control flow */
        static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
        goto *labels[iter % 4];
        
        label0:
            v5 = v5 + arr_int[(iter * 89) % ARRAY_SIZE];
            goto after_labels;
        label1:
            v6 = v6 - arr_int[(iter * 97) % ARRAY_SIZE];
            goto after_labels;
        label2:
            v7 = v7 * arr_int[(iter * 101) % ARRAY_SIZE];
            goto after_labels;
        label3:
            v8 = v8 / (arr_int[(iter * 103) % ARRAY_SIZE] + 1);
            goto after_labels;
            
        after_labels:
        
        /* More helper calls with long long arguments */
        helper4(arr_ll[idx1], arr_ll[idx2], &vsink_ll, &vsink_ll);
        
        /* Volatile memory accesses with offset calculations */
        volatile int* volatile_ptr = (volatile int*)arr_int;
        for (int off = 0; off < 4; off++) {
            vsink_int += volatile_ptr[idx1 + off * 13];
            vsink_int -= volatile_ptr[idx2 + off * 17];
        }
    }
    
    /* Compute checksum */
    int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    checksum += (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    checksum += (int)vsink_int + (int)vsink_ll;
    
    return checksum;
}

int main() {
    /* Allocate and initialize large arrays */
    int* arr_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(ARRAY_SIZE * sizeof(double));
    long long* arr_ll = (long long*)malloc(ARRAY_SIZE * sizeof(long long));
    float* arr_flt = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!arr_int || !arr_dbl || !arr_ll || !arr_flt) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = i * 3 - i / 2;
        arr_dbl[i] = i * 1.5 - i / 3.0;
        arr_ll[i] = (long long)i * 7LL - i * 3LL;
        arr_flt[i] = i * 0.7f - i * 0.3f;
    }
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_ll, arr_flt);
    
    printf("Result checksum: %d\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_ll);
    free(arr_flt);
    
    return 0;
}
