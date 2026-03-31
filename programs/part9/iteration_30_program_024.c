/* reload1_stress_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))
#define VOLATILE volatile

/* Opaque functions to prevent optimization */
NOINLINE int func1(int a, int b, int c, int d, int e, int f);
NOINLINE float func2(float a, float b, float c, float d, float e);
NOINLINE double func3(double a, double b, double c, double d);
NOINLINE void func4(int* a, float* b, double* c, long* d);
NOINLINE void* func5(void* a, void* b, void* c, void* d, void* e);

/* Implementation of opaque functions */
NOINLINE int func1(int a, int b, int c, int d, int e, int f) {
    VOLATILE int dummy = a + b - c * d + e / (f ? f : 1);
    return dummy ^ 0x55AA55AA;
}

NOINLINE float func2(float a, float b, float c, float d, float e) {
    VOLATILE float dummy = a * b + c / d - e;
    return dummy * 1.5f;
}

NOINLINE double func3(double a, double b, double c, double d) {
    VOLATILE double dummy = (a + b) * (c - d);
    return dummy / 2.0;
}

NOINLINE void func4(int* a, float* b, double* c, long* d) {
    VOLATILE int dummy1 = *a;
    VOLATILE float dummy2 = *b;
    VOLATILE double dummy3 = *c;
    VOLATILE long dummy4 = *d;
    *a = dummy1 + 1;
    *b = dummy2 * 2.0f;
    *c = dummy3 - 1.0;
    *d = dummy4 ^ 0x12345678;
}

NOINLINE void* func5(void* a, void* b, void* c, void* d, void* e) {
    VOLATILE uintptr_t sum = (uintptr_t)a + (uintptr_t)b + 
                            (uintptr_t)c + (uintptr_t)d + (uintptr_t)e;
    return (void*)(sum & ~0x3);
}

