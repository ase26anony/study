/* reload1_trigger.c - Program to trigger uncovered reload types in reload1.cc */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Opaque functions to prevent optimization */
#define NOINLINE __attribute__((noinline))

NOINLINE int use_int(int x) { return x ^ 0x55AA55AA; }
NOINLINE float use_float(float x) { return x * 1.5f; }
NOINLINE double use_double(double x) { return x * 2.5; }
NOINLINE long long use_ll(long long x) { return x + 0x123456789ABCDEFLL; }
NOINLINE void* use_ptr(void* p) { return (void*)((uintptr_t)p + 1); }
NOINLINE int use_two(int a, int b) { return a + b * 3; }
NOINLINE double use_three(double a, double b, double c) { return a + b - c; }

/* Global volatile to prevent dead code elimination */
volatile int global_sink = 0;

/* Main stress function */
NOINLINE int stress_reload(int* arr_int, double* arr_dbl, 
                           float* arr_flt, long long* arr_ll,
                           int size) {
    /* Declare many local variables to exhaust registers */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
    volatile double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    volatile long long ll0, ll1, ll2, ll3, ll4, ll5;
    volatile int* ptr0, *ptr1, *ptr2, *ptr3;
    volatile double* dptr0, *dptr1, *dptr2;
    
    int result = 0;
    int i, j, k;
    
    /* Initialize with complex expressions */
    v0 = arr_int[0];
    v1 = arr_int[1];
    v2 = arr_int[2];
    v3 = arr_int[3];
    v4 = arr_int[4];
    v5 = arr_int[5];
    v6 = arr_int[6];
    v7 = arr_int[7];
    v8 = arr_int[8];
    v9 = arr_int[9];
    
    f0 = arr_flt[0];
    f1 = arr_flt[1];
    f2 = arr_flt[2];
    f3 = arr_flt[3];
    f4 = arr_flt[4];
    f5 = arr_flt[5];
    f6 = arr_flt[6];
    f7 = arr_flt[7];
    f8 = arr_flt[8];
    f9 = arr_flt[9];
    
    d0 = arr_dbl[0];
    d1 = arr_dbl[1];
    d2 = arr_dbl[2];
    d3 = arr_dbl[3];
    d4 = arr_dbl[4];
    d5 = arr_dbl[5];
    d6 = arr_dbl[6];
    d7 = arr_dbl[7];
    d8 = arr_dbl[8];
    d9 = arr_dbl[9];
    
    ll0 = arr_ll[0];
    ll1 = arr_ll[1];
    ll2 = arr_ll[2];
    ll3 = arr_ll[3];
    ll4 = arr_ll[4];
    ll5 = arr_ll[5];
    
    /* Take addresses of locals to force stack addressing */
    ptr0 = &v0;
    ptr1 = &v1;
    ptr2 = &v2;
    ptr3 = &v3;
    dptr0 = &d0;
    dptr1 = &d1;
    dptr2 = &d2;
    
    /* Main loop with extreme register pressure */
    for (i = 0; i < 1000; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v0 * 3 + v1) % size;
        int idx2 = (i * 11 + v2 * 5 + v3) % size;
        int idx3 = (i * 13 + v4 * 7 + v5) % size;
        int idx4 = (i * 17 + v6 * 11 + v7) % size;
        int idx5 = (i * 19 + v8 * 13 + v9) % size;
        
        /* Volatile memory accesses with offset calculations */
        volatile int* addr1 = &arr_int[idx1];
        volatile int* addr2 = &arr_int[idx2];
        volatile double* addr3 = &arr_dbl[idx3];
        volatile float* addr4 = &arr_flt[idx4];
        volatile long long* addr5 = &arr_ll[idx5];
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2;
        double dtemp1, dtemp2;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "mov %[val1], %[tmp1]\n\t"
            "add %[val2], %[tmp1]\n\t"
            : [tmp1] "=r" (temp1)
            : [val1] "r" (*addr1), [val2] "r" (v0)
            : "cc"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS */
        asm volatile (
            "mov %[addr], %%rsi\n\t"
            "mov (%%rsi), %[out]\n\t"
            : [out] "=r" (temp2)
            : [addr] "r" (addr2)
            : "rsi", "memory"
        );
        
        /* Mixed types causing different reload categories */
        dtemp1 = use_double(*addr3);
        dtemp2 = use_three(d0, d1, *addr3);
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* RELOAD_FOR_INPADDR_ADDRESS */
                int* complex_addr = &arr_int[(idx1 + idx2 * 3) % size];
                v0 = use_int(*complex_addr);
                v1 = use_two(v0, *complex_addr);
                break;
            }
            case 1: {
                /* RELOAD_FOR_OUTADDR_ADDRESS */
                double* dcomplex_addr = &arr_dbl[(idx3 * 2 + idx4) % size];
                *dcomplex_addr = dtemp1;
                d0 = use_double(*dcomplex_addr);
                break;
            }
            case 2: {
                /* RELOAD_FOR_OPERAND_ADDRESS */
                float* faddr = &arr_flt[(idx4 + i * 5) % size];
                f0 = use_float(*faddr);
                f1 = use_float(f0 + *faddr);
                break;
            }
            case 3: {
                /* RELOAD_FOR_OPADDR_ADDR */
                long long* lladdr = &arr_ll[(idx5 + v0) % size];
                ll0 = use_ll(*lladdr);
                ll1 = use_ll(ll0 ^ *lladdr);
                break;
            }
            case 4: {
                /* RELOAD_FOR_OTHER_ADDRESS */
                int* other_addr = &arr_int[(i * 23 + v1 * 17) % size];
                v2 = use_int(*other_addr);
                v3 = use_two(v2, *other_addr);
                v4 = use_two(v3, *other_addr);
                break;
            }
            case 5: {
                /* RELOAD_OTHER through complex control flow */
                int* addr_a = &arr_int[idx1];
                int* addr_b = &arr_int[idx2];
                if (*addr_a > *addr_b) {
                    v5 = use_int(*addr_a);
                    v6 = use_int(*addr_b);
                } else {
                    v5 = use_int(*addr_b);
                    v6 = use_int(*addr_a);
                }
                break;
            }
            case 6: {
                /* Multiple address computations in one block */
                int* addr_x = &arr_int[(v7 * 3 + i) % size];
                int* addr_y = &arr_int[(v8 * 5 + i * 2) % size];
                int* addr_z = &arr_int[(v9 * 7 + i * 3) % size];
                v7 = use_int(*addr_x);
                v8 = use_int(*addr_y);
                v9 = use_int(*addr_z);
                break;
            }
            case 7: {
                /* Mixed address and data reloads */
                double* daddr1 = &arr_dbl[idx3];
                double* daddr2 = &arr_dbl[idx4 % size];
                d0 = *daddr1 + *daddr2;
                d1 = use_double(d0);
                d2 = use_double(*daddr1);
                break;
            }
        }
        
        /* Call multiple functions to force register shuffling */
        v0 = use_int(v0);
        v1 = use_two(v0, v1);
        v2 = use_two(v1, v2);
        v3 = use_two(v2, v3);
        
        f0 = use_float(f0);
        f1 = use_float(f1);
        f2 = use_float(f2);
        
        d0 = use_double(d0);
        d1 = use_double(d1);
        d2 = use_double(d2);
        d3 = use_three(d0, d1, d2);
        
        ll0 = use_ll(ll0);
        ll1 = use_ll(ll1);
        
        /* Update pointers with complex addressing */
        ptr0 = use_ptr((void*)ptr0);
        ptr1 = use_ptr((void*)ptr1);
        dptr0 = use_ptr((void*)dptr0);
        
        /* Update variables to keep them live */
        v0 = v0 + temp1;
        v1 = v1 + temp2;
        v2 = v2 + *addr1;
        v3 = v3 + *addr2;
        v4 = v4 + i;
        v5 = v5 + idx1;
        v6 = v6 + idx2;
        v7 = v7 + idx3;
        v8 = v8 + idx4;
        v9 = v9 + idx5;
        
        f0 = f0 + *addr4;
        f1 = f1 + f0;
        f2 = f2 + f1;
        
        d0 = d0 + *addr3;
        d1 = d1 + d0;
        d2 = d2 + d1;
        
        ll0 = ll0 + *addr5;
        ll1 = ll1 + ll0;
        
        /* Accumulate result */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        result += (int)f0 + (int)f1 + (int)f2;
        result += (int)d0 + (int)d1 + (int)d2;
        result += (int)ll0 + (int)ll1;
    }
    
    /* Final sink to prevent optimization */
    global_sink = result;
    return result;
}

int main() {
    const int SIZE = 10000;
    
    /* Allocate and initialize arrays with pattern data */
    int* arr_int = (int*)malloc(SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(SIZE * sizeof(double));
    float* arr_flt = (float*)malloc(SIZE * sizeof(float));
    long long* arr_ll = (long long*)malloc(SIZE * sizeof(long long));
    
    if (!arr_int || !arr_dbl || !arr_flt || !arr_ll) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = i * 3 + 1;
        arr_dbl[i] = i * 1.5 + 2.0;
        arr_flt[i] = i * 0.7f + 1.0f;
        arr_ll[i] = i * 5LL + 3LL;
    }
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_flt, arr_ll, SIZE);
    
    printf("Result: %d\n", result);
    printf("Global sink: %d\n", global_sink);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_flt);
    free(arr_ll);
    
    return 0;
}
