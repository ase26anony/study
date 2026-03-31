/* reload_stress_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))
#define VOLATILE_VAR volatile

/* Opaque helper functions to prevent optimization */
NOINLINE int helper1(int a, int b, int c, int d, int e, int f);
NOINLINE float helper2(float a, float b, float c, float d, float e);
NOINLINE double helper3(double a, double b, double c, double* addr);
NOINLINE long helper4(long a, long b, long* addr1, long* addr2);
NOINLINE void* helper5(void* a, void* b, void* c, int offset);

/* Implementations (must be in separate compilation unit ideally) */
NOINLINE int helper1(int a, int b, int c, int d, int e, int f) {
    VOLATILE_VAR int sink;
    sink = a + b - c + d - e + f;
    return sink;
}

NOINLINE float helper2(float a, float b, float c, float d, float e) {
    VOLATILE_VAR float sink;
    sink = a * b + c * d - e;
    return sink;
}

NOINLINE double helper3(double a, double b, double c, double* addr) {
    VOLATILE_VAR double sink;
    sink = a + b + c + *addr;
    return sink;
}

NOINLINE long helper4(long a, long b, long* addr1, long* addr2) {
    VOLATILE_VAR long sink;
    sink = a * b + *addr1 - *addr2;
    return sink;
}

NOINLINE void* helper5(void* a, void* b, void* c, int offset) {
    VOLATILE_VAR char* sink;
    sink = (char*)a + (intptr_t)b + (intptr_t)c + offset;
    return (void*)sink;
}

