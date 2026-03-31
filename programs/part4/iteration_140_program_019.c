/* Test program to trigger various reload types in GCC's reload1.cc */
#include <stdio.h>
#include <stdint.h>

/* Force reloads by using volatile arrays and complex address calculations */
volatile int arr1[256];
volatile int arr2[256];
volatile int arr3[256];

/* Test 1: Complex alternative constraints and hard register clobbers */
void test_complex_constraints(void) {
    int a = 123, b = 456, c = 789;
    int result1, result2, result3;
    
    /* Multiple alternative constraints forcing different reload types */
    asm volatile (
        "movl %[input1], %%eax\n\t"
        "addl %[input2], %%eax\n\t"
        "movl %%eax, %[output1]\n\t"
        "leal (%[input3], %%eax, 4), %%ebx\n\t"
        "movl %%ebx, %[output2]\n\t"
        "imull %%eax, %%ebx\n\t"
        "movl %%ebx, %[output3]"
        : [output1] "=r,m" (result1), 
          [output2] "=r,m" (result2),
          [output3] "=r,m" (result3)
        : [input1] "r,m,i" (a),
          [input2] "r,m,i" (b),
          [input3] "r,m,i" (c)
        : "eax", "ebx", "memory", "cc"
    );
    
    printf("Test1 results: %d %d %d\n", result1, result2, result3);
}

/* Test 2: Nested address computations with volatile */
void test_nested_address(void) {
    int idx = 100;
    int offset = 50;
    uintptr_t addr1, addr2, addr3;
    
    /* Complex address calculations that may need RELOAD_FOR_INPUT_ADDRESS */
    asm volatile (
        "movq %[addr1], %%rax\n\t"
        "addq $4, %%rax\n\t"
        "movq %%rax, %[out1]\n\t"
        "movq %[addr2], %%rbx\n\t"
        "subq $8, %%rbx\n\t"
        "movq %%rbx, %[out2]"
        : [out1] "=r" (addr1),
          [out2] "=r" (addr2)
        : [addr1] "r" (&arr1[idx + offset]),
          [addr2] "r" (&arr2[idx * 2 - offset])
        : "rax", "rbx", "memory"
    );
    
    /* Address of address computation - may trigger RELOAD_FOR_INPADDR_ADDRESS */
    int *ptr = &arr3[0];
    asm volatile (
        "movq %[ptrptr], %%rcx\n\t"
        "movq (%%rcx), %%rdx\n\t"
        "movq %%rdx, %[out3]"
        : [out3] "=r" (addr3)
        : [ptrptr] "r" (&ptr)
        : "rcx", "rdx", "memory"
    );
    
    printf("Test2 addresses: %p %p %p\n", 
           (void*)addr1, (void*)addr2, (void*)addr3);
}

/* Test 3: Register variables with explicit binding */
void test_register_variables(void) {
    /* Bind to specific registers to force conflicts */
    register int x asm("r10") = 1000;
    register int y asm("r11") = 2000;
    register int z asm("r12") = 3000;
    
    int result[3];
    int *ptr = &result[0];
    
    /* Complex output addressing - may trigger RELOAD_FOR_OUTPUT_ADDRESS */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "movl %%eax, (%[out1])\n\t"
        "addl %[y], %%eax\n\t"
        "movl %%eax, 4(%[out1])\n\t"
        "addl %[z], %%eax\n\t"
        "movl %%eax, 8(%[out1])"
        : 
        : [x] "r" (x),
          [y] "r" (y),
          [z] "r" (z),
          [out1] "r" (ptr)
        : "eax", "memory"
    );
    
    printf("Test3 results: %d %d %d\n", result[0], result[1], result[2]);
}

