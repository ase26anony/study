/* reload_stress_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define NUM_LOCALS 40
#define ITERATIONS 1000

/* Opaque functions to prevent optimization */
__attribute__((noinline)) int helper1(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b + c + d + e + f;
    return sink;
}

__attribute__((noinline)) double helper2(double a, double b, double c, 
                                        double d, double e, double f) {
    volatile double sink = a + b + c + d + e + f;
    return sink;
}

__attribute__((noinline)) void* helper3(void* p1, void* p2, void* p3, 
                                       int i1, int i2, int i3) {
    volatile intptr_t sink = (intptr_t)p1 + (intptr_t)p2 + (intptr_t)p3 + i1 + i2 + i3;
    return (void*)sink;
}

__attribute__((noinline)) long helper4(long a, long b, long c, long d,
                                      long e, long f, long g, long h) {
    volatile long sink = a + b + c + d + e + f + g + h;
    return sink;
}

/* Main stress function */
__attribute__((noinline)) 
unsigned long stress_reload(int* arr_int, double* arr_dbl, 
                           long* arr_long, float* arr_flt) {
    /* Declare many local variables to exhaust registers */
    /* Group 1: Integer variables */
    volatile int v1 = arr_int[0];
    volatile int v2 = arr_int[1];
    volatile int v3 = arr_int[2];
    volatile int v4 = arr_int[3];
    volatile int v5 = arr_int[4];
    volatile int v6 = arr_int[5];
    volatile int v7 = arr_int[6];
    volatile int v8 = arr_int[7];
    volatile int v9 = arr_int[8];
    volatile int v10 = arr_int[9];
    
    /* Group 2: Double variables */
    volatile double d1 = arr_dbl[0];
    volatile double d2 = arr_dbl[1];
    volatile double d3 = arr_dbl[2];
    volatile double d4 = arr_dbl[3];
    volatile double d5 = arr_dbl[4];
    volatile double d6 = arr_dbl[5];
    volatile double d7 = arr_dbl[6];
    volatile double d8 = arr_dbl[7];
    volatile double d9 = arr_dbl[8];
    volatile double d10 = arr_dbl[9];
    
    /* Group 3: Long variables */
    volatile long l1 = arr_long[0];
    volatile long l2 = arr_long[1];
    volatile long l3 = arr_long[2];
    volatile long l4 = arr_long[3];
    volatile long l5 = arr_long[4];
    volatile long l6 = arr_long[5];
    volatile long l7 = arr_long[6];
    volatile long l8 = arr_long[7];
    volatile long l9 = arr_long[8];
    volatile long l10 = arr_long[9];
    
    /* Group 4: Float variables */
    volatile float f1 = arr_flt[0];
    volatile float f2 = arr_flt[1];
    volatile float f3 = arr_flt[2];
    volatile float f4 = arr_flt[3];
    volatile float f5 = arr_flt[4];
    volatile float f6 = arr_flt[5];
    volatile float f7 = arr_flt[6];
    volatile float f8 = arr_flt[7];
    volatile float f9 = arr_flt[8];
    volatile float f10 = arr_flt[9];
    
    /* Additional pointer variables */
    volatile int* p_int = arr_int;
    volatile double* p_dbl = arr_dbl;
    volatile long* p_long = arr_long;
    volatile float* p_flt = arr_flt;
    
    /* Control flow labels for computed goto */
    void* labels[] = { &&label0, &&label1, &&label2, &&label3, 
                      &&label4, &&label5, &&label6, &&label7 };
    
    unsigned long checksum = 0;
    
    for (int i = 0; i < ITERATIONS; i++) {
        /* Complex addressing mode computations */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % ARRAY_SIZE;
        int idx2 = (i * 11 + v3 * 13 + v4 * 17) % ARRAY_SIZE;
        int idx3 = (i * 19 + v5 * 23 + v6 * 29) % ARRAY_SIZE;
        int idx4 = (i * 31 + v7 * 37 + v8 * 41) % ARRAY_SIZE;
        
        /* Force address computations into registers */
        volatile int* addr1 = &arr_int[idx1];
        volatile double* addr2 = &arr_dbl[idx2];
        volatile long* addr3 = &arr_long[idx3];
        volatile float* addr4 = &arr_flt[idx4];
        
        /* Inline assembly with conflicting constraints to force reloads */
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS */
        asm volatile (
            "addl %[val1], %[val2]\n\t"
            "movl %[val2], (%[addr])\n\t"
            : [val2] "+r" (v2)
            : [val1] "r" (v1), [addr] "r" (addr1)
            : "memory"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS */
        int out_val;
        asm volatile (
            "movl (%[in_addr]), %[out]\n\t"
            "addl $1, %[out]\n\t"
            : [out] "=r" (out_val)
            : [in_addr] "r" (addr1)
            : "memory"
        );
        
        /* Mixed type operations to force different reload types */
        checksum += (unsigned long)out_val;
        
        /* Switch statement to split live ranges */
        switch (i % 8) {
            case 0:
                /* RELOAD_FOR_OPERAND_ADDRESS */
                asm volatile (
                    "movq (%[addr]), %%rax\n\t"
                    "addq $1, %%rax\n\t"
                    "movq %%rax, (%[addr])\n\t"
                    : 
                    : [addr] "r" (addr3)
                    : "rax", "memory"
                );
                break;
                
            case 1:
                /* RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
                {
                    volatile long temp;
                    asm volatile (
                        "movq (%[src]), %%rax\n\t"
                        "movq %%rax, (%[dst])\n\t"
                        : 
                        : [src] "r" (addr3), [dst] "r" (&l1)
                        : "rax", "memory"
                    );
                }
                break;
                
            case 2:
                /* Complex addressing with multiple terms */
                idx1 = (idx1 * 3 + idx2 * 5 + idx3 * 7) % ARRAY_SIZE;
                addr1 = &arr_int[idx1];
                
                /* Force RELOAD_FOR_OTHER_ADDRESS */
                asm volatile (
                    "movl (%[base], %[index], 4), %%eax\n\t"
                    "addl %%eax, %[sum]\n\t"
                    : [sum] "+r" (v3)
                    : [base] "r" (arr_int), [index] "r" (idx1)
                    : "eax", "memory"
                );
                break;
                
            case 3:
            case 4:
                /* Nested addressing computations */
                {
                    int* complex_addr = &arr_int[(v1 + v2 * 2 + v3 * 3) % ARRAY_SIZE];
                    /* RELOAD_FOR_OPADDR_ADDR */
                    asm volatile (
                        "leal (%[base], %[idx], 4), %%eax\n\t"
                        "movl %%eax, %[out]\n\t"
                        : [out] "=r" (v4)
                        : [base] "r" (arr_int), [idx] "r" (v5)
                        : "eax"
                    );
                }
                break;
                
            default:
                /* Computed goto for non-trivial control flow */
                goto *labels[i % 8];
        }
        
        /* Backward jump targets for computed goto */
        label0:
            v1 = arr_int[(i + v1) % ARRAY_SIZE];
            goto after_labels;
        label1:
            v2 = arr_int[(i + v2) % ARRAY_SIZE];
            goto after_labels;
        label2:
            v3 = arr_int[(i + v3) % ARRAY_SIZE];
            goto after_labels;
        label3:
            v4 = arr_int[(i + v4) % ARRAY_SIZE];
            goto after_labels;
        label4:
            v5 = arr_int[(i + v5) % ARRAY_SIZE];
            goto after_labels;
        label5:
            v6 = arr_int[(i + v6) % ARRAY_SIZE];
            goto after_labels;
        label6:
            v7 = arr_int[(i + v7) % ARRAY_SIZE];
            goto after_labels;
        label7:
            v8 = arr_int[(i + v8) % ARRAY_SIZE];
            goto after_labels;
            
        after_labels:
        
        /* Call helper functions with many arguments to force register shuffling */
        v9 = helper1(v1, v2, v3, v4, v5, v6);
        d9 = helper2(d1, d2, d3, d4, d5, d6);
        
        /* Pass addresses as arguments */
        helper3((void*)addr1, (void*)addr2, (void*)addr3, v7, v8, v9);
        
        /* Many-argument call */
        l9 = helper4(l1, l2, l3, l4, l5, l6, l7, l8);
        
        /* Update most variables to keep them live */
        v1 = v1 + arr_int[(i * 2) % ARRAY_SIZE];
        v2 = v2 + arr_int[(i * 3) % ARRAY_SIZE];
        v3 = v3 + arr_int[(i * 5) % ARRAY_SIZE];
        v4 = v4 + arr_int[(i * 7) % ARRAY_SIZE];
        v5 = v5 + arr_int[(i * 11) % ARRAY_SIZE];
        v6 = v6 + arr_int[(i * 13) % ARRAY_SIZE];
        
        d1 = d1 + arr_dbl[(i * 2) % ARRAY_SIZE];
        d2 = d2 + arr_dbl[(i * 3) % ARRAY_SIZE];
        d3 = d3 + arr_dbl[(i * 5) % ARRAY_SIZE];
        
        l1 = l1 + arr_long[(i * 2) % ARRAY_SIZE];
        l2 = l2 + arr_long[(i * 3) % ARRAY_SIZE];
        
        f1 = f1 + arr_flt[(i * 2) % ARRAY_SIZE];
        f2 = f2 + arr_flt[(i * 3) % ARRAY_SIZE];
        
        /* Complex checksum update using all variable types */
        checksum += (unsigned long)v1 + (unsigned long)v2 + 
                   (unsigned long)(d1 * 1000) + (unsigned long)(d2 * 1000) +
                   (unsigned long)l1 + (unsigned long)l2 +
                   (unsigned long)(f1 * 1000) + (unsigned long)(f2 * 1000);
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize arrays */
    int* arr_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* arr_dbl = (double*)malloc(ARRAY_SIZE * sizeof(double));
    long* arr_long = (long*)malloc(ARRAY_SIZE * sizeof(long));
    float* arr_flt = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!arr_int || !arr_dbl || !arr_long || !arr_flt) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = i * 3 + 1;
        arr_dbl[i] = i * 1.5 + 0.5;
        arr_long[i] = i * 7L + 3L;
        arr_flt[i] = i * 0.7f + 0.3f;
    }
    
    /* Call the stress function */
    unsigned long result = stress_reload(arr_int, arr_dbl, arr_long, arr_flt);
    
    printf("Checksum result: %lu\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_long);
    free(arr_flt);
    
    return 0;
}
