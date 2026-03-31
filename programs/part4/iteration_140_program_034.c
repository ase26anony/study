/* Test program to trigger various reload types in GCC's reload1.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Function 1: Complex alternative constraints and hard register clobbers */
void test_complex_constraints(void) {
    volatile int arr1[256];
    volatile int arr2[256];
    int i, result1, result2;
    
    /* Initialize arrays */
    for (i = 0; i < 256; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
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
        : "eax", "ebx", "ecx", "edx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory"
    );
    
    printf("Test 1 result: %d, %d\n", result1, result2);
}

/* Function 2: Nested address computations with volatile */
void test_nested_addresses(void) {
    volatile int big_array[512];
    volatile int* volatile ptr_array[64];
    int i, result = 0;
    
    /* Initialize arrays with complex pattern */
    for (i = 0; i < 512; i++) {
        big_array[i] = i * 7 + 3;
    }
    for (i = 0; i < 64; i++) {
        ptr_array[i] = &big_array[i * 8];
    }
    
    /* Multiple asm statements with complex address computations */
    for (i = 0; i < 8; i++) {
        int idx = i * 3 + 5;
        int* addr;
        
        /* Complex address computation that may need reloads */
        asm volatile (
            "leaq %[base], %[addr]\n\t"
            "addq $%[offset], %[addr]"
            : [addr] "=r" (addr)
            : [base] "m" (big_array[0]),
              [offset] "i" (idx * sizeof(int) * 2)
            : "memory"
        );
        
        /* Use the computed address in another asm */
        asm volatile (
            "movl (%[ptr]), %%eax\n\t"
            "addl %%eax, %[sum]"
            : [sum] "+r" (result)
            : [ptr] "r" (addr)
            : "eax", "memory"
        );
        
        /* Address of volatile array element with pointer arithmetic */
        volatile int* volatile_ptr = &big_array[idx + 10];
        asm volatile (
            ""
            : "=r" (result)
            : "r" (&volatile_ptr[idx * 2]),
              "0" (result)
            : "memory"
        );
    }
    
    printf("Test 2 result: %d\n", result);
}

/* Function 3: Large multi-operand asm with register-bound variables */
void test_multi_operand_asm(void) {
    /* Declare register-bound variables */
    register int r0 asm("r10");
    register int r1 asm("r11");
    register int r2 asm("r12");
    register int r3 asm("r13");
    register int r4 asm("r14");
    register int r5 asm("r15");
    
    volatile int data[100];
    int out0, out1, out2, out3, out4, out5, out6, out7, out8, out9;
    int i;
    
    /* Initialize data */
    for (i = 0; i < 100; i++) {
        data[i] = i * 11;
    }
    
    /* Initialize register variables */
    r0 = data[0];
    r1 = data[1];
    r2 = data[2];
    r3 = data[3];
    r4 = data[4];
    r5 = data[5];
    
    /* Large asm with many operands - forces many different reload types */
    asm volatile (
        /* Complex operations using all operands */
        "movl %[in0], %%eax\n\t"
        "addl %[in1], %%eax\n\t"
        "movl %%eax, %[out0]\n\t"
        "movl %[in2], %%ebx\n\t"
        "subl %[in3], %%ebx\n\t"
        "movl %%ebx, %[out1]\n\t"
        "imull %[in4], %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        "movl %[in5], %%ecx\n\t"
        "andl %[in6], %%ecx\n\t"
        "movl %%ecx, %[out3]\n\t"
        "leal (%[in7],%[in8],4), %%edx\n\t"
        "movl %%edx, %[out4]\n\t"
        "movl %[in9], %%esi\n\t"
        "orl  %[in10], %%esi\n\t"
        "movl %%esi, %[out5]\n\t"
        "movl %[in11], %%edi\n\t"
        "xorl %[in12], %%edi\n\t"
        "movl %%edi, %[out6]\n\t"
        "movl %[in13], %%r8d\n\t"
        "shll $3, %%r8d\n\t"
        "movl %%r8d, %[out7]\n\t"
        "movl %[in14], %%r9d\n\t"
        "addl %%r9d, %[out8]\n\t"
        "movl %%r9d, %[out9]"
        
        /* Outputs - mix of types */
        : [out0] "=r,m" (out0),
          [out1] "=r,m" (out1),
          [out2] "=r,m" (out2),
          [out3] "=r,m" (out3),
          [out4] "=r,m" (out4),
          [out5] "=r,m" (out5),
          [out6] "=r,m" (out6),
          [out7] "=r,m" (out7),
          [out8] "+r,m" (out8),  /* Read-write operand */
          [out9] "=r,m" (out9)
        
        /* Inputs - mix of register-bound, memory, and constants */
        : [in0] "r,m,i" (r0),
          [in1] "r,m,i" (r1),
          [in2] "r,m,i" (r2),
          [in3] "r,m,i" (r3),
          [in4] "r,m,i" (r4),
          [in5] "r,m,i" (r5),
          [in6] "r,m,i" (data[10]),
          [in7] "r,m,i" (data[20]),
          [in8] "r,m,i" (data[30]),
          [in9] "r,m,i" (data[40]),
          [in10] "r,m,i" (data[50]),
          [in11] "r,m,i" (data[60]),
          [in12] "r,m,i" (data[70]),
          [in13] "r,m,i" (data[80]),
          [in14] "r,m,i" (data[90]),
          "0" (0)  /* out8 starts at 0 */
        
        /* Clobber many registers to increase pressure */
        : "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory", "cc"
    );
    
    printf("Test 3 results: %d %d %d %d %d %d %d %d %d %d\n",
           out0, out1, out2, out3, out4, out5, out6, out7, out8, out9);
}

