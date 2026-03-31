/* reload1_stress_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))
#define VOLATILE volatile

/* Opaque functions to prevent optimization */
NOINLINE int func1(int a, int b, int c, int d, int e, int f);
NOINLINE double func2(double a, double b, double c, double d, double e);
NOINLINE long long func3(long long a, long long b, long long c, 
                         long long d, long long e, long long f);
NOINLINE void func4(void* addr1, void* addr2, void* addr3, 
                    int val1, int val2, int val3);

/* Implementation of opaque functions */
NOINLINE int func1(int a, int b, int c, int d, int e, int f) {
    VOLATILE int sink = a + b + c + d + e + f;
    return sink ^ 0x12345678;
}

NOINLINE double func2(double a, double b, double c, double d, double e) {
    VOLATILE double sink = a * b + c * d - e;
    return sink * 1.23456789;
}

NOINLINE long long func3(long long a, long long b, long long c,
                         long long d, long long e, long long f) {
    VOLATILE long long sink = (a * b) + (c * d) - (e * f);
    return sink ^ 0x123456789ABCDEFLL;
}

NOINLINE void func4(void* addr1, void* addr2, void* addr3,
                    int val1, int val2, int val3) {
    VOLATILE int* p1 = (int*)addr1;
    VOLATILE int* p2 = (int*)addr2;
    VOLATILE int* p3 = (int*)addr3;
    *p1 += val1;
    *p2 += val2;
    *p3 += val3;
}

