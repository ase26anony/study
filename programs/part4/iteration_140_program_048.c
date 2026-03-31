/* Test program to trigger various reload types in GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Function 1: Complex alternative constraints and hard register clobbers */
void test_complex_constraints(void) {
    volatile int arr[256];
    int i, result1, result2;
    
    /* Initialize array */
    for (i = 0; i < 256; i++) {
        arr[i] = i * 3;
    }
    
    /* Complex asm with alternative constraints and explicit clobbers */
    asm volatile (
        /* Output with alternative constraint */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        /* Alternative path using memory */
        "movl %[in3], %%ebx\n\t"
        "subl %[in4], %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        /* Force address reload with complex addressing */
        "leal (%[base], %[idx], 4), %%ecx\n\t"
        "movl (%%ecx), %%edx\n\t"
        "addl %%edx, %[out1]\n\t"
        : [out1] "=r,m" (result1),  /* Alternative: register or memory */
          [out2] "=r,m" (result2)
        : [in1] "r,i" (arr[10]),     /* Alternative: register or immediate */
          [in2] "r,i" (arr[20]),
          [in3] "r,m" (arr[30]),     /* Alternative: register or memory */
          [in4] "r,m" (arr[40]),
          [base] "r" (arr),
          [idx] "r" (5)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    printf("Test 1 result: %d, %d\n", result1, result2);
}

/* Function 2: Nested address computations with volatile */
void test_nested_addresses(void) {
    volatile int volatile_arr[256];
    volatile int volatile_arr2[256];
    int i, out1, out2;
    
    for (i = 0; i < 256; i++) {
        volatile_arr[i] = i * 2;
        volatile_arr2[i] = i * 3;
    }
    
    /* Complex address calculation that may need RELOAD_FOR_INPUT_ADDRESS */
    int complex_index = 50;
    
    /* Multiple levels of address taking */
    asm volatile (
        "movl (%[addr1]), %%eax\n\t"
        "addl (%[addr2]), %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        /* Another address calculation */
        "leal 16(%[base], %[idx], 2), %%ebx\n\t"
        "movl (%%ebx), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        : [out1] "=r" (out1),
          [out2] "=r" (out2)
        : [addr1] "r" (&volatile_arr[complex_index + 10]),
          [addr2] "r" (&volatile_arr2[complex_index - 10]),
          [base] "r" (volatile_arr),
          [idx] "r" (complex_index)
        : "eax", "ebx", "ecx", "memory"
    );
    
    /* Even more complex: address of address calculation */
    int* volatile ptr = volatile_arr;
    asm volatile (
        "movl %[ptr], %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        "addl %%ebx, %[out1]\n\t"
        : [out1] "+r" (out1)
        : [ptr] "r" (ptr)
        : "eax", "ebx", "memory"
    );
    
    printf("Test 2 result: %d, %d\n", out1, out2);
}

/* Function 3: Register variables with explicit binding */
void test_register_variables(void) {
    volatile int data[256];
    int i;
    
    for (i = 0; i < 256; i++) {
        data[i] = i * 5;
    }
    
    /* Explicit register-bound variables */
    register int r10_var asm("r10") = 100;
    register int r11_var asm("r11") = 200;
    register int r12_var asm("r12") = 300;
    register int r13_var asm("r13") = 400;
    
    int result1, result2, result3;
    
    /* Force these fixed registers to be used and potentially spilled */
    asm volatile (
        /* Use all register-bound variables */
        "movl %[r10], %%eax\n\t"
        "addl %[r11], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        /* Complex address using register variables */
        "movl %[r12], %%ebx\n\t"
        "leal (%[base], %%ebx, 2), %%ecx\n\t"
        "movl (%%ecx), %%edx\n\t"
        "movl %%edx, %[out2]\n\t"
        /* More operations forcing reloads */
        "movl %[r13], %%esi\n\t"
        "imull %%ebx, %%esi\n\t"
        "movl %%esi, %[out3]\n\t"
        : [out1] "=r" (result1),
          [out2] "=r" (result2),
          [out3] "=r" (result3)
        : [r10] "r" (r10_var),
          [r11] "r" (r11_var),
          [r12] "r" (r12_var),
          [r13] "r" (r13_var),
          [base] "r" (data)
        : "eax", "ebx", "ecx", "edx", "esi", "memory"
    );
    
    printf("Test 3 result: %d, %d, %d\n", result1, result2, result3);
}

/* Function 4: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int out1, out2, out3, out4, out5, out6, out7, out8, out9, out10;
    
    /* Large asm with many operands to maximize conflicts */
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
        "xorl %[in8], %%edx\n\t"
        "movl %%edx, %[out4]\n\t"
        "movl %[in9], %%esi\n\t"
        "orl %[in10], %%esi\n\t"
        "movl %%esi, %[out5]\n\t"
        /* More operations using the results */
        "addl %%eax, %%ebx\n\t"
        "movl %%ebx, %[out6]\n\t"
        "subl %%ecx, %%edx\n\t"
        "movl %%edx, %[out7]\n\t"
        "imull %%esi, %%eax\n\t"
        "movl %%eax, %[out8]\n\t"
        "xorl %%ebx, %%ecx\n\t"
        "movl %%ecx, %[out9]\n\t"
        "orl %%edx, %%esi\n\t"
        "movl %%esi, %[out10]\n\t"
        : [out1] "=r" (out1),
          [out2] "=r" (out2),
          [out3] "=r" (out3),
          [out4] "=r" (out4),
          [out5] "=r" (out5),
          [out6] "=r" (out6),
          [out7] "=r" (out7),
          [out8] "=r" (out8),
          [out9] "=r" (out9),
          [out10] "=r" (out10)
        : [in1] "r" (v1),
          [in2] "r" (v2),
          [in3] "r" (v3),
          [in4] "r" (v4),
          [in5] "r" (v5),
          [in6] "r" (v6),
          [in7] "r" (v7),
          [in8] "r" (v8),
          [in9] "r" (v9),
          [in10] "r" (v10)
        : "eax", "ebx", "ecx", "edx", "esi", "memory"
    );
    
    printf("Test 4 results: %d %d %d %d %d %d %d %d %d %d\n", 
           out1, out2, out3, out4, out5, out6, out7, out8, out9, out10);
}