/* Main stress function */
NOINLINE int stress_reload(int* arr_int, double* arr_dbl, 
                          long* arr_long, float* arr_flt, 
                          int size) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    VOLATILE_VAR int* ptr1;
    VOLATILE_VAR double* ptr2;
    VOLATILE_VAR long* ptr3;
    VOLATILE_VAR float* ptr4;
    
    /* Initialize with complex expressions */
    v1 = arr_int[0] * 3;
    v2 = arr_int[1] + 7;
    v3 = arr_int[2] - 5;
    v4 = arr_int[3] * 2;
    v5 = arr_int[4] / 3;
    v6 = arr_int[5] + 11;
    v7 = arr_int[6] - 13;
    v8 = arr_int[7] * 17;
    v9 = arr_int[8] + 19;
    v10 = arr_int[9] - 23;
    
    f1 = arr_flt[0] * 2.0f;
    f2 = arr_flt[1] + 3.0f;
    f3 = arr_flt[2] - 4.0f;
    f4 = arr_flt[3] * 1.5f;
    f5 = arr_flt[4] + 2.5f;
    f6 = arr_flt[5] - 3.5f;
    f7 = arr_flt[6] * 0.5f;
    f8 = arr_flt[7] + 1.5f;
    f9 = arr_flt[8] - 2.5f;
    f10 = arr_flt[9] * 3.5f;
    
    d1 = arr_dbl[0] * 2.0;
    d2 = arr_dbl[1] + 3.0;
    d3 = arr_dbl[2] - 4.0;
    d4 = arr_dbl[3] * 1.5;
    d5 = arr_dbl[4] + 2.5;
    d6 = arr_dbl[5] - 3.5;
    d7 = arr_dbl[6] * 0.5;
    d8 = arr_dbl[7] + 1.5;
    d9 = arr_dbl[8] - 2.5;
    d10 = arr_dbl[9] * 3.5;
    
    l1 = arr_long[0] * 2;
    l2 = arr_long[1] + 3;
    l3 = arr_long[2] - 4;
    l4 = arr_long[3] * 5;
    l5 = arr_long[4] + 6;
    l6 = arr_long[5] - 7;
    l7 = arr_long[6] * 8;
    l8 = arr_long[7] + 9;
    l9 = arr_long[8] - 10;
    l10 = arr_long[9] * 11;
    
    ptr1 = arr_int;
    ptr2 = arr_dbl;
    ptr3 = arr_long;
    ptr4 = arr_flt;
    
    VOLATILE_VAR int result = 0;
    
    /* Complex loop with extreme register pressure */
    for (int i = 0; i < 1000; i++) {
        /* Complex array indexing with multi-term expressions */
        int idx1 = (i * 7 + v1 * 3 + v2 * 5) % size;
        int idx2 = (i * 11 + v3 * 2 + v4 * 7) % size;
        int idx3 = (i * 13 + v5 * 3 + v6 * 11) % size;
        int idx4 = (i * 17 + v7 * 5 + v8 * 13) % size;
        int idx5 = (i * 19 + v9 * 7 + v10 * 17) % size;
        
        /* Address computations that need registers */
        int* addr1 = &arr_int[idx1];
        double* addr2 = &arr_dbl[idx2];
        long* addr3 = &arr_long[idx3];
        float* addr4 = &arr_flt[idx4];
        int* addr5 = &arr_int[idx5];
        
        /* Inline assembly with conflicting constraints to force reloads */
        int temp1, temp2;
        asm volatile (
            "mov %[val1], %[tmp1]\n\t"
            "add %[val2], %[tmp1]\n\t"
            "mov %[tmp1], %[out1]"
            : [out1] "=r" (temp1), [tmp1] "=&r" (temp2)
            : [val1] "rm" (*addr1), [val2] "rm" (v1)
            : "cc"
        );
        
        double temp3;
        asm volatile (
            "movsd %[val1], %[out1]\n\t"
            "addsd %[val2], %[out1]"
            : [out1] "=x" (temp3)
            : [val1] "xm" (*addr2), [val2] "xm" (d1)
            : 
        );
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* RELOAD_FOR_INPUT_ADDRESS pattern */
                int complex_idx = (idx1 * 3 + idx2 * 5 + idx3 * 7) % size;
                int* complex_addr = &arr_int[complex_idx];
                
                /* Use as both data and address */
                int data_val = *complex_addr;
                result += helper1(data_val, v1, v2, v3, v4, v5);
                
                /* Force address to be live across call */
                ptr1 = complex_addr;
                break;
            }
            case 1: {
                /* RELOAD_FOR_OUTPUT_ADDRESS pattern */
                long* out_addr = &arr_long[(idx4 * 2 + idx5 * 3) % size];
                
                /* Inline assembly that modifies memory */
                long old_val;
                asm volatile (
                    "mov %[addr], %[ptr]\n\t"
                    "mov (%[ptr]), %[val]\n\t"
                    "add %[inc], %[val]\n\t"
                    "mov %[val], (%[ptr])"
                    : [val] "=&r" (old_val), [ptr] "=&r" (out_addr)
                    : [addr] "rm" (out_addr), [inc] "rm" (l1)
                    : "memory"
                );
                
                result += old_val;
                break;
            }
            case 2: {
                /* RELOAD_FOR_OPERAND_ADDRESS pattern */
                float* faddr1 = &arr_flt[(idx1 + idx3) % size];
                float* faddr2 = &arr_flt[(idx2 + idx4) % size];
                
                /* Multiple address computations */
                float res1 = helper2(*faddr1, f1, f2, f3, f4);
                float res2 = helper2(*faddr2, f5, f6, f7, f8);
                result += (int)(res1 + res2);
                break;
            }
            case 3: {
                /* RELOAD_FOR_OTHER_ADDRESS pattern */
                /* Complex addressing with multiple steps */
                int base_idx = (i * 23 + v6 * 11) % size;
                int offset_idx = (i * 29 + v7 * 13) % size;
                int final_idx = (base_idx + offset_idx * 3) % size;
                
                double* dbl_addr = &arr_dbl[final_idx];
                double res = helper3(d1, d2, d3, dbl_addr);
                result += (int)res;
                break;
            }
            case 4: {
                /* RELOAD_FOR_INPADDR_ADDRESS pattern */
                /* Take address of an element, then use that address */
                int* indirect_addr = &arr_int[idx1];
                int* final_addr = (int*)helper5(indirect_addr, 
                                              (void*)(intptr_t)idx2,
                                              (void*)(intptr_t)idx3,
                                              v8);
                
                if (final_addr >= arr_int && final_addr < &arr_int[size]) {
                    result += *final_addr;
                }
                break;
            }
            case 5: {
                /* RELOAD_FOR_OUTADDR_ADDRESS pattern */
                long* laddr1 = &arr_long[idx1];
                long* laddr2 = &arr_long[idx2];
                
                long res = helper4(l1, l2, laddr1, laddr2);
                result += res;
                
                /* Force addresses to be live */
                ptr3 = laddr1;
                break;
            }
            case 6: {
                /* RELOAD_OTHER pattern - mixed operations */
                /* Multiple dependent address calculations */
                int* a1 = &arr_int[(i + v1) % size];
                int* a2 = &arr_int[(i + v2) % size];
                int* a3 = &arr_int[(i + v3) % size];
                
                /* Chain of operations */
                int sum = *a1 + *a2 + *a3;
                sum += helper1(v4, v5, v6, v7, v8, v9);
                
                /* Use in another address calculation */
                int* final_addr = &arr_int[sum % size];
                result += *final_addr;
                break;
            }
            case 7: {
                /* RELOAD_FOR_OPADDR_ADDR pattern */
                /* Address of address computation */
                int** addr_of_addr = &ptr1;
                int offset = v10 * 3;
                
                /* Complex pointer arithmetic */
                int* computed_addr = *addr_of_addr + offset;
                if (computed_addr >= arr_int && computed_addr < &arr_int[size]) {
                    result += *computed_addr;
                }
                break;
            }
        }
        
        /* Update local variables to keep them live */
        v1 += arr_int[idx1];
        v2 += arr_int[idx2];
        v3 += arr_int[idx3];
        v4 += temp1;
        v5 += idx4;
        
        f1 += arr_flt[idx1 % size];
        f2 += arr_flt[idx2 % size];
        f3 += temp3;
        
        d1 += arr_dbl[idx1 % size];
        d2 += arr_dbl[idx2 % size];
        
        l1 += arr_long[idx1 % size];
        l2 += arr_long[idx2 % size];
        
        /* More complex updates with cross-type dependencies */
        v6 += (int)f1;
        v7 += (int)d1;
        v8 += (int)l1;
        
        /* Prevent loop invariant motion */
        if (i % 13 == 0) {
            v9 = arr_int[(i * 31) % size];
            v10 = arr_int[(i * 37) % size];
        }
        
        /* Volatile sink to prevent elimination */
        VOLATILE_VAR int sink = result;
        (void)sink;
    }
    
    return result;
}

int main() {
    const int SIZE = 10000;
    
    /* Allocate and initialize arrays with pattern */
    int* arr_int = malloc(SIZE * sizeof(int));
    double* arr_dbl = malloc(SIZE * sizeof(double));
    long* arr_long = malloc(SIZE * sizeof(long));
    float* arr_flt = malloc(SIZE * sizeof(float));
    
    if (!arr_int || !arr_dbl || !arr_long || !arr_flt) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random pattern */
    for (int i = 0; i < SIZE; i++) {
        arr_int[i] = (i * 13 + 7) % 1000;
        arr_dbl[i] = (i * 17 + 11) / 100.0;
        arr_long[i] = (i * 19 + 13) % 2000;
        arr_flt[i] = (i * 23 + 17) / 50.0f;
    }
    
    /* Call stress function */
    int result = stress_reload(arr_int, arr_dbl, arr_long, arr_flt, SIZE);
    
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_dbl);
    free(arr_long);
    free(arr_flt);
    
    return 0;
}