/* Main stress function */
NOINLINE uint64_t stress_reload(VOLATILE int* arr_int, 
                                VOLATILE double* arr_dbl,
                                VOLATILE long long* arr_ll,
                                int size) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    double d11, d12, d13, d14, d15, d16, d17, d18, d19, d20;
    long long ll1, ll2, ll3, ll4, ll5, ll6, ll7, ll8, ll9, ll10;
    
    /* Initialize with complex expressions */
    v1 = arr_int[0] ^ 0x11111111;
    v2 = arr_int[1] ^ 0x22222222;
    v3 = arr_int[2] ^ 0x33333333;
    v4 = arr_int[3] ^ 0x44444444;
    v5 = arr_int[4] ^ 0x55555555;
    v6 = arr_int[5] ^ 0x66666666;
    v7 = arr_int[6] ^ 0x77777777;
    v8 = arr_int[7] ^ 0x88888888;
    v9 = arr_int[8] ^ 0x99999999;
    v10 = arr_int[9] ^ 0xAAAAAAAA;
    
    d1 = arr_dbl[0] * 1.1;
    d2 = arr_dbl[1] * 1.2;
    d3 = arr_dbl[2] * 1.3;
    d4 = arr_dbl[3] * 1.4;
    d5 = arr_dbl[4] * 1.5;
    d6 = arr_dbl[5] * 1.6;
    d7 = arr_dbl[6] * 1.7;
    d8 = arr_dbl[7] * 1.8;
    d9 = arr_dbl[8] * 1.9;
    d10 = arr_dbl[9] * 2.0;
    
    ll1 = arr_ll[0] ^ 0x1111111111111111LL;
    ll2 = arr_ll[1] ^ 0x2222222222222222LL;
    ll3 = arr_ll[2] ^ 0x3333333333333333LL;
    ll4 = arr_ll[3] ^ 0x4444444444444444LL;
    ll5 = arr_ll[4] ^ 0x5555555555555555LL;
    
    /* More initialization */
    v11 = v1 + v2;
    v12 = v3 + v4;
    v13 = v5 + v6;
    v14 = v7 + v8;
    v15 = v9 + v10;
    v16 = v1 * v2;
    v17 = v3 * v4;
    v18 = v5 * v6;
    v19 = v7 * v8;
    v20 = v9 * v10;
    
    d11 = d1 + d2;
    d12 = d3 + d4;
    d13 = d5 + d6;
    d14 = d7 + d8;
    d15 = d9 + d10;
    d16 = d1 * d2;
    d17 = d3 * d4;
    d18 = d5 * d6;
    d19 = d7 * d8;
    d20 = d9 * d10;
    
    ll6 = ll1 + ll2;
    ll7 = ll3 + ll4;
    ll8 = ll5 * ll1;
    ll9 = ll2 * ll3;
    ll10 = ll4 * ll5;
    
    /* Even more variables */
    v21 = v11 ^ v12;
    v22 = v13 ^ v14;
    v23 = v15 ^ v16;
    v24 = v17 ^ v18;
    v25 = v19 ^ v20;
    v26 = v21 + v22;
    v27 = v23 + v24;
    v28 = v25 + v26;
    v29 = v27 + v28;
    v30 = v29 * 0x12345678;
    
    VOLATILE uint64_t checksum = 0;
    
    /* Complex loop with extreme register pressure */
    for (int i = 0; i < 1000; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % size;
        int idx2 = (i * 11 + v3 * 13 + v4 * 17) % size;
        int idx3 = (i * 19 + v5 * 23 + v6 * 29) % size;
        int idx4 = (i * 31 + v7 * 37 + v8 * 41) % size;
        int idx5 = (i * 43 + v9 * 47 + v10 * 53) % size;
        
        double idx_d1 = (i * 2.5 + d1 * 1.7 + d2 * 2.3);
        int idx6 = ((int)idx_d1) % size;
        
        /* Take addresses of locals - forces stack addressing */
        int* ptr1 = &v1;
        int* ptr2 = &v2;
        int* ptr3 = &v3;
        double* ptr4 = &d1;
        double* ptr5 = &d2;
        long long* ptr6 = &ll1;
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Complex addressing in this branch */
                VOLATILE int* addr1 = &arr_int[idx1];
                VOLATILE int* addr2 = &arr_int[idx2];
                VOLATILE int* addr3 = &arr_int[idx3];
                
                /* Inline assembly with conflicting constraints */
                int temp1, temp2, temp3;
                asm volatile (
                    "mov %[val1], %[tmp1]\n\t"
                    "mov %[val2], %[tmp2]\n\t"
                    "add %[tmp1], %[tmp2]\n\t"
                    "mov %[tmp2], %[tmp3]"
                    : [tmp1] "=&r" (temp1), [tmp2] "=&r" (temp2), [tmp3] "=r" (temp3)
                    : [val1] "rm" (v1), [val2] "rm" (v2)
                    : "cc"
                );
                
                /* Use computed addresses */
                v1 = *addr1 + temp1;
                v2 = *addr2 + temp2;
                v3 = *addr3 + temp3;
                
                /* Call with address arguments */
                func4(addr1, addr2, addr3, v1, v2, v3);
                break;
            }
            case 1: {
                /* Different addressing pattern */
                VOLATILE double* addr4 = &arr_dbl[idx4 % size];
                VOLATILE double* addr5 = &arr_dbl[idx5 % size];
                
                /* More inline assembly */
                double dtemp1, dtemp2;
                asm volatile (
                    "movsd %[val1], %[tmp1]\n\t"
                    "movsd %[val2], %[tmp2]\n\t"
                    "addsd %[tmp1], %[tmp2]"
                    : [tmp1] "=&x" (dtemp1), [tmp2] "=&x" (dtemp2)
                    : [val1] "xm" (d1), [val2] "xm" (d2)
                );
                
                d1 = *addr4 + dtemp1;
                d2 = *addr5 + dtemp2;
                
                /* Call with mixed arguments */
                d3 = func2(d1, d2, d3, d4, d5);
                break;
            }
            case 2: {
                /* Address computation that spans basic blocks */
                VOLATILE long long* addr6 = &arr_ll[idx6];
                VOLATILE long long* addr7 = &arr_ll[(idx6 + 1) % size];
                
                /* Complex expression using address */
                ll1 = *addr6 ^ ll2;
                ll2 = *addr7 ^ ll3;
                
                /* Call with many arguments */
                ll3 = func3(ll1, ll2, ll3, ll4, ll5, ll6);
                break;
            }
            case 3: {
                /* More address computations */
                int idx7 = (idx1 * 3 + idx2 * 5 + idx3 * 7) % size;
                VOLATILE int* addr8 = &arr_int[idx7];
                
                /* Inline assembly with memory constraint */
                int result;
                asm volatile (
                    "imul %[a], %[b]\n\t"
                    "add %[c], %[b]"
                    : [b] "+r" (v4)
                    : [a] "r" (v5), [c] "m" (*addr8)
                    : "cc"
                );
                
                v5 = func1(v4, v5, v6, v7, v8, v9);
                break;
            }
            case 4:
            case 5:
            case 6:
            case 7: {
                /* Default case with even more complexity */
                int idx8 = (i * 59 + v11 * 61 + v12 * 67) % size;
                int idx9 = (i * 71 + v13 * 73 + v14 * 79) % size;
                
                VOLATILE int* addr9 = &arr_int[idx8];
                VOLATILE int* addr10 = &arr_int[idx9];
                
                /* Multiple operations in sequence */
                v6 = *addr9 + v15;
                v7 = *addr10 + v16;
                
                /* Nested addressing */
                int idx10 = (v6 * 83 + v7 * 89) % size;
                VOLATILE double* addr11 = &arr_dbl[idx10];
                
                d4 = *addr11 * d11;
                d5 = d4 + d12;
                
                /* Call chain */
                v8 = func1(v6, v7, v8, v9, v10, v11);
                d6 = func2(d4, d5, d6, d7, d8);
                break;
            }
        }
        
        /* Update many variables to keep them live */
        v1 = v1 ^ arr_int[(i + 1) % size];
        v2 = v2 + arr_int[(i + 2) % size];
        v3 = v3 * arr_int[(i + 3) % size];
        v4 = v4 ^ arr_int[(i + 4) % size];
        v5 = v5 + arr_int[(i + 5) % size];
        
        d1 = d1 + arr_dbl[(i + 1) % size];
        d2 = d2 * arr_dbl[(i + 2) % size];
        d3 = d3 - arr_dbl[(i + 3) % size];
        d4 = d4 + arr_dbl[(i + 4) % size];
        d5 = d5 * arr_dbl[(i + 5) % size];
        
        ll1 = ll1 ^ arr_ll[(i + 1) % size];
        ll2 = ll2 + arr_ll[(i + 2) % size];
        ll3 = ll3 * arr_ll[(i + 3) % size];
        
        /* Update checksum */
        checksum += v1 + v2 + v3 + v4 + v5;
        checksum += (uint64_t)(d1 + d2 + d3 + d4 + d5);
        checksum += ll1 + ll2 + ll3;
        
        /* Force spill by using all variables */
        v6 = v1 + v2 - v3 * v4 / (v5 ? v5 : 1);
        v7 = v6 ^ v8 ^ v9 ^ v10;
        v8 = v11 + v12 - v13 * v14 / (v15 ? v15 : 1);
        v9 = v16 ^ v17 ^ v18 ^ v19 ^ v20;
        v10 = v21 + v22 - v23 * v24 / (v25 ? v25 : 1);
        
        d6 = d1 + d2 - d3 * d4 / d5;
        d7 = d6 + d8 - d9 * d10 / d11;
        d8 = d12 + d13 - d14 * d15 / d16;
        d9 = d17 + d18 - d19 * d20 / d1;
        d10 = d2 + d3 - d4 * d5 / d6;
        
        ll4 = ll1 + ll2 - ll3 * ll5 / (ll6 ? ll6 : 1);
        ll5 = ll4 ^ ll7 ^ ll8 ^ ll9 ^ ll10;
        ll6 = ll2 + ll3 - ll4 * ll5 / (ll7 ? ll7 : 1);
    }
    
    return checksum;
}

int main() {
    const int SIZE = 1000;
    
    /* Allocate and initialize arrays */
    VOLATILE int* arr_int = (int*)malloc(SIZE * sizeof(int));
    VOLATILE double* arr_dbl = (double*)malloc(SIZE * sizeof(double));
    VOLATILE long long* arr_ll = (long long*)malloc(SIZE * sizeof(long long));
    
    if (!arr_int || !arr_dbl || !arr_ll) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = i ^ 0x12345678;
        arr_dbl[i] = i * 1.23456789;
        arr_ll[i] = ((long long)i << 32) | i;
    }
    
    /* Call stress function */
    uint64_t result = stress_reload(arr_int, arr_dbl, arr_ll, SIZE);
    
    printf("Checksum: %llu\n", (unsigned long long)result);
    
    /* Cleanup */
    free((void*)arr_int);
    free((void*)arr_dbl);
    free((void*)arr_ll);
    
    return 0;
}
