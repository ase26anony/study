/* reload1_stress_test.c */
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
    volatile intptr_t sink = (intptr_t)a + (intptr_t)b - (intptr_t)c + d - e + f;
    return (void*)sink;
}

int __attribute__((noinline)) use_mixed(int a, double b, long c, float d, 
                                       int* e, double* f) {
    volatile double sink = a + b + c + d + *e + *f;
    return (int)sink;
}

/* Helper to create complex control flow */
typedef void* label_t;
label_t __attribute__((noinline)) get_label(int idx) {
    static void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    return labels[idx % 6];
}

/* Main stress function */
int __attribute__((noinline)) stress_reload(int* arr_int, double* arr_dbl, 
                                          float* arr_flt, long* arr_lng) {
    /* Declare many local variables to exhaust registers */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    volatile double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    volatile long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Additional pointer variables for address computations */
    int* p1, *p2, *p3, *p4, *p5;
    double* dp1, *dp2, *dp3, *dp4, *dp5;
    float* fp1, *fp2, *fp3, *fp4, *fp5;
    long* lp1, *lp2, *lp3, *lp4, *lp5;
    
    /* Variables for complex index calculations */
    int idx1, idx2, idx3, idx4, idx5;
    double didx1, didx2, didx3;
    
    /* Initialize with values from arrays */
    v1 = arr_int[0]; v2 = arr_int[1]; v3 = arr_int[2]; v4 = arr_int[3];
    v5 = arr_int[4]; v6 = arr_int[5]; v7 = arr_int[6]; v8 = arr_int[7];
    v9 = arr_int[8]; v10 = arr_int[9];
    
    d1 = arr_dbl[0]; d2 = arr_dbl[1]; d3 = arr_dbl[2]; d4 = arr_dbl[3];
    d5 = arr_dbl[4]; d6 = arr_dbl[5]; d7 = arr_dbl[6]; d8 = arr_dbl[7];
    d9 = arr_dbl[8]; d10 = arr_dbl[9];
    
    f1 = arr_flt[0]; f2 = arr_flt[1]; f3 = arr_flt[2]; f4 = arr_flt[3];
    f5 = arr_flt[4]; f6 = arr_flt[5]; f7 = arr_flt[6]; f8 = arr_flt[7];
    f9 = arr_flt[8]; f10 = arr_flt[9];
    
    l1 = arr_lng[0]; l2 = arr_lng[1]; l3 = arr_lng[2]; l4 = arr_lng[3];
    l5 = arr_lng[4]; l6 = arr_lng[5]; l7 = arr_lng[6]; l8 = arr_lng[7];
    l9 = arr_lng[8]; l10 = arr_lng[9];
    
    /* Take addresses of locals to force stack-based addressing */
    p1 = &v1; p2 = &v2; p3 = &v3; p4 = &v4; p5 = &v5;
    dp1 = &d1; dp2 = &d2; dp3 = &d3; dp4 = &d4; dp5 = &d5;
    fp1 = &f1; fp2 = &f2; fp3 = &f3; fp4 = &f4; fp5 = &f5;
    lp1 = &l1; lp2 = &l2; lp3 = &l3; lp4 = &l4; lp5 = &l5;
    
    int checksum = 0;
    
    /* Main loop with extreme register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex index calculations using multiple variables */
        idx1 = (i * v1 + v2 * 3 + v3 * 7 - v4) % ARRAY_SIZE;
        idx2 = (v5 * 5 + v6 * 11 + i * 13 - v7) % ARRAY_SIZE;
        idx3 = (v8 * 17 + v9 * 19 + v10 * 23 + i) % ARRAY_SIZE;
        idx4 = (v1 * 29 + v3 * 31 + v5 * 37 + v7 * 41) % ARRAY_SIZE;
        idx5 = (v2 * 43 + v4 * 47 + v6 * 53 + v8 * 59) % ARRAY_SIZE;
        
        didx1 = (d1 * 2.5 + d2 * 3.7 + d3 * 1.3) / 10.0;
        didx2 = (d4 * 4.2 + d5 * 5.9 + d6 * 2.1) / 8.0;
        didx3 = (d7 * 6.3 + d8 * 7.1 + d9 * 3.8) / 12.0;
        
        /* Complex array accesses with multi-term expressions */
        int* addr1 = &arr_int[(idx1 * 7 + idx2 * 3 + idx3) % ARRAY_SIZE];
        double* addr2 = &arr_dbl[((int)(didx1 * 100) + idx4 * 5) % ARRAY_SIZE];
        float* addr3 = &arr_flt[(idx2 * 11 + idx5 * 13 + i * 17) % ARRAY_SIZE];
        long* addr4 = &arr_lng[(idx3 * 19 + idx1 * 23 + idx4 * 29) % ARRAY_SIZE];
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2, temp3;
        double dtemp1, dtemp2;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "mov %[val1], %[res1]\n\t"
            "add %[val2], %[res1]\n\t"
            "mov %[addr1], %[res2]\n\t"
            : [res1] "=r" (temp1), [res2] "=r" (temp2)
            : [val1] "r" (*addr1), [val2] "r" (v1), 
              [addr1] "m" (addr1)
            : "memory"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS */
        int* output_addr;
        asm volatile (
            "lea (%[base], %[index], 4), %[out]\n\t"
            : [out] "=r" (output_addr)
            : [base] "r" (arr_int), [index] "r" (idx1)
            : "cc"
        );
        
        /* Use computed address */
        *output_addr = temp1 + temp2;
        
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        long complex_offset = (l1 * l2 + l3 * l4) % 256;
        long* complex_addr;
        asm volatile (
            "mov %[base], %[addr]\n\t"
            "add %[offset], %[addr]\n\t"
            : [addr] "=&r" (complex_addr)
            : [base] "r" (arr_lng), [offset] "r" (complex_offset)
            : "cc"
        );
        
        /* Switch statement for control flow splitting */
        switch (i % 8) {
            case 0: {
                /* Branch with address computation */
                int* branch_addr = &arr_int[(idx1 * v1 + v2 * 3) % ARRAY_SIZE];
                checksum += *branch_addr + v1;
                /* Force address to be live across call */
                use_int(*branch_addr, v2, v3, v4, v5, v6);
                break;
            }
            case 1: {
                /* Different address pattern */
                double* branch_daddr = &arr_dbl[((int)(didx2 * 50) + idx2) % ARRAY_SIZE];
                dtemp1 = *branch_daddr * d1;
                use_double(dtemp1, d2, d3, d4, d5, d6);
                break;
            }
            case 2: {
                /* Mixed address types */
                checksum += use_mixed(v7, d7, l7, f7, 
                                    &arr_int[idx3], &arr_dbl[idx4]);
                break;
            }
            case 3: {
                /* Address of address computation */
                void* addr_of_addr = (void*)&addr2;
                use_address(addr_of_addr, (void*)addr3, (void*)addr4, 
                           v8, v9, v10);
                break;
            }
            case 4: {
                /* Complex chain of address computations */
                int** ptr_to_ptr = &p1;
                *ptr_to_ptr = &arr_int[(i * 127 + v1 * 255) % ARRAY_SIZE];
                checksum += **ptr_to_ptr;
                break;
            }
            case 5: {
                /* RELOAD_FOR_OTHER_ADDRESS pattern */
                volatile int* volatile_ptr = &arr_int[idx5];
                for (int j = 0; j < 4; j++) {
                    checksum += volatile_ptr[j];
                }
                break;
            }
            case 6: {
                /* Computed goto for non-trivial control flow */
                label_t target = get_label(i);
                goto *target;
                L0: checksum += 1; goto end_label;
                L1: checksum += 2; goto end_label;
                L2: checksum += 3; goto end_label;
                L3: checksum += 4; goto end_label;
                L4: checksum += 5; goto end_label;
                L5: checksum += 6; goto end_label;
                end_label:;
                break;
            }
            case 7: {
                /* Nested addressing */
                int nested_idx = (idx1 + arr_int[idx2]) % ARRAY_SIZE;
                checksum += arr_int[nested_idx] + arr_int[arr_int[idx3] % ARRAY_SIZE];
                break;
            }
        }
        
        /* Update most variables to keep them live */
        v1 = v1 * 3 + arr_int[idx1];
        v2 = v2 * 5 - arr_int[idx2];
        v3 = v3 * 7 + arr_int[idx3];
        v4 = v4 * 11 - arr_int[idx4];
        v5 = v5 * 13 + arr_int[idx5];
        v6 = v6 * 17 - *addr1;
        v7 = v7 * 19 + temp1;
        v8 = v8 * 23 - temp2;
        v9 = v9 * 29 + *output_addr;
        v10 = v10 * 31 - checksum;
        
        d1 = d1 * 1.1 + arr_dbl[idx1 % ARRAY_SIZE];
        d2 = d2 * 1.3 - arr_dbl[idx2 % ARRAY_SIZE];
        d3 = d3 * 1.7 + arr_dbl[idx3 % ARRAY_SIZE];
        d4 = d4 * 2.1 - *addr2;
        d5 = d5 * 2.5 + didx1;
        
        f1 = f1 * 1.2f + arr_flt[idx4 % ARRAY_SIZE];
        f2 = f2 * 1.4f - arr_flt[idx5 % ARRAY_SIZE];
        f3 = f3 * 1.6f + *addr3;
        
        l1 = l1 * 3 + arr_lng[idx1 % ARRAY_SIZE];
        l2 = l2 * 5 - arr_lng[idx2 % ARRAY_SIZE];
        l3 = l3 * 7 + *addr4;
        l4 = l4 * 11 - complex_offset;
        
        /* Call multiple functions to force register shuffling */
        checksum += use_int(v1, v2, v3, v4, v5, v6);
        dtemp2 = use_double(d1, d2, d3, d4, d5, d6);
        checksum += (int)dtemp2;
        use_address((void*)p1, (void*)p2, (void*)p3, v7, v8, v9);
        checksum += use_mixed(v10, d7, l5, f4, p4, dp1);
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize large arrays */
    int* arr_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float* arr_flt = (float*)malloc(ARRAY_SIZE * sizeof(float));
    long* arr_lng = (long*)malloc(ARRAY_SIZE * sizeof(long));
    
    if (!arr_int || !arr_dbl || !arr_flt || !arr_lng) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = i * 3 + 1;
        arr_dbl[i] = i * 1.5 + 2.7;
        arr_flt[i] = i * 0.7f + 1.3f;
        arr_lng[i] = i * 5L + 3L;
    }
    
    printf("Starting reload stress test...\n");
    
    /* Call the stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_flt, arr_lng);
    
    printf("Checksum result: %d\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_flt);
    free(arr_lng);
    
    return 0;
}
