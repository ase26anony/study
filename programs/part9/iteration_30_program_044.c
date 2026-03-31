/* reload_stress_test.c
 * Designed to trigger multiple reload types in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-move-loop-invariants reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define NUM_LOCALS 40
#define ITERATIONS 1000

/* Opaque functions to prevent optimization */
int __attribute__((noinline)) helper1(int a, int b, int c, int d, int e, int f, int* addr) {
    volatile int sink = a + b + c + d + e + f + *addr;
    return sink & 0xFF;
}

double __attribute__((noinline)) helper2(double a, double b, double c, double* addr1, double* addr2) {
    volatile double sink = a * b + c + *addr1 - *addr2;
    return sink;
}

long long __attribute__((noinline)) helper3(long long a, long long b, long long c, 
                                          int* addr1, double* addr2, float* addr3) {
    volatile long long sink = a + b + c + (long long)*addr1 + (long long)*addr2 + (long long)*addr3;
    return sink;
}

void __attribute__((noinline)) helper4(int* base, int offset1, int offset2, int offset3,
                                      double* dbase, int doffset1, int doffset2) {
    volatile int sink1 = base[offset1] + base[offset2] + base[offset3];
    volatile double sink2 = dbase[doffset1] * dbase[doffset2];
    (void)sink1; (void)sink2;
}

