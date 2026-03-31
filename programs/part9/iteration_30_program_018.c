/* reload_stress_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000

/* Opaque functions to prevent optimization */
int __attribute__((noinline)) use_int(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b - c + d - e + f;
    return sink;
}

double __attribute__((noinline)) use_double(double a, double b, double c, 
                                           double d, double e, double f) {
    volatile double sink = a * b + c - d * e + f;
    return sink;
}

void* __attribute__((noinline)) use_address(void* a, void* b, void* c, 
                                           int d, int e, int f) {
    volatile intptr_t sink = (intptr_t)a + (intptr_t)b - (intptr_t)c + d + e + f;
    return (void*)sink;
}

float __attribute__((noinline)) use_mixed(int a, double b, float c, 
                                         long d, int e, float f) {
    volatile float sink = (float)a + (float)b + c + (float)d + (float)e + f;
    return sink;
}

/* Main stress function */
unsigned long __attribute__((noinline)) 
stress_reload(int* arr_int, double* arr_dbl, float* arr_flt, 
              long* arr_long, short* arr_short) {
    
    /* Declare many local variables to exhaust registers */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    volatile short s1, s2, s3, s4, s5, s6, s7, s8, s9, s10;
    
    /* Additional variables for address computations */
    volatile int* ptr1, *ptr2, *ptr3, *ptr4, *ptr5;
    volatile double* dptr1, *dptr2, *dptr3, *dptr4, *dptr5;
    
    /* Initialize with array values to create dependencies */
    v1 = arr_int[0]; v2 = arr_int[1]; v3 = arr_int[2]; v4 = arr_int[3]; v5 = arr_int[4];
    v6 = arr_int[5]; v7 = arr_int[6]; v8 = arr_int[7]; v9 = arr_int[8]; v10 = arr_int[9];
    
    d1 = arr_dbl[0]; d2 = arr_dbl[1]; d3 = arr_dbl[2]; d4 = arr_dbl[3]; d5 = arr_dbl[4];
    d6 = arr_dbl[5]; d7 = arr_dbl[6]; d8 = arr_dbl[7]; d9 = arr_dbl[8]; d10 = arr_dbl[9];
    
    f1 = arr_flt[0]; f2 = arr_flt[1]; f3 = arr_flt[2]; f4 = arr_flt[3]; f5 = arr_flt[4];
    f6 = arr_flt[5]; f7 = arr_flt[6]; f8 = arr_flt[7]; f9 = arr_flt[8]; f10 = arr_flt[9];
    
    l1 = arr_long[0]; l2 = arr_long[1]; l3 = arr_long[2]; l4 = arr_long[3]; l5 = arr_long[4];
    l6 = arr_long[5]; l7 = arr_long[6]; l8 = arr_long[7]; l9 = arr_long[8]; l10 = arr_long[9];
    
    s1 = arr_short[0]; s2 = arr_short[1]; s3 = arr_short[2]; s4 = arr_short[3]; s5 = arr_short[4];
    s6 = arr_short[5]; s7 = arr_short[6]; s8 = arr_short[7]; s9 = arr_short[8]; s10 = arr_short[9];
    
    /* Take addresses of locals to force stack-based reloads */
    ptr1 = &v1; ptr2 = &v2; ptr3 = &v3; ptr4 = &v4; ptr5 = &v5;
    dptr1 = &d1; dptr2 = &d2; dptr3 = &d3; dptr4 = &d4; dptr5 = &d5;
    
    unsigned long checksum = 0;
    
    /* Complex loop with register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % ARRAY_SIZE;
        int idx2 = (i * 11 + v3 * 2 + v4 * 13) % ARRAY_SIZE;
        int idx3 = (i * 17 + v5 * 7 + v6 * 19) % ARRAY_SIZE;
        int idx4 = (i * 23 + v7 * 11 + v8 * 29) % ARRAY_SIZE;
        int idx5 = (i * 31 + v9 * 13 + v10 * 37) % ARRAY_SIZE;
        
        /* More complex indices for double arrays */
        int didx1 = (i * 41 + l1 * 17 + l2 * 43) % ARRAY_SIZE;
        int didx2 = (i * 47 + l3 * 19 + l4 * 53) % ARRAY_SIZE;
        int didx3 = (i * 59 + l5 * 23 + l6 * 61) % ARRAY_SIZE;
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* Branch 0: Complex address computations */
                int* addr1 = &arr_int[idx1 * 3 + idx2];
                int* addr2 = &arr_int[idx3 * 5 + idx4];
                double* daddr1 = &arr_dbl[didx1 * 2 + didx2];
                
                /* Inline assembly with conflicting constraints */
                int temp1, temp2;
                asm volatile (
                    "mov %[src1], %[dst1]\n\t"
                    "add %[src2], %[dst1]\n\t"
                    "mov %[dst1], %[dst2]"
                    : [dst1] "+r" (temp1), [dst2] "=r" (temp2)
                    : [src1] "r" (*addr1), [src2] "r" (*addr2)
                    : "cc"
                );
                
                /* Use computed addresses */
                checksum += (uintptr_t)addr1 + (uintptr_t)addr2;
                v1 = *addr1 + temp1;
                v2 = *addr2 + temp2;
                
                /* Call with address arguments */
                use_address(addr1, addr2, (void*)&checksum, v1, v2, idx1);
                break;
            }
            
            case 1: {
                /* Branch 1: Different address pattern */
                double* daddr1 = &arr_dbl[(didx1 * 7 + didx2 * 3) % ARRAY_SIZE];
                float* faddr1 = &arr_flt[(idx1 * 11 + idx2 * 5) % ARRAY_SIZE];
                
                /* Inline assembly with memory constraints */
                double dtemp;
                asm volatile (
                    "movsd %[src], %[dst]\n\t"
                    "addsd %[dst], %[dst]"
                    : [dst] "=x" (dtemp)
                    : [src] "m" (*daddr1)
                    : 
                );
                
                d1 = dtemp + *faddr1;
                checksum += (uintptr_t)daddr1;
                
                use_double(d1, *daddr1, d2, d3, d4, d5);
                break;
            }
            
            case 2:
            case 3: {
                /* Branches 2-3: Mixed operations */
                int* addr3 = &arr_int[(idx4 * 17 + idx5 * 13) % ARRAY_SIZE];
                long* laddr1 = &arr_long[(l7 * 7 + l8 * 11) % ARRAY_SIZE];
                
                /* Force output address reload */
                int output_val;
                asm volatile (
                    "mov %[src], %[dst]"
                    : [dst] "=r" (output_val)
                    : [src] "m" (*addr3)
                    :
                );
                
                /* Use as both data and address */
                checksum += output_val + (uintptr_t)addr3;
                v3 = output_val;
                ptr3 = addr3;
                
                use_int(v3, *addr3, v4, v5, v6, (int)*laddr1);
                break;
            }
            
            case 4: {
                /* Branch 4: Nested addressing */
                int complex_idx = (idx1 * idx2 + idx3 * idx4) % ARRAY_SIZE;
                int* nested_addr = &arr_int[complex_idx];
                int** ptr_to_addr = &nested_addr;
                
                /* Multiple indirections */
                checksum += **ptr_to_addr;
                v4 = **ptr_to_addr + complex_idx;
                
                /* Force operand address reload */
                asm volatile (
                    "mov %[ptr], %%rax\n\t"
                    "mov (%%rax), %[val]"
                    : [val] "=r" (v5)
                    : [ptr] "r" (ptr_to_addr)
                    : "rax", "memory"
                );
                break;
            }
            
            case 5:
            case 6:
            case 7: {
                /* Branches 5-7: Various patterns */
                /* Force other address reload types */
                volatile int* volatile_addr = &arr_int[idx5];
                volatile double* volatile_daddr = &arr_dbl[didx3];
                
                /* Use in arithmetic */
                int base_idx = (i * 73) % ARRAY_SIZE;
                int offset = v7 * 2 + v8 * 3;
                int* computed_addr = &arr_int[base_idx + offset];
                
                /* Complex expression forcing address computation into register */
                checksum += (uintptr_t)computed_addr + *computed_addr;
                
                /* Call with mixed types */
                use_mixed(*computed_addr, *volatile_daddr, f1, 
                         (long)volatile_addr, idx1, f2);
                
                /* Update many variables to keep them live */
                v6 = *computed_addr + i;
                v7 = v6 * 2;
                v8 = v7 / 3;
                v9 = v8 + *volatile_addr;
                v10 = v9 - offset;
                break;
            }
        }
        
        /* Update all variables to maintain liveness across iterations */
        v1 = v1 + arr_int[idx1] - i;
        v2 = v2 + arr_int[idx2] * 2;
        v3 = v3 + arr_int[idx3] / 3;
        v4 = v4 ^ arr_int[idx4];
        v5 = v5 | arr_int[idx5];
        
        d1 = d1 + arr_dbl[didx1] * 1.5;
        d2 = d2 - arr_dbl[didx2] * 0.5;
        d3 = d3 * arr_dbl[didx3];
        
        f1 = f1 + arr_flt[idx1 % ARRAY_SIZE];
        f2 = f2 - arr_flt[idx2 % ARRAY_SIZE];
        
        l1 = l1 + arr_long[idx3 % ARRAY_SIZE];
        l2 = l2 - arr_long[idx4 % ARRAY_SIZE];
        
        /* More updates to keep variables live */
        s1 = (s1 + i) & 0xFFFF;
        s2 = (s2 - i) & 0xFFFF;
        
        /* Use pointers to force address reloads */
        *ptr1 = v1;
        *ptr2 = v2;
        *dptr1 = d1;
        *dptr2 = d2;
        
        /* Periodic function calls with many arguments */
        if (i % 13 == 0) {
            use_int(v1, v2, v3, v4, v5, v6);
            use_double(d1, d2, d3, d4, d5, d6);
            use_mixed(v7, d7, f3, l3, v8, f4);
        }
        
        /* Computed goto to create complex control flow */
        if (i % 17 == 0) {
            static void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
            goto *labels[i % 4];
            
        label1:
            checksum += 1;
            continue;
        label2:
            checksum += 2;
            continue;
        label3:
            checksum += 3;
            continue;
        label4:
            checksum += 4;
            continue;
        }
    }
    
    /* Final aggregation */
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (unsigned long)d1 + (unsigned long)d2 + (unsigned long)d3;
    checksum += (unsigned long)f1 + (unsigned long)f2 + (unsigned long)f3;
    checksum += l1 + l2 + l3 + l4 + l5;
    checksum += s1 + s2 + s3 + s4 + s5;
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays */
    int* arr_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* arr_flt = (float*)malloc(ARRAY_SIZE * sizeof(float));
    long* arr_long = (long*)malloc(ARRAY_SIZE * sizeof(long));
    short* arr_short = (short*)malloc(ARRAY_SIZE * sizeof(short));
    
    if (!arr_int || !arr_dbl || !arr_flt || !arr_long || !arr_short) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = i * 3 + 7;
        arr_dbl[i] = i * 1.5 + 2.3;
        arr_flt[i] = i * 0.7f + 1.2f;
        arr_long[i] = i * 5L + 11L;
        arr_short[i] = (i * 13 + 17) & 0xFFFF;
    }
    
    /* Call stress function */
    unsigned long result = stress_reload(arr_int, arr_dbl, arr_flt, 
                                        arr_long, arr_short);
    
    printf("Checksum: %lu\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_flt);
    free(arr_long);
    free(arr_short);
    
    return 0;
}
