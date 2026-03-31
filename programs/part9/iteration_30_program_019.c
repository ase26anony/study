/* reload1_stress.c - Extreme register pressure test for reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Opaque noinline functions to prevent optimization */
#define NOINLINE __attribute__((noinline))

NOINLINE int helper1(int a, int b, int c, int d, int e, int f);
NOINLINE float helper2(float a, float b, float c, float d, float e);
NOINLINE double helper3(double a, double b, double c, double* addr);
NOINLINE long helper4(long a, long b, long* addr1, long* addr2);
NOINLINE void* helper5(void* a, void* b, void* c, int offset);

/* Implementations to prevent dead code elimination */
NOINLINE int helper1(int a, int b, int c, int d, int e, int f) {
    volatile int sink;
    sink = a + b - c * d + e / (f ? f : 1);
    return sink;
}

NOINLINE float helper2(float a, float b, float c, float d, float e) {
    volatile float sink;
    sink = a * b + c - d / e;
    return sink;
}

NOINLINE double helper3(double a, double b, double c, double* addr) {
    volatile double sink;
    sink = a + b * c + *addr;
    return sink;
}

NOINLINE long helper4(long a, long b, long* addr1, long* addr2) {
    volatile long sink;
    sink = a ^ b ^ *addr1 ^ *addr2;
    return sink;
}

NOINLINE void* helper5(void* a, void* b, void* c, int offset) {
    volatile char* sink;
    sink = (char*)a + (char*)b - (char*)c + offset;
    return (void*)sink;
}

