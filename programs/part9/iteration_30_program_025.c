/* reload_stress_test.c - Extreme register pressure test for GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Opaque functions to prevent optimization */
NOINLINE int func1(int a, int b, int c, int d, int e, int f);
NOINLINE double func2(double a, double b, double c, double d);
NOINLINE void* func3(void* p1, void* p2, void* p3);
NOINLINE long long func4(long long a, long long b, long long c);
NOINLINE float func5(float a, float b, float c, float d, float e, float f);

/* Implementation of opaque functions */
NOINLINE int func1(int a, int b, int c, int d, int e, int f) {
    VOLATILE_VAR int sink = a + b - c * d + e / (f ? f : 1);
    return sink;
}

NOINLINE double func2(double a, double b, double c, double d) {
    VOLATILE_VAR double sink = a * b - c / d;
    return sink;
}

NOINLINE void* func3(void* p1, void* p2, void* p3) {
    VOLATILE_VAR char sink = ((char*)p1)[0] + ((char*)p2)[0] + ((char*)p3)[0];
    return (void*)((uintptr_t)p1 + (uintptr_t)p2 + (uintptr_t)p3 + sink);
}

NOINLINE long long func4(long long a, long long b, long long c) {
    VOLATILE_VAR long long sink = a ^ b ^ c;
    return sink;
}

NOINLINE float func5(float a, float b, float c, float d, float e, float f) {
    VOLATILE_VAR float sink = a + b - c * d + e / f;
    return sink;
}

