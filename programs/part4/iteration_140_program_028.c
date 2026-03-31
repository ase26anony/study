/* Test program to trigger various reload types in GCC's reload pass */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Function 1: Complex alternative constraints and hard register clobbers */
void test_complex_constraints(void) {
    volatile int arr1[256];
    volatile int arr2[256];
    int result1, result2, result3;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i;
        arr2[i] = 256 - i;
    }
    
    /* Complex asm with alternative constraints and explicit clobbers */
    asm volatile (
        /* Outputs with alternative constraints */
        "=r,m" (result1),   /* Alternative: register or memory */
        "=r,m" (result2),   /* Alternative: register or memory */
        "=r,m" (result3)    /* Alternative: register or memory */
        :
        /* Inputs with conflicting requirements */
        "r,m,i" (arr1[10]),  /* Can be register, memory, or immediate */
        "r,m,i" (arr2[20]),  /* Can be register, memory, or immediate */
        "r,m,i" (100)        /* Constant with alternatives */
        :
        /* Explicit clobbers of hard registers to increase pressure */
        "eax", "ebx", "ecx", "edx", "esi", "edi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "memory"
    );
    
    printf("Test 1 result: %d %d %d\n", result1, result2, result3);
}

/* Function 2: Nested address computations with volatile */
void test_nested_addresses(void) {
    volatile int volatile_arr[256];
    volatile long volatile_long_arr[128];
    int *ptr1, *ptr2;
    long *lptr;
    int idx1, idx2;
    
    /* Initialize */
    for (int i = 0; i < 256; i++) volatile_arr[i] = i * 2;
    for (int i = 0; i < 128; i++) volatile_long_arr[i] = i * 3;
    
    idx1 = 50;
    idx2 = 75;
    
    /* Complex address computations in asm operands */
    asm volatile (
        "mov %[addr1], %%rax\n\t"
        "mov %[addr2], %%rbx\n\t"
        "mov %[addr3], %%rcx\n\t"
        "add $1, %%rax\n\t"
        "add $2, %%rbx\n\t"
        "add $3, %%rcx\n\t"
        "mov %%rax, %[out1]\n\t"
        "mov %%rbx, %[out2]\n\t"
        "mov %%rcx, %[out3]\n\t"
        : [out1] "=r" (ptr1),
          [out2] "=r" (ptr2),
          [out3] "=r" (lptr)
        : [addr1] "r" (&volatile_arr[idx1 + 10]),  /* RELOAD_FOR_INPUT_ADDRESS */
          [addr2] "r" (&volatile_arr[volatile_arr[idx2] & 0xFF]),  /* Complex address */
          [addr3] "r" (&volatile_long_arr[(idx1 * idx2) % 128])    /* More complexity */
        : "rax", "rbx", "rcx", "memory"
    );
    
    /* Use the pointers to ensure they're not optimized away */
    printf("Test 2 pointers: %p %p %p\n", (void*)ptr1, (void*)ptr2, (void*)lptr);
    printf("Test 2 values: %d %d %ld\n", 
           ptr1 ? *ptr1 : 0, 
           ptr2 ? *ptr2 : 0, 
           lptr ? *lptr : 0);
}

/* Function 3: Register variables with explicit binding */
void test_register_variables(void) {
    /* Declare register variables bound to specific hard registers */
    register int r10_var asm("r10") = 100;
    register int r11_var asm("r11") = 200;
    register int r12_var asm("r12") = 300;
    register int r13_var asm("r13") = 400;
    register int r14_var asm("r14") = 500;
    register int r15_var asm("r15") = 600;
    
    volatile int mem_buffer[64];
    int results[6];
    
    /* Initialize memory */
    for (int i = 0; i < 64; i++) mem_buffer[i] = i * 10;
    
    /* Complex asm that uses all register variables and forces spills */
    asm volatile (
        /* Multiple outputs with address computations */
        "=r" (results[0]),
        "=r" (results[1]),
        "=r" (results[2]),
        "=r" (results[3]),
        "=r" (results[4]),
        "=r" (results[5])
        :
        /* Inputs including register variables and their addresses */
        "r" (r10_var),
        "r" (r11_var),
        "r" (r12_var),
        "r" (r13_var),
        "r" (r14_var),
        "r" (r15_var),
        "r" (&mem_buffer[r10_var & 0x3F]),  /* RELOAD_FOR_OUTPUT_ADDRESS */
        "r" (&mem_buffer[r11_var & 0x3F]),  /* RELOAD_FOR_OUTADDR_ADDRESS */
        "r" (mem_buffer[r12_var & 0x3F]),   /* Memory operand */
        "r" (mem_buffer[r13_var & 0x3F])    /* Another memory operand */
        :
        /* Clobber all used registers */
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "memory"
    );
    
    printf("Test 3 results: ");
    for (int i = 0; i < 6; i++) printf("%d ", results[i]);
    printf("\n");
}

