/* reload_coverage.c - Program to trigger multiple reload types in GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization across function boundaries */
#define NO_OPT __attribute__((noinline, noipa))

/* Memory barrier to prevent reordering */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Volatile variables to prevent constant propagation */
static volatile int v1, v2, v3, v4, v5, v6, v7, v8;
static volatile long vl1, vl2, vl3;
static volatile double vd1, vd2;

/* Complex addressing structure */
struct nested {
    int *ptr1;
    long **ptr2;
    double ***ptr3;
};

NO_OPT static double trigger_reloads(
    volatile int idx1, volatile int idx2, 
    volatile int idx3, volatile int idx4,
    volatile long l_idx1, volatile long l_idx2,
    volatile double scale1, volatile double scale2)
{
    /* Create high register pressure with many live values */
    int local_arr1[100];
    int local_arr2[75];
    double dbl_arr1[50];
    double dbl_arr2[40];
    long long_arr1[60];
    
    /* Intermediate pointers for complex addressing */
    int *ptr_a, *ptr_b, **ptr_c;
    double *dptr_a, **dptr_b;
    long *lptr_a, **lptr_b;
    
    /* Force spills across register classes */
    int int_sum = 0;
    double double_sum = 0.0;
    long long_sum = 0;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < 100; i++) {
        local_arr1[i] = rand() % 1000;
        if (i < 75) local_arr2[i] = rand() % 1000;
        if (i < 50) dbl_arr1[i] = (rand() % 1000) * 0.1;
        if (i < 40) dbl_arr2[i] = (rand() % 1000) * 0.1;
        if (i < 60) long_arr1[i] = rand() % 1000;
    }
    
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS ===== */
    /* Complex multi-dimensional array access with volatile indices */
    for (int i = 0; i < 10; i++) {
        /* Multi-level pointer arithmetic */
        ptr_a = &local_arr1[idx1 + i * idx2];
        ptr_b = &local_arr2[idx3 * i + idx4];
        
        /* Chain dereferencing with mixed types */
        int val1 = *(ptr_a + (idx1 % 10));
        int val2 = *(ptr_b + (idx2 % 5));
        
        /* Force address computation into register */
        int *temp_ptr = ptr_a + (idx3 % 8) * (idx4 % 4);
        int val3 = *temp_ptr;
        
        int_sum += val1 + val2 + val3;
    }
    
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS ===== */
    /* Complex stores with address computation */
    for (int i = 0; i < 8; i++) {
        /* Output with complex address calculation */
        local_arr1[idx1 * i + idx2] = 
            local_arr2[idx3 + i * idx4] * scale1;
        
        /* Another level of indirection */
        int *dest = &local_arr1[(idx1 + i) * (idx2 % 16)];
        *dest = local_arr2[(idx3 * i) % 75] + idx4;
        
        /* Mixed type pointer arithmetic */
        char *char_ptr = (char *)local_arr1;
        char_ptr[(idx1 * i * sizeof(int)) % 400] = 
            (char)(local_arr2[idx2] % 256);
    }
    
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR ===== */
    /* Inline assembly with complex address constraints */
    int asm_result1, asm_result2;
    double asm_double;
    
    /* Assembly that takes memory address as input */
    __asm__ volatile(
        "movl (%[input]), %[output]\n\t"
        : [output] "=r" (asm_result1)
        : [input] "r" (&local_arr1[idx1 + idx2]),
          "m" (local_arr1[idx1 + idx2])
        : "memory"
    );
    
    /* Assembly with output address constraint */
    __asm__ volatile(
        "movq %[in1], %%rax\n\t"
        "addq %[in2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=m" (local_arr2[idx3])
        : [in1] "r" (l_idx1),
          [in2] "r" (l_idx2)
        : "rax", "memory"
    );
    
    COMPILER_BARRIER();
    
    /* ===== RELOAD_FOR_OTHER_ADDRESS and RELOAD_OTHER ===== */
    /* Mixed floating-point and integer computations */
    for (int i = 0; i < 20; i++) {
        /* Complex array addressing with floating point */
        double temp = dbl_arr1[idx1 % 50] * scale1;
        
        /* Convert to integer and back */
        int int_temp = (int)(temp * 100.0);
        double_sum += dbl_arr2[int_temp % 40] * scale2;
        
        /* More complex addressing with type mixing */
        long_arr1[(idx2 + i) % 60] = 
            (long)(dbl_arr1[(idx3 * i) % 50] * 1000.0);
    }
    
    COMPILER_BARRIER();
    
    /* Additional pressure with pointer chains */
    struct nested nested_struct;
    int **pptr1 = &ptr_a;
    double ***pptr2 = &dptr_b;
    
    /* Multi-level indirection */
    ptr_c = &ptr_a;
    dptr_b = &dptr_a;
    lptr_b = &lptr_a;
    
    /* Complex computation using all variables */
    for (int i = 0; i < 5; i++) {
        /* Use all pointer types to increase pressure */
        *ptr_a = local_arr2[idx1 + i];
        **ptr_c = local_arr1[idx2 + i];
        
        dbl_arr1[i] = (**dptr_b) * scale1;
        long_arr1[i] = (**lptr_b) + l_idx1;
        
        /* More inline assembly for operand addresses */
        __asm__ volatile(
            "movsd (%[addr]), %%xmm0\n\t"
            "mulsd %[scale], %%xmm0\n\t"
            "movsd %%xmm0, %[result]\n\t"
            : [result] "=m" (dbl_arr2[i])
            : [addr] "r" (&dbl_arr1[idx4 % 50]),
              [scale] "x" (scale2)
            : "xmm0", "memory"
        );
    }
    
    COMPILER_BARRIER();
    
    /* Final computation using all values */
    double final_result = 0.0;
    for (int i = 0; i < 25; i++) {
        /* Mix all addressing modes */
        final_result += 
            local_arr1[(idx1 * i + idx2) % 100] * 0.01 +
            dbl_arr1[(idx3 + i) % 50] * scale1 +
            (double)long_arr1[(idx4 * i) % 60] * 0.001;
    }
    
    /* Force use of all computed values */
    v1 = int_sum;
    v2 = (int)double_sum;
    vl1 = long_sum;
    vd1 = final_result;
    
    return final_result + int_sum * 0.001 + double_sum;
}

int main(void) {
    srand(time(NULL));
    
    /* Initialize volatile indices with random values */
    v1 = rand() % 100;
    v2 = rand() % 100;
    v3 = rand() % 100;
    v4 = rand() % 100;
    v5 = rand() % 100;
    v6 = rand() % 100;
    v7 = rand() % 100;
    v8 = rand() % 100;
    
    vl1 = rand() % 1000;
    vl2 = rand() % 1000;
    vl3 = rand() % 1000;
    
    vd1 = (rand() % 100) * 0.1;
    vd2 = (rand() % 100) * 0.1;
    
    /* Call the function with volatile arguments */
    double result = trigger_reloads(v1, v2, v3, v4, vl1, vl2, vd1, vd2);
    
    printf("Result: %f\n", result);
    printf("Volatile values: %d %d %ld %f\n", v1, v2, vl1, vd1);
    
    return 0;
}
