/* reload_coverage.c - Program to trigger multiple reload types in GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no optimization across function boundaries */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Memory barrier to prevent reordering */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Volatile variables to prevent constant propagation */
static volatile int v1, v2, v3, v4, v5, v6, v7, v8;
static volatile long vl1, vl2;
static volatile double vd1, vd2;

/* Complex addressing structure */
struct nested {
    int *ptr1;
    long **ptr2;
    double ***ptr3;
};

/* Function to trigger multiple reload types */
NO_OPT static double trigger_reloads(
    volatile int idx1, volatile int idx2, 
    volatile long idx3, volatile int idx4,
    volatile double scale1, volatile double scale2)
{
    /* High register pressure: many local variables with overlapping live ranges */
    int local_arr1[100];
    double local_arr2[50];
    long local_arr3[75];
    float local_arr4[60];
    
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    double dtemp1, dtemp2, dtemp3;
    long ltemp1, ltemp2;
    float ftemp1, ftemp2;
    
    /* Pointer chains for complex addressing */
    int *p1, **p2, ***p3;
    double *dp1, **dp2;
    long *lp1, **lp2;
    
    /* Initialize arrays with volatile indices to prevent optimization */
    for (int i = 0; i < 100; i++) {
        local_arr1[i] = i * idx1;
    }
    
    for (int i = 0; i < 50; i++) {
        local_arr2[i] = i * scale1;
    }
    
    /* COMPLEX ADDRESSING PATTERN 1: Multi-level pointer dereferencing */
    /* This should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS */
    p1 = &local_arr1[idx1];
    p2 = &p1;
    p3 = &p2;
    
    /* Force address computation into register */
    temp1 = ***p3 + idx2;
    COMPILER_BARRIER();
    
    /* COMPLEX ADDRESSING PATTERN 2: Multi-dimensional array with volatile indices */
    /* Mix different types to pressure different register classes */
    temp2 = local_arr1[idx1 * idx2 + idx4] + 
            local_arr1[idx3 % 50 + idx2];
    COMPILER_BARRIER();
    
    /* Floating point operations to pressure FP registers */
    dtemp1 = local_arr2[idx1 % 25] * scale2;
    dtemp2 = local_arr2[idx2 % 25] / scale1;
    COMPILER_BARRIER();
    
    /* INLINE ASSEMBLY 1: Memory operand with complex addressing */
    /* Should trigger RELOAD_FOR_OPERAND_ADDRESS */
    __asm__ volatile (
        "mov %[addr], %%rsi\n\t"
        "mov (%%rsi), %[out]\n\t"
        : [out] "=r" (temp3)
        : [addr] "m" (local_arr1[idx1 + idx2 * 2])
        : "rsi", "memory"
    );
    COMPILER_BARRIER();
    
    /* COMPLEX ADDRESSING PATTERN 3: Pointer arithmetic with mixed types */
    lp1 = &local_arr3[0];
    for (int i = 0; i < 20; i++) {
        /* Complex address calculation requiring temporary register */
        ltemp1 = *(lp1 + idx1 + i * idx2);
        temp4 += ltemp1;
    }
    COMPILER_BARRIER();
    
    /* INLINE ASSEMBLY 2: Output address reload */
    /* Should trigger RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    int output_val;
    __asm__ volatile (
        "lea %[in], %%rax\n\t"
        "mov $0x1234, (%%rax)\n\t"
        "mov (%%rax), %[out]\n\t"
        : [out] "=r" (output_val)
        : [in] "m" (local_arr1[idx3 % 30])
        : "rax", "memory"
    );
    COMPILER_BARRIER();
    
    /* Mixed integer/float operations to force moves between register classes */
    ftemp1 = (float)temp1 * 1.5f;
    dtemp3 = (double)temp2 + ftemp1;
    COMPILER_BARRIER();
    
    /* COMPLEX ADDRESSING PATTERN 4: Nested structure access */
    struct nested n;
    n.ptr1 = &local_arr1[0];
    n.ptr2 = &lp1;
    
    /* Multi-level indirection */
    temp5 = *(n.ptr1 + idx1 * 3 + idx2);
    ltemp2 = **(n.ptr2) + idx3;
    COMPILER_BARRIER();
    
    /* INLINE ASSEMBLY 3: Multiple memory inputs */
    /* Should trigger RELOAD_FOR_OTHER_ADDRESS */
    int result;
    __asm__ volatile (
        "mov %[in1], %%rbx\n\t"
        "mov %[in2], %%rcx\n\t"
        "add (%%rbx), %%ecx\n\t"
        "mov %%ecx, %[out]\n\t"
        : [out] "=r" (result)
        : [in1] "m" (&local_arr1[idx1]), 
          [in2] "m" (local_arr2[idx2 % 20])
        : "rbx", "rcx", "memory"
    );
    COMPILER_BARRIER();
    
    /* More register pressure */
    for (int i = 0; i < 10; i++) {
        temp6 += local_arr1[i * idx1 % 50];
        temp7 += local_arr1[i * idx2 % 50];
        temp8 += local_arr1[i * idx4 % 50];
    }
    COMPILER_BARRIER();
    
    /* Final computation mixing all types */
    double final_result = 
        temp1 * scale1 + 
        temp2 * scale2 + 
        dtemp1 + dtemp2 + 
        temp3 + temp4 + 
        output_val + result +
        temp5 + temp6 + temp7 + temp8 +
        ltemp1 + ltemp2 +
        ftemp1 + dtemp3;
    
    return final_result;
}

/* Helper to prevent dead store elimination */
NO_OPT static void use_result(double result) {
    vd1 = result;
    printf("Result: %f\n", result);
}

int main(void) {
    srand(time(NULL));
    
    /* Initialize volatile variables with random values */
    v1 = rand() % 100;
    v2 = rand() % 100 + 1;
    v3 = rand() % 100;
    v4 = rand() % 100;
    v5 = rand() % 100;
    v6 = rand() % 100;
    v7 = rand() % 100;
    v8 = rand() % 100;
    vl1 = rand() % 100;
    vl2 = rand() % 100;
    vd1 = (double)rand() / RAND_MAX * 100.0;
    vd2 = (double)rand() / RAND_MAX * 100.0;
    
    /* Call the function multiple times with different volatile arguments */
    double total = 0.0;
    
    total += trigger_reloads(v1, v2, vl1, v3, vd1, vd2);
    COMPILER_BARRIER();
    
    total += trigger_reloads(v4, v5, vl2, v6, vd2, vd1);
    COMPILER_BARRIER();
    
    total += trigger_reloads(v7, v8, vl1 + vl2, v1 + v2, vd1 + vd2, vd1 - vd2);
    COMPILER_BARRIER();
    
    use_result(total);
    
    return 0;
}