/* Function 4: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    int out1, out2, out3, out4, out5, out6, out7, out8, out9, out10;
    int out11, out12, out13, out14, out15;
    
    /* Very large asm with many operands to stress reload pass */
    asm volatile (
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out2]\n\t"
        "sub %[in3], %[out3]\n\t"
        "imul %[in4], %[out4]\n\t"
        "and %[in5], %[out5]\n\t"
        "or %[in6], %[out6]\n\t"
        "xor %[in7], %[out7]\n\t"
        "lea (%[in8],%[in9],2), %[out8]\n\t"
        "mov %[in10], %[out9]\n\t"
        "add %[in11], %[out10]\n\t"
        "sub %[in12], %[out11]\n\t"
        "imul %[in13], %[out12]\n\t"
        "and %[in14], %[out13]\n\t"
        "or %[in15], %[out14]\n\t"
        "xor $0xFF, %[out15]\n\t"
        : [out1] "=r" (out1),
          [out2] "=r" (out2),
          [out3] "=r" (out3),
          [out4] "=r" (out4),
          [out5] "=r" (out5),
          [out6] "=r" (out6),
          [out7] "=r" (out7),
          [out8] "=r" (out8),
          [out9] "=r" (out9),
          [out10] "=r" (out10),
          [out11] "=r" (out11),
          [out12] "=r" (out12),
          [out13] "=r" (out13),
          [out14] "=r" (out14),
          [out15] "=r" (out15)
        : [in1] "r" (v1),
          [in2] "r" (v2),
          [in3] "r" (v3),
          [in4] "r" (v4),
          [in5] "r" (v5),
          [in6] "r" (v6),
          [in7] "r" (v7),
          [in8] "r" (v8),
          [in9] "r" (v9),
          [in10] "r" (v10),
          [in11] "r" (v11),
          [in12] "r" (v12),
          [in13] "r" (v13),
          [in14] "r" (v14),
          [in15] "r" (v15)
        : "memory"
    );
    
    printf("Test 4 outputs: ");
    printf("%d %d %d %d %d ", out1, out2, out3, out4, out5);
    printf("%d %d %d %d %d ", out6, out7, out8, out9, out10);
    printf("%d %d %d %d %d\n", out11, out12, out13, out14, out15);
}

/* Function 5: __builtin_constant_p in address contexts */
void test_builtin_constant_p(void) {
    volatile int data[100];
    int index = 50;
    int result1, result2;
    
    for (int i = 0; i < 100; i++) data[i] = i * 2;
    
    /* Use __builtin_constant_p to create different reload scenarios */
    asm volatile (
        "movl %[addr], %%eax\n\t"
        "movl (%%eax), %[out1]\n\t"
        "addl $4, %%eax\n\t"
        "movl (%%eax), %[out2]\n\t"
        : [out1] "=r" (result1),
          [out2] "=r" (result2)
        : [addr] "r" (__builtin_constant_p(index) 
                      ? &data[10]  /* Constant address */
                      : &data[index] /* Non-constant address */)
        : "eax", "memory"
    );
    
    printf("Test 5 results: %d %d\n", result1, result2);
}

/* Function 6: Mixed float/integer via bitcast */
void test_mixed_types(void) {
    volatile float farr[32];
    volatile double darr[32];
    uint32_t int_results[4];
    uint64_t long_results[4];
    
    /* Initialize float arrays */
    for (int i = 0; i < 32; i++) {
        farr[i] = i * 1.5f;
        darr[i] = i * 2.5;
    }
    
    /* Use __builtin_bit_cast to treat floats as integers in asm */
    asm volatile (
        "mov %[f1], %%eax\n\t"
        "mov %[f2], %%ebx\n\t"
        "mov %[d1], %%rcx\n\t"
        "mov %[d2], %%rdx\n\t"
        "add $1, %%eax\n\t"
        "add $2, %%ebx\n\t"
        "add $3, %%rcx\n\t"
        "add $4, %%rdx\n\t"
        "mov %%eax, %[out1]\n\t"
        "mov %%ebx, %[out2]\n\t"
        "mov %%rcx, %[out3]\n\t"
        "mov %%rdx, %[out4]\n\t"
        : [out1] "=r" (int_results[0]),
          [out2] "=r" (int_results[1]),
          [out3] "=r" (long_results[0]),
          [out4] "=r" (long_results[1])
        : [f1] "r" (__builtin_bit_cast(uint32_t, farr[0])),
          [f2] "r" (__builtin_bit_cast(uint32_t, farr[1])),
          [d1] "r" (__builtin_bit_cast(uint64_t, darr[0])),
          [d2] "r" (__builtin_bit_cast(uint64_t, darr[1]))
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    printf("Test 6 results: %u %u %lu %lu\n", 
           int_results[0], int_results[1], 
           long_results[0], long_results[1]);
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
