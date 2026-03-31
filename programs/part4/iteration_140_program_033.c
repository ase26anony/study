/* Test program to trigger various reload types in GCC's reload pass */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force reloads by using volatile arrays */
volatile int arr1[256];
volatile long arr2[256];
volatile char arr3[256];

/* Test 1: Complex inline assembly with multiple constraints */
void test_complex_constraints(void) {
    int a = 1, b = 2, c = 3, d = 4;
    int result1, result2, result3;
    volatile int *ptr = &arr1[0];
    
    /* Multiple alternative constraints forcing different reload types */
    asm volatile (
        /* Output with alternative constraints */
        "movl %[input], %[out1]\n\t"
        "addl $1, %[out1]\n\t"
        : [out1] "=r,m" (result1)  /* Alternative: register or memory */
        : [input] "r,m,i" (a)       /* Three alternatives */
        : "cc", "eax", "r12"        /* Clobber specific registers */
    );
    
    /* Complex input/output with memory constraints */
    asm volatile (
        "leal (%[in1], %[in2], 4), %[out2]\n\t"
        "movl %[out2], (%[mem])\n\t"
        : [out2] "=r" (result2), "=m" (*ptr)
        : [in1] "r" (b), [in2] "r" (c), [mem] "r" (ptr)
        : "memory", "ebx"
    );
    
    /* Multiple outputs with conflicting requirements */
    asm volatile (
        "movl %[in], %%eax\n\t"
        "imull %%eax, %%eax\n\t"
        "movl %%eax, %[out3]\n\t"
        "movl %%eax, %[out4]\n\t"
        : [out3] "=rm" (result3), [out4] "=rm" (d)
        : [in] "rmi" (c)
        : "eax", "edx", "cc"
    );
    
    printf("Test1: %d %d %d %d\n", result1, result2, result3, d);
}

/* Test 2: Nested address computations with volatile */
void test_nested_addresses(void) {
    volatile int *ptr1, *ptr2;
    int idx1 = 100, idx2 = 200;
    int result1, result2;
    
    /* Complex address calculation requiring reloads */
    ptr1 = &arr1[idx1 + idx2 - 50];
    ptr2 = &arr2[(idx1 * 3) / 2];
    
    /* Address of volatile array element in assembly */
    asm volatile (
        "movl (%[addr]), %[out1]\n\t"
        "addl $42, %[out1]\n\t"
        : [out1] "=r" (result1)
        : [addr] "r" (&arr1[idx1 + 10]), "m" (arr1[idx1 + 10])
        : "memory"
    );
    
    /* More complex nested address */
    asm volatile (
        "movq %[addr1], %%rax\n\t"
        "movq %[addr2], %%rbx\n\t"
        "subq %%rbx, %%rax\n\t"
        "shrq $2, %%rax\n\t"
        "movl %%eax, %[out2]\n\t"
        : [out2] "=r" (result2)
        : [addr1] "r" (&arr1[idx1 * 2]), 
          [addr2] "r" (&arr1[idx1]),
          "m" (arr1[0]), "m" (arr1[255])
        : "rax", "rbx", "rcx", "cc", "memory"
    );
    
    /* Using __builtin_constant_p in address context */
    int offset = __builtin_constant_p(idx1) ? 0 : 10;
    asm volatile (
        "movl (%[base], %[off], 4), %[out3]\n\t"
        : [out3] "=r" (result1)
        : [base] "r" (arr1), [off] "r" (idx1 + offset)
        : "memory"
    );
    
    printf("Test2: %d %d\n", result1, result2);
}

/* Test 3: Register variables with explicit binding */
void test_register_variables(void) {
    /* Explicit register bindings */
    register int r1 asm("r10") = 100;
    register int r2 asm("r11") = 200;
    register int r3 asm("r12") = 300;
    register void *rptr asm("r13") = (void*)arr3;
    
    int result1, result2, result3;
    volatile int *memptr = &arr1[50];
    
    /* Force spilling of fixed registers */
    asm volatile (
        "movl %[reg1], %%eax\n\t"
        "addl %[reg2], %%eax\n\t"
        "movl %%eax, (%[mem])\n\t"
        "movl (%[mem]), %[out1]\n\t"
        : [out1] "=r" (result1), "=m" (*memptr)
        : [reg1] "r" (r1), [reg2] "r" (r2), [mem] "r" (memptr)
        : "rax", "memory", "cc"
    );
    
    /* Complex address calculation using register variables */
    asm volatile (
        "movq %[rptr], %%rax\n\t"
        "addq $100, %%rax\n\t"
        "movb $0x55, (%%rax)\n\t"
        "movzbq (%%rax), %[out2]\n\t"
        : [out2] "=r" (result2)
        : [rptr] "r" (rptr)
        : "rax", "memory"
    );
    
    /* Multiple register variables in one asm */
    asm volatile (
        "movl %[a], %%eax\n\t"
        "imull %[b], %%eax\n\t"
        "addl %[c], %%eax\n\t"
        "movl %%eax, %[out3]\n\t"
        : [out3] "=r" (result3)
        : [a] "r" (r1), [b] "r" (r2), [c] "r" (r3)
        : "rax", "rdx", "cc"
    );
    
    printf("Test3: %d %d %d\n", result1, result2, result3);
}

