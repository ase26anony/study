/* reload_stress_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024
#define NUM_LOCALS 40
#define ITERATIONS 1000

/* Opaque functions that compiler cannot analyze */
__attribute__((noinline)) int helper1(int a, int b, int c, int d, int e, int f) {
    volatile int sink = a + b + c + d + e + f;
    return sink & 0xFF;
}

__attribute__((noinline)) double helper2(double a, double b, double c, 
                                        int* addr1, int* addr2) {
    volatile double sink = a + b + c + *addr1 + *addr2;
    return sink;
}

__attribute__((noinline)) long helper3(long a, long b, long c, long d,
                                      float* fptr, double* dptr) {
    volatile long sink = a + b + c + d + (long)*fptr + (long)*dptr;
    return sink;
}

__attribute__((noinline)) void* helper4(void* p1, void* p2, void* p3, 
                                       int offset1, int offset2) {
    volatile char* sink = (char*)p1 + offset1;
    sink = (char*)p2 + offset2;
    return (void*)sink;
}

/* Main stress function */
__attribute__((noinline)) int stress_reload(int* arr_int, double* arr_dbl, 
                                           float* arr_flt, long* arr_lng) {
    /* Declare many local variables to exhaust registers */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    volatile float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
    volatile long l0, l1, l2, l3, l4, l5, l6, l7, l8, l9;
    volatile int* ptr0, *ptr1, *ptr2, *ptr3, *ptr4;
    volatile double* dptr0, *dptr1, *dptr2;
    volatile float* fptr0, *fptr1;
    volatile long* lptr0, *lptr1;
    
    int checksum = 0;
    
    /* Initialize with complex addressing */
    for (int i = 0; i < 10; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + i * 3) % ARRAY_SIZE;
        int idx2 = (i * 11 + i * 5) % ARRAY_SIZE;
        int idx3 = (i * 13 + i * 7) % ARRAY_SIZE;
        
        /* Force address computations into registers */
        switch (i % 4) {
            case 0:
                ptr0 = &arr_int[idx1];
                ptr1 = &arr_int[idx2];
                ptr2 = &arr_int[idx3];
                break;
            case 1:
                dptr0 = &arr_dbl[idx1];
                dptr1 = &arr_dbl[idx2];
                dptr2 = &arr_dbl[idx3];
                break;
            case 2:
                fptr0 = &arr_flt[idx1];
                fptr1 = &arr_flt[idx2];
                break;
            case 3:
                lptr0 = &arr_lng[idx1];
                lptr1 = &arr_lng[idx2];
                break;
        }
    }
    
    /* Main stress loop */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Complex index calculations using multiple local variables */
        int base = iter % 256;
        int offset1 = (base * 3 + 7) & 0xFF;
        int offset2 = (base * 5 + 11) & 0xFF;
        int offset3 = (base * 7 + 13) & 0xFF;
        int offset4 = (base * 11 + 17) & 0xFF;
        
        /* Force many values to be live simultaneously */
        v0 = arr_int[(base + offset1) % ARRAY_SIZE];
        v1 = arr_int[(base + offset2) % ARRAY_SIZE];
        v2 = arr_int[(base + offset3) % ARRAY_SIZE];
        v3 = arr_int[(base + offset4) % ARRAY_SIZE];
        
        d0 = arr_dbl[(base * 2 + offset1) % ARRAY_SIZE];
        d1 = arr_dbl[(base * 2 + offset2) % ARRAY_SIZE];
        d2 = arr_dbl[(base * 2 + offset3) % ARRAY_SIZE];
        
        f0 = arr_flt[(base * 3 + offset1) % ARRAY_SIZE];
        f1 = arr_flt[(base * 3 + offset2) % ARRAY_SIZE];
        
        l0 = arr_lng[(base * 4 + offset1) % ARRAY_SIZE];
        l1 = arr_lng[(base * 4 + offset2) % ARRAY_SIZE];
        
        /* Complex addressing modes that need address reloads */
        int idx_complex1 = (v0 * 7 + v1 * 3 + v2 * 5) % ARRAY_SIZE;
        int idx_complex2 = (v3 * 11 + v0 * 13 + v1 * 17) % ARRAY_SIZE;
        int idx_complex3 = (v2 * 19 + v3 * 23 + base * 29) % ARRAY_SIZE;
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2, temp3;
        double dtemp1, dtemp2;
        
        /* RELOAD_FOR_INPUT and RELOAD_FOR_INPUT_ADDRESS patterns */
        asm volatile (
            "mov %[val1], %[tmp1]\n\t"
            "add %[val2], %[tmp1]\n\t"
            "mov %[tmp1], %[out1]"
            : [out1] "=r" (temp1), [tmp1] "=&r" (temp2)
            : [val1] "r" (arr_int[idx_complex1]), 
              [val2] "r" (arr_int[idx_complex2])
            : "cc"
        );
        
        /* RELOAD_FOR_OUTPUT_ADDRESS pattern */
        void* addr1, *addr2, *addr3;
        asm volatile (
            "lea (%[base], %[idx1], 4), %[addr1]\n\t"
            "lea (%[base], %[idx2], 8), %[addr2]\n\t"
            "add %[addr1], %[addr2]\n\t"
            "mov %[addr2], %[addr3]"
            : [addr1] "=&r" (addr1), [addr2] "=&r" (addr2), [addr3] "=r" (addr3)
            : [base] "r" (arr_int), [idx1] "r" (idx_complex1), [idx2] "r" (idx_complex2)
            : "cc"
        );
        
        /* Mixed types with memory constraints */
        asm volatile (
            "movsd (%[dptr]), %%xmm0\n\t"
            "addsd %%xmm1, %%xmm0\n\t"
            "movsd %%xmm0, %[result]"
            : [result] "=m" (dtemp1)
            : [dptr] "r" (&arr_dbl[idx_complex3]), "m" (arr_dbl[idx_complex3])
            : "xmm0", "xmm1", "cc"
        );
        
        /* Control flow that splits live ranges */
        switch (iter % 8) {
            case 0: {
                /* Address computation in one branch */
                int* computed_addr = &arr_int[(v0 * 3 + v1 * 5) % ARRAY_SIZE];
                /* Use in another context - forces RELOAD_FOR_OTHER_ADDRESS */
                checksum += helper1(v0, v1, v2, v3, *computed_addr, base);
                break;
            }
            case 1: {
                double* daddr = &arr_dbl[(v1 * 7 + v2 * 11) % ARRAY_SIZE];
                checksum += (int)helper2(d0, d1, d2, &v0, &v1);
                /* Keep address live across call */
                dtemp2 = *daddr + d0;
                break;
            }
            case 2: {
                float* faddr = &arr_flt[(v2 * 13 + v3 * 17) % ARRAY_SIZE];
                checksum += helper3(l0, l1, (long)v0, (long)v1, faddr, &d0);
                break;
            }
            case 3: {
                /* RELOAD_FOR_OPERAND_ADDRESS pattern */
                void* opaque = helper4(&arr_int[idx_complex1], 
                                      &arr_int[idx_complex2],
                                      &arr_int[idx_complex3],
                                      offset1, offset2);
                checksum += *(int*)opaque;
                break;
            }
            case 4: {
                /* Complex addressing with multiple terms */
                int idx = (iter * 17 + v0 * 19 + v1 * 23) % ARRAY_SIZE;
                int idx2 = (iter * 29 + v2 * 31 + v3 * 37) % ARRAY_SIZE;
                checksum += arr_int[idx] + arr_int[idx2];
                break;
            }
            case 5: {
                /* Force stack addressing */
                int local_array[16];
                for (int j = 0; j < 16; j++) {
                    local_array[j] = arr_int[(iter + j) % ARRAY_SIZE];
                }
                /* Use address of local - forces frame pointer usage */
                checksum += helper1(local_array[0], local_array[1],
                                   local_array[2], local_array[3],
                                   local_array[4], local_array[5]);
                break;
            }
            case 6: {
                /* Computed goto to create non-trivial control flow */
                static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
                goto *labels[iter % 4];
                
                label0:
                    checksum += arr_int[(v0 + offset1) % ARRAY_SIZE];
                    goto end_switch;
                label1:
                    checksum += arr_int[(v1 + offset2) % ARRAY_SIZE];
                    goto end_switch;
                label2:
                    checksum += arr_int[(v2 + offset3) % ARRAY_SIZE];
                    goto end_switch;
                label3:
                    checksum += arr_int[(v3 + offset4) % ARRAY_SIZE];
                    goto end_switch;
                end_switch:
                break;
            }
            case 7: {
                /* Multiple nested addressing computations */
                int* addr_array[4];
                addr_array[0] = &arr_int[(v0 * 2) % ARRAY_SIZE];
                addr_array[1] = &arr_int[(v1 * 3) % ARRAY_SIZE];
                addr_array[2] = &arr_int[(v2 * 5) % ARRAY_SIZE];
                addr_array[3] = &arr_int[(v3 * 7) % ARRAY_SIZE];
                
                for (int j = 0; j < 4; j++) {
                    checksum += *addr_array[j];
                }
                break;
            }
        }
        
        /* Update most local variables to keep them live */
        v0 = (v0 * 3 + 1) & 0xFF;
        v1 = (v1 * 5 + 1) & 0xFF;
        v2 = (v2 * 7 + 1) & 0xFF;
        v3 = (v3 * 11 + 1) & 0xFF;
        
        d0 = d0 * 1.1 + 0.5;
        d1 = d1 * 1.2 + 0.6;
        
        f0 = f0 * 1.3f + 0.7f;
        f1 = f1 * 1.4f + 0.8f;
        
        l0 = (l0 * 3 + 1) & 0xFFFF;
        l1 = (l1 * 5 + 1) & 0xFFFF;
        
        /* Prevent optimization of address computations */
        volatile int sink_addr = (int)(uintptr_t)&arr_int[idx_complex1];
        checksum += sink_addr & 1;
    }
    
    return checksum;
}

int main() {
    /* Allocate and initialize large arrays */
    int* arr_int = malloc(ARRAY_SIZE * sizeof(int));
    double* arr_dbl = malloc(ARRAY_SIZE * sizeof(double));
    float* arr_flt = malloc(ARRAY_SIZE * sizeof(float));
    long* arr_lng = malloc(ARRAY_SIZE * sizeof(long));
    
    if (!arr_int || !arr_dbl || !arr_flt || !arr_lng) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_int[i] = (i * 3 + 7) & 0xFF;
        arr_dbl[i] = (i * 5 + 11) / 100.0;
        arr_flt[i] = (i * 7 + 13) / 50.0f;
        arr_lng[i] = (i * 11 + 17) & 0xFFFF;
    }
    
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