/* Function 5: __builtin_constant_p in address contexts */
void test_builtin_constant_p(void) {
    volatile int arr[256];
    int i, result = 0;
    
    for (i = 0; i < 256; i++) {
        arr[i] = i;
    }
    
    /* Use __builtin_constant_p to create different reload scenarios */
    for (i = 0; i < 10; i++) {
        int idx = i * 10;
        
        /* Conditional address expression */
        void* addr = __builtin_constant_p(i) 
            ? (void*)&arr[100]  /* Constant address */
            : (void*)&arr[idx]; /* Non-constant address */
        
        asm volatile (
            "movl (%[addr]), %%eax\n\t"
            "addl %%eax, %[sum]\n\t"
            : [sum] "+r" (result)
            : [addr] "r" (addr)
            : "eax", "memory"
        );
    }
    
    /* Another variation with pointer arithmetic */
    int* ptr = arr;
    int offset = 50;
    
    asm volatile (
        "movl %[ptr], %%eax\n\t"
        /* Conditional offset based on constantness */
        "addl %[off], %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        "addl %%ebx, %[sum]\n\t"
        : [sum] "+r" (result)
        : [ptr] "r" (ptr),
          [off] "r" (__builtin_constant_p(offset) ? 50 : offset)
        : "eax", "ebx", "memory"
    );
    
    printf("Test 5 result: %d\n", result);
}

/* Function 6: Mixed float/integer via bitcast */
void test_mixed_types(void) {
    volatile float farr[16];
    volatile double darr[16];
    int i;
    
    for (i = 0; i < 16; i++) {
        farr[i] = i * 1.5f;
        darr[i] = i * 2.5;
    }
    
    /* Convert float to int via bitcast for use in integer asm */
    uint32_t f_as_int = __builtin_bit_cast(uint32_t, farr[5]);
    uint64_t d_as_int = __builtin_bit_cast(uint64_t, darr[5]);
    
    uint64_t result1, result2;
    
    asm volatile (
        /* Use the bitcasted float as integer */
        "movl %[fint], %%eax\n\t"
        "addl $1000, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        /* Use the bitcasted double */
        "movq %[dint], %%rax\n\t"
        "addq $2000, %%rax\n\t"
        "movq %%rax, %[out2]\n\t"
        : [out1] "=r" (result1),
          [out2] "=r" (result2)
        : [fint] "r" (f_as_int),
          [dint] "r" (d_as_int)
        : "rax", "eax", "memory"
    );
    
    printf("Test 6 result: %lu, %lu\n", (unsigned long)result1, (unsigned long)result2);
}

int main(void) {
    printf("Starting reload pass stress tests...\n");
    
    test_complex_constraints();
    test_nested_addresses();
    test_register_variables();
    test_multi_operand_asm();
    test_builtin_constant_p();
    test_mixed_types();
    
    printf("All tests completed.\n");
    return 0;
}
