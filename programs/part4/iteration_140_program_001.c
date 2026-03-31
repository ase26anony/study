/* Test program to trigger various reload types in GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

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
    
    /* Complex asm with alternative constraints and hard register clobbers */
    asm volatile (
        /* Output operands with alternative constraints */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %[in3], %%eax\n\t"
        "movl %%eax, %[out2]"
        : [out1] "=r,m" (result1),  /* Alternative: register or memory */
          [out2] "=r,m" (result2)   /* Alternative: register or memory */
        : [in1] "r,m,i" (arr1[10]),  /* Three alternatives */
          [in2] "r,m,i" (arr2[20]),  /* Register, memory, or immediate */
          [in3] "r,m,i" (arr1[30])
        : "eax", "ebx", "ecx", "edx", "memory"  /* Multiple hard register clobbers */
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
    
    /* Complex address computations */
    int idx = 50;
    int offset = 25;
    
    /* Multiple asm statements with complex address operands */
    asm volatile (
        "movl (%[addr1]), %%eax\n\t"
        "addl (%[addr2]), %%eax\n\t"
        "movl %%eax, %[result]"
        : [result] "=r" (out1)
        : [addr1] "r" (&volatile_arr[idx + offset]),  /* Address computation */
          [addr2] "r" (&volatile_arr2[idx - offset])  /* Another address computation */
        : "eax", "memory"
    );
    
    /* Even more complex: address of address computation */
    int* ptr1 = &volatile_arr[100];
    int* ptr2 = &volatile_arr[150];
    
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl (%%eax), %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=r" (out2)
        : "b" (ptr1),    /* ebx constraint */
          "c" (ptr2)     /* ecx constraint */
        : "eax", "memory"
    );
    
    printf("Test 2 result: %d, %d\n", out1, out2);
}

/* Function 3: Register variables with explicit binding */
void test_register_variables(void) {
    /* Declare register variables bound to specific registers */
    register int r10_var asm("r10") = 100;
    register int r11_var asm("r11") = 200;
    register int r12_var asm("r12") = 300;
    register int r13_var asm("r13") = 400;
    
    volatile int mem_array[256];
    int i, results[4];
    
    for (i = 0; i < 256; i++) {
        mem_array[i] = i;
    }
    
    /* Complex asm using register variables and memory addresses */
    asm volatile (
        /* Multiple operations forcing register pressure */
        "leal (%[r10], %[r11], 2), %%eax\n\t"
        "addl %[mem1], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        
        "leal (%[r12], %[r13], 4), %%ebx\n\t"
        "subl %[mem2], %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        
        "imull %%eax, %%ebx\n\t"
        "movl %%ebx, %[out3]\n\t"
        
        "movl %[r10], %%ecx\n\t"
        "addl %[r11], %%ecx\n\t"
        "addl %[r12], %%ecx\n\t"
        "addl %[r13], %%ecx\n\t"
        "movl %%ecx, %[out4]"
        : [out1] "=r" (results[0]),
          [out2] "=r" (results[1]),
          [out3] "=r" (results[2]),
          [out4] "=r" (results[3])
        : [r10] "r" (r10_var),
          [r11] "r" (r11_var),
          [r12] "r" (r12_var),
          [r13] "r" (r13_var),
          [mem1] "m" (mem_array[50]),
          [mem2] "m" (mem_array[100])
        : "eax", "ebx", "ecx", "memory"
    );
    
    printf("Test 3 results: %d, %d, %d, %d\n", 
           results[0], results[1], results[2], results[3]);
}

/* Function 4: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    volatile int varr[256];
    int i;
    
    for (i = 0; i < 256; i++) {
        varr[i] = i;
    }
    
    /* Many input/output operands to stress reload */
    int out1, out2, out3, out4, out5, out6, out7, out8, out9, out10;
    int in1 = 100, in2 = 200, in3 = 300, in4 = 400, in5 = 500;
    
    asm volatile (
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out2]\n\t"
        "subl %[in3], %[out3]\n\t"
        "imull %[in4], %[out4]\n\t"
        "andl %[in5], %[out5]\n\t"
        "orl %[mem1], %[out6]\n\t"
        "xorl %[mem2], %[out7]\n\t"
        "leal (%[mem3], %[in1]), %[out8]\n\t"
        "movl %[mem4], %[out9]\n\t"
        "addl %[mem5], %[out10]"
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
        : [in1] "r,i" (in1),
          [in2] "r,i" (in2),
          [in3] "r,i" (in3),
          [in4] "r,i" (in4),
          [in5] "r,i" (in5),
          [mem1] "m" (varr[10]),
          [mem2] "m" (varr[20]),
          [mem3] "m" (varr[30]),
          [mem4] "m" (varr[40]),
          [mem5] "m" (varr[50])
        : "memory"
    );
    
    printf("Test 4 results: %d %d %d %d %d %d %d %d %d %d\n",
           out1, out2, out3, out4, out5, out6, out7, out8, out9, out10);
}

/* Function 5: __builtin_constant_p in address contexts */
void test_builtin_constant(void) {
    volatile int carr[256];
    int i, result;
    
    for (i = 0; i < 256; i++) {
        carr[i] = i * i;
    }
    
    int dynamic_idx = 100;
    
    /* Use __builtin_constant_p to select address expression */
    asm volatile (
        "movl (%[addr]), %%eax\n\t"
        "movl %%eax, %[result]"
        : [result] "=r" (result)
        : [addr] "r" (__builtin_constant_p(dynamic_idx) 
                      ? &carr[50]        /* Constant address */
                      : &carr[dynamic_idx] /* Dynamic address */)
        : "eax", "memory"
    );
    
    /* Another variation with more complex expression */
    int base = 100;
    int offset = 20;
    
    asm volatile (
        "movl (%[ptr]), %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=r" (result)
        : [ptr] "r" (__builtin_constant_p(base + offset)
                     ? &carr[120]  /* Compile-time constant */
                     : &carr[base + offset])  /* Runtime computation */
        : "eax", "memory"
    );
    
    printf("Test 5 result: %d\n", result);
}

/* Main function that runs all tests */
int main(void) {
    printf("Starting reload pass coverage tests...\n");
    
    test_complex_constraints();
    test_nested_addresses();
    test_register_variables();
    test_multi_operand_asm();
    test_builtin_constant();
    
    printf("All tests completed.\n");
    return 0;
}
