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
        /* Output with alternative constraint: register OR memory */
        "mov %[val1], %[out1]\n\t"
        "add $1, %[out1]\n\t"
        : [out1] "=r,m" (result1)
        /* Input with conflicting requirements */
        : [val1] "ir,m" (arr[10]),
          /* Force address reloads */
          [base] "r" (&arr[0])
        /* Clobber specific hard registers to increase pressure */
        : "eax", "ebx", "ecx", "edx", "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    /* Another asm with complex address computation */
    register int x asm("r10") = 100;
    asm volatile (
        "lea (%[idx],%[base],4), %[addr]\n\t"
        "mov (%[addr]), %[out2]\n\t"
        : [out2] "=r" (result2),
          [addr] "=&r" (i)
        : [base] "r" (&arr[0]),
          [idx] "r" (x),
          "m" (arr[0])
        : "memory"
    );
    
    printf("Test 1 results: %d, %d\n", result1, result2);
}

/* Function 2: Nested address computations with volatile */
void test_nested_addresses(void) {
    volatile int volatile_arr[512];
    volatile long volatile_larr[256];
    int i, j, k;
    long result;
    
    /* Initialize */
    for (i = 0; i < 512; i++) {
        volatile_arr[i] = i * 2;
    }
    for (i = 0; i < 256; i++) {
        volatile_larr[i] = i * 5L;
    }
    
    /* Complex address computation that may need RELOAD_FOR_INPUT_ADDRESS */
    i = 100;
    j = 50;
    
    asm volatile (
        /* Take address of volatile array element with complex index */
        "lea (%[arr],%[idx],4), %[addr1]\n\t"
        "mov (%[addr1]), %[tmp]\n\t"
        /* More complex address computation */
        "lea (%[larr],%[tmp],8), %[addr2]\n\t"
        "mov (%[addr2]), %[out]\n\t"
        : [out] "=r" (result),
          [addr1] "=&r" (k),
          [addr2] "=&r" (j),
          [tmp] "=&r" (i)
        : [arr] "r" (&volatile_arr[0]),
          [idx] "r" (i),
          [larr] "r" (&volatile_larr[0]),
          "m" (volatile_arr[0]),
          "m" (volatile_larr[0])
        : "memory"
    );
    
    /* Test with __builtin_constant_p for dynamic reload type selection */
    int idx = 200;
    long* ptr;
    
    if (__builtin_constant_p(idx)) {
        ptr = &volatile_larr[idx];
    } else {
        ptr = &volatile_larr[idx + 1];
    }
    
    asm volatile (
        "mov (%[ptr]), %[out2]\n\t"
        : [out2] "=r" (result)
        : [ptr] "r" (ptr),
          "m" (*ptr)
        : "memory"
    );
    
    printf("Test 2 result: %ld\n", result);
}

/* Function 3: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile long v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int out1, out2, out3, out4, out5;
    long out6, out7, out8, out9, out10;
    
    /* Register-bound variables to force specific register allocation */
    register int r1 asm("r12") = 100;
    register int r2 asm("r13") = 200;
    register int r3 asm("r14") = 300;
    register long r4 asm("r15") = 400;
    
    /* Large asm with many operands - may trigger various reload types */
    asm volatile (
        /* Multiple operations mixing inputs and outputs */
        "mov %[in1], %[o1]\n\t"
        "add %[in2], %[o1]\n\t"
        "mov %[in3], %[o2]\n\t"
        "imul %[in4], %[o2]\n\t"
        "mov %[in5], %[o3]\n\t"
        "sub %[r1], %[o3]\n\t"
        "mov %[in6], %[o4]\n\t"
        "add %[r2], %[o4]\n\t"
        "mov %[in7], %[o5]\n\t"
        "sub %[r3], %[o5]\n\t"
        "mov %[in8], %[o6]\n\t"
        "add %[r4], %[o6]\n\t"
        "mov %[in9], %[o7]\n\t"
        "mov %[in10], %[o8]\n\t"
        "lea (%[o7],%[o8],2), %[o9]\n\t"
        "mov %[o9], %[o10]\n\t"
        : [o1] "=&r" (out1),
          [o2] "=&r" (out2),
          [o3] "=&r" (out3),
          [o4] "=&r" (out4),
          [o5] "=&r" (out5),
          [o6] "=&r" (out6),
          [o7] "=&r" (out7),
          [o8] "=&r" (out8),
          [o9] "=&r" (out9),
          [o10] "=&r" (out10)
        : [in1] "irm" (v1),
          [in2] "irm" (v2),
          [in3] "irm" (v3),
          [in4] "irm" (v4),
          [in5] "irm" (v5),
          [in6] "irm" (v6),
          [in7] "irm" (v7),
          [in8] "irm" (v8),
          [in9] "irm" (v9),
          [in10] "irm" (v10),
          [r1] "r" (r1),
          [r2] "r" (r2),
          [r3] "r" (r3),
          [r4] "r" (r4),
          "m" (v1), "m" (v2), "m" (v3), "m" (v4), "m" (v5),
          "m" (v6), "m" (v7), "m" (v8), "m" (v9), "m" (v10)
        : "memory", "cc"
    );
    
    printf("Test 3 results: %d %d %d %d %d %ld %ld %ld %ld %ld\n",
           out1, out2, out3, out4, out5, out6, out7, out8, out9, out10);
}

