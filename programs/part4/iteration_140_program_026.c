/* Test program to trigger various reload types in GCC's reload pass */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Function 1: Complex alternative constraints and hard register clobbers */
void test_complex_constraints(void) {
    volatile int arr1[256];
    volatile int arr2[256];
    int i, result1, result2;
    
    /* Initialize arrays */
    for (i = 0; i < 256; i++) {
        arr1[i] = i;
        arr2[i] = 255 - i;
    }
    
    /* Complex asm with alternative constraints and explicit clobbers */
    asm volatile (
        /* Multiple outputs with alternative constraints */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %[in3], %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        /* Force address reloads with complex addressing */
        "movl (%[addr1],%[idx1],4), %%ebx\n\t"
        "addl %%ebx, %[out1]\n\t"
        : [out1] "=r,m" (result1),  /* Alternative: register or memory */
          [out2] "=r,m" (result2)
        : [in1] "r,i,m" (arr1[10]),  /* Three alternatives */
          [in2] "r,i,m" (arr2[20]),
          [in3] "r,i" (5),
          [addr1] "r" (arr1),
          [idx1] "r" (5)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    printf("Test 1 result: %d, %d\n", result1, result2);
}

/* Function 2: Nested address computations with volatile */
void test_nested_addresses(void) {
    volatile int volatile_arr[256];
    volatile int volatile_arr2[128];
    int i, out1, out2;
    
    /* Initialize */
    for (i = 0; i < 256; i++) {
        volatile_arr[i] = i * 2;
    }
    for (i = 0; i < 128; i++) {
        volatile_arr2[i] = i * 3;
    }
    
    /* Complex address calculations that may need reloads */
    int complex_index = 50;
    const int offset = 10;
    
    /* Multiple asm statements with nested address-of operations */
    asm volatile (
        "movl (%[addr]), %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=r" (out1)
        : [addr] "r" (&volatile_arr[complex_index + offset])
        : "eax", "memory"
    );
    
    /* Even more complex: address of address calculation */
    asm volatile (
        "leal (%[base],%[index],4), %%ecx\n\t"
        "movl (%%ecx), %%edx\n\t"
        "addl (%[addr2],%[idx2],2), %%edx\n\t"
        "movl %%edx, %[result]\n\t"
        : [result] "=r" (out2)
        : [base] "r" (volatile_arr),
          [index] "r" (complex_index),
          [addr2] "r" (volatile_arr2),
          [idx2] "r" (offset)
        : "ecx", "edx", "memory"
    );
    
    printf("Test 2 result: %d, %d\n", out1, out2);
}

/* Function 3: Register variables with explicit binding */
void test_register_variables(void) {
    /* Declare register variables bound to specific registers */
    register int r10_var asm("r10") = 100;
    register int r11_var asm("r11") = 200;
    register int r12_var asm("r12") = 300;
    volatile int mem_array[256];
    int output1, output2, output3;
    int i;
    
    for (i = 0; i < 256; i++) {
        mem_array[i] = i;
    }
    
    /* Force spilling of fixed registers */
    asm volatile (
        /* Use all register-bound variables */
        "movl %[r10], %%eax\n\t"
        "addl %[r11], %%eax\n\t"
        "addl %[r12], %%eax\n\t"
        /* Force address calculation reload */
        "movl (%[mem],%%eax,4), %%ebx\n\t"
        "movl %%ebx, %[out1]\n\t"
        /* More operations to increase pressure */
        "leal (%[r10],%[r11],2), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "imull %[r12], %%ecx\n\t"
        "movl %%ecx, %[out3]\n\t"
        : [out1] "=r" (output1),
          [out2] "=r" (output2),
          [out3] "=r" (output3)
        : [r10] "r" (r10_var),
          [r11] "r" (r11_var),
          [r12] "r" (r12_var),
          [mem] "r" (mem_array)
        : "eax", "ebx", "ecx", "memory"
    );
    
    printf("Test 3 result: %d, %d, %d\n", output1, output2, output3);
}

/* Function 4: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int o1, o2, o3, o4, o5, o6, o7, o8, o9, o10;
    float f1 = 1.5f, f2 = 2.5f;
    uint32_t f1_bits, f2_bits;
    
    /* Convert floats to integers for use in integer asm */
    f1_bits = __builtin_bit_cast(uint32_t, f1);
    f2_bits = __builtin_bit_cast(uint32_t, f2);
    
    /* Large asm with many operands - will stress operand numbering */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %[in3], %%ebx\n\t"
        "subl %[in4], %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        "movl %[in5], %%ecx\n\t"
        "imull %[in6], %%ecx\n\t"
        "movl %%ecx, %[out3]\n\t"
        "movl %[in7], %%edx\n\t"
        "addl %[in8], %%edx\n\t"
        "movl %%edx, %[out4]\n\t"
        "movl %[in9], %%esi\n\t"
        "xorl %[in10], %%esi\n\t"
        "movl %%esi, %[out5]\n\t"
        /* Use float bits */
        "addl %[f1], %%eax\n\t"
        "movl %%eax, %[out6]\n\t"
        "addl %[f2], %%ebx\n\t"
        "movl %%ebx, %[out7]\n\t"
        /* More operations to use all outputs */
        "leal (%%eax,%%ebx,2), %%edi\n\t"
        "movl %%edi, %[out8]\n\t"
        "leal (%%ecx,%%edx,4), %%ebp\n\t"
        "movl %%ebp, %[out9]\n\t"
        "addl %%esi, %%ebp\n\t"
        "movl %%ebp, %[out10]\n\t"
        : [out1] "=r" (o1), [out2] "=r" (o2), [out3] "=r" (o3),
          [out4] "=r" (o4), [out5] "=r" (o5), [out6] "=r" (o6),
          [out7] "=r" (o7), [out8] "=r" (o8), [out9] "=r" (o9),
          [out10] "=r" (o10)
        : [in1] "r" (v1), [in2] "r" (v2), [in3] "r" (v3),
          [in4] "r" (v4), [in5] "r" (v5), [in6] "r" (v6),
          [in7] "r" (v7), [in8] "r" (v8), [in9] "r" (v9),
          [in10] "r" (v10), [f1] "r" (f1_bits), [f2] "r" (f2_bits)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "memory"
    );
    
    printf("Test 4 results: %d %d %d %d %d %d %d %d %d %d\n", 
           o1, o2, o3, o4, o5, o6, o7, o8, o9, o10);
}

/* Function 5: __builtin_constant_p in address contexts */
void test_builtin_constant_p(void) {
    volatile int data[100];
    int i, result;
    
    for (i = 0; i < 100; i++) {
        data[i] = i * 10;
    }
    
    /* Use __builtin_constant_p to create conditional address expressions */
    int dynamic_index = 50;
    
    /* This creates a situation where the reload pass might need to handle
       different reload types based on whether an expression is constant */
    asm volatile (
        "movl %[addr], %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        "movl %%ebx, %[out]\n\t"
        : [out] "=r" (result)
        : [addr] "r" (__builtin_constant_p(dynamic_index) 
                      ? &data[10]  /* constant address */
                      : &data[dynamic_index] /* non-constant address */)
        : "eax", "ebx", "memory"
    );
    
    printf("Test 5 result: %d\n", result);
}

/* Main function that runs all tests */
int main(void) {
    printf("Starting reload pass stress tests...\n");
    
    test_complex_constraints();
    test_nested_addresses();
    test_register_variables();
    test_multi_operand_asm();
    test_builtin_constant_p();
    
    printf("All tests completed.\n");
    return 0;
}
