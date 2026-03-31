/* Test program to trigger various reload types in GCC's reload pass */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Function 1: Complex alternative constraints and hard register clobbers */
void test_complex_constraints(void) {
    volatile int arr1[256];
    volatile int arr2[256];
    int i, j, k;
    
    /* Initialize arrays */
    for (i = 0; i < 256; i++) {
        arr1[i] = i;
        arr2[i] = 255 - i;
    }
    
    /* Complex asm with alternative constraints and explicit clobbers */
    for (i = 0; i < 10; i++) {
        int result1, result2;
        int *ptr1 = &arr1[i];
        int *ptr2 = &arr2[i * 2];
        
        /* This asm has multiple alternative constraints that force reloads */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out2]\n\t"
            : [out1] "=r,m" (result1), [out2] "=r,m" (result2)
            : [in1] "r,m,i" (*ptr1), [in2] "r,m,i" (*ptr2), 
              [in3] "r,m,i" (i)
            : "eax", "memory", "cc"
        );
        
        /* Use results to prevent optimization */
        arr1[i] = result1 + result2;
    }
}

/* Function 2: Nested address computations with volatile */
void test_nested_addresses(void) {
    volatile int big_array[1024];
    volatile int small_array[64];
    int i, sum = 0;
    
    /* Initialize arrays */
    for (i = 0; i < 1024; i++) big_array[i] = i % 256;
    for (i = 0; i < 64; i++) small_array[i] = i * 2;
    
    /* Complex address calculations that may trigger address reloads */
    for (i = 0; i < 32; i++) {
        int idx1, idx2;
        int *addr1, *addr2;
        
        /* Complex index calculation */
        idx1 = (i * 13 + 7) % 1024;
        idx2 = (i * 17 + 11) % 64;
        
        /* Nested address-of operations */
        addr1 = &big_array[idx1 + small_array[idx2 % 32]];
        addr2 = &small_array[(idx1 + idx2) % 64];
        
        /* Inline asm using these complex addresses */
        asm volatile (
            "movl (%[addr1]), %%ebx\n\t"
            "addl (%[addr2]), %%ebx\n\t"
            "movl %%ebx, %[sum]\n\t"
            : [sum] "=r" (sum)
            : [addr1] "r" (addr1), [addr2] "r" (addr2)
            : "ebx", "memory"
        );
        
        /* Another asm with address in output operand */
        int *dummy_ptr;
        asm volatile (
            "leal (%[idx1],%[idx2],4), %%ecx\n\t"
            "movl %%ecx, %[ptr]\n\t"
            : [ptr] "=r" (dummy_ptr)
            : [idx1] "r" (idx1), [idx2] "r" (idx2)
            : "ecx"
        );
    }
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
    int results[10];
    int i;
    
    /* Initialize */
    for (i = 0; i < 100; i++) data[i] = i;
    
    r0 = data[0];
    r1 = data[1];
    r2 = data[2];
    r3 = data[3];
    r4 = data[4];
    r5 = data[5];
    
    /* Large asm with many operands - forces many different reload types */
    asm volatile (
        "movl %[a0], %%eax\n\t"
        "addl %[a1], %%eax\n\t"
        "movl %%eax, %[out0]\n\t"
        "addl %[a2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "addl %[a3], %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        "addl %[a4], %%eax\n\t"
        "movl %%eax, %[out3]\n\t"
        "addl %[a5], %%eax\n\t"
        "movl %%eax, %[out4]\n\t"
        "imull %[a6], %%eax\n\t"
        "movl %%eax, %[out5]\n\t"
        : [out0] "=r,m" (results[0]),
          [out1] "=r,m" (results[1]),
          [out2] "=r,m" (results[2]),
          [out3] "=r,m" (results[3]),
          [out4] "=r,m" (results[4]),
          [out5] "=r,m" (results[5])
        : [a0] "r,m,i" (r0),
          [a1] "r,m,i" (r1),
          [a2] "r,m,i" (r2),
          [a3] "r,m,i" (r3),
          [a4] "r,m,i" (r4),
          [a5] "r,m,i" (r5),
          [a6] "r,m,i" (data[10])
        : "eax", "memory", "cc"
    );
    
    /* Use __builtin_constant_p to create dynamic address expressions */
    for (i = 0; i < 5; i++) {
        int idx = __builtin_constant_p(i) ? 0 : i * 7;
        int *addr = &data[idx + results[i % 6]];
        
        asm volatile (
            "movl (%[addr]), %%edx\n\t"
            "addl $1, %%edx\n\t"
            "movl %%edx, (%[addr])\n\t"
            :
            : [addr] "r" (addr)
            : "edx", "memory"
        );
    }
}

/* Function 4: Mixed float/integer operands via bitcast */
void test_mixed_types(void) {
    volatile float farr[32];
    volatile double darr[32];
    uint64_t int_results[8];
    int i;
    
    /* Initialize */
    for (i = 0; i < 32; i++) {
        farr[i] = i * 1.5f;
        darr[i] = i * 2.5;
    }
    
    /* Use bitcast to treat floats as integers in asm */
    for (i = 0; i < 8; i++) {
        uint32_t float_as_int = __builtin_bit_cast(uint32_t, farr[i * 2]);
        uint64_t double_as_int = __builtin_bit_cast(uint64_t, darr[i * 2]);
        
        asm volatile (
            "movl %[fval], %%eax\n\t"
            "movq %[dval], %%rbx\n\t"
            "addl $0x3F800000, %%eax\n\t"  /* Add 1.0 in float representation */
            "addq $0x3FF0000000000000, %%rbx\n\t"  /* Add 1.0 in double representation */
            "movl %%eax, %[out1]\n\t"
            "movq %%rbx, %[out2]\n\t"
            : [out1] "=r,m" (float_as_int),
              [out2] "=r,m" (double_as_int)
            : [fval] "r,m,i" (float_as_int),
              [dval] "r,m,i" (double_as_int)
            : "eax", "rbx", "cc"
        );
        
        int_results[i] = double_as_int + float_as_int;
    }
}

/* Function 5: Output address reloads */
void test_output_address_reloads(void) {
    volatile int out_data[64];
    int *output_ptrs[8];
    int i;
    
    /* Complex output addressing */
    for (i = 0; i < 8; i++) {
        int complex_idx = (i * 29 + 17) % 64;
        
        /* This asm produces an address that needs to be stored */
        asm volatile (
            "leal (%[base], %[idx], 4), %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=r,m" (output_ptrs[i])
            : [base] "r" (out_data),
              [idx] "r" (complex_idx)
            : "eax"
        );
        
        /* Use the pointer */
        *output_ptrs[i] = i * 100;
    }
}

int main(void) {
    printf("Starting reload stress tests...\n");
    
    printf("Test 1: Complex constraints...\n");
    test_complex_constraints();
    
    printf("Test 2: Nested addresses...\n");
    test_nested_addresses();
    
    printf("Test 3: Multi-operand asm...\n");
    test_multi_operand_asm();
    
    printf("Test 4: Mixed types...\n");
    test_mixed_types();
    
    printf("Test 5: Output address reloads...\n");
    test_output_address_reloads();
    
    printf("All tests completed.\n");
    return 0;
}