NOINLINE unsigned long stress_reload(int* arr_int, double* arr_dbl, 
                                     long long* arr_ll, float* arr_flt,
                                     int size) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    long long ll1, ll2, ll3, ll4, ll5, ll6, ll7, ll8, ll9, ll10;
    
    /* Initialize with complex expressions */
    v1 = arr_int[0] * 3;
    v2 = arr_int[1] + v1;
    v3 = arr_int[2] - v2;
    v4 = arr_int[3] ^ v3;
    v5 = arr_int[4] | v4;
    v6 = arr_int[5] & v5;
    v7 = arr_int[6] + v6;
    v8 = arr_int[7] - v7;
    v9 = arr_int[8] * v8;
    v10 = arr_int[9] / (v9 ? v9 : 1);
    
    v11 = v1 + v2 - v3;
    v12 = v4 * v5 + v6;
    v13 = v7 ^ v8 | v9;
    v14 = v10 & v11 * v12;
    v15 = v13 - v14 + v1;
    v16 = v2 * v3 / (v4 ? v4 : 1);
    v17 = v5 + v6 - v7;
    v18 = v8 ^ v9 & v10;
    v19 = v11 | v12 * v13;
    v20 = v14 - v15 + v16;
    
    v21 = v17 * v18 + v19;
    v22 = v20 ^ v1 & v2;
    v23 = v3 | v4 * v5;
    v24 = v6 - v7 + v8;
    v25 = v9 ^ v10 & v11;
    v26 = v12 | v13 * v14;
    v27 = v15 - v16 + v17;
    v28 = v18 ^ v19 & v20;
    v29 = v21 | v22 * v23;
    v30 = v24 - v25 + v26;
    
    d1 = arr_dbl[0] * 2.5;
    d2 = arr_dbl[1] + d1;
    d3 = arr_dbl[2] - d2;
    d4 = arr_dbl[3] * d3;
    d5 = arr_dbl[4] / d4;
    d6 = arr_dbl[5] + d5;
    d7 = arr_dbl[6] - d6;
    d8 = arr_dbl[7] * d7;
    d9 = arr_dbl[8] / d8;
    d10 = arr_dbl[9] + d9;
    
    f1 = arr_flt[0] * 1.5f;
    f2 = arr_flt[1] + f1;
    f3 = arr_flt[2] - f2;
    f4 = arr_flt[3] * f3;
    f5 = arr_flt[4] / f4;
    f6 = arr_flt[5] + f5;
    f7 = arr_flt[6] - f6;
    f8 = arr_flt[7] * f7;
    f9 = arr_flt[8] / f8;
    f10 = arr_flt[9] + f9;
    
    ll1 = arr_ll[0] * 7LL;
    ll2 = arr_ll[1] + ll1;
    ll3 = arr_ll[2] - ll2;
    ll4 = arr_ll[3] ^ ll3;
    ll5 = arr_ll[4] | ll4;
    ll6 = arr_ll[5] & ll5;
    ll7 = arr_ll[6] + ll6;
    ll8 = arr_ll[7] - ll7;
    ll9 = arr_ll[8] * ll8;
    ll10 = arr_ll[9] / (ll9 ? ll9 : 1LL);
    
    VOLATILE_VAR unsigned long checksum = 0;
    VOLATILE_VAR int* volatile ptr1;
    VOLATILE_VAR double* volatile ptr2;
    VOLATILE_VAR float* volatile ptr3;
    VOLATILE_VAR long long* volatile ptr4;
    
    /* Complex loop with extreme register pressure */
    for (int i = 0; i < 1000; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % size;
        int idx2 = (i * 11 + v3 * 13 + v4 * 17) % size;
        int idx3 = (i * 19 + v5 * 23 + v6 * 29) % size;
        int idx4 = (i * 31 + v7 * 37 + v8 * 41) % size;
        int idx5 = (i * 43 + v9 * 47 + v10 * 53) % size;
        int idx6 = (i * 59 + v11 * 61 + v12 * 67) % size;
        int idx7 = (i * 71 + v13 * 73 + v14 * 79) % size;
        int idx8 = (i * 83 + v15 * 89 + v16 * 97) % size;
        
        /* Address computations that need registers */
        int* addr1 = &arr_int[idx1];
        int* addr2 = &arr_int[idx2];
        double* addr3 = &arr_dbl[idx3 % (size/2)];
        double* addr4 = &arr_dbl[idx4 % (size/2)];
        float* addr5 = &arr_flt[idx5 % (size/2)];
        float* addr6 = &arr_flt[idx6 % (size/2)];
        long long* addr7 = &arr_ll[idx7 % (size/2)];
        long long* addr8 = &arr_ll[idx8 % (size/2)];
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2;
        double dtemp1, dtemp2;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "mov %[val1], %[tmp1]\n\t"
            "add %[val2], %[tmp1]\n\t"
            "mov %[tmp1], %[out1]\n\t"
            : [out1] "=r" (temp1)
            : [val1] "r" (*addr1), [val2] "r" (*addr2), [tmp1] "r" (0)
            : "memory"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS */
        asm volatile (
            "mov %[addr], %%rsi\n\t"
            "mov (%%rsi), %[out]\n\t"
            : [out] "=r" (temp2)
            : [addr] "r" (addr3)
            : "rsi", "memory"
        );
        
        /* Mixed constraints causing various reload types */
        asm volatile (
            "movsd (%[src]), %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, (%[dst])\n\t"
            : 
            : [src] "r" (addr4), [dst] "r" (&dtemp1)
            : "xmm0", "memory"
        );
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Address computation in one branch, use in another */
                int* complex_addr = &arr_int[(v1 * i + v2 * 3) % size];
                ptr1 = complex_addr;
                /* Force RELOAD_FOR_OTHER_ADDRESS */
                checksum += (unsigned long)complex_addr;
                break;
            }
            case 1: {
                double* dbl_addr = &arr_dbl[(v3 * i + v4 * 5) % (size/2)];
                ptr2 = dbl_addr;
                /* Multiple address computations */
                float* flt_addr1 = &arr_flt[(v5 * i + v6 * 7) % (size/2)];
                float* flt_addr2 = &arr_flt[(v7 * i + v8 * 11) % (size/2)];
                ptr3 = flt_addr1;
                checksum += (unsigned long)flt_addr2;
                break;
            }
            case 2: {
                /* RELOAD_FOR_OPERAND_ADDRESS */
                long long* ll_addr = &arr_ll[(v9 * i + v10 * 13) % (size/2)];
                ptr4 = ll_addr;
                /* Use computed goto for non-trivial control flow */
                void* target = &&label2;
                goto *target;
            label2:
                checksum += *ll_addr;
                break;
            }
            case 3:
            case 4:
            case 5: {
                /* Multiple address reloads in same block */
                int idx_a = (v11 * i + v12 * 17) % size;
                int idx_b = (v13 * i + v14 * 19) % size;
                int idx_c = (v15 * i + v16 * 23) % size;
                
                int* a1 = &arr_int[idx_a];
                int* a2 = &arr_int[idx_b];
                int* a3 = &arr_int[idx_c];
                
                /* Force different reload types */
                asm volatile (
                    "mov (%[a1]), %%eax\n\t"
                    "add (%[a2]), %%eax\n\t"
                    "mov %%eax, (%[a3])\n\t"
                    : 
                    : [a1] "r" (a1), [a2] "r" (a2), [a3] "r" (a3)
                    : "eax", "memory"
                );
                break;
            }
            default: {
                /* RELOAD_OTHER cases */
                int* other_addr = &arr_int[(v17 * i + v18 * 29) % size];
                /* Complex expression requiring temporary */
                int other_val = *other_addr + v19 * v20 - v21 / (v22 ? v22 : 1);
                checksum += other_val;
                break;
            }
        }
        
        /* Call noinline functions with many arguments to force register shuffling */
        v1 = func1(v1, v2, v3, v4, v5, v6);
        d1 = func2(d1, d2, d3, d4);
        ptr1 = func3(ptr1, (void*)&v7, (void*)&v8);
        ll1 = func4(ll1, ll2, ll3);
        f1 = func5(f1, f2, f3, f4, f5, f6);
        
        /* Update all variables to keep them live */
        v1 = v1 + arr_int[idx1 % size];
        v2 = v2 ^ arr_int[idx2 % size];
        v3 = v3 | arr_int[idx3 % size];
        v4 = v4 & arr_int[idx4 % size];
        v5 = v5 + arr_int[idx5 % size];
        v6 = v6 - arr_int[idx6 % size];
        v7 = v7 * arr_int[idx7 % size];
        v8 = v8 / (arr_int[idx8 % size] ? arr_int[idx8 % size] : 1);
        
        v9 = v9 + v1;
        v10 = v10 - v2;
        v11 = v11 * v3;
        v12 = v12 / (v4 ? v4 : 1);
        v13 = v13 ^ v5;
        v14 = v14 | v6;
        v15 = v15 & v7;
        v16 = v16 + v8;
        v17 = v17 - v9;
        v18 = v18 * v10;
        v19 = v19 / (v11 ? v11 : 1);
        v20 = v20 ^ v12;
        
        v21 = v21 | v13;
        v22 = v22 & v14;
        v23 = v23 + v15;
        v24 = v24 - v16;
        v25 = v25 * v17;
        v26 = v26 / (v18 ? v18 : 1);
        v27 = v27 ^ v19;
        v28 = v28 | v20;
        v29 = v29 & v21;
        v30 = v30 + v22;
        
        d1 = d1 + arr_dbl[idx1 % (size/2)];
        d2 = d2 - arr_dbl[idx2 % (size/2)];
        d3 = d3 * arr_dbl[idx3 % (size/2)];
        d4 = d4 / arr_dbl[idx4 % (size/2)];
        d5 = d5 + arr_dbl[idx5 % (size/2)];
        d6 = d6 - arr_dbl[idx6 % (size/2)];
        d7 = d7 * arr_dbl[idx7 % (size/2)];
        d8 = d8 / arr_dbl[idx8 % (size/2)];
        
        f1 = f1 + arr_flt[idx1 % (size/2)];
        f2 = f2 - arr_flt[idx2 % (size/2)];
        f3 = f3 * arr_flt[idx3 % (size/2)];
        f4 = f4 / arr_flt[idx4 % (size/2)];
        f5 = f5 + arr_flt[idx5 % (size/2)];
        f6 = f6 - arr_flt[idx6 % (size/2)];
        f7 = f7 * arr_flt[idx7 % (size/2)];
        f8 = f8 / arr_flt[idx8 % (size/2)];
        
        ll1 = ll1 + arr_ll[idx1 % (size/2)];
        ll2 = ll2 ^ arr_ll[idx2 % (size/2)];
        ll3 = ll3 | arr_ll[idx3 % (size/2)];
        ll4 = ll4 & arr_ll[idx4 % (size/2)];
        ll5 = ll5 + arr_ll[idx5 % (size/2)];
        ll6 = ll6 - arr_ll[idx6 % (size/2)];
        ll7 = ll7 * arr_ll[idx7 % (size/2)];
        ll8 = ll8 / (arr_ll[idx8 % (size/2)] ? arr_ll[idx8 % (size/2)] : 1LL);
        
        checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                   (unsigned long)d1 + (unsigned long)d2 + (unsigned long)d3 +
                   (unsigned long)d4 + (unsigned long)d5 + (unsigned long)d6 +
                   (unsigned long)d7 + (unsigned long)d8 + (unsigned long)d9 +
                   (unsigned long)d10 + (unsigned long)f1 + (unsigned long)f2 +
                   (unsigned long)f3 + (unsigned long)f4 + (unsigned long)f5 +
                   (unsigned long)f6 + (unsigned long)f7 + (unsigned long)f8 +
                   (unsigned long)f9 + (unsigned long)f10 + (unsigned long)ll1 +
                   (unsigned long)ll2 + (unsigned long)ll3 + (unsigned long)ll4 +
                   (unsigned long)ll5 + (unsigned long)ll6 + (unsigned long)ll7 +
                   (unsigned long)ll8 + (unsigned long)ll9 + (unsigned long)ll10;
    }
    
    return checksum;
}

int main() {
    const int SIZE = 1000;
    
    /* Allocate and initialize arrays with pattern data */
    int* arr_int = (int*)malloc(SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(SIZE * sizeof(double));
    long long* arr_ll = (long long*)malloc(SIZE * sizeof(long long));
    float* arr_flt = (float*)malloc(SIZE * sizeof(float));
    
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = i * 3 - 7;
        arr_dbl[i] = i * 1.5 - 3.14;
        arr_ll[i] = i * 7LL - 42LL;
        arr_flt[i] = i * 0.7f - 1.618f;
    }
    
    /* Call the stress function */
    unsigned long result = stress_reload(arr_int, arr_dbl, arr_ll, arr_flt, SIZE);
    
    printf("Checksum: %lu\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_ll);
    free(arr_flt);
    
    return 0;
}
