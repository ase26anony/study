/* reload_stress.c - Extreme register pressure test for GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Opaque helper functions to prevent optimization */
NOINLINE int helper1(int a, int b, int c, int d, int e, int f);
NOINLINE float helper2(float a, float b, float c, float d, float e);
NOINLINE double helper3(double a, double b, double c, double* addr);
NOINLINE long helper4(long a, long b, long* addr1, long* addr2);
NOINLINE void helper5(int* addr1, double* addr2, float* addr3);

/* Implementation of helpers (prevents inlining) */
NOINLINE int helper1(int a, int b, int c, int d, int e, int f) {
    VOLATILE_VAR int sink = 0;
    sink += a * b;
    sink += c * d;
    sink += e * f;
    return sink;
}

NOINLINE float helper2(float a, float b, float c, float d, float e) {
    VOLATILE_VAR float sink = 0.0f;
    sink += a * b;
    sink += c * d;
    sink += e;
    return sink;
}

NOINLINE double helper3(double a, double b, double c, double* addr) {
    VOLATILE_VAR double sink = 0.0;
    sink += a * b;
    sink += c * (*addr);
    if (addr) sink += 1.0;
    return sink;
}

NOINLINE long helper4(long a, long b, long* addr1, long* addr2) {
    VOLATILE_VAR long sink = 0;
    sink += a * b;
    if (addr1) sink += *addr1;
    if (addr2) sink += *addr2;
    return sink;
}

NOINLINE void helper5(int* addr1, double* addr2, float* addr3) {
    VOLATILE_VAR int sink1 = 0;
    VOLATILE_VAR double sink2 = 0.0;
    VOLATILE_VAR float sink3 = 0.0f;
    
    if (addr1) sink1 = *addr1;
    if (addr2) sink2 = *addr2;
    if (addr3) sink3 = *addr3;
    
    /* Force memory side effects */
    if (addr1) *addr1 = sink1 + 1;
    if (addr2) *addr2 = sink2 + 1.0;
    if (addr3) *addr3 = sink3 + 1.0f;
}

