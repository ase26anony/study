/* reload1_stress_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))
#define VOLATILE_ARRAY(type, name, size) volatile type name[size]

/* Opaque functions to prevent optimization */
NOINLINE int use_int(int a, int b, int c, int d, int e, int f);
NOINLINE double use_double(double a, double b, double c, double d);
NOINLINE void* use_pointer(void* a, void* b, void* c);
NOINLINE long long use_longlong(long long a, long long b, long long c);
NOINLINE float use_float(float a, float b, float c, float d, float e);

/* Implementation of opaque functions */
NOINLINE int use_int(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

NOINLINE double use_double(double a, double b, double c, double d) {
    volatile double sink = a * b - c / (d + 1.0);
    return sink;
}

NOINLINE void* use_pointer(void* a, void* b, void* c) {
    volatile char* sink = (char*)a + ((char*)b - (char*)c);
    return (void*)sink;
}

NOINLINE long long use_longlong(long long a, long long b, long long c) {
    volatile long long sink = a * b + c;
    return sink;
}

NOINLINE float use_float(float a, float b, float c, float d, float e) {
    volatile float sink = a + b * c - d / e;
    return sink;
}

NOINLINE unsigned long stress_reload(
    VOLATILE_ARRAY(int, arr1, 1024),
    VOLATILE_ARRAY(double, arr2, 1024),
    VOLATILE_ARRAY(long long, arr3, 1024),
    VOLATILE_ARRAY(float, arr4, 1024)
) {
    /* Declare many local variables to exhaust registers */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile long long ll1, ll2, ll3, ll4, ll5, ll6, ll7, ll8;
    volatile int* p1, *p2, *p3, *p4, *p5;
    volatile double* dp1, *dp2, *dp3;
    volatile float* fp1, *fp2, *fp3, *fp4;
    
    unsigned long checksum = 0;
    
    /* Initialize with complex expressions */
    v1 = arr1[0] + arr1[1];
    v2 = arr1[2] * arr1[3];
    v3 = arr1[4] - arr1[5];
    v4 = arr1[6] / (arr1[7] + 1);
    v5 = arr1[8] | arr1[9];
    v6 = arr1[10] & arr1[11];
    v7 = arr1[12] ^ arr1[13];
    v8 = arr1[14] << 2;
    v9 = arr1[15] >> 1;
    v10 = arr1[16] % (arr1[17] + 1);
    
    d1 = arr2[0] + arr2[1];
    d2 = arr2[2] * arr2[3];
    d3 = arr2[4] - arr2[5];
    d4 = arr2[6] / (arr2[7] + 1.0);
    d5 = arr2[8] + arr2[9];
    d6 = arr2[10] * arr2[11];
    d7 = arr2[12] - arr2[13];
    d8 = arr2[14] / (arr2[15] + 1.0);
    
    f1 = arr4[0] + arr4[1];
    f2 = arr4[2] * arr4[3];
    f3 = arr4[4] - arr4[5];
    f4 = arr4[6] / (arr4[7] + 1.0f);
    f5 = arr4[8] + arr4[9];
    
    ll1 = arr3[0] + arr3[1];
    ll2 = arr3[2] * arr3[3];
    ll3 = arr3[4] - arr3[5];
    ll4 = arr3[6] ^ arr3[7];
    
    /* Take addresses of locals to force stack-based reloads */
    p1 = &v1; p2 = &v2; p3 = &v3; p4 = &v4; p5 = &v5;
    dp1 = &d1; dp2 = &d2; dp3 = &d3;
    fp1 = &f1; fp2 = &f2; fp3 = &f3; fp4 = &f4;
    
    /* Complex loop with many register pressure points */
    for (int i = 0; i < 1000; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2) % 1024;
        int idx2 = (i * 11 + v3 * 5 + v4) % 1024;
        int idx3 = (i * 13 + v5 * 7 + v6) % 1024;
        int idx4 = (i * 17 + v7 * 11 + v8) % 1024;
        int idx5 = (i * 19 + v9 * 13 + v10) % 1024;
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
                int* addr1 = &arr1[idx1];
                int* addr2 = &arr1[idx2];
                
                /* Inline assembly with conflicting constraints */
                int val1, val2;
                asm volatile (
                    "movl (%1), %0\n\t"
                    "addl %0, %2\n\t"
                    : "=r" (val1), "+r" (addr1), "+r" (val2)
                    : "2" (val2), "m" (*addr2)
                    : "memory"
                );
                
                v11 = val1 + val2;
                checksum += v11;
                
                /* Call with address computations */
                use_pointer(addr1, addr2, p1);
                break;
            }
            
            case 1: {
                /* RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
                double* daddr1 = &arr2[idx1];
                double* daddr2 = &arr2[idx2];
                
                double dval1, dval2;
                asm volatile (
                    "movsd (%1), %0\n\t"
                    "addsd %0, %2\n\t"
                    : "=x" (dval1), "+r" (daddr1), "+x" (dval2)
                    : "2" (dval2), "m" (*daddr2)
                    : "memory"
                );
                
                d9 = dval1 + dval2;
                checksum += (unsigned long)d9;
                
                /* Mixed type function calls */
                use_double(dval1, dval2, *daddr1, *daddr2);
                break;
            }
            
            case 2: {
                /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
                float* faddr1 = &arr4[idx3];
                float* faddr2 = &arr4[idx4];
                
                /* Complex addressing with multiple computations */
                int offset = (v1 * v2 + v3 * v4) % 256;
                float* faddr3 = faddr1 + offset;
                float* faddr4 = faddr2 + (offset * 2);
                
                asm volatile (
                    "movss (%1), %0\n\t"
                    "mulss %0, %2\n\t"
                    : "=x" (f6), "+r" (faddr3), "+x" (f7)
                    : "2" (f7), "r" (faddr4)
                    : "memory"
                );
                
                checksum += (unsigned long)f6;
                break;
            }
            
            case 3: {
                /* RELOAD_OTHER and RELOAD_FOR_OTHER_ADDRESS */
                long long* lladdr1 = &arr3[idx4];
                long long* lladdr2 = &arr3[idx5];
                
                /* Multiple address computations in different basic blocks */
                if (i % 3 == 0) {
                    lladdr1 = &arr3[(idx1 + idx2) % 1024];
                    lladdr2 = &arr3[(idx3 + idx4) % 1024];
                } else {
                    lladdr1 = &arr3[(idx1 * 2) % 1024];
                    lladdr2 = &arr3[(idx2 * 3) % 1024];
                }
                
                long long llval1, llval2;
                asm volatile (
                    "movq (%1), %0\n\t"
                    "addq %0, %2\n\t"
                    : "=r" (llval1), "+r" (lladdr1), "+r" (llval2)
                    : "2" (llval2), "m" (*lladdr2)
                    : "memory"
                );
                
                ll5 = llval1 + llval2;
                checksum += ll5;
                
                use_longlong(llval1, llval2, *lladdr1);
                break;
            }
            
            default: {
                /* Mix of all reload types */
                int* addr3 = &arr1[idx3];
                double* daddr3 = &arr2[idx4];
                float* faddr5 = &arr4[idx5];
                
                /* Multiple inline asm blocks with different constraints */
                int tmp1, tmp2;
                asm volatile (
                    "movl (%1), %0\n\t"
                    : "=r" (tmp1)
                    : "r" (addr3)
                    : "memory"
                );
                
                double dtmp;
                asm volatile (
                    "movsd (%1), %0\n\t"
                    : "=x" (dtmp)
                    : "r" (daddr3)
                    : "memory"
                );
                
                float ftmp;
                asm volatile (
                    "movss (%1), %0\n\t"
                    : "=x" (ftmp)
                    : "r" (faddr5)
                    : "memory"
                );
                
                /* Function calls with many arguments to force spilling */
                v12 = use_int(tmp1, v1, v2, v3, v4, v5);
                f8 = use_float(ftmp, f1, f2, f3, f4);
                d10 = use_double(dtmp, d1, d2, d3);
                
                checksum += v12 + (unsigned long)f8 + (unsigned long)d10;
                break;
            }
        }
        
        /* Update most variables to keep them live */
        v1 = v1 + arr1[idx1 % 1024];
        v2 = v2 - arr1[idx2 % 1024];
        v3 = v3 * (arr1[idx3 % 1024] + 1);
        v4 = v4 / (arr1[idx4 % 1024] + 1);
        v5 = v5 | arr1[idx5 % 1024];
        
        d1 = d1 + arr2[idx1 % 1024];
        d2 = d2 - arr2[idx2 % 1024];
        d3 = d3 * arr2[idx3 % 1024];
        
        f1 = f1 + arr4[idx1 % 1024];
        f2 = f2 - arr4[idx2 % 1024];
        
        ll1 = ll1 + arr3[idx1 % 1024];
        ll2 = ll2 - arr3[idx2 % 1024];
        
        /* Additional complex computation */
        int complex_idx = (i * 23 + v1 * 17 + v2 * 13 + v3 * 11) % 1024;
        v6 = arr1[complex_idx] + v4;
        v7 = arr1[(complex_idx * 3) % 1024] - v5;
        
        /* Pointer arithmetic that may need address reloads */
        p1 = &arr1[(i * 29) % 1024];
        p2 = p1 + (v1 % 64);
        p3 = p2 + (v2 % 32);
        
        /* Use pointers in opaque function */
        use_pointer(p1, p2, p3);
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays with pattern data */
    VOLATILE_ARRAY(int, arr1, 1024);
    VOLATILE_ARRAY(double, arr2, 1024);
    VOLATILE_ARRAY(long long, arr3, 1024);
    VOLATILE_ARRAY(float, arr4, 1024);
    
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 0.5 + 1.0;
        arr3[i] = (long long)i * 7 + 3;
        arr4[i] = i * 0.25f + 0.5f;
    }
    
    printf("Starting stress test...\n");
    unsigned long result = stress_reload(arr1, arr2, arr3, arr4);
    printf("Checksum: %lu\n", result);
    
    return 0;
}
