/* Test program to exercise GCC's reload pass for various reload types */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force reloads by using volatile arrays */
volatile int arr1[256];
volatile long arr2[256];
volatile double arr3[256];

/* Test 1: Complex alternative constraints and hard register clobbers */
void test_complex_constraints(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int result1, result2, result3;
    
    /* Multiple alternative constraints with register pressure */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %[in3], %[out2]\n\t"
        "leal (%[in4], %[in1], 2), %[out3]\n\t"
        : [out1] "=r,m" (result1),
          [out2] "=r,m" (result2),
          [out3] "=r,m" (result3)
        : [in1] "r,m,i" (a),
          [in2] "r,m,i" (b),
          [in3] "r,m,i" (c),
          [in4] "r,m,i" (d)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    printf("Test1 results: %d %d %d\n", result1, result2, result3);
}

/* Test 2: Nested address computations with volatile */
void test_nested_addresses(void) {
    int index = 100;
    int offset = 50;
    uintptr_t addr1, addr2, addr3;
    int result;
    
    /* Complex address calculations that may need RELOAD_FOR_INPUT_ADDRESS */
    asm volatile (
        "movq %[addr1], %%rax\n\t"
        "addq %[addr2], %%rax\n\t"
        "movq %%rax, %[out]\n\t"
        : [out] "=r" (result)
        : [addr1] "r" (&arr1[index + offset]),
          [addr2] "r" (&arr2[index * 2]),
          "m" (arr1[index + offset]),
          "m" (arr2[index * 2])
        : "rax", "memory"
    );
    
    /* Multiple levels of address indirection */
    volatile int* ptr1 = &arr1[10];
    volatile long* ptr2 = &arr2[20];
    
    asm volatile (
        "movq %[p1], %%r10\n\t"
        "movq %[p2], %%r11\n\t"
        "addq %%r10, %%r11\n\t"
        "movq %%r11, %[addr1]\n\t"
        : [addr1] "=r" (addr1),
          [addr2] "=r" (addr2)
        : [p1] "r" (ptr1 + 5),
          [p2] "r" (ptr2 + 3),
          "m" (*ptr1),
          "m" (*ptr2)
        : "r10", "r11", "memory"
    );
    
    /* Address of address calculation */
    asm volatile (
        ""
        : "=r" (addr3)
        : "r" (&(&arr3[index])[offset]),
          "m" (arr3[index])
        : "memory"
    );
    
    printf("Test2 addresses: %lu %lu %lu\n", 
           (unsigned long)addr1, (unsigned long)addr2, (unsigned long)addr3);
}

/* Test 3: Register variables with explicit binding */
void test_register_variables(void) {
    /* Explicit register bindings */
    register int r10_var asm("r10") = 100;
    register int r11_var asm("r11") = 200;
    register int r12_var asm("r12") = 300;
    register int r13_var asm("r13") = 400;
    
    int out1, out2, out3, out4;
    
    /* Force output address reloads */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %[in3], %[out2]\n\t"
        "movl %[in4], %[out3]\n\t"
        "leal (%%eax, %[in1], 4), %[out4]\n\t"
        : [out1] "=m,r" (out1),
          [out2] "=m,r" (out2),
          [out3] "=m,r" (out3),
          [out4] "=m,r" (out4)
        : [in1] "r,m" (r10_var),
          [in2] "r,m" (r11_var),
          [in3] "r,m" (r12_var),
          [in4] "r,m" (r13_var)
        : "eax", "memory"
    );
    
    printf("Test3 outputs: %d %d %d %d\n", out1, out2, out3, out4);
}

/* Test 4: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    int out1, out2, out3, out4, out5;
    int out6, out7, out8, out9, out10;
    
    /* Large asm with many operands to stress reload pass */
    asm volatile (
        "addl %[i1], %[i2]\n\t"
        "movl %%eax, %[o1]\n\t"
        "imull %[i3], %[o2]\n\t"
        "subl %[i4], %[o3]\n\t"
        "andl %[i5], %[o4]\n\t"
        "orl %[i6], %[o5]\n\t"
        "xorl %[i7], %[o6]\n\t"
        "shll $2, %[o7]\n\t"
        "shrl $1, %[o8]\n\t"
        "leal (%[i8], %[i9], 2), %[o9]\n\t"
        "negl %[o10]\n\t"
        : [o1] "=r,m" (out1),
          [o2] "=r,m" (out2),
          [o3] "=r,m" (out3),
          [o4] "=r,m" (out4),
          [o5] "=r,m" (out5),
          [o6] "=r,m" (out6),
          [o7] "=r,m" (out7),
          [o8] "=r,m" (out8),
          [o9] "=r,m" (out9),
          [o10] "=r,m" (out10)
        : [i1] "r,m,i" (in1),
          [i2] "r,m,i" (in2),
          [i3] "r,m,i" (in3),
          [i4] "r,m,i" (in4),
          [i5] "r,m,i" (in5),
          [i6] "r,m,i" (in6),
          [i7] "r,m,i" (in7),
          [i8] "r,m,i" (in8),
          [i9] "r,m,i" (in9),
          [i10] "r,m,i" (in10)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory"
    );
    
    printf("Test4: %d %d %d %d %d\n", out1, out2, out3, out4, out5);
}

