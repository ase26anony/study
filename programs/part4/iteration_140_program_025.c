/* Test program to stress GCC's reload pass and cover various reload types */
#include <stdio.h>
#include <stdint.h>

/* Function 1: Complex alternative constraints and hard register clobbers */
void test_complex_constraints(void) {
    volatile int arr1[256];
    volatile int arr2[256];
    int i, result1, result2, result3;
    
    /* Initialize arrays */
    for (i = 0; i < 256; i++) {
        arr1[i] = i;
        arr2[i] = 255 - i;
    }
    
    /* Complex asm with alternative constraints and hard register clobbers */
    asm volatile (
        /* Multiple outputs with alternative constraints */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %[in3], %%ebx\n\t"
        "subl %[in4], %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        /* Complex address calculation with clobbered registers */
        "leal (%[base], %[index], 4), %%ecx\n\t"
        "movl (%%ecx), %%edx\n\t"
        "movl %%edx, %[out3]\n\t"
        : [out1] "=r,m" (result1),  /* Alternative: register or memory */
          [out2] "=r,m" (result2),
          [out3] "=r,m" (result3)
        : [in1] "r,m,i" (arr1[10]),  /* Three alternatives */
          [in2] "r,m,i" (arr2[20]),
          [in3] "r,m" (arr1[30]),
          [in4] "r,m" (arr2[40]),
          [base] "r" (arr1),
          [index] "r" (50)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    printf("Test 1 results: %d %d %d\n", result1, result2, result3);
}

/* Function 2: Nested address-of operations on volatile data */
void test_nested_addresses(void) {
    volatile int data[100];
    volatile int* ptrs[10];
    int i, result;
    
    /* Initialize */
    for (i = 0; i < 100; i++) data[i] = i * 2;
    for (i = 0; i < 10; i++) ptrs[i] = &data[i * 10];
    
    /* Complex address calculation that may need reloads */
    register int idx asm("r10") = 25;
    register int base_reg asm("r11") = 5;
    
    /* Multiple levels of address calculation */
    asm volatile (
        "movl %[addr], %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        "addl %[offset], %%ebx\n\t"
        "movl %%ebx, %[result]\n\t"
        : [result] "=r" (result)
        : [addr] "r" (&ptrs[base_reg][idx + __builtin_constant_p(idx) ? 0 : 3]),
          [offset] "i" (100)
        : "eax", "ebx", "memory"
    );
    
    /* Another complex case with address in memory constraint */
    int* volatile addr_ptr = &data[75];
    asm volatile (
        "movl (%[ptr]), %%eax\n\t"
        "movl %%eax, %[out]\n\t"
        : [out] "=r" (result)
        : [ptr] "m" (addr_ptr)  /* Memory constraint on pointer */
        : "eax", "memory"
    );
    
    printf("Test 2 result: %d\n", result);
}

/* Function 3: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int out1, out2, out3, out4, out5, out6, out7, out8, out9, out10;
    
    /* Register-bound variables to increase pressure */
    register int r1 asm("r12") = 100;
    register int r2 asm("r13") = 200;
    register int r3 asm("r14") = 300;
    register int r4 asm("r15") = 400;
    
    /* Large asm with many operands mixing types */
    asm volatile (
        /* Process first group */
        "movl %[a1], %%eax\n\t"
        "addl %[a2], %%eax\n\t"
        "movl %%eax, %[o1]\n\t"
        
        "movl %[a3], %%ebx\n\t"
        "subl %[a4], %%ebx\n\t"
        "movl %%ebx, %[o2]\n\t"
        
        "movl %[a5], %%ecx\n\t"
        "imull %[a6], %%ecx\n\t"
        "movl %%ecx, %[o3]\n\t"
        
        /* Use register-bound variables */
        "movl %[r1], %%edx\n\t"
        "addl %[a7], %%edx\n\t"
        "movl %%edx, %[o4]\n\t"
        
        "movl %[r2], %%esi\n\t"
        "subl %[a8], %%esi\n\t"
        "movl %%esi, %[o5]\n\t"
        
        /* More operations */
        "movl %[a9], %%edi\n\t"
        "addl %[r3], %%edi\n\t"
        "movl %%edi, %[o6]\n\t"
        
        "movl %[a10], %%r8d\n\t"
        "subl %[r4], %%r8d\n\t"
        "movl %%r8d, %[o7]\n\t"
        
        /* Address calculations */
        "leal (%[a1], %[a2], 2), %%r9d\n\t"
        "movl %%r9d, %[o8]\n\t"
        
        "leal (%[a3], %[a4], 4), %%r10d\n\t"
        "movl %%r10d, %[o9]\n\t"
        
        "leal (%[a5], %[a6], 8), %%r11d\n\t"
        "movl %%r11d, %[o10]\n\t"
        
        : [o1] "=r" (out1), [o2] "=r" (out2), [o3] "=r" (out3),
          [o4] "=r" (out4), [o5] "=r" (out5), [o6] "=r" (out6),
          [o7] "=r" (out7), [o8] "=r" (out8), [o9] "=r" (out9),
          [o10] "=r" (out10)
        : [a1] "r,m" (v1), [a2] "r,m" (v2), [a3] "r,m" (v3),
          [a4] "r,m" (v4), [a5] "r,m" (v5), [a6] "r,m" (v6),
          [a7] "r,m" (v7), [a8] "r,m" (v8), [a9] "r,m" (v9),
          [a10] "r,m" (v10),
          [r1] "r" (r1), [r2] "r" (r2), [r3] "r" (r3), [r4] "r" (r4)
        : "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "memory"
    );
    
    printf("Test 3 results: %d %d %d %d %d %d %d %d %d %d\n",
           out1, out2, out3, out4, out5, out6, out7, out8, out9, out10);
}