/* Test 4: Large multi-operand asm statement */
void test_multi_operand(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int k = 11, l = 12, m = 13, n = 14, o = 15;
    
    int out1, out2, out3, out4, out5;
    int out6, out7, out8, out9, out10;
    
    /* Massive asm with many operands to stress reload logic */
    asm volatile (
        "movl %[a], %%eax\n\t"
        "addl %[b], %%eax\n\t"
        "movl %%eax, %[o1]\n\t"
        "movl %[c], %%ebx\n\t"
        "subl %[d], %%ebx\n\t"
        "movl %%ebx, %[o2]\n\t"
        "imull %[e], %%eax\n\t"
        "movl %%eax, %[o3]\n\t"
        "movl %[f], %%ecx\n\t"
        "andl %[g], %%ecx\n\t"
        "movl %%ecx, %[o4]\n\t"
        "orl  %[h], %%ebx\n\t"
        "movl %%ebx, %[o5]\n\t"
        "movl %[i], %%edx\n\t"
        "xorl %[j], %%edx\n\t"
        "movl %%edx, %[o6]\n\t"
        "movl %[k], %%esi\n\t"
        "addl %[l], %%esi\n\t"
        "movl %%esi, %[o7]\n\t"
        "movl %[m], %%edi\n\t"
        "subl %[n], %%edi\n\t"
        "movl %%edi, %[o8]\n\t"
        "movl %[o], %%r8d\n\t"
        "imull %%eax, %%r8d\n\t"
        "movl %%r8d, %[o9]\n\t"
        "leal (%%eax, %%ebx, 2), %%r9d\n\t"
        "movl %%r9d, %[o10]"
        : [o1] "=r" (out1), [o2] "=r" (out2),
          [o3] "=r" (out3), [o4] "=r" (out4),
          [o5] "=r" (out5), [o6] "=r" (out6),
          [o7] "=r" (out7), [o8] "=r" (out8),
          [o9] "=r" (out9), [o10] "=r" (out10)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f),
          [g] "r" (g), [h] "r" (h), [i] "r" (i),
          [j] "r" (j), [k] "r" (k), [l] "r" (l),
          [m] "r" (m), [n] "r" (n), [o] "r" (o)
        : "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "memory", "cc"
    );
    
    printf("Test4 results: %d %d %d %d %d %d %d %d %d %d\n",
           out1, out2, out3, out4, out5, out6, out7, out8, out9, out10);
}

/* Test 5: __builtin_constant_p in address contexts */
void test_builtin_constant(void) {
    int idx = 100;
    uintptr_t addr;
    
    /* Conditional address expression */
    uintptr_t addr_expr = (uintptr_t)(
        __builtin_constant_p(idx) 
        ? &arr1[100]  /* constant index */
        : &arr1[idx]   /* variable index */
    );
    
    /* May trigger RELOAD_FOR_OPERAND_ADDRESS */
    asm volatile (
        "movq %[expr], %%rax\n\t"
        "addq $16, %%rax\n\t"
        "movq %%rax, %[out]"
        : [out] "=r" (addr)
        : [expr] "r" (addr_expr)
        : "rax", "memory"
    );
    
    printf("Test5 address: %p\n", (void*)addr);
}

/* Test 6: Mixed float/integer via bitcast */
void test_mixed_types(void) {
    float f1 = 3.14f, f2 = 2.71f;
    int i1, i2, i3;
    
    /* Convert floats to integers via bitcast for asm */
    uint32_t f1_bits = __builtin_bit_cast(uint32_t, f1);
    uint32_t f2_bits = __builtin_bit_cast(uint32_t, f2);
    
    /* Mixed type operations */
    asm volatile (
        "movl %[f1bits], %%eax\n\t"
        "movl %[f2bits], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "xorl %%ebx, %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        "subl $0x40000000, %%eax\n\t"
        "movl %%eax, %[out3]"
        : [out1] "=r" (i1),
          [out2] "=r" (i2),
          [out3] "=r" (i3)
        : [f1bits] "r" (f1_bits),
          [f2bits] "r" (f2_bits)
        : "eax", "ebx", "cc"
    );
    
    printf("Test6 results: 0x%x 0x%x 0x%x\n", i1, i2, i3);
}

int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
    }
    
    printf("Starting reload stress tests...\n");
    
    test_complex_constraints();
    test_nested_address();
    test_register_variables();
    test_multi_operand();
    test_builtin_constant();
    test_mixed_types();
    
    printf("All tests completed.\n");
    
    /* Use results to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += arr1[i] + arr2[i] + arr3[i];
    }
    printf("Final sum: %d\n", sum);
    
    return 0;
}
