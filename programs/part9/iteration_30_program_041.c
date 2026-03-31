/* reload1_stress_test.c
 * Designed to trigger complex reload scenarios in GCC's reload pass
 * Specifically targeting lines 7146-7174 in reload1.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Opaque noinline functions to prevent optimization */
#define NOINLINE __attribute__((noinline, noclone))

/* Function prototypes */
NOINLINE void helper1(int a, int b, int c, int d, int e, int f, int g, int h);
NOINLINE void helper2(double a, double b, double c, double d, 
                      int* addr1, int* addr2, int* addr3);
NOINLINE void helper3(long long a, long long b, float c, float d,
                      volatile int* addr, volatile double* daddr);
NOINLINE void helper4(int* base, int offset1, int offset2, int offset3,
                      double* dbase, int doffset);

/* Volatile globals to prevent dead code elimination */
volatile int global_sink = 0;
volatile double global_double_sink = 0.0;

/* Helper functions that use their arguments in non-trivial ways */
NOINLINE void helper1(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int local_sink = 0;
    local_sink += a * b + c * d - e * f + g * h;
    global_sink += local_sink;
}

NOINLINE void helper2(double a, double b, double c, double d,
                      int* addr1, int* addr2, int* addr3) {
    volatile double local_sink = 0.0;
    local_sink += a * b - c * d;
    
    /* Force memory accesses with different addressing modes */
    if (addr1) local_sink += *addr1;
    if (addr2) local_sink += *addr2 * 0.5;
    if (addr3) local_sink -= *addr3 * 0.25;
    
    global_double_sink += local_sink;
}

NOINLINE void helper3(long long a, long long b, float c, float d,
                      volatile int* addr, volatile double* daddr) {
    volatile float local_sink = 0.0f;
    
    /* Complex addressing within the helper */
    local_sink = c * d + (float)(a % 1000) - (float)(b % 500);
    
    /* Force address computations */
    if (addr) {
        int idx = (int)(a ^ b) & 0xFF;
        local_sink += addr[idx] * 2.0f;
    }
    
    if (daddr) {
        int idx = (int)(a + b) & 0x7F;
        local_sink += (float)daddr[idx * 2];
    }
    
    global_sink += (int)local_sink;
}

NOINLINE void helper4(int* base, int offset1, int offset2, int offset3,
                      double* dbase, int doffset) {
    volatile int local_sink = 0;
    
    /* Multiple address computations with different offsets */
    local_sink += base[offset1];
    local_sink -= base[offset2];
    local_sink += base[offset3] * 2;
    
    if (dbase) {
        local_sink += (int)(dbase[doffset] * 100.0);
        local_sink -= (int)(dbase[doffset + 1] * 50.0);
    }
    
    global_sink += local_sink;
}

