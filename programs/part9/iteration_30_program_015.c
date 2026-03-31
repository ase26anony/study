/* reload1_stress_test.c
 * Designed to trigger complex reload patterns in GCC's reload pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-move-loop-invariants reload1_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Opaque functions to prevent optimization */
int __attribute__((noinline)) helper1(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b + c + d + e + f;
    return sink & 0xFF;
}

double __attribute__((noinline)) helper2(double a, double b, double c, double* addr) {
    volatile double sink = a + b + c + *addr;
    return sink;
}

void __attribute__((noinline)) helper3(long long* addr1, long long* addr2, long long* addr3) {
    volatile long long sink = *addr1 + *addr2 + *addr3;
}

void __attribute__((noinline)) helper4(float* fptr, int* iptr, double* dptr) {
    volatile float fsink = *fptr;
    volatile int isink = *iptr;
    volatile double dsink = *dptr;
}

/* Main stress function */
int __attribute__((noinline)) stress_reload(int* int_arr, double* dbl_arr, 
                                           float* flt_arr, long long* ll_arr) {
    /* Declare many local variables to exhaust registers */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile long long ll1, ll2, ll3, ll4, ll5;
    
    /* Additional variables for address computations */
    int idx1, idx2, idx3, idx4, idx5;
    double* dptr1, *dptr2, *dptr3;
    int* iptr1, *iptr2, *iptr3;
    float* fptr1, *fptr2;
    long long* llptr1, *llptr2;
    
    int checksum = 0;
    
    /* Initialize with array values */
    v1 = int_arr[0]; v2 = int_arr[1]; v3 = int_arr[2]; v4 = int_arr[3];
    v5 = int_arr[4]; v6 = int_arr[5]; v7 = int_arr[6]; v8 = int_arr[7];
    v9 = int_arr[8]; v10 = int_arr[9]; v11 = int_arr[10]; v12 = int_arr[11];
    v13 = int_arr[12]; v14 = int_arr[13]; v15 = int_arr[14]; v16 = int_arr[15];
    v17 = int_arr[16]; v18 = int_arr[17]; v19 = int_arr[18]; v20 = int_arr[19];
    
    d1 = dbl_arr[0]; d2 = dbl_arr[1]; d3 = dbl_arr[2]; d4 = dbl_arr[3];
    d5 = dbl_arr[4]; d6 = dbl_arr[5]; d7 = dbl_arr[6]; d8 = dbl_arr[7];
    d9 = dbl_arr[8]; d10 = dbl_arr[9];
    
    f1 = flt_arr[0]; f2 = flt_arr[1]; f3 = flt_arr[2]; f4 = flt_arr[3];
    f5 = flt_arr[4]; f6 = flt_arr[5]; f7 = flt_arr[6]; f8 = flt_arr[7];
    f9 = flt_arr[8]; f10 = flt_arr[9];
    
    ll1 = ll_arr[0]; ll2 = ll_arr[1]; ll3 = ll_arr[2];
    ll4 = ll_arr[3]; ll5 = ll_arr[4];
    
    /* Complex loop with register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex address computations with multi-term expressions */
        idx1 = (i * 7 + v1 * 3 + v2) % ARRAY_SIZE;
        idx2 = (i * 11 + v3 * 5 + v4 * 2) % ARRAY_SIZE;
        idx3 = (i * 13 + v5 * 7 + v6 * 3) % ARRAY_SIZE;
        idx4 = (i * 17 + v7 * 11 + v8 * 5) % ARRAY_SIZE;
        idx5 = (i * 19 + v9 * 13 + v10 * 7) % ARRAY_SIZE;
        
        /* Compute addresses that will need reloads */
        dptr1 = &dbl_arr[idx1];
        dptr2 = &dbl_arr[idx2];
        dptr3 = &dbl_arr[idx3];
        
        iptr1 = &int_arr[idx1];
        iptr2 = &int_arr[idx2];
        iptr3 = &int_arr[idx3];
        
        fptr1 = &flt_arr[idx4];
        fptr2 = &flt_arr[idx5];
        
        llptr1 = &ll_arr[idx1 % 100];
        llptr2 = &ll_arr[idx2 % 100];
        
        /* Control flow to split live ranges */
        switch (i % 8) {
            case 0:
                /* Use inline assembly with conflicting constraints */
                asm volatile (
                    "addl %[val1], %[val2]\n\t"
                    "movl %[val2], %[val3]\n\t"
                    : [val2] "+r" (v2), [val3] "=r" (v3)
                    : [val1] "r" (v1), [mem] "m" (*iptr1)
                    : "cc"
                );
                checksum += v2 + v3;
                break;
                
            case 1:
                /* Different addressing mode */
                asm volatile (
                    "movq (%[addr]), %%rax\n\t"
                    "addq %%rax, %[sum]\n\t"
                    : [sum] "+r" (ll1)
                    : [addr] "r" (llptr1), "m" (*llptr1)
                    : "rax", "cc"
                );
                checksum += (int)ll1;
                break;
                
            case 2:
                /* Mixed register/memory constraints */
                asm volatile (
                    "movsd (%[daddr]), %%xmm0\n\t"
                    "addsd %%xmm0, %[dval]\n\t"
                    : [dval] "+r" (d1)
                    : [daddr] "r" (dptr1), "m" (*dptr1)
                    : "xmm0"
                );
                checksum += (int)d1;
                break;
                
            case 3:
                /* Output address reload */
                asm volatile (
                    "leaq (%[idx],%[idx],2), %%rax\n\t"
                    "movq %%rax, %[out]\n\t"
                    : [out] "=r" (llptr1)
                    : [idx] "r" (idx1)
                    : "rax"
                );
                checksum += (int)*llptr1;
                break;
                
            case 4:
                /* Input address reload with offset */
                asm volatile (
                    "movl 4(%[addr]), %[val]\n\t"
                    : [val] "=r" (v4)
                    : [addr] "r" (iptr2), "m" (*(iptr2 + 1))
                );
                checksum += v4;
                break;
                
            case 5:
                /* Complex addressing in memory operand */
                asm volatile (
                    "imull %[a], %[b]\n\t"
                    "addl %[b], %[c]\n\t"
                    : [b] "+r" (v5), [c] "+r" (v6)
                    : [a] "r" (v7), [mem] "m" (int_arr[(v8 + v9) % ARRAY_SIZE])
                    : "cc"
                );
                checksum += v5 + v6;
                break;
                
            case 6:
                /* Operand address reload */
                {
                    int* complex_addr = &int_arr[(v10 * 3 + v11 * 7) % ARRAY_SIZE];
                    asm volatile (
                        "movl (%[addr]), %%eax\n\t"
                        "addl %%eax, %[sum]\n\t"
                        : [sum] "+r" (checksum)
                        : [addr] "r" (complex_addr), "m" (*complex_addr)
                        : "eax", "cc"
                    );
                }
                break;
                
            case 7:
                /* Other address reload pattern */
                {
                    double* daddr = &dbl_arr[(v12 * 5 + v13 * 11) % ARRAY_SIZE];
                    asm volatile (
                        "movsd (%[addr]), %%xmm0\n\t"
                        "addsd %[val], %%xmm0\n\t"
                        "movsd %%xmm0, %[val]\n\t"
                        : [val] "+r" (d2)
                        : [addr] "r" (daddr), "m" (*daddr)
                        : "xmm0"
                    );
                    checksum += (int)d2;
                }
                break;
        }
        
        /* Call helper functions with different argument combinations */
        if (i % 3 == 0) {
            v1 = helper1(v1, v2, v3, v4, v5, v6);
            d1 = helper2(d1, d2, d3, dptr1);
        } else if (i % 3 == 1) {
            helper3(llptr1, llptr2, &ll_arr[idx3 % 100]);
            helper4(fptr1, iptr1, dptr2);
        } else {
            /* Use computed goto for non-trivial control flow */
            static void* labels[] = { &&L1, &&L2, &&L3, &&L4 };
            goto *labels[i % 4];
            
        L1:
            v7 = int_arr[(v8 + i) % ARRAY_SIZE];
            v8 = int_arr[(v9 + i * 2) % ARRAY_SIZE];
            checksum += v7 + v8;
            continue;
            
        L2:
            v9 = int_arr[(v10 + i * 3) % ARRAY_SIZE];
            v10 = int_arr[(v11 + i * 4) % ARRAY_SIZE];
            checksum += v9 + v10;
            continue;
            
        L3:
            v11 = int_arr[(v12 + i * 5) % ARRAY_SIZE];
            v12 = int_arr[(v13 + i * 6) % ARRAY_SIZE];
            checksum += v11 + v12;
            continue;
            
        L4:
            v13 = int_arr[(v14 + i * 7) % ARRAY_SIZE];
            v14 = int_arr[(v15 + i * 8) % ARRAY_SIZE];
            checksum += v13 + v14;
            continue;
        }
        
        /* Update variables to keep them live */
        v1 = v1 + int_arr[idx1] + 1;
        v2 = v2 + int_arr[idx2] + 2;
        v3 = v3 + int_arr[idx3] + 3;
        v4 = v4 + int_arr[idx4] + 4;
        v5 = v5 + int_arr[idx5] + 5;
        
        d1 = d1 + dbl_arr[idx1] + 1.0;
        d2 = d2 + dbl_arr[idx2] + 2.0;
        d3 = d3 + dbl_arr[idx3] + 3.0;
        
        f1 = f1 + flt_arr[idx4] + 1.0f;
        f2 = f2 + flt_arr[idx5] + 2.0f;
        
        ll1 = ll1 + ll_arr[idx1 % 100];
        ll2 = ll2 + ll_arr[idx2 % 100];
        
        /* Take addresses of locals to force stack-based reloads */
        {
            int* local_addr = &v15;
            double* dlocal_addr = &d4;
            *local_addr += i;
            *dlocal_addr += (double)i;
        }
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays with pattern data */
    int* int_arr = malloc(ARRAY_SIZE * sizeof(int));
    double* dbl_arr = malloc(ARRAY_SIZE * sizeof(double));
    float* flt_arr = malloc(ARRAY_SIZE * sizeof(float));
    long long* ll_arr = malloc(ARRAY_SIZE * sizeof(long long));
    
    if (!int_arr || !dbl_arr || !flt_arr || !ll_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_arr[i] = (i * 37) & 0xFFF;
        dbl_arr[i] = (double)((i * 53) & 0xFFF) / 100.0;
        flt_arr[i] = (float)((i * 71) & 0xFFF) / 100.0f;
        ll_arr[i] = (long long)((i * 89) & 0xFFF);
    }
    
    /* Call the stress function */
    int result = stress_reload(int_arr, dbl_arr, flt_arr, ll_arr);
    
    printf("Checksum result: %d\n", result);
    
    /* Cleanup */
    free(int_arr);
    free(dbl_arr);
    free(flt_arr);
    free(ll_arr);
    
    return 0;
}