/* Function 4: Mixed float/integer via bitcast for more reload types */
void test_mixed_types(void) {
    volatile float f1 = 1.5f, f2 = 2.5f;
    volatile double d1 = 3.14159;
    int if1, if2, id1;
    float out_f;
    double out_d;
    
    /* Convert floats to ints via bitcast for integer asm */
    if1 = __builtin_bit_cast(int, f1);
    if2 = __builtin_bit_cast(int, f2);
    id1 = __builtin_bit_cast(int64_t, d1) & 0xFFFFFFFF;
    
    /* Mixed type asm with address calculations */
    volatile float* fptr = &f1;
    asm volatile (
        /* Integer operation on float bits */
        "movl %[bits1], %%eax\n\t"
        "addl %[bits2], %%eax\n\t"
        "movl %%eax, %[out_i]\n\t"
        /* Address calculation for float pointer */
        "movss (%[fptr]), %%xmm0\n\t"
        "addss %[f2], %%xmm0\n\t"
        "movss %%xmm0, %[out_f]\n\t"
        : [out_i] "=r" (if1), [out_f] "=m" (out_f)
        : [bits1] "r,m" (if1), [bits2] "r,m" (if2),
          [fptr] "r" (fptr), [f2] "m" (f2)
        : "eax", "xmm0", "memory"
    );
    
    printf("Test 4: int result = %d, float result = %f\n", if1, out_f);
}

/* Function 5: Conditional address expressions with __builtin_constant_p */
void test_conditional_address(void) {
    volatile int array[100];
    int i, result;
    
    for (i = 0; i < 100; i++) array[i] = i * 3;
    
    /* Conditional address expression */
    int index = 50;
    int* addr = __builtin_constant_p(index) 
                ? &array[10]  /* Constant address */
                : &array[index + 5];  /* Non-constant address */
    
    /* Use in asm with multiple constraints */
    asm volatile (
        "movl (%[addr]), %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=r,m" (result)
        : [addr] "r,m" (addr)
        : "eax", "memory"
    );
    
    /* Another case with output address reload */
    int out_val;
    int* out_ptr = &out_val;
    asm volatile (
        "movl $42, %%eax\n\t"
        "movl %%eax, (%[out_ptr])\n\t"
        : 
        : [out_ptr] "r" (out_ptr)
        : "eax", "memory"
    );
    
    printf("Test 5: array[%d] = %d, out_val = %d\n", 
           __builtin_constant_p(index) ? 10 : index + 5, result, out_val);
}

int main(void) {
    printf("Starting reload pass stress tests...\n");
    
    test_complex_constraints();
    test_nested_addresses();
    test_multi_operand_asm();
    test_mixed_types();
    test_conditional_address();
    
    printf("All tests completed.\n");
    return 0;
}
