/* reload1_stress.c - Stress test for GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define NUM_LOCALS 40

/* Opaque functions to prevent optimization */
int __attribute__((noinline)) use_int(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

double __attribute__((noinline)) use_double(double a, double b, double c, 
                                           double d, double e) {
    volatile double sink = a * b + c - d / (e + 1.0);
    return sink;
}

void* __attribute__((noinline)) use_address(void* a, void* b, void* c, 
                                           int offset1, int offset2) {
    volatile char* sink1 = (char*)a + offset1;
    volatile char* sink2 = (char*)b + offset2;
    volatile char* sink3 = (char*)c;
    return (void*)(sink1 + (intptr_t)sink2 + (intptr_t)sink3);
}

int __attribute__((noinline)) use_mixed(int a, double b, long c, 
                                       float d, int* e, double* f) {
    volatile int sink = a + (int)b + (int)c + (int)d + *e + (int)*f;
    return sink;
}

/* Main stress function */
int __attribute__((noinline, noipa))
stress_reload(int* arr_int, double* arr_dbl, float* arr_flt, 
              long* arr_lng, short* arr_shrt) {
    /* Declare many local variables to exhaust registers */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    volatile long l0, l1, l2, l3, l4, l5, l6, l7, l8, l9;
    volatile float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
    volatile short s0, s1, s2, s3, s4, s5, s6, s7, s8, s9;
    volatile int* p0, *p1, *p2, *p3, *p4;
    volatile double* dp0, *dp1, *dp2;
    volatile long* lp0, *lp1;
    
    int result = 0;
    
    /* Initialize with complex expressions */
    v0 = arr_int[0]; v1 = arr_int[1]; v2 = arr_int[2]; v3 = arr_int[3];
    v4 = arr_int[4]; v5 = arr_int[5]; v6 = arr_int[6]; v7 = arr_int[7];
    v8 = arr_int[8]; v9 = arr_int[9];
    
    d0 = arr_dbl[0]; d1 = arr_dbl[1]; d2 = arr_dbl[2]; d3 = arr_dbl[3];
    d4 = arr_dbl[4]; d5 = arr_dbl[5]; d6 = arr_dbl[6]; d7 = arr_dbl[7];
    d8 = arr_dbl[8]; d9 = arr_dbl[9];
    
    /* Take addresses of locals to force stack-based reloads */
    p0 = &v0; p1 = &v1; p2 = &v2; p3 = &v3; p4 = &v4;
    dp0 = &d0; dp1 = &d1; dp2 = &d2;
    lp0 = &l0; lp1 = &l1;
    
    /* Complex loop with many live variables */
    for (int i = 0; i < 1000; i++) {
        /* Complex addressing mode computations */
        int idx1 = (i * 7 + v0 * 3 + v1) % ARRAY_SIZE;
        int idx2 = (i * 13 + v2 * 5 + v3) % ARRAY_SIZE;
        int idx3 = (i * 17 + v4 * 11 + v5) % ARRAY_SIZE;
        int idx4 = (i * 19 + v6 * 7 + v7) % ARRAY_SIZE;
        int idx5 = (i * 23 + v8 * 13 + v9) % ARRAY_SIZE;
        
        /* Multiple address computations held in registers */
        int* addr1 = &arr_int[idx1];
        double* addr2 = &arr_dbl[idx2];
        long* addr3 = &arr_lng[idx3];
        float* addr4 = &arr_flt[idx4];
        short* addr5 = &arr_shrt[idx5];
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2, temp3;
        double dtemp1, dtemp2;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "mov %[val1], %[tmp1]\n\t"
            "add %[val2], %[tmp1]\n\t"
            "mov %[tmp1], %[out1]"
            : [out1] "=r" (temp1), [tmp1] "=&r" (temp2)
            : [val1] "rm" (*addr1), [val2] "rm" (v0)
            : "cc"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS */
        asm volatile (
            "lea (%[base], %[index], 4), %[addr]\n\t"
            "mov (%[addr]), %[val]"
            : [addr] "=&r" (temp3), [val] "=r" (temp2)
            : [base] "r" (arr_int), [index] "r" (idx1)
            : "memory"
        );
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Address computation in one branch */
                int* complex_addr = &arr_int[(idx1 * 3 + idx2 * 5) % ARRAY_SIZE];
                /* Use in another context - forces RELOAD_FOR_OTHER_ADDRESS */
                result += use_int(*complex_addr, v0, v1, v2, v3, v4);
                break;
            }
            case 1: {
                /* Different addressing pattern */
                double* dbl_addr = &arr_dbl[(idx2 * 7 + idx3 * 11) % ARRAY_SIZE];
                dtemp1 = use_double(*dbl_addr, d0, d1, d2, d3);
                
                /* Inline asm with memory constraint */
                asm volatile (
                    "movsd %[in], %%xmm0\n\t"
                    "addsd %[add], %%xmm0\n\t"
                    "movsd %%xmm0, %[out]"
                    : [out] "=m" (*dbl_addr)
                    : [in] "m" (*dbl_addr), [add] "m" (d0)
                    : "xmm0", "memory"
                );
                break;
            }
            case 2: {
                /* RELOAD_FOR_OPERAND_ADDRESS pattern */
                void* addr_result = use_address(
                    (void*)&arr_int[idx1],
                    (void*)&arr_dbl[idx2],
                    (void*)&arr_lng[idx3],
                    v0 * 2,
                    v1 * 3
                );
                result += (intptr_t)addr_result;
                break;
            }
            case 3: {
                /* Mixed types forcing multiple reload types */
                result += use_mixed(
                    arr_int[idx4],
                    arr_dbl[idx5],
                    arr_lng[idx1],
                    arr_flt[idx2],
                    &arr_int[idx3],
                    &arr_dbl[idx4]
                );
                break;
            }
            case 4: {
                /* Complex addressing with multiple terms */
                int offset = (v0 * v1 + v2 * v3 - v4 * v5) % 256;
                short* shrt_addr = arr_shrt + idx1 + offset;
                
                /* Force address to be recalculated */
                asm volatile (
                    "mov %[addr], %%rsi\n\t"
                    "movzwl (%%rsi), %[out]"
                    : [out] "=r" (temp1)
                    : [addr] "r" (shrt_addr)
                    : "rsi", "memory"
                );
                break;
            }
            case 5: {
                /* RELOAD_FOR_INPADDR_ADDRESS pattern */
                int** ptr_to_addr = &addr1;
                asm volatile (
                    "mov %[ptr], %%rax\n\t"
                    "mov (%%rax), %%rbx\n\t"
                    "mov (%%rbx), %[val]"
                    : [val] "=r" (temp1)
                    : [ptr] "r" (ptr_to_addr)
                    : "rax", "rbx", "memory"
                );
                break;
            }
            case 6: {
                /* RELOAD_FOR_OUTADDR_ADDRESS pattern */
                int* out_addr;
                asm volatile (
                    "lea (%[base], %[idx], 2), %[out]"
                    : [out] "=r" (out_addr)
                    : [base] "r" (arr_int), [idx] "r" (idx1)
                );
                *out_addr = v0 + v1 + v2;
                break;
            }
            case 7: {
                /* RELOAD_OTHER pattern with computed goto */
                static void* labels[] = { &&label0, &&label1, &&label2 };
                goto *labels[i % 3];
                
                label0:
                    result += arr_int[idx1] * 2;
                    goto end_switch;
                label1:
                    result += arr_int[idx2] * 3;
                    goto end_switch;
                label2:
                    result += arr_int[idx3] * 5;
                    goto end_switch;
                end_switch:
                break;
            }
        }
        
        /* Update many variables to keep them live */
        v0 = v0 + arr_int[idx1] - arr_int[idx2];
        v1 = v1 * 3 + arr_int[idx3];
        v2 = v2 / 2 + arr_int[idx4];
        v3 = v3 - arr_int[idx5] + i;
        v4 = v4 ^ arr_int[idx1];
        v5 = v5 | arr_int[idx2];
        v6 = v6 & arr_int[idx3];
        v7 = v7 + arr_int[idx4] * 2;
        v8 = v8 - arr_int[idx5] / 3;
        v9 = v9 + i * 7;
        
        d0 = d0 + arr_dbl[idx1] * 0.5;
        d1 = d1 - arr_dbl[idx2] * 0.3;
        d2 = d2 * arr_dbl[idx3];
        d3 = d3 / (arr_dbl[idx4] + 1.0);
        d4 = d4 + arr_dbl[idx5];
        
        /* Force spilling around function calls */
        result += use_int(v0, v1, v2, v3, v4, v5);
        dtemp2 = use_double(d0, d1, d2, d3, d4);
        result += (int)dtemp2;
        
        /* Complex array update with addressing */
        arr_int[(i * 11 + v0) % ARRAY_SIZE] = result % 1000;
        arr_dbl[(i * 13 + v1) % ARRAY_SIZE] = (double)(result % 100) / 10.0;
    }
    
    return result;
}

int main() {
    /* Allocate and initialize arrays */
    int* arr_int = malloc(ARRAY_SIZE * sizeof(int));
    double* arr_dbl = malloc(ARRAY_SIZE * sizeof(double));
    float* arr_flt = malloc(ARRAY_SIZE * sizeof(float));
    long* arr_lng = malloc(ARRAY_SIZE * sizeof(long));
    short* arr_shrt = malloc(ARRAY_SIZE * sizeof(short));
    
    if (!arr_int || !arr_dbl || !arr_flt || !arr_lng || !arr_shrt) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = (i * 7) % 100;
        arr_dbl[i] = (double)(i * 13) / 10.0;
        arr_flt[i] = (float)(i * 17) / 5.0f;
        arr_lng[i] = i * 23L;
        arr_shrt[i] = (short)(i * 29);
    }
    
    /* Call stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_flt, arr_lng, arr_shrt);
    
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_flt);
    free(arr_lng);
    free(arr_shrt);
    
    return 0;
}