/* Main stress function with extreme register pressure */
NOINLINE unsigned long stress_reload(int* arr_int, double* arr_dbl, 
                                     long long* arr_ll, float* arr_flt) {
    /* Declare MANY local variables to exhaust registers */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile long long ll1, ll2, ll3, ll4, ll5, ll6, ll7, ll8, ll9, ll10;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Additional volatile pointers for address computations */
    volatile int* p1, *p2, *p3, *p4, *p5;
    volatile double* dp1, *dp2, *dp3;
    volatile float* fp1, *fp2;
    volatile long long* lp1, *lp2;
    
    unsigned long checksum = 0;
    int i, j, k;
    
    /* Initialize with array values to create dependencies */
    v1 = arr_int[0]; v2 = arr_int[1]; v3 = arr_int[2]; v4 = arr_int[3];
    v5 = arr_int[4]; v6 = arr_int[5]; v7 = arr_int[6]; v8 = arr_int[7];
    v9 = arr_int[8]; v10 = arr_int[9];
    
    f1 = arr_flt[0]; f2 = arr_flt[1]; f3 = arr_flt[2]; f4 = arr_flt[3];
    f5 = arr_flt[4]; f6 = arr_flt[5]; f7 = arr_flt[6]; f8 = arr_flt[7];
    f9 = arr_flt[8]; f10 = arr_flt[9];
    
    d1 = arr_dbl[0]; d2 = arr_dbl[1]; d3 = arr_dbl[2]; d4 = arr_dbl[3];
    d5 = arr_dbl[4]; d6 = arr_dbl[5]; d7 = arr_dbl[6]; d8 = arr_dbl[7];
    d9 = arr_dbl[8]; d10 = arr_dbl[9];
    
    ll1 = arr_ll[0]; ll2 = arr_ll[1]; ll3 = arr_ll[2]; ll4 = arr_ll[3];
    ll5 = arr_ll[4]; ll6 = arr_ll[5]; ll7 = arr_ll[6]; ll8 = arr_ll[7];
    ll9 = arr_ll[8]; ll10 = arr_ll[9];
    
    /* Take addresses of locals to force stack-based reloads */
    p1 = &v1; p2 = &v2; p3 = &v3; p4 = &v4; p5 = &v5;
    dp1 = &d1; dp2 = &d2; dp3 = &d3;
    fp1 = &f1; fp2 = &f2;
    lp1 = &ll1; lp2 = &ll2;
    
    /* Complex loop with extreme register pressure */
    for (i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2) % ARRAY_SIZE;
        int idx2 = (i * 13 + v3 * 5 + v4 * 2) % ARRAY_SIZE;
        int idx3 = (i * 17 + v5 * 7 + v6 * 3) % ARRAY_SIZE;
        int idx4 = (i * 23 + v7 * 11 + v8 * 5) % ARRAY_SIZE;
        
        /* More complex indices using float/double conversions */
        int idx5 = (int)(f1 * 100 + f2 * 50 + i) % ARRAY_SIZE;
        int idx6 = (int)(d1 * 200 + d2 * 100 + i * 2) % ARRAY_SIZE;
        int idx7 = (ll1 + ll2 + i * 3) % ARRAY_SIZE;
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Block 0: Complex address computations */
                int* addr1 = &arr_int[idx1 * 3 + idx2];
                int* addr2 = &arr_int[idx3 * 5 + idx4];
                double* addr3 = &arr_dbl[idx5 * 2 + idx6];
                
                /* Inline assembly with conflicting constraints */
                int temp1, temp2;
                asm volatile (
                    "mov %[val1], %[tmp1]\n\t"
                    "add %[val2], %[tmp1]\n\t"
                    "mov %[tmp1], %[tmp2]\n\t"
                    : [tmp1] "=&r" (temp1), [tmp2] "=r" (temp2)
                    : [val1] "r" (*addr1), [val2] "r" (*addr2)
                    : "cc"
                );
                
                /* Use computed addresses in helper calls */
                v1 = helper1(*addr1, *addr2, v3, v4, temp1, temp2);
                checksum += v1;
                
                /* Force RELOAD_FOR_INPUT_ADDRESS */
                asm volatile (
                    "mov %[addr], %%rax\n\t"
                    "mov (%%rax), %%rbx\n\t"
                    "add %%rbx, %[sum]\n\t"
                    : [sum] "+r" (checksum)
                    : [addr] "m" (addr3)
                    : "rax", "rbx", "cc"
                );
                break;
            }
            
            case 1: {
                /* Block 1: Different address pattern */
                float* faddr1 = &arr_flt[idx1 + idx2 * 2];
                float* faddr2 = &arr_flt[idx3 + idx4 * 3];
                
                /* Force RELOAD_FOR_OUTPUT_ADDRESS */
                float result;
                asm volatile (
                    "mov %[in1], %%xmm0\n\t"
                    "mov %[in2], %%xmm1\n\t"
                    "mulps %%xmm1, %%xmm0\n\t"
                    "movss %%xmm0, %[out]\n\t"
                    : [out] "=m" (result)
                    : [in1] "m" (*faddr1), [in2] "m" (*faddr2)
                    : "xmm0", "xmm1"
                );
                
                f1 = helper2(result, f2, f3, f4, f5);
                checksum += (int)f1;
                break;
            }
            
            case 2: {
                /* Block 2: Mixed operand types */
                long long* laddr1 = &arr_ll[idx5];
                long long* laddr2 = &arr_ll[idx6];
                
                /* Complex addressing with pointer arithmetic */
                void* ptr1 = (void*)((char*)laddr1 + v1 * sizeof(long long));
                void* ptr2 = (void*)((char*)laddr2 + v2 * sizeof(long long));
                void* ptr3 = (void*)((char*)arr_int + v3 * sizeof(int));
                
                /* Force RELOAD_FOR_OPERAND_ADDRESS */
                void* result_ptr = helper5(ptr1, ptr2, ptr3, v4);
                checksum += (unsigned long)result_ptr;
                
                /* Use the pointer */
                int val = *(int*)result_ptr;
                v2 = helper1(val, v5, v6, v7, v8, v9);
                break;
            }
            
            case 3: {
                /* Block 3: Nested addressing */
                double* daddr1 = &arr_dbl[(idx1 * idx2) % ARRAY_SIZE];
                double* daddr2 = &arr_dbl[(idx3 * idx4) % ARRAY_SIZE];
                double* daddr3 = &arr_dbl[(idx5 * idx6) % ARRAY_SIZE];
                
                d1 = helper3(*daddr1, *daddr2, d3, daddr3);
                checksum += (unsigned long)d1;
                
                /* Force RELOAD_FOR_INPADDR_ADDRESS */
                asm volatile (
                    "mov %[addr], %%rsi\n\t"
                    "movsd (%%rsi), %%xmm0\n\t"
                    "addsd %%xmm0, %%xmm0\n\t"
                    "movsd %%xmm0, %[out]\n\t"
                    : [out] "=m" (d2)
                    : [addr] "m" (daddr1)
                    : "rsi", "xmm0", "cc"
                );
                break;
            }
            
            case 4: {
                /* Block 4: More complex patterns */
                long* addr1 = (long*)&arr_int[idx1];
                long* addr2 = (long*)&arr_int[idx2];
                
                l1 = helper4(*addr1, *addr2, &l3, &l4);
                checksum += l1;
                
                /* Force RELOAD_FOR_OUTADDR_ADDRESS */
                long output;
                asm volatile (
                    "mov %[in], %%rax\n\t"
                    "not %%rax\n\t"
                    "mov %%rax, %[out]\n\t"
                    : [out] "=m" (output)
                    : [in] "m" (*addr1)
                    : "rax", "cc"
                );
                l2 = output;
                break;
            }
            
            default: {
                /* Default case: Mix everything */
                int* addr1 = &arr_int[(i * 19 + v9 * 7) % ARRAY_SIZE];
                double* addr2 = &arr_dbl[(i * 29 + v10 * 11) % ARRAY_SIZE];
                
                /* Force RELOAD_FOR_OTHER_ADDRESS */
                asm volatile (
                    "mov %[a1], %%rdi\n\t"
                    "mov %[a2], %%rsi\n\t"
                    "mov (%%rdi), %%eax\n\t"
                    "cvtsi2sd %%eax, %%xmm0\n\t"
                    "addsd (%%rsi), %%xmm0\n\t"
                    "movsd %%xmm0, %[out]\n\t"
                    : [out] "=m" (d5)
                    : [a1] "m" (addr1), [a2] "m" (addr2)
                    : "rdi", "rsi", "rax", "xmm0", "cc"
                );
                
                v10 = helper1(*addr1, v1, v2, v3, v4, v5);
                break;
            }
        }
        
        /* Update most variables to keep them live */
        v1 = v1 * 3 + arr_int[idx1];
        v2 = v2 * 5 + arr_int[idx2];
        v3 = v3 * 7 + arr_int[idx3];
        v4 = v4 * 11 + arr_int[idx4];
        v5 = v5 * 13 + v6;
        v6 = v6 * 17 + v7;
        v7 = v7 * 19 + v8;
        v8 = v8 * 23 + v9;
        v9 = v9 * 29 + v10;
        v10 = v10 * 31 + i;
        
        f1 = f1 * 1.1f + arr_flt[idx5];
        f2 = f2 * 1.2f + arr_flt[idx6];
        f3 = f3 * 1.3f + f4;
        f4 = f4 * 1.4f + f5;
        f5 = f5 * 1.5f + f6;
        
        d1 = d1 * 1.01 + arr_dbl[idx7];
        d2 = d2 * 1.02 + d3;
        d3 = d3 * 1.03 + d4;
        d4 = d4 * 1.04 + d5;
        
        ll1 = ll1 * 3 + arr_ll[idx1];
        ll2 = ll2 * 5 + arr_ll[idx2];
        ll3 = ll3 * 7 + ll4;
        ll4 = ll4 * 11 + ll5;
        
        /* Periodic calls to force register shuffling */
        if (i % 100 == 0) {
            v1 = helper1(v1, v2, v3, v4, v5, v6);
            f1 = helper2(f1, f2, f3, f4, f5);
            d1 = helper3(d1, d2, d3, dp1);
            l1 = helper4(l1, l2, &l3, &l4);
        }
    }
    
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
        arr_int[i] = i * 3 + 1;
        arr_dbl[i] = i * 1.5 + 0.5;
        arr_ll[i] = i * 7LL + 3LL;
        arr_flt[i] = i * 0.7f + 0.3f;
    }
    
    printf("Starting stress test...\n");
    unsigned long result = stress_reload(arr_int, arr_dbl, arr_ll, arr_flt);
    printf("Checksum: %lu\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_ll);
    free(arr_flt);
    
    return 0;
}