/* Test 4: Large number of operands in single asm */
void test_many_operands(void) {
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    int out1, out2, out3, out4, out5, out6, out7, out8, out9, out10;
    volatile int *mem1 = &arr1[0];
    volatile int *mem2 = &arr1[64];
    volatile int *mem3 = &arr1[128];
    
    /* Large asm with many operands - forces many reloads */
    asm volatile (
        "movl %[i1], %[o1]\n\t"
        "addl %[i2], %[o1]\n\t"
        "movl %[i3], %[o2]\n\t"
        "subl %[i4], %[o2]\n\t"
        "movl %[i5], %[o3]\n\t"
        "imull %[i6], %[o3]\n\t"
        "movl %[i7], %[o4]\n\t"
        "andl %[i8], %[o4]\n\t"
        "movl %[i9], %[o5]\n\t"
        "orl  %[i10], %[o5]\n\t"
        "movl %[o1], (%[m1])\n\t"
        "movl %[o2], (%[m2])\n\t"
        "movl %[o3], (%[m3])\n\t"
        "movl (%[m1]), %[o6]\n\t"
        "movl (%[m2]), %[o7]\n\t"
        "movl (%[m3]), %[o8]\n\t"
        "leal (%[o1], %[o2], 2), %[o9]\n\t"
        "leal (%[o3], %[o4], 4), %[o10]\n\t"
        : [o1] "=&r" (out1), [o2] "=&r" (out2), [o3] "=&r" (out3),
          [o4] "=&r" (out4), [o5] "=&r" (out5), [o6] "=&r" (out6),
          [o7] "=&r" (out7), [o8] "=&r" (out8), [o9] "=&r" (out9),
          [o10] "=&r" (out10),
          "=m" (*mem1), "=m" (*mem2), "=m" (*mem3)
        : [i1] "r" (in1), [i2] "r" (in2), [i3] "r" (in3),
          [i4] "r" (in4), [i5] "r" (in5), [i6] "r" (in6),
          [i7] "r" (in7), [i8] "r" (in8), [i9] "r" (in9),
          [i10] "r" (in10),
          [m1] "r" (mem1), [m2] "r" (mem2), [m3] "r" (mem3),
          "m" (*mem1), "m" (*mem2), "m" (*mem3)
        : "cc", "memory", "rax", "rbx", "rcx", "rdx"
    );
    
    printf("Test4: %d %d %d %d %d %d %d %d %d %d\n", 
           out1, out2, out3, out4, out5, out6, out7, out8, out9, out10);
}

/* Test 5: Mixed float/integer via bitcast */
void test_mixed_types(void) {
    float f1 = 3.14f, f2 = 2.71f;
    double d1 = 1.414, d2 = 1.732;
    int if1, if2, id1, id2;
    int result1, result2;
    
    /* Convert floats to integers via bitcast for use in integer asm */
    if1 = __builtin_bit_cast(int, f1);
    if2 = __builtin_bit_cast(int, f2);
    
    /* Use float bitcasts in integer assembly */
    asm volatile (
        "movl %[f1], %%eax\n\t"
        "xorl %[f2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        : [out1] "=r" (result1)
        : [f1] "r" (if1), [f2] "r" (if2)
        : "rax", "cc"
    );
    
    /* Mixed address calculation with different types */
    volatile float *fptr = (volatile float*)arr1;
    asm volatile (
        "movq %[ptr], %%rax\n\t"
        "movss (%%rax), %%xmm0\n\t"
        "cvtss2si %%xmm0, %[out2]\n\t"
        : [out2] "=r" (result2)
        : [ptr] "r" (&fptr[16])
        : "rax", "xmm0", "memory"
    );
    
    printf("Test5: %d %d\n", result1, result2);
}

/* Main function that runs all tests */
int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i & 0xFF;
    }
    
    printf("Starting reload stress tests...\n");
    
    test_complex_constraints();
    test_nested_addresses();
    test_register_variables();
    test_many_operands();
    test_mixed_types();
    
    printf("All tests completed.\n");
    
    /* Verify some results to prevent optimization */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += arr1[i] + arr2[i] + arr3[i];
    }
    printf("Final sum: %d\n", sum);
    
    return 0;
}
