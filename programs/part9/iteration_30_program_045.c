/* reload1_stress.c - Extreme register pressure test for GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define LOCAL_VARS 40

/* Opaque functions to prevent optimization */
int __attribute__((noinline)) use_int(int a, int b, int c, int d, int e) {
    volatile int sink = a + b - c + d - e;
    return sink;
}

double __attribute__((noinline)) use_double(double a, double b, double c, 
                                           double d, double e, double f) {
    volatile double sink = a * b - c * d + e - f;
    return sink;
}

void* __attribute__((noinline)) use_address(void* a, void* b, void* c, 
                                           int offset1, int offset2) {
    volatile char* sink1 = (char*)a + offset1;
    volatile char* sink2 = (char*)b + offset2;
    return (void*)((uintptr_t)sink1 + (uintptr_t)sink2);
}

long long __attribute__((noinline)) use_longlong(long long a, long long b,
                                                long long c, long long d) {
    volatile long long sink = a * b + c * d;
    return sink;
}

/* Complex addressing helper */
int __attribute__((noinline)) compute_index(int i, int j, int k, 
                                           int m, int n, int p) {
    return (i * 7 + j * 3 + k * 5 + m * 11 + n * 13 + p * 17) % SIZE;
}

/* Main stress function */
int __attribute__((noinline)) stress_reload(int* arr_int, double* arr_dbl,
                                          long long* arr_ll, float* arr_flt) {
    /* Declare many local variables to exhaust registers */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    long long ll0, ll1, ll2, ll3, ll4;
    float f0, f1, f2, f3, f4;
    
    /* Volatile pointers to force address computations */
    volatile int* volatile_ptr_int = arr_int;
    volatile double* volatile_ptr_dbl = arr_dbl;
    
    /* Initialize with complex patterns */
    for (int i = 0; i < 10; i++) {
        v0 = arr_int[i * 3];
        v1 = arr_int[i * 3 + 1];
        v2 = arr_int[i * 3 + 2];
        d0 = arr_dbl[i] * 2.0;
        d1 = arr_dbl[i + 1] / 3.0;
    }
    
    int checksum = 0;
    
    /* Main loop with extreme register pressure */
    for (int iter = 0; iter < 1000; iter++) {
        /* Complex index calculations - will need address registers */
        int idx1 = compute_index(iter, v0, v1, v2, v3, v4);
        int idx2 = compute_index(iter + 1, v5, v6, v7, v8, v9);
        int idx3 = compute_index(iter + 2, v10, v11, v12, v13, v14);
        int idx4 = compute_index(iter + 3, v15, v16, v17, v18, v19);
        int idx5 = compute_index(iter + 4, v20, v21, v22, v23, v24);
        
        /* Force RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
        /* by using addresses in multiple contexts */
        int* addr1 = &arr_int[idx1];
        int* addr2 = &arr_int[idx2];
        double* addr3 = &arr_dbl[idx3];
        double* addr4 = &arr_dbl[idx4];
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2;
        double dtemp1, dtemp2;
        
        /* Force RELOAD_FOR_INPUT: input value needs reload */
        asm volatile (
            "mov %[in1], %[out1]\n\t"
            "add %[in2], %[out1]\n\t"
            : [out1] "=r" (temp1)
            : [in1] "r" (*addr1), [in2] "r" (*addr2)
            : "cc"
        );
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS: output address needs reload */
        asm volatile (
            "mov %[val], (%[addr])\n\t"
            : 
            : [val] "r" (temp1), [addr] "r" (&arr_int[idx5])
            : "memory"
        );
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        /* by using same value as both data and address */
        int* complex_addr = &arr_int[(idx1 + idx2 * 3 + idx3 * 7) % SIZE];
        int complex_val = *complex_addr;
        
        asm volatile (
            "imul %[mul], %[val]\n\t"
            "mov %[val], (%[addr])\n\t"
            : [val] "+r" (complex_val)
            : [mul] "r" (iter), [addr] "r" (complex_addr)
            : "cc", "memory"
        );
        
        /* Control flow to split live ranges and trigger RELOAD_OTHER */
        switch (iter % 8) {
            case 0: {
                /* Block 0: compute addresses here, use in next block */
                int* branch_addr1 = &arr_int[(idx1 * 2 + 1) % SIZE];
                int* branch_addr2 = &arr_int[(idx2 * 3 + 2) % SIZE];
                double* branch_addr3 = &arr_dbl[(idx3 * 5 + 3) % SIZE];
                
                /* Values live across switch cases */
                v25 = *branch_addr1 + *branch_addr2;
                d2 = *branch_addr3 * 1.5;
                
                /* Call functions to force register shuffling */
                v26 = use_int(v25, *branch_addr1, *branch_addr2, iter, idx1);
                d3 = use_double(d2, *branch_addr3, d0, d1, 2.0, 3.0);
                
                /* Force RELOAD_FOR_OTHER_ADDRESS */
                void* addr_result = use_address(branch_addr1, branch_addr2, 
                                              branch_addr3, idx1, idx2);
                checksum += (int)(uintptr_t)addr_result;
                break;
            }
            case 1: {
                /* Block 1: use addresses computed in different paths */
                int* alt_addr1 = &arr_int[(idx4 * 7 + idx5 * 11) % SIZE];
                int* alt_addr2 = &arr_int[(idx5 * 13 + idx1 * 17) % SIZE];
                
                /* Complex addressing with multiple terms */
                int offset = (v25 * 3 + v26 * 7) % 256;
                volatile int* volatile_addr = volatile_ptr_int + offset;
                
                /* This will need various address reloads */
                v27 = *alt_addr1 + *alt_addr2 + *volatile_addr;
                
                /* More inline assembly with memory constraints */
                int result;
                asm volatile (
                    "mov (%[addr1]), %%eax\n\t"
                    "add (%[addr2]), %%eax\n\t"
                    "mov %%eax, %[res]\n\t"
                    : [res] "=r" (result)
                    : [addr1] "r" (alt_addr1), [addr2] "r" (alt_addr2)
                    : "%eax", "cc", "memory"
                );
                
                checksum += result;
                break;
            }
            case 2:
            case 3: {
                /* Mixed float/double operations for different reload types */
                float* faddr1 = &arr_flt[(idx1 * 5) % SIZE];
                float* faddr2 = &arr_flt[(idx2 * 7) % SIZE];
                
                f0 = *faddr1 * 2.0f;
                f1 = *faddr2 * 3.0f;
                
                /* Force spilling of float registers */
                for (int j = 0; j < 5; j++) {
                    f2 = f0 + f1 + j;
                    f3 = f1 - f0 - j;
                    f4 = f2 * f3;
                    checksum += (int)f4;
                }
                break;
            }
            case 4:
            case 5: {
                /* Long long operations - may need multiple registers */
                long long* lladdr1 = &arr_ll[(idx3 * 3) % SIZE];
                long long* lladdr2 = &arr_ll[(idx4 * 5) % SIZE];
                
                ll0 = *lladdr1;
                ll1 = *lladdr2;
                ll2 = ll0 * ll1;
                ll3 = ll1 / (iter + 1);
                ll4 = use_longlong(ll0, ll1, ll2, ll3);
                
                checksum += (int)ll4;
                break;
            }
            default: {
                /* Default case with pointer arithmetic */
                int base_idx = (iter * 17) % SIZE;
                int stride = 3;
                
                /* Force RELOAD_FOR_OUTADDR_ADDRESS */
                for (int j = 0; j < 4; j++) {
                    int* elem_addr = &arr_int[base_idx + j * stride];
                    int* next_addr = &arr_int[base_idx + (j + 1) * stride];
                    
                    /* Chain of address-dependent computations */
                    *elem_addr = *next_addr + j;
                    checksum += *elem_addr;
                }
                break;
            }
        }
        
        /* Update many variables to keep them live across iterations */
        v0 = v0 + v1 - v2;
        v1 = v1 * 2 - v3;
        v2 = v2 + v4 / 3;
        v3 = v3 ^ v5;
        v4 = v4 + v6;
        v5 = v5 - v7;
        v6 = v6 * v8;
        v7 = v7 + v9;
        v8 = v8 ^ v10;
        v9 = v9 - v11;
        
        d0 = d0 * 1.01;
        d1 = d1 / 1.01;
        d2 = d2 + d3;
        d3 = d3 - d4;
        d4 = d4 * d5;
        
        /* Volatile store to prevent dead code elimination */
        volatile int vol_sink = checksum;
        (void)vol_sink;
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays with pattern data */
    int* arr_int = malloc(SIZE * sizeof(int));
    double* arr_dbl = malloc(SIZE * sizeof(double));
    long long* arr_ll = malloc(SIZE * sizeof(long long));
    float* arr_flt = malloc(SIZE * sizeof(float));
    
    if (!arr_int || !arr_dbl || !arr_ll || !arr_flt) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic pattern */
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = (i * 37 + 123) % 1000;
        arr_dbl[i] = (i * 51 + 456) % 1000 / 3.14159;
        arr_ll[i] = (long long)i * i * 7919;
        arr_flt[i] = (i * 83 + 789) % 1000 / 2.71828f;
    }
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_ll, arr_flt);
    
    printf("Reload stress test checksum: %d\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_ll);
    free(arr_flt);
    
    return 0;
}