/* Main stress function */
int __attribute__((noinline)) stress_reload(int* arr1, int* arr2, double* darr1, double* darr2,
                                           float* farr1, float* farr2, long long* llarr) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    long long ll1, ll2, ll3, ll4, ll5;
    volatile int checksum = 0;
    
    /* Initialize locals with array values to create dependencies */
    v1 = arr1[0]; v2 = arr1[1]; v3 = arr1[2]; v4 = arr1[3]; v5 = arr1[4];
    v6 = arr2[0]; v7 = arr2[1]; v8 = arr2[2]; v9 = arr2[3]; v10 = arr2[4];
    v11 = arr1[10]; v12 = arr1[11]; v13 = arr1[12]; v14 = arr1[13]; v15 = arr1[14];
    v16 = arr2[10]; v17 = arr2[11]; v18 = arr2[12]; v19 = arr2[13]; v20 = arr2[14];
    
    d1 = darr1[0]; d2 = darr1[1]; d3 = darr1[2]; d4 = darr1[3]; d5 = darr1[4];
    d6 = darr2[0]; d7 = darr2[1]; d8 = darr2[2]; d9 = darr2[3]; d10 = darr2[4];
    
    f1 = farr1[0]; f2 = farr1[1]; f3 = farr1[2]; f4 = farr1[3];
    f5 = farr2[0]; f6 = farr2[1]; f7 = farr2[2]; f8 = farr2[3];
    
    ll1 = llarr[0]; ll2 = llarr[1]; ll3 = llarr[2]; ll4 = llarr[3]; ll5 = llarr[4];
    
    /* Complex loop with multiple addressing modes */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing expressions - will require address computations */
        int idx1 = (i * 7 + v1 * 3 + v2) % ARRAY_SIZE;
        int idx2 = (i * 11 + v3 * 5 + v4) % ARRAY_SIZE;
        int idx3 = (i * 13 + v5 * 7 + v6) % ARRAY_SIZE;
        int idx4 = (i * 17 + v7 * 11 + v8) % ARRAY_SIZE;
        int idx5 = (i * 19 + v9 * 13 + v10) % ARRAY_SIZE;
        
        double didx1 = (i * 23 + v11 * 17) % ARRAY_SIZE;
        double didx2 = (i * 29 + v12 * 19) % ARRAY_SIZE;
        double didx3 = (i * 31 + v13 * 23) % ARRAY_SIZE;
        
        /* Control flow to split live ranges */
        switch (i % 8) {
            case 0: {
                /* Block 0: Complex addressing in one branch */
                int* addr1 = &arr1[idx1];
                int* addr2 = &arr2[idx2];
                double* daddr1 = &darr1[(int)didx1];
                double* daddr2 = &darr2[(int)didx2];
                
                /* Inline assembly with conflicting constraints */
                int result1, result2;
                asm volatile (
                    "mov %[val1], %[res1]\n\t"
                    "add %[val2], %[res1]\n\t"
                    "mov %[val3], %[res2]\n\t"
                    : [res1] "=r" (result1), [res2] "=r" (result2)
                    : [val1] "r" (*addr1), [val2] "r" (*addr2), [val3] "r" (v1)
                    : "cc"
                );
                
                checksum += result1 + result2;
                
                /* Force RELOAD_FOR_INPUT_ADDRESS */
                volatile int* volatile_addr = addr1;
                checksum += volatile_addr[idx3 % 16];
                break;
            }
            
            case 1: {
                /* Block 1: Different addressing pattern */
                int complex_idx = (idx1 * 3 + idx2 * 5 + idx3 * 7) % ARRAY_SIZE;
                int* addr3 = &arr1[complex_idx];
                
                /* Force RELOAD_FOR_OUTPUT_ADDRESS */
                int output_val;
                asm volatile (
                    "mov %[in], %[out]\n\t"
                    "add $1, %[out]\n\t"
                    : [out] "=m" (*addr3)
                    : [in] "r" (v2)
                    : "cc"
                );
                
                /* Mixed operand types */
                helper1(v3, v4, v5, *addr3, v6, v7, &arr2[idx4]);
                break;
            }
            
            case 2: {
                /* Block 2: More complex addressing with multiple terms */
                int idx6 = (idx1 + idx2 * 2 + idx3 * 3 + idx4 * 4) % ARRAY_SIZE;
                int idx7 = (idx2 + idx3 * 2 + idx4 * 3 + idx5 * 4) % ARRAY_SIZE;
                
                double result = helper2(d1, d2, d3, 
                                       &darr1[idx6 % ARRAY_SIZE],
                                       &darr2[idx7 % ARRAY_SIZE]);
                
                /* Force address computation to be live across call */
                int* saved_addr = &arr1[idx6];
                checksum += *saved_addr + (int)result;
                break;
            }
            
            case 3: {
                /* Block 3: Long long operations with addressing */
                long long llidx = (ll1 + i * 37) % ARRAY_SIZE;
                long long result = helper3(ll2, ll3, ll4,
                                          &arr1[idx1],
                                          &darr1[idx2 % ARRAY_SIZE],
                                          &farr1[idx3 % ARRAY_SIZE]);
                
                ll5 = result + llarr[llidx % ARRAY_SIZE];
                break;
            }
            
            case 4: {
                /* Block 4: Multiple address computations */
                int base_idx = (v14 * 41 + v15 * 43) % ARRAY_SIZE;
                helper4(&arr1[base_idx], v16, v17, v18,
                       &darr1[base_idx % ARRAY_SIZE], v19, v20);
                break;
            }
            
            case 5: {
                /* Block 5: Pointer arithmetic creating complex addresses */
                int* ptr1 = arr1 + idx1 + v1;
                int* ptr2 = arr2 + idx2 + v2;
                double* dptr1 = darr1 + (idx3 % ARRAY_SIZE) + (int)d1;
                
                /* Force RELOAD_FOR_OPERAND_ADDRESS */
                volatile int sum = 0;
                for (int j = 0; j < 4; j++) {
                    sum += ptr1[j] + ptr2[j * 2];
                }
                checksum += sum + (int)(*dptr1);
                break;
            }
            
            case 6: {
                /* Block 6: Computed goto to create complex control flow */
                static void* labels[] = { &&label0, &&label1, &&label2 };
                goto *labels[i % 3];
                
                label0:
                    checksum += arr1[idx1] + arr2[idx2];
                    goto end_case6;
                label1:
                    checksum += darr1[idx3 % ARRAY_SIZE] + darr2[idx4 % ARRAY_SIZE];
                    goto end_case6;
                label2:
                    checksum += farr1[idx5 % ARRAY_SIZE] + farr2[idx1 % ARRAY_SIZE];
                    goto end_case6;
                end_case6:
                    break;
            }
            
            case 7: {
                /* Block 7: Everything mixed together */
                int* addr_array[4];
                addr_array[0] = &arr1[idx1];
                addr_array[1] = &arr2[idx2];
                addr_array[2] = &arr1[idx3];
                addr_array[3] = &arr2[idx4];
                
                for (int j = 0; j < 4; j++) {
                    checksum += *addr_array[j] + j;
                }
                
                /* Force RELOAD_FOR_OTHER_ADDRESS */
                volatile int* other_addr = (int*)((char*)addr_array[0] + v9);
                checksum += *other_addr;
                break;
            }
        }
        
        /* Update most local variables to keep them live */
        v1 = v1 + arr1[idx1 % ARRAY_SIZE];
        v2 = v2 + arr2[idx2 % ARRAY_SIZE];
        v3 = v3 ^ arr1[idx3 % ARRAY_SIZE];
        v4 = v4 ^ arr2[idx4 % ARRAY_SIZE];
        v5 = v5 * (1 + arr1[idx5 % ARRAY_SIZE]);
        v6 = v6 * (1 + arr2[idx1 % ARRAY_SIZE]);
        v7 = v7 - arr1[idx2 % ARRAY_SIZE];
        v8 = v8 - arr2[idx3 % ARRAY_SIZE];
        v9 = (v9 + i) % 256;
        v10 = (v10 + i * 2) % 256;
        
        d1 = d1 + darr1[idx1 % ARRAY_SIZE];
        d2 = d2 + darr2[idx2 % ARRAY_SIZE];
        d3 = d3 * (1.0 + darr1[idx3 % ARRAY_SIZE] * 0.01);
        d4 = d4 * (1.0 + darr2[idx4 % ARRAY_SIZE] * 0.01);
        
        f1 = f1 + farr1[idx1 % ARRAY_SIZE];
        f2 = f2 + farr2[idx2 % ARRAY_SIZE];
        
        ll1 = ll1 + llarr[idx1 % ARRAY_SIZE];
        ll2 = ll2 + llarr[idx2 % ARRAY_SIZE];
        
        /* Prevent loop invariant motion */
        asm volatile ("" : : "r" (v1), "r" (v2), "r" (v3), "r" (v4), "r" (v5),
                         "r" (d1), "r" (d2), "r" (f1), "r" (f2), "r" (ll1));
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays with pattern data */
    int* arr1 = malloc(ARRAY_SIZE * sizeof(int));
    int* arr2 = malloc(ARRAY_SIZE * sizeof(int));
    double* darr1 = malloc(ARRAY_SIZE * sizeof(double));
    double* darr2 = malloc(ARRAY_SIZE * sizeof(double));
    float* farr1 = malloc(ARRAY_SIZE * sizeof(float));
    float* farr2 = malloc(ARRAY_SIZE * sizeof(float));
    long long* llarr = malloc(ARRAY_SIZE * sizeof(long long));
    
    if (!arr1 || !arr2 || !darr1 || !darr2 || !farr1 || !farr2 || !llarr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 + 2;
        darr1[i] = i * 0.7 + 1.5;
        darr2[i] = i * 0.9 + 2.5;
        farr1[i] = i * 1.1f + 3.5f;
        farr2[i] = i * 1.3f + 4.5f;
        llarr[i] = i * 1000LL + 12345LL;
    }
    
    printf("Starting reload stress test...\n");
    
    /* Call the stress function */
    int result = stress_reload(arr1, arr2, darr1, darr2, farr1, farr2, llarr);
    
    printf("Result checksum: %d\n", result);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(darr1);
    free(darr2);
    free(farr1);
    free(farr2);
    free(llarr);
    
    return 0;
}