/* Main stress function */
NOINLINE unsigned long stress_reload(int* arr_int, double* arr_dbl, 
                                     float* arr_flt, long* arr_lng, int size) {
    /* Declare many local variables to exhaust registers */
    VOLATILE int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    VOLATILE int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    VOLATILE float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    VOLATILE double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    VOLATILE long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    VOLATILE int* p1, *p2, *p3, *p4, *p5;
    VOLATILE double* dp1, *dp2, *dp3;
    VOLATILE float* fp1, *fp2, *fp3;
    VOLATILE long* lp1, *lp2, *lp3;
    
    unsigned long checksum = 0;
    int i, j, k;
    
    /* Initialize locals with array values */
    v1 = arr_int[0]; v2 = arr_int[1]; v3 = arr_int[2]; v4 = arr_int[3];
    v5 = arr_int[4]; v6 = arr_int[5]; v7 = arr_int[6]; v8 = arr_int[7];
    v9 = arr_int[8]; v10 = arr_int[9];
    
    f1 = arr_flt[0]; f2 = arr_flt[1]; f3 = arr_flt[2]; f4 = arr_flt[3];
    f5 = arr_flt[4]; f6 = arr_flt[5]; f7 = arr_flt[6]; f8 = arr_flt[7];
    f9 = arr_flt[8]; f10 = arr_flt[9];
    
    d1 = arr_dbl[0]; d2 = arr_dbl[1]; d3 = arr_dbl[2]; d4 = arr_dbl[3];
    d5 = arr_dbl[4]; d6 = arr_dbl[5]; d7 = arr_dbl[6]; d8 = arr_dbl[7];
    d9 = arr_dbl[8]; d10 = arr_dbl[9];
    
    l1 = arr_lng[0]; l2 = arr_lng[1]; l3 = arr_lng[2]; l4 = arr_lng[3];
    l5 = arr_lng[4]; l6 = arr_lng[5]; l7 = arr_lng[6]; l8 = arr_lng[7];
    l9 = arr_lng[8]; l10 = arr_lng[9];
    
    /* Take addresses of locals to force stack addressing */
    p1 = &v1; p2 = &v2; p3 = &v3; p4 = &v4; p5 = &v5;
    dp1 = &d1; dp2 = &d2; dp3 = &d3;
    fp1 = &f1; fp2 = &f2; fp3 = &f3;
    lp1 = &l1; lp2 = &l2; lp3 = &l3;
    
    /* Complex loop with extreme register pressure */
    for (i = 0; i < 1000; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2) % size;
        int idx2 = (i * 11 + v3 * 5 + v4 * 2) % size;
        int idx3 = (i * 13 + v5 * 7 + v6 * 3) % size;
        int idx4 = (i * 17 + v7 * 11 + v8 * 5) % size;
        int idx5 = (i * 19 + v9 * 13 + v10 * 7) % size;
        
        float fidx1 = (f1 * i + f2 * v1 + f3) % size;
        float fidx2 = (f4 * i + f5 * v2 + f6) % size;
        
        double didx1 = (d1 * i + d2 * v3 + d3) % size;
        double didx2 = (d4 * i + d5 * v4 + d6) % size;
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Block 0: Complex addressing computations */
                int* addr1 = &arr_int[idx1 * 3 + idx2];
                int* addr2 = &arr_int[idx3 * 5 + idx4];
                float* addr3 = &arr_flt[(int)fidx1 * 2 + (int)fidx2];
                double* addr4 = &arr_dbl[(int)didx1 * 3 + (int)didx2];
                
                /* Inline assembly with conflicting constraints */
                int temp1, temp2;
                asm volatile (
                    "mov %[val1], %[tmp1]\n\t"
                    "add %[val2], %[tmp1]\n\t"
                    "mov %[tmp1], %[out1]\n\t"
                    : [out1] "=r" (temp1), [tmp1] "=&r" (temp2)
                    : [val1] "r" (*addr1), [val2] "r" (*addr2)
                    : "cc"
                );
                
                v1 = temp1;
                checksum += v1;
                
                /* Use computed addresses in function calls */
                func4(addr1, addr3, addr4, &l1);
                break;
            }
            
            case 1: {
                /* Block 1: Different addressing pattern */
                long* addr5 = &arr_lng[idx1 + idx2 * 2 + idx3 * 3];
                double* addr6 = &arr_dbl[idx4 * 7 + idx5 * 11];
                
                /* Inline assembly forcing address reloads */
                long temp_l;
                asm volatile (
                    "mov %[addr], %%rsi\n\t"
                    "mov (%%rsi), %[out]\n\t"
                    : [out] "=r" (temp_l)
                    : [addr] "r" (addr5)
                    : "rsi", "memory"
                );
                
                l2 = temp_l;
                checksum += l2;
                
                /* Mixed function calls */
                v2 = func1(v2, v3, v4, v5, v6, (int)*addr6);
                f2 = func2(f2, f3, f4, f5, f6);
                break;
            }
            
            case 2:
            case 3: {
                /* Block 2-3: Nested addressing */
                int complex_idx = (idx1 * 17 + idx2 * 13 + idx3 * 11 + 
                                  idx4 * 7 + idx5 * 5) % size;
                float* complex_addr = &arr_flt[complex_idx];
                
                /* Force output address reload */
                float temp_f;
                asm volatile (
                    "movss %[in], %[out]\n\t"
                    "mulss %[mul], %[out]\n\t"
                    : [out] "=x" (temp_f)
                    : [in] "m" (*complex_addr), [mul] "x" (f7)
                    : 
                );
                
                f3 = temp_f;
                checksum += (int)f3;
                
                /* Multiple function calls with many arguments */
                d3 = func3(d1, d2, d3, d4);
                func5(complex_addr, &v3, &f3, &d3, &l3);
                break;
            }
            
            case 4: {
                /* Block 4: More address computations */
                int* addr7 = &arr_int[(v1 * v2 + v3 * v4) % size];
                int* addr8 = &arr_int[(v5 * v6 + v7 * v8) % size];
                
                /* Inline assembly with memory constraint */
                int result;
                asm volatile (
                    "imull %[a], %[b]\n\t"
                    "addl %[c], %[b]\n\t"
                    : [b] "+r" (result)
                    : [a] "r" (*addr7), [c] "m" (*addr8)
                    : "cc"
                );
                
                v4 = result;
                checksum += v4;
                break;
            }
            
            case 5:
            case 6:
            case 7: {
                /* Block 5-7: Mixed operations */
                /* Complex addressing with floating point */
                double* addr9 = &arr_dbl[((int)(d1 * 100) + (int)(d2 * 50) + 
                                        (int)(d3 * 25)) % size];
                float* addr10 = &arr_flt[((int)(f1 * 10) + (int)(f2 * 20) + 
                                         (int)(f3 * 30)) % size];
                
                /* Force operand address reloads */
                double temp_d;
                asm volatile (
                    "movsd %[in], %[out]\n\t"
                    "addsd %[add], %[out]\n\t"
                    : [out] "=x" (temp_d)
                    : [in] "m" (*addr9), [add] "x" (d4)
                    :
                );
                
                d5 = temp_d;
                checksum += (long)d5;
                
                /* Update many variables to keep them live */
                v5 = func1(v5, v6, v7, v8, v9, v10);
                f5 = func2(f5, f6, f7, f8, f9);
                d6 = func3(d6, d7, d8, d9);
                func4(&v6, &f6, &d7, &l6);
                break;
            }
        }
        
        /* Update all variables to keep them live across iterations */
        v1 += arr_int[idx1];
        v2 -= arr_int[idx2];
        v3 ^= arr_int[idx3];
        v4 |= arr_int[idx4];
        v5 &= arr_int[idx5];
        
        f1 += arr_flt[(int)fidx1];
        f2 -= arr_flt[(int)fidx2];
        f3 *= 1.01f;
        f4 /= 1.02f;
        
        d1 += arr_dbl[(int)didx1];
        d2 -= arr_dbl[(int)didx2];
        d3 *= 1.001;
        d4 /= 1.002;
        
        l1 += arr_lng[idx1];
        l2 -= arr_lng[idx2];
        l3 ^= arr_lng[idx3];
        l4 |= arr_lng[idx4];
        l5 &= arr_lng[idx5];
        
        /* More complex updates */
        v6 = (v6 * 3 + v7 * 5) % 1000;
        v7 = (v7 * 7 + v8 * 11) % 1000;
        v8 = (v8 * 13 + v9 * 17) % 1000;
        v9 = (v9 * 19 + v10 * 23) % 1000;
        v10 = (v10 * 29 + v1 * 31) % 1000;
        
        f6 = f6 * 1.1f + f7 * 0.9f;
        f7 = f7 * 1.2f - f8 * 0.8f;
        f8 = f8 * 1.3f + f9 * 0.7f;
        f9 = f9 * 1.4f - f10 * 0.6f;
        f10 = f10 * 1.5f + f1 * 0.5f;
        
        d6 = d6 * 1.01 + d7 * 0.99;
        d7 = d7 * 1.02 - d8 * 0.98;
        d8 = d8 * 1.03 + d9 * 0.97;
        d9 = d9 * 1.04 - d10 * 0.96;
        d10 = d10 * 1.05 + d1 * 0.95;
        
        l6 = (l6 * 3 + l7 * 5) ^ 0xAAAAAAAA;
        l7 = (l7 * 7 + l8 * 11) ^ 0x55555555;
        l8 = (l8 * 13 + l9 * 17) ^ 0x33333333;
        l9 = (l9 * 19 + l10 * 23) ^ 0xCCCCCCCC;
        l10 = (l10 * 29 + l1 * 31) ^ 0x0F0F0F0F;
        
        /* Periodic function calls to force register shuffling */
        if (i % 50 == 0) {
            v11 = func1(v1, v2, v3, v4, v5, v6);
            f11 = func2(f1, f2, f3, f4, f5);
            d11 = func3(d1, d2, d3, d4);
            func4(&v7, &f6, &d5, &l7);
            func5(&v8, &f7, &d6, &l8, &v9);
        }
    }
    
    /* Final mixing of all values */
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    checksum += (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10;
    checksum += (long)d1 + (long)d2 + (long)d3 + (long)d4 + (long)d5;
    checksum += (long)d6 + (long)d7 + (long)d8 + (long)d9 + (long)d10;
    checksum += l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10;
    
    return checksum;
}

int main() {
    const int SIZE = 10000;
    
    /* Allocate and initialize large arrays */
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
        arr_dbl[i] = i * 1.5 + 2.7;
        arr_flt[i] = i * 0.7f + 1.3f;
        arr_lng[i] = i * 5L + 3L;
    }
    
    /* Call the stress function */
    unsigned long result = stress_reload(arr_int, arr_dbl, arr_flt, arr_lng, SIZE);
    
    printf("Checksum: %lu\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_flt);
    free(arr_lng);
    
    return 0;
}
