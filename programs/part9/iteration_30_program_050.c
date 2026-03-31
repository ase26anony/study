/* reload1_coverage.c - Extreme register pressure test targeting specific reload types */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define LOCAL_VARS 40
#define ITERATIONS 1000

/* Opaque noinline functions to prevent optimization */
__attribute__((noinline)) int helper1(int a, int b, int c, int d, int e, int f, void* addr) {
    volatile int sink = (int)((uintptr_t)addr + a + b + c + d + e + f);
    return sink & 0xFF;
}

__attribute__((noinline)) double helper2(double a, double b, double* addr1, double* addr2, int idx) {
    volatile double sink = *addr1 + *addr2 + a + b + idx;
    return sink;
}

__attribute__((noinline)) long helper3(long a, long b, long c, void* addr, int* arr, int idx) {
    volatile long sink = a + b + c + (long)addr + arr[idx];
    return sink;
}

__attribute__((noinline)) float helper4(float a, float b, float c, float d, 
                                       float* farr, int fidx, int* iarr, int iidx) {
    volatile float sink = a + b + c + d + farr[fidx] + iarr[iidx];
    return sink;
}

/* Main stress function with extreme register pressure */
__attribute__((noinline, noipa))
int stress_reload(int* arr1, double* arr2, float* arr3, long* arr4, 
                  int* arr5, double* arr6) {
    /* Declare many local variables to exhaust registers */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8;
    volatile int* p1, *p2, *p3, *p4;
    volatile double* dp1, *dp2;
    volatile float* fp1, *fp2;
    volatile long* lp1, *lp2;
    
    /* Additional volatile variables to prevent optimization */
    volatile int vi1, vi2, vi3, vi4, vi5;
    volatile double vd1, vd2;
    volatile float vf1, vf2;
    volatile long vl1, vl2;
    
    int result = 0;
    
    /* Initialize with complex expressions */
    v1 = arr1[0] + 1;
    v2 = arr1[1] * 2;
    v3 = arr1[2] / 3;
    v4 = arr1[3] - 4;
    v5 = arr1[4] ^ 5;
    v6 = arr1[5] | 6;
    v7 = arr1[6] & 7;
    v8 = arr1[7] << 2;
    v9 = arr1[8] >> 1;
    v10 = arr1[9] % 11;
    
    d1 = arr2[0] + 1.0;
    d2 = arr2[1] * 2.0;
    d3 = arr2[2] / 3.0;
    d4 = arr2[3] - 4.0;
    d5 = arr2[4] + 5.0;
    d6 = arr2[5] * 6.0;
    d7 = arr2[6] / 7.0;
    d8 = arr2[7] - 8.0;
    d9 = arr2[8] + 9.0;
    d10 = arr2[9] * 10.0;
    
    f1 = arr3[0] + 1.0f;
    f2 = arr3[1] * 2.0f;
    f3 = arr3[2] / 3.0f;
    f4 = arr3[3] - 4.0f;
    f5 = arr3[4] + 5.0f;
    f6 = arr3[5] * 6.0f;
    f7 = arr3[6] / 7.0f;
    f8 = arr3[7] - 8.0f;
    
    l1 = arr4[0] + 1L;
    l2 = arr4[1] * 2L;
    l3 = arr4[2] / 3L;
    l4 = arr4[3] - 4L;
    l5 = arr4[4] + 5L;
    l6 = arr4[5] * 6L;
    l7 = arr4[6] / 7L;
    l8 = arr4[7] - 8L;
    
    /* Take addresses of locals to force stack addressing */
    p1 = &v1; p2 = &v2; p3 = &v3; p4 = &v4;
    dp1 = &d1; dp2 = &d2;
    fp1 = &f1; fp2 = &f2;
    lp1 = &l1; lp2 = &l2;
    
    /* Main loop with extreme register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex index calculations requiring address reloads */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % ARRAY_SIZE;
        int idx2 = (i * 11 + v3 * 13 + v4 * 17) % ARRAY_SIZE;
        int idx3 = (i * 19 + v5 * 23 + v6 * 29) % ARRAY_SIZE;
        int idx4 = (i * 31 + v7 * 37 + v8 * 41) % ARRAY_SIZE;
        int idx5 = (i * 43 + v9 * 47 + v10 * 53) % ARRAY_SIZE;
        
        /* Complex double index calculations */
        int didx1 = (i * 59 + (int)d1 * 61 + (int)d2 * 67) % ARRAY_SIZE;
        int didx2 = (i * 71 + (int)d3 * 73 + (int)d4 * 79) % ARRAY_SIZE;
        
        /* Complex float index calculations */
        int fidx1 = (i * 83 + (int)f1 * 89 + (int)f2 * 97) % ARRAY_SIZE;
        int fidx2 = (i * 101 + (int)f3 * 103 + (int)f4 * 107) % ARRAY_SIZE;
        
        /* Complex long index calculations */
        int lidx1 = (i * 109 + (int)l1 * 113 + (int)l2 * 127) % ARRAY_SIZE;
        int lidx2 = (i * 131 + (int)l3 * 137 + (int)l4 * 139) % ARRAY_SIZE;
        
        /* Control flow to split live ranges */
        switch (i % 8) {
            case 0: {
                /* Block 0: Use idx1, idx2 with inline asm forcing reloads */
                int* addr1 = &arr1[idx1];
                int* addr2 = &arr1[idx2];
                
                /* Inline assembly with conflicting constraints to force reloads */
                int temp1, temp2;
                asm volatile (
                    "mov %[src1], %[dst1]\n\t"
                    "add %[src2], %[dst1]\n\t"
                    "mov %[dst1], %[dst2]"
                    : [dst1] "+r" (temp1), [dst2] "=r" (temp2)
                    : [src1] "m" (*addr1), [src2] "r" (v1)
                    : "cc"
                );
                
                /* Force address reload for input */
                asm volatile (
                    "mov %[addr], %%rax\n\t"
                    "mov (%%rax), %%ebx"
                    : 
                    : [addr] "r" (addr1)
                    : "rax", "rbx", "cc"
                );
                
                result += temp1 + temp2;
                break;
            }
            
            case 1: {
                /* Block 1: Complex addressing with multiple terms */
                double* daddr1 = &arr2[didx1];
                double* daddr2 = &arr2[didx2];
                
                /* Force output address reload */
                double dtemp;
                asm volatile (
                    "movsd %[src], %[dst]"
                    : [dst] "=m" (dtemp)
                    : [src] "x" (d1)
                    : "memory"
                );
                
                /* Use computed addresses */
                dtemp = *daddr1 + *daddr2;
                result += (int)dtemp;
                break;
            }
            
            case 2: {
                /* Block 2: Mixed types and addressing */
                float* faddr1 = &arr3[fidx1];
                float* faddr2 = &arr3[fidx2];
                
                /* Force operand address reload */
                float ftemp1, ftemp2;
                asm volatile (
                    "movss %[src1], %[dst1]\n\t"
                    "addss %[src2], %[dst1]"
                    : [dst1] "=x" (ftemp1)
                    : [src1] "m" (*faddr1), [src2] "x" (f1)
                    : "cc"
                );
                
                ftemp2 = *faddr2 + f2;
                result += (int)(ftemp1 + ftemp2);
                break;
            }
            
            case 3: {
                /* Block 3: Long operations with address computations */
                long* laddr1 = &arr4[lidx1];
                long* laddr2 = &arr4[lidx2];
                
                /* Force other address reload */
                long ltemp;
                asm volatile (
                    "mov %[src], %[dst]"
                    : [dst] "=r" (ltemp)
                    : [src] "m" (*laddr1)
                    : "cc"
                );
                
                ltemp += *laddr2 + l1;
                result += (int)ltemp;
                break;
            }
            
            case 4: {
                /* Block 4: Multiple array accesses with complex indices */
                int idx6 = (idx1 * 149 + idx2 * 151) % ARRAY_SIZE;
                int idx7 = (idx3 * 157 + idx4 * 163) % ARRAY_SIZE;
                
                /* Force inpaddr address reload */
                int* addr3 = &arr5[idx6];
                int* addr4 = &arr5[idx7];
                
                int temp3, temp4;
                asm volatile (
                    "mov %[src1], %[dst1]\n\t"
                    "imul %[src2], %[dst1]"
                    : [dst1] "=r" (temp3)
                    : [src1] "r" (*addr3), [src2] "r" (v3)
                    : "cc"
                );
                
                temp4 = *addr4 * v4;
                result += temp3 + temp4;
                break;
            }
            
            case 5: {
                /* Block 5: Double array with complex addressing */
                int didx3 = (didx1 * 167 + didx2 * 173) % ARRAY_SIZE;
                int didx4 = (didx1 * 179 + didx2 * 181) % ARRAY_SIZE;
                
                /* Force outaddr address reload */
                double* daddr3 = &arr6[didx3];
                double* daddr4 = &arr6[didx4];
                
                double dtemp3, dtemp4;
                asm volatile (
                    "movsd %[src], %[dst]"
                    : [dst] "=m" (*daddr3)
                    : [src] "x" (d3)
                    : "memory"
                );
                
                dtemp4 = *daddr4 * d4;
                result += (int)dtemp4;
                break;
            }
            
            case 6: {
                /* Block 6: Mixed addressing modes */
                /* Force other operand reload */
                int* addr5 = &arr1[(i * 191 + v5 * 193) % ARRAY_SIZE];
                int* addr6 = &arr1[(i * 197 + v6 * 199) % ARRAY_SIZE];
                
                asm volatile (
                    "mov %[addr5], %%rsi\n\t"
                    "mov %[addr6], %%rdi\n\t"
                    "mov (%%rsi), %%eax\n\t"
                    "add (%%rdi), %%eax"
                    : 
                    : [addr5] "r" (addr5), [addr6] "r" (addr6)
                    : "rsi", "rdi", "rax", "cc"
                );
                
                result += *addr5 + *addr6;
                break;
            }
            
            case 7: {
                /* Block 7: Maximum complexity */
                /* Force other input address reload */
                int idx8 = (i * 211 + v7 * 223 + v8 * 227) % ARRAY_SIZE;
                int idx9 = (i * 229 + v9 * 233 + v10 * 239) % ARRAY_SIZE;
                
                int* addr7 = &arr5[idx8];
                int* addr8 = &arr5[idx9];
                
                asm volatile (
                    "mov %[val1], %%eax\n\t"
                    "add %[val2], %%eax\n\t"
                    "mov %%eax, %[out]"
                    : [out] "=m" (*addr7)
                    : [val1] "r" (v7), [val2] "r" (v8)
                    : "rax", "cc", "memory"
                );
                
                result += *addr7 + *addr8;
                break;
            }
        }
        
        /* Call helper functions with many arguments to force register shuffling */
        int h1 = helper1(v1, v2, v3, v4, v5, v6, &arr1[idx1]);
        double h2 = helper2(d1, d2, &arr2[didx1], &arr2[didx2], i);
        long h3 = helper3(l1, l2, l3, &arr4[lidx1], arr1, idx2);
        float h4 = helper4(f1, f2, f3, f4, arr3, fidx1, arr1, idx3);
        
        result += h1 + (int)h2 + (int)h3 + (int)h4;
        
        /* Update local variables to keep them live */
        v1 = arr1[(i + 1) % ARRAY_SIZE];
        v2 = arr1[(i + 2) % ARRAY_SIZE];
        v3 = arr1[(i + 3) % ARRAY_SIZE];
        v4 = arr1[(i + 4) % ARRAY_SIZE];
        v5 = arr1[(i + 5) % ARRAY_SIZE];
        v6 = arr1[(i + 6) % ARRAY_SIZE];
        v7 = arr1[(i + 7) % ARRAY_SIZE];
        v8 = arr1[(i + 8) % ARRAY_SIZE];
        v9 = arr1[(i + 9) % ARRAY_SIZE];
        v10 = arr1[(i + 10) % ARRAY_SIZE];
        
        d1 = arr2[(i + 11) % ARRAY_SIZE];
        d2 = arr2[(i + 12) % ARRAY_SIZE];
        d3 = arr2[(i + 13) % ARRAY_SIZE];
        d4 = arr2[(i + 14) % ARRAY_SIZE];
        d5 = arr2[(i + 15) % ARRAY_SIZE];
        d6 = arr2[(i + 16) % ARRAY_SIZE];
        d7 = arr2[(i + 17) % ARRAY_SIZE];
        d8 = arr2[(i + 18) % ARRAY_SIZE];
        d9 = arr2[(i + 19) % ARRAY_SIZE];
        d10 = arr2[(i + 20) % ARRAY_SIZE];
        
        f1 = arr3[(i + 21) % ARRAY_SIZE];
        f2 = arr3[(i + 22) % ARRAY_SIZE];
        f3 = arr3[(i + 23) % ARRAY_SIZE];
        f4 = arr3[(i + 24) % ARRAY_SIZE];
        f5 = arr3[(i + 25) % ARRAY_SIZE];
        f6 = arr3[(i + 26) % ARRAY_SIZE];
        f7 = arr3[(i + 27) % ARRAY_SIZE];
        f8 = arr3[(i + 28) % ARRAY_SIZE];
        
        l1 = arr4[(i + 29) % ARRAY_SIZE];
        l2 = arr4[(i + 30) % ARRAY_SIZE];
        l3 = arr4[(i + 31) % ARRAY_SIZE];
        l4 = arr4[(i + 32) % ARRAY_SIZE];
        l5 = arr4[(i + 33) % ARRAY_SIZE];
        l6 = arr4[(i + 34) % ARRAY_SIZE];
        l7 = arr4[(i + 35) % ARRAY_SIZE];
        l8 = arr4[(i + 36) % ARRAY_SIZE];
    }
    
    return result;
}

int main() {
    /* Allocate and initialize arrays with pattern data */
    int* arr1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* arr2 = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* arr3 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    long* arr4 = (long*)malloc(ARRAY_SIZE * sizeof(long));
    int* arr5 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* arr6 = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !arr5 || !arr6) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 1.5 + 2.0;
        arr3[i] = i * 0.7f + 1.0f;
        arr4[i] = i * 5L + 3L;
        arr5[i] = i * 7 + 11;
        arr6[i] = i * 2.3 + 4.0;
    }
    
    printf("Starting extreme register pressure test...\n");
    
    /* Call the stress function */
    int result = stress_reload(arr1, arr2, arr3, arr4, arr5, arr6);
    
    printf("Result: %d\n", result);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(arr5);
    free(arr6);
    
    return 0;
}
