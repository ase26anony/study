/* reload_stress_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline))
#define VOLATILE volatile

/* Opaque functions to prevent optimization */
NOINLINE int64_t opaque1(int64_t a, int64_t b, int64_t c, int64_t d, 
                         int64_t e, int64_t f, int64_t g, int64_t h);
NOINLINE double opaque2(double a, double b, double c, double d,
                       double e, double f, double g, double h);
NOINLINE void* opaque3(void* a, void* b, void* c, void* d);
NOINLINE int64_t opaque4(int64_t* a, double* b, float* c, int64_t* d);

/* Global volatile to prevent optimization */
VOLATILE int64_t global_sink = 0;
VOLATILE double global_double_sink = 0.0;

/* Helper functions with complex parameter passing */
NOINLINE int64_t helper1(int64_t a, int64_t b, int64_t c, int64_t d,
                        int64_t e, int64_t f, int64_t g, int64_t h) {
    VOLATILE int64_t result = a + b - c + d - e + f - g + h;
    global_sink = result;
    return result;
}

NOINLINE double helper2(double a, double b, double c, double d,
                       double e, double f, double g, double h) {
    VOLATILE double result = a * b / c + d * e - f / g + h;
    global_double_sink = result;
    return result;
}

NOINLINE void* helper3(void* a, void* b, void* c, void* d) {
    /* Complex address computation */
    uintptr_t addr = (uintptr_t)a + (uintptr_t)b - (uintptr_t)c + (uintptr_t)d;
    return (void*)(addr & ~0x3); /* Align */
}

NOINLINE int64_t helper4(int64_t* a, double* b, float* c, int64_t* d) {
    VOLATILE int64_t sum = *a + (int64_t)(*b) + (int64_t)(*c) + *d;
    return sum;
}