/* Main stress function with extreme register pressure */
NOINLINE int stress_reload(int* arr1, double* arr2, long long* arr3, 
                           float* arr4, int size) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    int v31, v32, v33, v34, v35;
    
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    long long ll1, ll2, ll3, ll4, ll5, ll6;
    
    /* Volatile locals to prevent optimization */
    volatile int vol1 = 0, vol2 = 0, vol3 = 0;
    volatile double vol_d1 = 0.0, vol_d2 = 0.0;
    
    /* Initialize with values from arrays */
    v1 = arr1[0]; v2 = arr1[1]; v3 = arr1[2]; v4 = arr1[3]; v5 = arr1[4];
    v6 = arr1[5]; v7 = arr1[6]; v8 = arr1[7]; v9 = arr1[8]; v10 = arr1[9];
    v11 = arr1[10]; v12 = arr1[11]; v13 = arr1[12]; v14 = arr1[13]; v15 = arr1[14];
    v16 = arr1[15]; v17 = arr1[16]; v18 = arr1[17]; v19 = arr1[18]; v20 = arr1[19];
    
    d1 = arr2[0]; d2 = arr2[1]; d3 = arr2[2]; d4 = arr2[3]; d5 = arr2[4];
    d6 = arr2[5]; d7 = arr2[6]; d8 = arr2[7]; d9 = arr2[8]; d10 = arr2[9];
    
    f1 = arr4[0]; f2 = arr4[1]; f3 = arr4[2]; f4 = arr4[3];
    f5 = arr4[4]; f6 = arr4[5]; f7 = arr4[6]; f8 = arr4[7];
    
    ll1 = arr3[0]; ll2 = arr3[1]; ll3 = arr3[2]; 
    ll4 = arr3[3]; ll5 = arr3[4]; ll6 = arr3[5];
    
    int result = 0;
    
    /* Main loop with complex control flow */
    for (int i = 0; i < 1000; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % size;
        int idx2 = (i * 11 + v3 * 2 + v4 * 13) % size;
        int idx3 = (i * 17 + v5 * 7 + v6 * 19) % size;
        int idx4 = (i * 23 + v7 * 11 + v8 * 29) % size;
        int idx5 = (i * 31 + v9 * 3 + v10 * 37) % size;
        
        double didx1 = (i * 5.3 + d1 * 2.1 + d2 * 3.7);
        int darr_idx1 = ((int)didx1 * 2 + v11) % size;
        int darr_idx2 = ((int)didx1 * 3 + v12) % size;
        
        /* Switch statement to create complex control flow */
        switch (i % 8) {
            case 0: {
                /* Branch 0: Use inline assembly with conflicting constraints */
                int* addr1 = &arr1[idx1];
                int* addr2 = &arr1[idx2];
                int* addr3 = &arr1[idx3];
                
                /* Inline assembly that forces address reloads */
                asm volatile (
                    "addl %[a1], %[r]\n\t"
                    "subl %[a2], %[r]\n\t"
                    "addl %[a3], %[r]\n\t"
                    : [r] "+r" (result)
                    : [a1] "m" (*addr1), 
                      [a2] "m" (*addr2), 
                      [a3] "m" (*addr3)
                    : "cc"
                );
                
                /* Call helper with address arguments */
                helper2(d1, d2, d3, d4, addr1, addr2, addr3);
                break;
            }
            
            case 1: {
                /* Branch 1: Multiple address computations */
                int* base1 = arr1 + idx1;
                int* base2 = arr1 + idx2;
                double* dbase1 = arr2 + darr_idx1;
                
                /* Complex addressing expression */
                int offset1 = (v13 * 3 + v14 * 7) & 0xF;
                int offset2 = (v15 * 5 + v16 * 11) & 0xF;
                int offset3 = (v17 * 13 + v18 * 17) & 0xF;
                
                helper4(base1, offset1, offset2, offset3, dbase1, darr_idx2);
                
                /* More inline assembly */
                asm volatile (
                    "movl %[val1], %%eax\n\t"
                    "addl %[val2], %%eax\n\t"
                    "movl %%eax, %[out]\n\t"
                    : [out] "=r" (vol1)
                    : [val1] "m" (base1[offset1]),
                      [val2] "m" (base2[offset2])
                    : "eax", "cc"
                );
                break;
            }
            
            case 2: {
                /* Branch 2: Mixed types and addressing */
                volatile int* vaddr1 = &arr1[idx3];
                volatile double* vaddr2 = &arr2[darr_idx1];
                
                helper3(ll1, ll2, f1, f2, vaddr1, vaddr2);
                
                /* Force address computation into register */
                int complex_idx = (idx1 * 3 + idx2 * 5 + idx3 * 7) % size;
                int* complex_addr = &arr1[complex_idx];
                
                asm volatile (
                    "movl (%[addr]), %%ebx\n\t"
                    "addl %%ebx, %[res]\n\t"
                    : [res] "+r" (result)
                    : [addr] "r" (complex_addr)
                    : "ebx", "cc"
                );
                break;
            }
            
            case 3: {
                /* Branch 3: Deeply nested addressing */
                int* ptr1 = arr1 + ((idx1 + v19) % size);
                int* ptr2 = arr1 + ((idx2 + v20) % size);
                int* ptr3 = arr1 + ((idx3 + v21) % size);
                int* ptr4 = arr1 + ((idx4 + v22) % size);
                int* ptr5 = arr1 + ((idx5 + v23) % size);
                
                /* Use all pointers to force spilling */
                helper1(*ptr1, *ptr2, *ptr3, *ptr4, 
                       *ptr5, v24, v25, v26);
                
                /* More register pressure */
                asm volatile (
                    "movl %[p1], %%ecx\n\t"
                    "movl %[p2], %%edx\n\t"
                    "addl (%%ecx), %%eax\n\t"
                    "addl (%%edx), %%eax\n\t"
                    "movl %%eax, %[out]\n\t"
                    : [out] "=r" (vol2)
                    : [p1] "r" (ptr1), [p2] "r" (ptr2), "a" (result)
                    : "ecx", "edx", "cc"
                );
                break;
            }
            
            default: {
                /* Default case: Even more complex patterns */
                int* addrs[5];
                addrs[0] = &arr1[idx1];
                addrs[1] = &arr1[idx2];
                addrs[2] = &arr1[idx3];
                addrs[3] = &arr1[idx4];
                addrs[4] = &arr1[idx5];
                
                /* Compute with all addresses */
                for (int j = 0; j < 5; j++) {
                    result += *addrs[j] * (j + 1);
                }
                
                /* Mixed-type computation */
                double* dptr1 = &arr2[darr_idx1];
                double* dptr2 = &arr2[darr_idx2];
                
                asm volatile (
                    "movsd (%[d1]), %%xmm0\n\t"
                    "addsd (%[d2]), %%xmm0\n\t"
                    "movsd %%xmm0, %[out]\n\t"
                    : [out] "=m" (vol_d1)
                    : [d1] "r" (dptr1), [d2] "r" (dptr2)
                    : "xmm0", "memory"
                );
                break;
            }
        }
        
        /* Update many local variables to keep them live */
        v1 = (v1 + arr1[idx1]) ^ v2;
        v2 = (v2 + arr1[idx2]) ^ v3;
        v3 = (v3 + arr1[idx3]) ^ v4;
        v4 = (v4 + arr1[idx4]) ^ v5;
        v5 = (v5 + arr1[idx5]) ^ v6;
        v6 = (v6 + v1) ^ v7;
        v7 = (v7 + v2) ^ v8;
        v8 = (v8 + v3) ^ v9;
        v9 = (v9 + v4) ^ v10;
        v10 = (v10 + v5) ^ v11;
        
        v11 = (v11 + v6) ^ v12;
        v12 = (v12 + v7) ^ v13;
        v13 = (v13 + v8) ^ v14;
        v14 = (v14 + v9) ^ v15;
        v15 = (v15 + v10) ^ v16;
        v16 = (v16 + v11) ^ v17;
        v17 = (v17 + v12) ^ v18;
        v18 = (v18 + v13) ^ v19;
        v19 = (v19 + v14) ^ v20;
        v20 = (v20 + v15) ^ v21;
        
        d1 += arr2[darr_idx1] * 0.1;
        d2 += arr2[darr_idx2] * 0.2;
        d3 = d1 * d2 - d3;
        d4 = d2 * d3 - d4;
        d5 = d3 * d4 - d5;
        
        f1 = f1 * 0.9f + arr4[i % 8] * 0.1f;
        f2 = f2 * 0.8f + arr4[(i + 1) % 8] * 0.2f;
        
        ll1 = ll1 + (arr3[i % 6] ^ ll2);
        ll2 = ll2 + (arr3[(i + 1) % 6] ^ ll3);
        
        /* Force spill/reload around loop backedge */
        if (i % 100 == 0) {
            /* Call with many arguments to force register shuffling */
            helper1(v1, v2, v3, v4, v5, v6, v7, v8);
            helper1(v9, v10, v11, v12, v13, v14, v15, v16);
        }
    }
    
    /* Final computation using all variables */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    result += v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    result += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
    result += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    result += (int)ll1 + (int)ll2;
    
    return result;
}

int main() {
    const int SIZE = 1024;
    
    /* Allocate and initialize arrays with pattern data */
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    double* arr2 = (double*)malloc(SIZE * sizeof(double));
    long long* arr3 = (long long*)malloc(SIZE * sizeof(long long));
    float* arr4 = (float*)malloc(SIZE * sizeof(float));
    
    if (!arr1 || !arr2 || !arr3 || !arr4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 0.5 + 1.0;
        arr3[i] = (long long)i * 7LL + 3LL;
        arr4[i] = i * 0.25f + 0.5f;
    }
    
    /* Call the stress function */
    int result = stress_reload(arr1, arr2, arr3, arr4, SIZE);
    
    printf("Result: %d\n", result);
    printf("Global sinks: %d, %f\n", global_sink, global_double_sink);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