/* Test 5: __builtin_constant_p in address contexts */
void test_builtin_constant(void) {
    int x = 42;
    int y = 100;
    uintptr_t addr;
    int result;
    
    /* Conditional address expression */
    uintptr_t addr_expr = __builtin_constant_p(x) 
        ? (uintptr_t)&arr1[10] 
        : (uintptr_t)&arr2[y];
    
    asm volatile (
        "movq %[addr], %%rax\n\t"
        "movl (%%rax), %[out]\n\t"
        : [out] "=r" (result)
        : [addr] "r" (addr_expr),
          "m" (*(volatile int*)addr_expr)
        : "rax", "memory"
    );
    
    /* More complex conditional */
    int index = 50;
    uintptr_t complex_addr = __builtin_constant_p(index) 
        ? (uintptr_t)(&arr1[index] + 5)
        : (uintptr_t)(&arr2[index * 2]);
    
    asm volatile (
        "movq %[caddr], %%rbx\n\t"
        "movq %%rbx, %[outaddr]\n\t"
        : [outaddr] "=r" (addr)
        : [caddr] "r" (complex_addr),
          "m" (*(volatile int*)complex_addr)
        : "rbx", "memory"
    );
    
    printf("Test5: result=%d, addr=%lu\n", result, (unsigned long)addr);
}

/* Test 6: Mixed float/int via bitcast */
void test_mixed_types(void) {
    float f1 = 3.14f;
    float f2 = 2.71f;
    int if1, if2, result;
    
    /* Convert floats to ints via bitcast for asm */
    if1 = __builtin_bit_cast(int, f1);
    if2 = __builtin_bit_cast(int, f2);
    
    asm volatile (
        "movl %[f1], %%eax\n\t"
        "addl %[f2], %%eax\n\t"
        "movl %%eax, %[out]\n\t"
        : [out] "=r,m" (result)
        : [f1] "r,m,i" (if1),
          [f2] "r,m,i" (if2)
        : "eax", "memory"
    );
    
    printf("Test6: float bitcast result = %d\n", result);
}

/* Test 7: Address reloads with pointer arithmetic */
void test_address_reloads(void) {
    volatile int* ptr = arr1;
    int offset = 64;
    int result[4];
    
    /* Multiple address calculations in one asm */
    asm volatile (
        "movq %[ptr], %%rax\n\t"
        "addq %[off], %%rax\n\t"
        "movl (%%rax), %[r0]\n\t"
        "leaq 8(%%rax), %%rbx\n\t"
        "movl (%%rbx), %[r1]\n\t"
        "leaq 16(%%rax, %[off]), %%rcx\n\t"
        "movl (%%rcx), %[r2]\n\t"
        "leaq 24(%%rax, %[off], 2), %%rdx\n\t"
        "movl (%%rdx), %[r3]\n\t"
        : [r0] "=r" (result[0]),
          [r1] "=r" (result[1]),
          [r2] "=r" (result[2]),
          [r3] "=r" (result[3])
        : [ptr] "r" (ptr),
          [off] "r" (offset * sizeof(int)),
          "m" (ptr[offset]),
          "m" (ptr[offset + 2]),
          "m" (ptr[offset + 4]),
          "m" (ptr[offset + 6])
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    printf("Test7: %d %d %d %d\n", result[0], result[1], result[2], result[3]);
}

int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 0.5;
    }
    
    printf("Starting reload stress tests...\n");
    
    test_complex_constraints();
    test_nested_addresses();
    test_register_variables();
    test_multi_operand_asm();
    test_builtin_constant();
    test_mixed_types();
    test_address_reloads();
    
    printf("All tests completed.\n");
    
    /* Use results to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += arr1[i] + arr2[i] + (int)arr3[i];
    }
    printf("Final sum: %d\n", sum);
    
    return 0;
}
