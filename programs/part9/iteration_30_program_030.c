/* reload_stress_test.c - Extreme register pressure test for GCC reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define NUM_LOCALS 40
#define ITERATIONS 1000

/* Opaque functions to prevent optimization */
int __attribute__((noinline)) use_int(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

double __attribute__((noinline)) use_double(double a, double b, double c, 
                                           double d, double e, double f) {
    volatile double sink = a * b / c + d - e * f;
    return sink;
}

void* __attribute__((noinline)) use_address(void* a, void* b, void* c, 
                                           int d, int e, int f) {
    volatile intptr_t sink = (intptr_t)a + (intptr_t)b - (intptr_t)c + d - e + f;
    return (void*)sink;
}

float __attribute__((noinline)) use_float(float a, float b, float c, 
                                         float d, float e, float f,
                                         float g, float h, float i) {
    volatile float sink = a + b * c - d / e + f - g * h + i;
    return sink;
}

/* Main stress function */
int __attribute__((noinline)) stress_reload(int* arr_int, double* arr_dbl, 
                                          float* arr_flt, long* arr_lng) {
    /* Declare many local variables to exhaust registers */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    volatile float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
    volatile long l0, l1, l2, l3, l4, l5, l6, l7, l8, l9;
    volatile int* p0, *p1, *p2, *p3;
    volatile double* pd0, *pd1, *pd2;
    volatile float* pf0, *pf1, *pf2;
    volatile long* pl0, *pl1, *pl2;
    
    int result = 0;
    
    /* Initialize with complex expressions */
    v0 = arr_int[0] * 2;
    v1 = arr_int[1] + v0;
    v2 = arr_int[2] - v1;
    v3 = arr_int[3] * v2;
    v4 = arr_int[4] / (v3 ? v3 : 1);
    v5 = arr_int[5] + v4;
    v6 = arr_int[6] - v5;
    v7 = arr_int[7] * v6;
    v8 = arr_int[8] + v7;
    v9 = arr_int[9] - v8;
    
    d0 = arr_dbl[0] * 2.0;
    d1 = arr_dbl[1] + d0;
    d2 = arr_dbl[2] - d1;
    d3 = arr_dbl[3] * d2;
    d4 = arr_dbl[4] / (d3 != 0.0 ? d3 : 1.0);
    d5 = arr_dbl[5] + d4;
    d6 = arr_dbl[6] - d5;
    d7 = arr_dbl[7] * d6;
    d8 = arr_dbl[8] + d7;
    d9 = arr_dbl[9] - d8;
    
    f0 = arr_flt[0] * 2.0f;
    f1 = arr_flt[1] + f0;
    f2 = arr_flt[2] - f1;
    f3 = arr_flt[3] * f2;
    f4 = arr_flt[4] / (f3 != 0.0f ? f3 : 1.0f);
    f5 = arr_flt[5] + f4;
    f6 = arr_flt[6] - f5;
    f7 = arr_flt[7] * f6;
    f8 = arr_flt[8] + f7;
    f9 = arr_flt[9] - f8;
    
    l0 = arr_lng[0] * 2L;
    l1 = arr_lng[1] + l0;
    l2 = arr_lng[2] - l1;
    l3 = arr_lng[3] * l2;
    l4 = arr_lng[4] / (l3 ? l3 : 1L);
    l5 = arr_lng[5] + l4;
    l6 = arr_lng[6] - l5;
    l7 = arr_lng[7] * l6;
    l8 = arr_lng[8] + l7;
    l9 = arr_lng[9] - l8;
    
    /* Take addresses of locals to force stack addressing */
    p0 = &v0; p1 = &v1; p2 = &v2; p3 = &v3;
    pd0 = &d0; pd1 = &d1; pd2 = &d2;
    pf0 = &f0; pf1 = &f1; pf2 = &f2;
    pl0 = &l0; pl1 = &l1; pl2 = &l2;
    
    /* Main loop with extreme register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v0 * 3 + v1 * 5) % ARRAY_SIZE;
        int idx2 = (i * 11 + v2 * 13 + v3 * 17) % ARRAY_SIZE;
        int idx3 = (i * 19 + v4 * 23 + v5 * 29) % ARRAY_SIZE;
        int idx4 = (i * 31 + v6 * 37 + v7 * 41) % ARRAY_SIZE;
        int idx5 = (i * 43 + v8 * 47 + v9 * 53) % ARRAY_SIZE;
        
        double d_idx1 = (i * 1.5 + d0 * 2.5 + d1 * 3.5);
        int d_idx_int1 = ((int)d_idx1 * 59 + v0 * 61) % ARRAY_SIZE;
        int d_idx_int2 = ((int)d_idx1 * 67 + v1 * 71) % ARRAY_SIZE;
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
                int* addr1 = &arr_int[idx1];
                int* addr2 = &arr_int[idx2];
                
                /* Inline assembly with conflicting constraints */
                int temp1, temp2;
                asm volatile (
                    "mov %[src1], %[dst1]\n\t"
                    "add %[src2], %[dst1]\n\t"
                    "mov %[dst1], %[dst2]"
                    : [dst1] "+r" (temp1), [dst2] "=r" (temp2)
                    : [src1] "m" (*addr1), [src2] "r" (v0)
                    : "cc"
                );
                
                /* Use computed addresses */
                result += *addr1 + *addr2 + temp1 + temp2;
                
                /* Call with address operands */
                use_address(addr1, addr2, p0, v0, v1, v2);
                break;
            }
            
            case 1: {
                /* RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
                double* d_addr1 = &arr_dbl[d_idx_int1];
                double* d_addr2 = &arr_dbl[d_idx_int2];
                
                /* Complex addressing in output position */
                double d_temp1, d_temp2;
                asm volatile (
                    "movsd %[src1], %[dst1]\n\t"
                    "addsd %[src2], %[dst1]\n\t"
                    "movsd %[dst1], %[dst2]"
                    : [dst1] "=x" (d_temp1), [dst2] "=x" (d_temp2)
                    : [src1] "m" (*d_addr1), [src2] "x" (d0)
                    : 
                );
                
                /* Store through computed addresses */
                *d_addr1 = d_temp1 + d0;
                *d_addr2 = d_temp2 + d1;
                
                result += (int)(d_temp1 + d_temp2);
                break;
            }
            
            case 2: {
                /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
                float* f_addr1 = &arr_flt[idx3];
                float* f_addr2 = &arr_flt[idx4];
                
                /* Mixed register/memory constraints */
                float f_result;
                asm volatile (
                    "movss %[a], %[out]\n\t"
                    "mulss %[b], %[out]\n\t"
                    "addss %[c], %[out]"
                    : [out] "=x" (f_result)
                    : [a] "m" (*f_addr1), [b] "x" (f0), [c] "r" (f1)
                    : 
                );
                
                /* Multiple address computations */
                float* f_addr3 = f_addr1 + (v0 % 16);
                float* f_addr4 = f_addr2 + (v1 % 16);
                
                use_float(*f_addr1, *f_addr2, *f_addr3, *f_addr4, 
                         f0, f1, f2, f3, f_result);
                result += (int)f_result;
                break;
            }
            
            case 3: {
                /* RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
                long* l_addr1 = &arr_lng[idx5];
                long* l_addr2 = l_addr1 + (v2 % 8);
                
                /* Complex chain of operations */
                long l_temp1 = *l_addr1;
                long l_temp2 = *l_addr2;
                
                /* Force spilling with many live values */
                result += use_int(v0, v1, v2, v3, v4, v5);
                result += use_int(v6, v7, v8, v9, (int)l_temp1, (int)l_temp2);
                
                /* More address computations */
                long* l_addr3 = &arr_lng[(idx5 + v3) % ARRAY_SIZE];
                long* l_addr4 = &arr_lng[(idx5 + v4) % ARRAY_SIZE];
                
                use_address(l_addr1, l_addr2, l_addr3, 
                           (int)l_temp1, (int)l_temp2, result);
                break;
            }
            
            default: {
                /* Mix all types */
                int* mixed_addr1 = &arr_int[(i * v0 + v1 * v2) % ARRAY_SIZE];
                double* mixed_addr2 = &arr_dbl[(i * v3 + v4 * v5) % ARRAY_SIZE];
                float* mixed_addr3 = &arr_flt[(i * v6 + v7 * v8) % ARRAY_SIZE];
                
                /* Multiple function calls with different argument sets */
                int r1 = use_int(*mixed_addr1, v0, v1, v2, v3, v4);
                double r2 = use_double(*mixed_addr2, d0, d1, d2, d3, d4);
                float r3 = use_float(*mixed_addr3, f0, f1, f2, f3, f4, f5, f6, f7, f8);
                
                /* Update many variables to keep them live */
                v0 = v0 + r1;
                v1 = v1 - r1;
                v2 = v2 * (r1 ? r1 : 1);
                v3 = v3 / (r1 ? r1 : 1);
                v4 = v4 + (int)r2;
                v5 = v5 - (int)r2;
                v6 = v6 + (int)r3;
                v7 = v7 - (int)r3;
                
                result += r1 + (int)r2 + (int)r3;
                break;
            }
        }
        
        /* Update all variables to keep them live across iterations */
        v0 = v0 + arr_int[(i + v0) % ARRAY_SIZE];
        v1 = v1 - arr_int[(i + v1) % ARRAY_SIZE];
        v2 = v2 ^ arr_int[(i + v2) % ARRAY_SIZE];
        v3 = v3 | arr_int[(i + v3) % ARRAY_SIZE];
        v4 = v4 & arr_int[(i + v4) % ARRAY_SIZE];
        v5 = v5 + arr_int[(i + v5) % ARRAY_SIZE];
        v6 = v6 - arr_int[(i + v6) % ARRAY_SIZE];
        v7 = v7 * arr_int[(i + v7) % ARRAY_SIZE];
        v8 = v8 / (arr_int[(i + v8) % ARRAY_SIZE] ? arr_int[(i + v8) % ARRAY_SIZE] : 1);
        v9 = v9 % (arr_int[(i + v9) % ARRAY_SIZE] ? arr_int[(i + v9) % ARRAY_SIZE] : 1);
        
        d0 = d0 + arr_dbl[(i + (int)d0) % ARRAY_SIZE];
        d1 = d1 - arr_dbl[(i + (int)d1) % ARRAY_SIZE];
        d2 = d2 * arr_dbl[(i + (int)d2) % ARRAY_SIZE];
        d3 = d3 / (arr_dbl[(i + (int)d3) % ARRAY_SIZE] != 0.0 ? 
                   arr_dbl[(i + (int)d3) % ARRAY_SIZE] : 1.0);
        d4 = d4 + arr_dbl[(i + (int)d4) % ARRAY_SIZE];
        d5 = d5 - arr_dbl[(i + (int)d5) % ARRAY_SIZE];
        d6 = d6 * arr_dbl[(i + (int)d6) % ARRAY_SIZE];
        d7 = d7 / (arr_dbl[(i + (int)d7) % ARRAY_SIZE] != 0.0 ? 
                   arr_dbl[(i + (int)d7) % ARRAY_SIZE] : 1.0);
        d8 = d8 + arr_dbl[(i + (int)d8) % ARRAY_SIZE];
        d9 = d9 - arr_dbl[(i + (int)d9) % ARRAY_SIZE];
        
        /* Additional pointer arithmetic to stress address reloads */
        p0 = p0 + (i % 4);
        p1 = p1 - (i % 4);
        pd0 = pd0 + (i % 2);
        pd1 = pd1 - (i % 2);
        pf0 = pf0 + (i % 3);
        pf1 = pf1 - (i % 3);
        pl0 = pl0 + (i % 2);
        pl1 = pl1 - (i % 2);
    }
    
    /* Final aggregation */
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    result += (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4;
    result += (int)d5 + (int)d6 + (int)d7 + (int)d8 + (int)d9;
    result += (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4;
    result += (int)f5 + (int)f6 + (int)f7 + (int)f8 + (int)f9;
    result += (int)l0 + (int)l1 + (int)l2 + (int)l3 + (int)l4;
    result += (int)l5 + (int)l6 + (int)l7 + (int)l8 + (int)l9;
    
    return result;
}

int main() {
    /* Allocate and initialize arrays */
    int* arr_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* arr_flt = (float*)malloc(ARRAY_SIZE * sizeof(float));
    long* arr_lng = (long*)malloc(ARRAY_SIZE * sizeof(long));
    
    if (!arr_int || !arr_dbl || !arr_flt || !arr_lng) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = (i * 1103515245 + 12345) & 0x7fffffff;
        arr_dbl[i] = (double)(i * i) / 1000.0;
        arr_flt[i] = (float)(i * 3.14159) / 100.0f;
        arr_lng[i] = (long)i * 1000000L;
    }
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_flt, arr_lng);
    
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_flt);
    free(arr_lng);
    
    return 0;
}
