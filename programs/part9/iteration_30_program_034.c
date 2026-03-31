/* reload1_stress_test.c
 * Designed to trigger complex reload patterns in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-move-loop-invariants reload1_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define NUM_LOCALS 40
#define LOOP_ITERATIONS 1000

/* Opaque functions that compiler cannot analyze */
__attribute__((noinline)) int helper1(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

__attribute__((noinline)) float helper2(float a, float b, float c, float d, float e, float f) {
    volatile float sink = a * b + c * d - e / f;
    return sink;
}

__attribute__((noinline)) double helper3(double a, double b, double* ptr1, double* ptr2, long long ll) {
    volatile double sink = a + b + *ptr1 + *ptr2 + (double)ll;
    return sink;
}

__attribute__((noinline)) void* helper4(void* p1, void* p2, void* p3, int offset1, int offset2) {
    volatile char* result = (char*)p1 + offset1 + (char*)p2 - offset2 + (char*)p3;
    return (void*)result;
}

__attribute__((noinline)) long long helper5(long long a, long long b, long long c, 
                                           long long d, long long e, long long f,
                                           long long g, long long h) {
    volatile long long sink = a * b + c * d - e * f + g * h;
    return sink;
}

/* Main stress function with extreme register pressure */
__attribute__((noinline)) int stress_reload(int* arr_int, double* arr_dbl, 
                                           volatile int* volatile_arr, 
                                           volatile double* volatile_dbl_arr) {
    /* Declare many local variables to exhaust registers */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
    double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    long long ll0, ll1, ll2, ll3, ll4;
    
    /* Initialize with complex expressions */
    v0 = arr_int[0] + 1;
    v1 = arr_int[1] * 2;
    v2 = arr_int[2] / 3;
    v3 = arr_int[3] - 4;
    v4 = arr_int[4] % 5;
    
    for (int i = 5; i < 30; i++) {
        v0 += arr_int[i % ARRAY_SIZE];
        v1 -= arr_int[(i * 2) % ARRAY_SIZE];
        v2 *= arr_int[(i * 3) % ARRAY_SIZE] + 1;
        v3 = v3 ^ arr_int[(i * 5) % ARRAY_SIZE];
        v4 = v4 | arr_int[(i * 7) % ARRAY_SIZE];
    }
    
    /* More initialization with addressing computations */
    f0 = (float)arr_dbl[0];
    f1 = (float)arr_dbl[1];
    f2 = (float)arr_dbl[2];
    f3 = (float)arr_dbl[3];
    f4 = (float)arr_dbl[4];
    
    d0 = arr_dbl[5];
    d1 = arr_dbl[6];
    d2 = arr_dbl[7];
    d3 = arr_dbl[8];
    d4 = arr_dbl[9];
    
    ll0 = (long long)arr_int[10];
    ll1 = (long long)arr_int[11];
    ll2 = (long long)arr_int[12];
    ll3 = (long long)arr_int[13];
    ll4 = (long long)arr_int[14];
    
    /* Take addresses of locals to force stack-based reloads */
    int* ptr_v0 = &v0;
    int* ptr_v1 = &v1;
    float* ptr_f0 = &f0;
    double* ptr_d0 = &d0;
    long long* ptr_ll0 = &ll0;
    
    volatile int checksum = 0;
    
    /* Main loop with extreme register pressure */
    for (int iter = 0; iter < LOOP_ITERATIONS; iter++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (iter * 7 + v0 * 3 + v1 * 5) % ARRAY_SIZE;
        int idx2 = (iter * 11 + v2 * 13 + v3 * 17) % ARRAY_SIZE;
        int idx3 = (iter * 19 + v4 * 23 + v0 * 29) % ARRAY_SIZE;
        int idx4 = (iter * 31 + v1 * 37 + v2 * 41) % ARRAY_SIZE;
        int idx5 = (iter * 43 + v3 * 47 + v4 * 53) % ARRAY_SIZE;
        
        /* Force address computations into registers */
        int* addr1 = &arr_int[idx1];
        int* addr2 = &arr_int[idx2];
        int* addr3 = &arr_int[idx3];
        double* addr_dbl1 = &arr_dbl[idx1 % (ARRAY_SIZE/2)];
        double* addr_dbl2 = &arr_dbl[idx2 % (ARRAY_SIZE/2)];
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2, temp3;
        asm volatile (
            "mov %[val1], %[tmp1]\n\t"
            "add %[val2], %[tmp1]\n\t"
            "mov %[tmp1], %[out1]\n\t"
            : [out1] "=r" (temp1), [tmp1] "=&r" (temp2)
            : [val1] "r" (*addr1), [val2] "r" (*addr2)
            : "cc"
        );
        
        asm volatile (
            "imul %[val3], %[tmp2]\n\t"
            "add %%eax, %[out2]\n\t"
            : [out2] "=r" (temp3)
            : [val3] "r" (*addr3), [tmp2] "r" (temp1)
            : "eax", "cc"
        );
        
        /* More inline assembly with memory constraints */
        volatile int mem_result;
        asm volatile (
            "movl %[mem1], %%eax\n\t"
            "addl %[mem2], %%eax\n\t"
            "movl %%eax, %[result]\n\t"
            : [result] "=m" (mem_result)
            : [mem1] "m" (volatile_arr[idx1]), [mem2] "m" (volatile_arr[idx2])
            : "eax", "cc"
        );
        
        /* Call helper functions with many arguments */
        v5 = helper1(v0, v1, v2, v3, v4, temp1);
        f5 = helper2(f0, f1, f2, f3, f4, (float)temp2);
        d5 = helper3(d0, d1, addr_dbl1, addr_dbl2, ll0);
        
        /* Complex control flow to split live ranges */
        switch (iter % 8) {
            case 0: {
                /* Address computation in one block, use in another */
                int* complex_addr = &arr_int[(idx1 * 3 + idx2 * 7) % ARRAY_SIZE];
                v6 = *complex_addr + v0;
                void* ptr_result = helper4(addr1, addr2, complex_addr, idx1, idx2);
                v7 = *(int*)ptr_result;
                break;
            }
            case 1: {
                double* dbl_complex_addr = &arr_dbl[(idx3 * 5 + idx4 * 11) % (ARRAY_SIZE/2)];
                d6 = *dbl_complex_addr + d0;
                /* Force address reload */
                asm volatile (
                    "movsd %[src], %%xmm0\n\t"
                    "addsd %[add], %%xmm0\n\t"
                    "movsd %%xmm0, %[dst]\n\t"
                    : [dst] "=m" (volatile_dbl_arr[idx3])
                    : [src] "m" (*dbl_complex_addr), [add] "m" (d1)
                    : "xmm0", "cc"
                );
                break;
            }
            case 2: {
                /* Multiple address computations */
                int offset1 = (v0 * v1) % 64;
                int offset2 = (v2 * v3) % 64;
                int* addr_with_offset1 = addr1 + offset1;
                int* addr_with_offset2 = addr2 + offset2;
                v8 = *addr_with_offset1 + *addr_with_offset2;
                break;
            }
            case 3: {
                /* Use computed goto for non-trivial control flow */
                void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
                goto *labels[iter % 4];
                
                label0:
                    v9 = *addr1 * 2;
                    goto end_switch;
                label1:
                    v9 = *addr2 / 3;
                    goto end_switch;
                label2:
                    v9 = *addr3 + 5;
                    goto end_switch;
                label3:
                    v9 = *addr1 - *addr2;
                    goto end_switch;
                end_switch:
                    break;
            }
            case 4:
            case 5:
            case 6:
            case 7: {
                /* Mixed operand types and addressing */
                ll0 = helper5(ll0, ll1, ll2, ll3, ll4, 
                             (long long)v0, (long long)v1, (long long)v2);
                
                /* Force output address reload */
                volatile long long* volatile_ll = (volatile long long*)&volatile_arr[idx4];
                asm volatile (
                    "movq %[src], %%rax\n\t"
                    "addq $1, %%rax\n\t"
                    "movq %%rax, %[dst]\n\t"
                    : [dst] "=m" (*volatile_ll)
                    : [src] "r" (ll0)
                    : "rax", "cc"
                );
                break;
            }
        }
        
        /* Update most variables to keep them live */
        v0 = v0 + arr_int[idx1] - arr_int[idx2];
        v1 = v1 * arr_int[idx3] / (arr_int[idx4] + 1);
        v2 = v2 ^ arr_int[idx5];
        v3 = v3 | arr_int[(idx1 + idx2) % ARRAY_SIZE];
        v4 = v4 + arr_int[(idx3 + idx4) % ARRAY_SIZE];
        
        f0 = f0 + (float)arr_dbl[idx1 % (ARRAY_SIZE/2)];
        f1 = f1 * (float)arr_dbl[idx2 % (ARRAY_SIZE/2)];
        f2 = f2 - (float)arr_dbl[idx3 % (ARRAY_SIZE/2)];
        f3 = f3 / ((float)arr_dbl[idx4 % (ARRAY_SIZE/2)] + 0.1f);
        f4 = f0 + f1 - f2 + f3;
        
        d0 = d0 + arr_dbl[idx1 % (ARRAY_SIZE/2)];
        d1 = d1 * arr_dbl[idx2 % (ARRAY_SIZE/2)];
        d2 = d2 - arr_dbl[idx3 % (ARRAY_SIZE/2)];
        d3 = d3 / (arr_dbl[idx4 % (ARRAY_SIZE/2)] + 0.1);
        d4 = d0 + d1 - d2 + d3;
        
        ll0 = ll0 + (long long)arr_int[idx1];
        ll1 = ll1 * (long long)arr_int[idx2];
        ll2 = ll2 - (long long)arr_int[idx3];
        ll3 = ll3 ^ (long long)arr_int[idx4];
        ll4 = ll4 | (long long)arr_int[idx5];
        
        /* Accumulate checksum */
        checksum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        checksum += (int)f0 + (int)f1 + (int)f2 + (int)f3 + (int)f4;
        checksum += (int)d0 + (int)d1 + (int)d2 + (int)d3 + (int)d4;
        checksum += (int)ll0 + (int)ll1 + (int)ll2 + (int)ll3 + (int)ll4;
        checksum += temp1 + temp2 + temp3 + mem_result;
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays */
    int* arr_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(ARRAY_SIZE * sizeof(double));
    volatile int* volatile_arr = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    volatile double* volatile_dbl_arr = (volatile double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!arr_int || !arr_dbl || !volatile_arr || !volatile_dbl_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = (i * 37 + 123) % 1000;
        arr_dbl[i] = (double)(i * 51 + 456) / 100.0;
        volatile_arr[i] = (i * 73 + 789) % 500;
        volatile_dbl_arr[i] = (double)(i * 91 + 234) / 50.0;
    }
    
    printf("Starting reload stress test...\n");
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, volatile_arr, volatile_dbl_arr);
    
    printf("Checksum result: %d\n", result);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free((void*)volatile_arr);
    free((void*)volatile_dbl_arr);
    
    return 0;
}