/* Function 4: __builtin_constant_p in address contexts */
void test_builtin_constant_p(void) {
    volatile int varray[128];
    int i, result = 0;
    
    for (i = 0; i < 128; i++) {
        varray[i] = i * 13;
    }
    
    /* Use __builtin_constant_p to create different address expressions */
    for (i = 0; i < 16; i++) {
        int idx = i * 4;
        void* addr;
        
        /* Conditional address computation */
        if (__builtin_constant_p(i)) {
            addr = &varray[idx + 1];
        } else {
            /* More complex non-constant address */
            addr = &varray[idx * 2 + i];
        }
        
        /* Use the address in asm */
        asm volatile (
            "movl (%[addr]), %%eax\n\t"
            "addl %%eax, %[sum]"
            : [sum] "+r" (result)
            : [addr] "r" (addr)
            : "eax", "memory"
        );
        
        /* Another variation with pointer arithmetic in constraints */
        int offset = i * sizeof(int);
        asm volatile (
            ""
            : "=r" (result)
            : "r" ((char*)&varray[0] + offset),
              "0" (result)
            : "memory"
        );
    }
    
    printf("Test 4 result: %d\n", result);
}

/* Function 5: Mixed float/integer via bitcast for additional complexity */
void test_mixed_types(void) {
    volatile float farr[64];
    volatile double darr[64];
    uint64_t int_result;
    float float_result;
    int i;
    
    /* Initialize arrays */
    for (i = 0; i < 64; i++) {
        farr[i] = i * 1.5f;
        darr[i] = i * 2.5;
    }
    
    /* Use __builtin_bit_cast to treat floats as integers in asm */
    uint32_t float_as_int = __builtin_bit_cast(uint32_t, farr[10]);
    uint64_t double_as_int = __builtin_bit_cast(uint64_t, darr[20]);
    
    asm volatile (
        "movl %[fval], %%eax\n\t"
        "addl $0x3F800000, %%eax\n\t"  /* Add 1.0f in IEEE754 */
        "movl %%eax, %[out1]\n\t"
        "movq %[dval], %%rax\n\t"
        "addq $0x3FF0000000000000, %%rax\n\t"  /* Add 1.0 */
        "movq %%rax, %[out2]"
        : [out1] "=r" (float_as_int),
          [out2] "=r" (double_as_int)
        : [fval] "r,m" (float_as_int),
          [dval] "r,m" (double_as_int)
        : "rax", "memory"
    );
    
    /* Convert back */
    float_result = __builtin_bit_cast(float, float_as_int);
    int_result = double_as_int;
    
    printf("Test 5: float=%f, double_as_int=0x%lx\n", float_result, int_result);
}

int main(void) {
    printf("Starting reload type coverage tests...\n");
    
    test_complex_constraints();
    test_nested_addresses();
    test_multi_operand_asm();
    test_builtin_constant_p();
    test_mixed_types();
    
    printf("All tests completed.\n");
    return 0;
}