/* Main stress function */
NOINLINE int64_t stress_reload(int64_t* arr_int, double* arr_double, 
                              float* arr_float, int64_t size) {
    /* Declare many local variables to exhaust registers */
    int64_t var1 = arr_int[0];
    int64_t var2 = arr_int[1];
    int64_t var3 = arr_int[2];
    int64_t var4 = arr_int[3];
    int64_t var5 = arr_int[4];
    int64_t var6 = arr_int[5];
    int64_t var7 = arr_int[6];
    int64_t var8 = arr_int[7];
    int64_t var9 = arr_int[8];
    int64_t var10 = arr_int[9];
    
    double dvar1 = arr_double[0];
    double dvar2 = arr_double[1];
    double dvar3 = arr_double[2];
    double dvar4 = arr_double[3];
    double dvar5 = arr_double[4];
    double dvar6 = arr_double[5];
    double dvar7 = arr_double[6];
    double dvar8 = arr_double[7];
    double dvar9 = arr_double[8];
    double dvar10 = arr_double[9];
    
    float fvar1 = arr_float[0];
    float fvar2 = arr_float[1];
    float fvar3 = arr_float[2];
    float fvar4 = arr_float[3];
    float fvar5 = arr_float[4];
    float fvar6 = arr_float[5];
    float fvar7 = arr_float[6];
    float fvar8 = arr_float[7];
    float fvar9 = arr_float[8];
    float fvar10 = arr_float[9];
    
    /* Additional variables for address computations */
    int64_t addr_base1 = (int64_t)arr_int;
    int64_t addr_base2 = (int64_t)arr_double;
    int64_t addr_base3 = (int64_t)arr_float;
    int64_t offset1 = 0, offset2 = 0, offset3 = 0;
    int64_t index1 = 0, index2 = 0, index3 = 0;
    int64_t temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    double dtemp1 = 0.0, dtemp2 = 0.0;
    float ftemp1 = 0.0f, ftemp2 = 0.0f;
    
    VOLATILE int64_t checksum = 0;
    
    /* Complex loop with multiple addressing modes */
    for (int64_t i = 0; i < 1000; i++) {
        /* Complex array indexing with multi-term expressions */
        index1 = (i * 7 + var1 * 3 + var2) % size;
        index2 = (i * 11 + var3 * 5 + var4) % size;
        index3 = (i * 13 + var5 * 7 + var6) % size;
        
        /* Address computations that need registers */
        offset1 = index1 * sizeof(int64_t);
        offset2 = index2 * sizeof(double);
        offset3 = index3 * sizeof(float);
        
        /* Use inline assembly with conflicting constraints */
        int64_t* addr1;
        double* addr2;
        float* addr3;
        
        /* Force address reloads with 'm' constraint */
        asm volatile (
            "mov %[ptr1], %[base1]\n\t"
            "add %[ptr1], %[off1]\n\t"
            : [ptr1] "=r" (addr1)
            : [base1] "m" (arr_int), [off1] "r" (offset1)
            : "memory"
        );
        
        asm volatile (
            "mov %[ptr2], %[base2]\n\t"
            "add %[ptr2], %[off2]\n\t"
            : [ptr2] "=r" (addr2)
            : [base2] "m" (arr_double), [off2] "r" (offset2)
            : "memory"
        );
        
        asm volatile (
            "mov %[ptr3], %[base3]\n\t"
            "add %[ptr3], %[off3]\n\t"
            : [ptr3] "=r" (addr3)
            : [base3] "m" (arr_float), [off3] "r" (offset3)
            : "memory"
        );
        
        /* Control flow that splits live ranges */
        switch (i % 8) {
            case 0: {
                /* RELOAD_FOR_INPUT_ADDRESS pattern */
                int64_t val1 = *addr1;
                double val2 = *addr2;
                float val3 = *addr3;
                
                /* Force register pressure with many parameters */
                temp1 = helper1(val1, var1, var2, var3, var4, var5, var6, var7);
                dtemp1 = helper2(val2, dvar1, dvar2, dvar3, dvar4, dvar5, dvar6, dvar7);
                
                /* Complex addressing in different basic block */
                int64_t complex_idx = (val1 * 3 + temp1 * 5) % size;
                int64_t* complex_addr = arr_int + complex_idx;
                
                /* Use inline assembly with output address constraint */
                int64_t complex_val;
                asm volatile (
                    "mov %[val], [%[addr]]\n\t"
                    : [val] "=r" (complex_val)
                    : [addr] "r" (complex_addr)
                    : "memory"
                );
                
                checksum += complex_val;
                break;
            }
            
            case 1: {
                /* RELOAD_FOR_OUTPUT_ADDRESS pattern */
                int64_t new_val = var8 + var9 + i;
                *addr1 = new_val;
                
                /* Take address of local for frame pointer usage */
                int64_t* local_addr = &var10;
                
                /* Force output address reload */
                asm volatile (
                    "mov [%[addr]], %[val]\n\t"
                    : 
                    : [addr] "r" (local_addr), [val] "r" (new_val)
                    : "memory"
                );
                
                checksum += new_val;
                break;
            }
            
            case 2: {
                /* RELOAD_FOR_OPERAND_ADDRESS pattern */
                void* ptr1 = (void*)addr1;
                void* ptr2 = (void*)addr2;
                void* ptr3 = (void*)addr3;
                void* ptr4 = (void*)&var1;
                
                void* result = helper3(ptr1, ptr2, ptr3, ptr4);
                checksum += (int64_t)result;
                break;
            }
            
            case 3: {
                /* RELOAD_FOR_INPADDR_ADDRESS pattern */
                int64_t indirect_idx = (*addr1 + var1) % size;
                double* indirect_addr = arr_double + indirect_idx;
                
                /* Multiple levels of indirection */
                double indirect_val = *indirect_addr;
                dvar1 = indirect_val * 2.0;
                
                /* Call with mixed pointer types */
                temp2 = helper4(addr1, indirect_addr, addr3, &var2);
                checksum += temp2;
                break;
            }
            
            case 4: {
                /* RELOAD_FOR_OUTADDR_ADDRESS pattern */
                int64_t* out_addr = arr_int + ((i * 17) % size);
                double* in_addr = arr_double + ((i * 19) % size);
                
                /* Force output address computation */
                int64_t computed = (int64_t)(*in_addr) + var3;
                
                asm volatile (
                    "mov [%[out]], %[val]\n\t"
                    : 
                    : [out] "r" (out_addr), [val] "r" (computed)
                    : "memory"
                );
                
                checksum += computed;
                break;
            }
            
            case 5: {
                /* RELOAD_FOR_OTHER_ADDRESS pattern */
                /* Computed goto for non-trivial control flow */
                static void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
                goto *labels[i % 4];
                
            label1:
                temp3 = *addr1 + var4;
                goto join;
            label2:
                temp3 = *addr1 - var4;
                goto join;
            label3:
                temp3 = *addr1 * var4;
                goto join;
            label4:
                temp3 = *addr1 / (var4 ? var4 : 1);
                goto join;
            join:
                checksum += temp3;
                break;
            }
            
            case 6: {
                /* RELOAD_OTHER pattern with cross-basic-block live ranges */
                int64_t cross_var = *addr1;
                double cross_dvar = *addr2;
                
                if (cross_var > 0) {
                    /* Use in this branch */
                    temp4 = helper1(cross_var, var5, var6, var7, var8, var9, var10, i);
                    
                    /* Address computation that might span basic blocks */
                    int64_t* span_addr = arr_int + (cross_var % size);
                    
                    /* Use in both branches */
                    if (cross_dvar > 0.5) {
                        *span_addr = temp4;
                    } else {
                        *span_addr = -temp4;
                    }
                    
                    checksum += *span_addr;
                }
                break;
            }
            
            case 7: {
                /* Mixed reload types in sequence */
                /* First as data operand */
                int64_t data = *addr1;
                
                /* Then as address operand */
                float* derived_addr = arr_float + (data % size);
                float fval = *derived_addr;
                
                /* Force different reload types for same value */
                asm volatile (
                    "add %[data], %[inc]\n\t"
                    "mov [%[addr]], %[data]\n\t"
                    : [data] "+r" (data)
                    : [inc] "r" (var1), [addr] "r" (addr1)
                    : "memory"
                );
                
                checksum += data + (int64_t)fval;
                break;
            }
        }
        
        /* Update most variables to keep them live */
        var1 = var1 * 3 + i;
        var2 = var2 * 5 - i;
        var3 = var3 * 7 + checksum;
        var4 = var4 * 11 - checksum;
        var5 = (var5 + var6) * 13;
        var6 = (var6 - var7) * 17;
        var7 = var7 * 19 + var8;
        var8 = var8 * 23 - var9;
        var9 = var9 + var10 * 29;
        var10 = var10 * 31 + i;
        
        dvar1 = dvar1 * 1.1 + i;
        dvar2 = dvar2 * 1.2 - i;
        dvar3 = dvar3 * 1.3 + checksum;
        dvar4 = dvar4 * 1.4 - checksum;
        dvar5 = (dvar5 + dvar6) * 1.5;
        dvar6 = (dvar6 - dvar7) * 1.6;
        dvar7 = dvar7 * 1.7 + dvar8;
        dvar8 = dvar8 * 1.8 - dvar9;
        dvar9 = dvar9 + dvar10 * 1.9;
        dvar10 = dvar10 * 2.0 + i;
        
        /* Prevent loop invariant motion */
        VOLATILE int64_t barrier = i;
        (void)barrier;
    }
    
    return checksum;
}

