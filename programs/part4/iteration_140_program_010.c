/* Test program to trigger various reload types in GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Function 1: Complex alternative constraints and hard register clobbers */
void test_complex_constraints(void) {
    volatile int arr1[256];
    volatile int arr2[256];
    int i, j, k;
    
    /* Initialize arrays */
    for (i = 0; i < 256; i++) {
        arr1[i] = i;
        arr2[i] = 256 - i;
    }
    
    /* Complex asm with alternative constraints and hard register clobbers */
    for (i = 0; i < 10; i++) {
        int result1, result2, result3;
        int *ptr1 = &arr1[i];
        int *ptr2 = &arr2[i * 2];
        
        /* Multiple alternative constraints forcing different reload types */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "leal (%[mem1], %[in3], 4), %%ebx\n\t"
            "movl %%ebx, %[out2]\n\t"
            "imull %[imm], %%eax\n\t"
            "movl %%eax, %[out3]\n\t"
            : [out1] "=r,m" (result1),
              [out2] "=r,m" (result2),
              [out3] "=r,m" (result3)
            : [in1] "r,m" (*ptr1),
              [in2] "r,m" (*ptr2),
              [in3] "r,i" (i),
              [mem1] "m,r" (arr1[0]),
              [imm] "i,r" (42)
            : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        printf("Test1[%d]: %d %d %d\n", i, result1, result2, result3);
    }
}

/* Function 2: Nested address computations with volatile */
void test_nested_addresses(void) {
    volatile int big_array[512];
    volatile int small_array[64];
    register int idx asm("r10");
    register int sum asm("r11");
    
    /* Initialize arrays */
    for (int i = 0; i < 512; i++) big_array[i] = i * 2;
    for (int i = 0; i < 64; i++) small_array[i] = i * 3;
    
    sum = 0;
    for (idx = 0; idx < 50; idx++) {
        int temp1, temp2;
        int complex_idx = idx * 3 + 7;
        
        /* Nested address-of operations triggering address reloads */
        asm volatile (
            "movl (%[addr1]), %%r12d\n\t"
            "addl (%[addr2]), %%r12d\n\t"
            "movl %%r12d, %[out1]\n\t"
            "leaq (%[base], %[idx], 8), %%r13\n\t"
            "movl (%%r13), %%r14d\n\t"
            "addl %%r14d, %[out2]\n\t"
            : [out1] "=r" (temp1),
              [out2] "+r" (sum)
            : [addr1] "r" (&big_array[complex_idx + 16]),
              [addr2] "r" (&small_array[(complex_idx % 32) + 8]),
              [base] "r" (big_array),
              [idx] "r" (complex_idx)
            : "r12", "r13", "r14", "memory"
        );
        
        /* Another asm with address computation in constraint */
        int *volatile ptr = &big_array[idx * 4];
        asm volatile (
            "addl $1, %[val]\n\t"
            : [val] "+m" (*ptr)
            :
            : "memory"
        );
    }
    
    printf("Test2 sum: %d\n", sum);
}