/* Main stress function */
NOINLINE int stress_reload(int* arr_int, double* arr_dbl, float* arr_flt, 
                           long* arr_long, int size) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Volatile sinks to prevent optimization */
    VOLATILE_VAR int result = 0;
    VOLATILE_VAR double dresult = 0.0;
    VOLATILE_VAR float fresult = 0.0f;
    VOLATILE_VAR long lresult = 0;
    
    /* Initialize with complex expressions */
    v1 = arr_int[0] * 3;
    v2 = arr_int[1] + v1;
    v3 = arr_int[2] - v2;
    v4 = arr_int[3] | v3;
    v5 = arr_int[4] & v4;
    v6 = arr_int[5] ^ v5;
    v7 = arr_int[6] + v6;
    v8 = arr_int[7] - v7;
    v9 = arr_int[8] * v8;
    v10 = arr_int[9] / (v9 ? v9 : 1);
    
    f1 = arr_flt[0] * 1.5f;
    f2 = arr_flt[1] + f1;
    f3 = arr_flt[2] - f2;
    f4 = arr_flt[3] * f3;
    f5 = arr_flt[4] / (f4 ? f4 : 1.0f);
    
    d1 = arr_dbl[0] * 2.5;
    d2 = arr_dbl[1] + d1;
    d3 = arr_dbl[2] - d2;
    d4 = arr_dbl[3] * d3;
    d5 = arr_dbl[4] / (d4 ? d4 : 1.0);
    
    l1 = arr_long[0] * 7;
    l2 = arr_long[1] + l1;
    l3 = arr_long[2] - l2;
    l4 = arr_long[3] | l3;
    l5 = arr_long[4] & l4;
    
    /* More variables */
    v11 = v1 + v2; v12 = v3 + v4; v13 = v5 + v6; v14 = v7 + v8; v15 = v9 + v10;
    v16 = v11 * v12; v17 = v13 * v14; v18 = v15 * v16; v19 = v17 * v18; v20 = v19 + v1;
    
    f6 = f1 + f2; f7 = f3 + f4; f8 = f5 * f6; f9 = f7 * f8; f10 = f9 / 2.0f;
    
    d6 = d1 + d2; d7 = d3 + d4; d8 = d5 * d6; d9 = d7 * d8; d10 = d9 / 2.0;
    
    l6 = l1 + l2; l7 = l3 + l4; l8 = l5 * l6; l9 = l7 * l8; l10 = l9 >> 2;
    
    /* Complex loop with extreme register pressure */
    for (int i = 0; i < 1000; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % size;
        int idx2 = (i * 11 + v3 * 2 + v4 * 13) % size;
        int idx3 = (i * 17 + v5 * 7 + v6 * 19) % size;
        int idx4 = (i * 23 + v7 * 11 + v8 * 29) % size;
        int idx5 = (i * 31 + v9 * 13 + v10 * 37) % size;
        
        /* Address computations that need registers */
        int* addr1 = &arr_int[idx1];
        double* addr2 = &arr_dbl[idx2];
        float* addr3 = &arr_flt[idx3];
        long* addr4 = &arr_long[idx4];
        int* addr5 = &arr_int[idx5];
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2;
        double dtemp1, dtemp2;
        long ltemp1, ltemp2;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "mov %[val1], %[out1]\n\t"
            "add %[val2], %[out1]\n\t"
            : [out1] "=r" (temp1)
            : [val1] "r" (*addr1), [val2] "r" (v1)
            : "cc"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS */
        asm volatile (
            "mov %[in1], (%[addr])\n\t"
            : 
            : [in1] "r" (temp1), [addr] "r" (addr5)
            : "memory"
        );
        
        /* More complex addressing with multiple constraints */
        asm volatile (
            "imul %[a], %[b]\n\t"
            "add %%rax, %[out]\n\t"
            : [out] "+r" (ltemp1)
            : [a] "r" (l1), [b] "r" (l2)
            : "rax", "cc"
        );
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* RELOAD_FOR_INPADDR_ADDRESS */
                int idx6 = (idx1 * 3 + idx2 * 5) % size;
                int* addr6 = &arr_int[idx6];
                asm volatile (
                    "mov (%[addr]), %%eax\n\t"
                    "add %%eax, %[out]\n\t"
                    : [out] "+r" (temp2)
                    : [addr] "r" (addr6)
                    : "eax", "cc"
                );
                v1 = temp2;
                break;
            }
            case 1: {
                /* RELOAD_FOR_OPERAND_ADDRESS */
                double* addr7 = &arr_dbl[(idx3 * 7 + idx4 * 11) % size];
                dtemp1 = *addr7;
                asm volatile (
                    "addsd %[in], %[out]\n\t"
                    : [out] "+x" (dresult)
                    : [in] "x" (dtemp1)
                    : 
                );
                break;
            }
            case 2: {
                /* RELOAD_FOR_OPADDR_ADDR */
                long* addr8 = &arr_long[(idx2 * 13 + idx5 * 17) % size];
                asm volatile (
                    "mov (%[addr]), %%rax\n\t"
                    "add %%rax, %[out]\n\t"
                    : [out] "+r" (lresult)
                    : [addr] "r" (addr8)
                    : "rax", "cc"
                );
                break;
            }
            case 3: {
                /* RELOAD_FOR_OTHER_ADDRESS */
                float* addr9 = &arr_flt[(idx1 * 19 + idx3 * 23) % size];
                float ftemp;
                asm volatile (
                    "movss (%[addr]), %[out]\n\t"
                    : [out] "=x" (ftemp)
                    : [addr] "r" (addr9)
                    : 
                );
                fresult += ftemp;
                break;
            }
            default: {
                /* RELOAD_OTHER cases */
                int idx10 = (i * 41 + v11 * 43) % size;
                int* addr10 = &arr_int[idx10];
                *addr10 = v12 + v13;
                break;
            }
        }
        
        /* Call helper functions with many arguments to force register shuffling */
        v11 = helper1(v1, v2, v3, v4, v5, v6);
        fresult += helper2(f1, f2, f3, f4, f5);
        dresult += helper3(d1, d2, d3, addr2);
        lresult += helper4(l1, l2, addr4, addr5);
        helper5(addr1, addr2, addr3);
        
        /* Update local variables to keep them live */
        v1 = v1 + arr_int[idx1];
        v2 = v2 ^ arr_int[idx2];
        v3 = v3 * (arr_int[idx3] ? arr_int[idx3] : 1);
        v4 = v4 | arr_int[idx4];
        v5 = v5 & arr_int[idx5];
        
        f1 = f1 + arr_flt[idx1 % size];
        f2 = f2 * arr_flt[idx2 % size];
        f3 = f3 - arr_flt[idx3 % size];
        
        d1 = d1 + arr_dbl[idx1 % size];
        d2 = d2 * arr_dbl[idx2 % size];
        d3 = d3 - arr_dbl[idx3 % size];
        
        l1 = l1 + arr_long[idx1 % size];
        l2 = l2 ^ arr_long[idx2 % size];
        l3 = l3 * (arr_long[idx3 % size] ? arr_long[idx3 % size] : 1);
        
        /* More complex updates with addressing */
        int idx6 = (i * 47 + v6 * 53) % size;
        v6 = v6 + arr_int[idx6];
        v7 = v7 - arr_int[(idx6 * 2) % size];
        v8 = v8 * arr_int[(idx6 * 3) % size];
        v9 = v9 / (arr_int[(idx6 * 5) % size] ? arr_int[(idx6 * 5) % size] : 1);
        v10 = v10 % (arr_int[(idx6 * 7) % size] ? arr_int[(idx6 * 7) % size] : 1);
        
        /* Force spill/reload around loop */
        if (i % 100 == 0) {
            /* Take addresses of locals to force stack-based reloads */
            int* pv1 = &v1;
            float* pf1 = &f1;
            double* pd1 = &d1;
            long* pl1 = &l1;
            
            *pv1 = *pv1 + 1;
            *pf1 = *pf1 + 1.0f;
            *pd1 = *pd1 + 1.0;
            *pl1 = *pl1 + 1;
        }
    }
    
    /* Final computation to use all variables */
    result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
             v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    result += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
              (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10;
    
    result += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 +
              (int)d6 + (int)d7 + (int)d8 + (int)d9 + (int)d10;
    
    result += (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5 +
              (int)l6 + (int)l7 + (int)l8 + (int)l9 + (int)l10;
    
    result += (int)result + (int)dresult + (int)fresult + (int)lresult;
    
    return result;
}

int main() {
    const int SIZE = 10000;
    
    /* Allocate and initialize arrays with pattern data */
    int* arr_int = (int*)malloc(SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(SIZE * sizeof(double));
    float* arr_flt = (float*)malloc(SIZE * sizeof(float));
    long* arr_long = (long*)malloc(SIZE * sizeof(long));
    
    if (!arr_int || !arr_dbl || !arr_flt || !arr_long) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = i * 3 + 1;
        arr_dbl[i] = i * 1.5 + 0.5;
        arr_flt[i] = i * 0.7f + 0.3f;
        arr_long[i] = i * 7L + 3L;
    }
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_flt, arr_long, SIZE);
    
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_flt);
    free(arr_long);
    
    return 0;
}