/* Function 4: Address reloads for output operands */
void test_output_address_reloads(void) {
    volatile int buffer[128];
    int* ptrs[4];
    int i, j;
    
    /* Initialize */
    for (i = 0; i < 128; i++) {
        buffer[i] = i * 7;
    }
    
    /* Complex output address computation */
    register int* rptr1 asm("r11") = &buffer[0];
    register int* rptr2 asm("r10") = &buffer[64];
    
    asm volatile (
        /* Output address may need reloading */
        "mov %[in1], (%[out1])\n\t"
        "lea 4(%[out1]), %[tmp1]\n\t"
        "mov %[in2], (%[tmp1])\n\t"
        /* Another output with address computation */
        "mov %[in3], (%[out2],%[idx],4)\n\t"
        : 
        : [out1] "r" (rptr1),
          [out2] "r" (rptr2),
          [in1] "r" (0x1234),
          [in2] "r" (0x5678),
          [in3] "r" (0x9ABC),
          [idx] "r" (16),
          [tmp1] "=&r" (i)
        : "memory"
    );
    
    /* Test RELOAD_FOR_OUTADDR_ADDRESS */
    int index = 32;
    asm volatile (
        "movl $999, (%[base],%[index],4)\n\t"
        : 
        : [base] "r" (&buffer[0]),
          [index] "r" (index)
        : "memory"
    );
    
    printf("Test 4: buffer[0]=%d, buffer[64]=%d\n", buffer[0], buffer[64]);
}

/* Function 5: Mixed float/integer via bitcast for additional complexity */
void test_mixed_types(void) {
    volatile float farr[8];
    volatile double darr[8];
    uint32_t int_result;
    uint64_t long_result;
    
    /* Initialize */
    for (int i = 0; i < 8; i++) {
        farr[i] = i * 1.5f;
        darr[i] = i * 2.5;
    }
    
    /* Use __builtin_bit_cast to treat floats as integers in asm */
    uint32_t float_as_int = __builtin_bit_cast(uint32_t, farr[3]);
    uint64_t double_as_long = __builtin_bit_cast(uint64_t, darr[4]);
    
    asm volatile (
        /* Mix float bitcasts with integer operations */
        "mov %[fval], %[out1]\n\t"
        "add %[dval], %[out2]\n\t"
        : [out1] "=r" (int_result),
          [out2] "=r" (long_result)
        : [fval] "r" (float_as_int),
          [dval] "r" (double_as_long),
          "m" (farr[3]),
          "m" (darr[4])
        : "memory"
    );
    
    printf("Test 5: float_as_int=0x%08x, double_as_long=0x%016lx\n",
           int_result, long_result);
}

int main(void) {
    printf("Starting reload pass stress tests...\n");
    
    test_complex_constraints();
    test_nested_addresses();
    test_multi_operand_asm();
    test_output_address_reloads();
    test_mixed_types();
    
    printf("All tests completed.\n");
    return 0;
}