/* Function 3: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    volatile int data[20];
    register int r1 asm("r15");
    register int r2 asm("r14");
    register int r3 asm("r13");
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    
    /* Initialize */
    for (int i = 0; i < 20; i++) data[i] = 100 + i;
    r1 = 1; r2 = 2; r3 = 3;
    
    /* Large asm with many operands to stress reload pass */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %[in3], %%ebx\n\t"
        "subl %[in4], %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        "imull %[in5], %%eax\n\t"
        "movl %%eax, %[out3]\n\t"
        "movl %[in6], %%ecx\n\t"
        "andl %[in7], %%ecx\n\t"
        "movl %%ecx, %[out4]\n\t"
        "movl %[in8], %%edx\n\t"
        "orl  %[in9], %%edx\n\t"
        "movl %%edx, %[out5]\n\t"
        "leal (%[in10], %[in1], 2), %%esi\n\t"
        "movl %%esi, %[out6]\n\t"
        "movl %[in11], %%edi\n\t"
        "shll $2, %%edi\n\t"
        "movl %%edi, %[out7]\n\t"
        "movl %[in12], %%r8d\n\t"
        "addl %%eax, %%r8d\n\t"
        "movl %%r8d, %[out8]\n\t"
        "movl %[in13], %%r9d\n\t"
        "subl %%ebx, %%r9d\n\t"
        "movl %%r9d, %[out9]\n\t"
        "movl %[in14], %%r10d\n\t"
        "xorl %%ecx, %%r10d\n\t"
        "movl %%r10d, %[out10]\n\t"
        : [out1] "=r" (a1),
          [out2] "=r" (a2),
          [out3] "=r" (a3),
          [out4] "=r" (a4),
          [out5] "=r" (a5),
          [out6] "=r" (a6),
          [out7] "=r" (a7),
          [out8] "=r" (a8),
          [out9] "=r" (a9),
          [out10] "=r" (a10)
        : [in1] "r" (r1),
          [in2] "r" (r2),
          [in3] "r" (r3),
          [in4] "r" (data[0]),
          [in5] "i" (5),
          [in6] "r" (data[1]),
          [in7] "r" (data[2]),
          [in8] "r" (data[3]),
          [in9] "r" (data[4]),
          [in10] "r" (data[5]),
          [in11] "r" (data[6]),
          [in12] "r" (data[7]),
          [in13] "r" (data[8]),
          [in14] "r" (data[9])
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    printf("Test3 results: %d %d %d %d %d %d %d %d %d %d\n",
           a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}

/* Function 4: __builtin_constant_p in address contexts */
void test_builtin_constant(void) {
    volatile int varray[100];
    int dynamic_idx = 37;
    
    for (int i = 0; i < 100; i++) varray[i] = i * i;
    
    /* Use __builtin_constant_p to create different address expressions */
    for (int i = 0; i < 10; i++) {
        int result;
        int idx = __builtin_constant_p(i) ? i : dynamic_idx + i;
        
        asm volatile (
            "movl (%[addr]), %%eax\n\t"
            "addl $100, %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=r" (result)
            : [addr] "r" (&varray[idx * 2])
            : "rax", "memory"
        );
        
        /* Another variant with conditional address */
        int *addr;
        if (__builtin_constant_p(i) && i < 50) {
            addr = &varray[i];
        } else {
            addr = &varray[dynamic_idx];
        }
        
        asm volatile (
            "incl (%[ptr])\n\t"
            :
            : [ptr] "r" (addr)
            : "memory"
        );
        
        printf("Test4[%d]: %d\n", i, result);
    }
}

/* Function 5: Mixed float/integer via bitcast */
void test_mixed_types(void) {
    volatile float farr[16];
    volatile double darr[16];
    uint64_t int_result;
    float float_result;
    
    /* Initialize */
    for (int i = 0; i < 16; i++) {
        farr[i] = i * 1.5f;
        darr[i] = i * 2.5;
    }
    
    /* Use bitcast to treat floats as integers in asm */
    for (int i = 0; i < 8; i++) {
        uint32_t float_as_int = __builtin_bit_cast(uint32_t, farr[i]);
        uint64_t double_as_int = __builtin_bit_cast(uint64_t, darr[i]);
        
        asm volatile (
            "movl %[fin], %%eax\n\t"
            "addl $0x3F800000, %%eax\n\t"  /* Add 1.0f in IEEE754 */
            "movl %%eax, %[fout]\n\t"
            "movq %[din], %%rbx\n\t"
            "addq $0x3FF0000000000000, %%rbx\n\t"  /* Add 1.0 */
            "movq %%rbx, %[dout]\n\t"
            : [fout] "=r" (float_as_int),
              [dout] "=r" (double_as_int)
            : [fin] "r" (float_as_int),
              [din] "r" (double_as_int)
            : "rax", "rbx", "memory"
        );
        
        /* Convert back */
        float_result = __builtin_bit_cast(float, float_as_int);
        printf("Test5[%d]: float %f as int 0x%08lx\n",
               i, float_result, (unsigned long)float_as_int);
    }
}

int main(void) {
    printf("Starting reload pass stress tests...\n");
    
    test_complex_constraints();
    test_nested_addresses();
    test_multi_operand_asm();
    test_builtin_constant();
    test_mixed_types();
    
    printf("All tests completed.\n");
    return 0;
}