int main() {
    const int64_t SIZE = 10000;
    
    /* Allocate and initialize arrays with pattern data */
    int64_t* arr_int = (int64_t*)malloc(SIZE * sizeof(int64_t));
    double* arr_double = (double*)malloc(SIZE * sizeof(double));
    float* arr_float = (float*)malloc(SIZE * sizeof(float));
    
    if (!arr_int || !arr_double || !arr_float) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int64_t i = 0; i < SIZE; i++) {
        arr_int[i] = i * 3 + 7;
        arr_double[i] = i * 1.5 + 2.3;
        arr_float[i] = i * 0.7f + 1.2f;
    }
    
    /* Call stress function */
    int64_t result = stress_reload(arr_int, arr_double, arr_float, SIZE);
    
    printf("Checksum: %lld\n", (long long)result);
    
    /* Cleanup */
    free(arr_int);
    free(arr_double);
    free(arr_float);
    
    return 0;
}

/* Dummy implementations of opaque functions */
NOINLINE int64_t opaque1(int64_t a, int64_t b, int64_t c, int64_t d,
                        int64_t e, int64_t f, int64_t g, int64_t h) {
    return a + b + c + d + e + f + g + h;
}

NOINLINE double opaque2(double a, double b, double c, double d,
                       double e, double f, double g, double h) {
    return a + b + c + d + e + f + g + h;
}

NOINLINE void* opaque3(void* a, void* b, void* c, void* d) {
    return (void*)((uintptr_t)a ^ (uintptr_t)b ^ (uintptr_t)c ^ (uintptr_t)d);
}

NOINLINE int64_t opaque4(int64_t* a, double* b, float* c, int64_t* d) {
    return *a + (int64_t)(*b) + (int64_t)(*c) + *d;
}
