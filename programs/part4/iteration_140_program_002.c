/* Test program to stress GCC's reload pass and cover specific reload types */
#include <stdio.h>
#include <stdint.h>

/* Global volatile arrays to prevent optimization */
volatile int global_arr[256];
volatile long global_long_arr[128];
volatile void *global_ptr_arr[64];

/* Function 1: Complex alternative constraints and hard register clobbers */
void test_complex_constraints(void) {
    volatile int arr[100];
    int i, result1, result2, result3;
    
    /* Initialize array */
    for (i = 0; i < 100; i++) arr[i] = i;
    
    /* Complex asm with alternative constraints and multiple clobbers */
    asm volatile (
        /* Output with alternative constraints */
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        "movl %[out1], %[out2]\n\t"
        "imull %[in3], %[out2]\n\t"
        "leal (%[out2], %[in4], 2), %[out3]"
        : [out1] "=r,m" (result1),   /* Alternative: register or memory */
          [out2] "=r,m" (result2),
          [out3] "=r,m" (result3)
        : [in1] "r,m,i" (arr[10]),   /* Three alternatives */
          [in2] "r,m,i" (arr[20]),
          [in3] "r,m,i" (arr[30]),
          [in4] "r,m,i" (arr[40])
        : "eax", "ebx", "ecx", "edx", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory"
    );
    
    printf("Test 1 results: %d %d %d\n", result1, result2, result3);
}

/* Function 2: Nested address computations with volatile */
void test_nested_addresses(void) {
    volatile int arr[256];
    volatile long larr[128];
    int i, out1, out2;
    long out3;
    
    /* Initialize arrays */
    for (i = 0; i < 256; i++) arr[i] = i * 2;
    for (i = 0; i < 128; i++) larr[i] = i * 3;
    
    /* Complex address computations in asm operands */
    asm volatile (
        "movl (%[addr1]), %[out1]\n\t"
        "movq (%[addr2]), %[out3]\n\t"
        "addl %[out1], %[out2]"
        : [out1] "=r" (out1),
          [out2] "=r" (out2),
          [out3] "=r" (out3)
        : [addr1] "r" (&arr[arr[50] + arr[25] * 2]),  /* Nested address computation */
          [addr2] "r" (&larr[larr[30] / 4 + 10]),
          "0" (0)  /* out1 initialized to 0 */
        : "memory"
    );
    
    /* More complex: address of volatile element with pointer arithmetic */
    int idx = arr[60];
    asm volatile (
        ""
        : "=r" (out1)
        : "r" (&global_arr[global_arr[idx] + 100]),  /* Can trigger RELOAD_FOR_INPUT_ADDRESS */
          "r" (&global_long_arr[global_long_arr[20] % 64]),
          "r" (&global_ptr_arr[global_arr[70] & 63])
        : "memory"
    );
    
    printf("Test 2 results: %d %ld\n", out1 + out2, out3);
}

/* Function 3: Register variables with explicit binding */
void test_register_variables(void) {
    /* Declare register variables bound to specific hard registers */
    register int r10_var asm("r10") = 100;
    register int r11_var asm("r11") = 200;
    register int r12_var asm("r12") = 300;
    register int r13_var asm("r13") = 400;
    register void *r14_ptr asm("r14");
    
    volatile int arr[100];
    int i, out1, out2, out3;
    
    for (i = 0; i < 100; i++) arr[i] = i;
    
    r14_ptr = &arr[50];
    
    /* Force conflicts with register-bound variables */
    asm volatile (
        "movl %%r10d, %[out1]\n\t"
        "addl %%r11d, %[out1]\n\t"
        "movl %%r12d, %[out2]\n\t"
        "subl %%r13d, %[out2]\n\t"
        "movl (%%r14), %[out3]"
        : [out1] "=r,m" (out1),
          [out2] "=r,m" (out2),
          [out3] "=r,m" (out3)
        : /* No inputs, using register variables directly */
        : "r10", "r11", "r12", "r13", "r14", "memory"
    );
    
    /* Output address reloads */
    asm volatile (
        "movl %[val], (%[addr])"
        : 
        : [addr] "r" (&arr[r10_var + r11_var]),  /* Can trigger RELOAD_FOR_OUTPUT_ADDRESS */
          [val] "r" (r12_var)
        : "memory"
    );
    
    printf("Test 3 results: %d %d %d\n", out1, out2, out3);
}

/* Function 4: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    volatile int arr[50];
    int i;
    int out1, out2, out3, out4, out5, out6, out7, out8, out9, out10;
    int in1, in2, in3, in4, in5, in6, in7, in8, in9, in10;
    
    /* Initialize */
    for (i = 0; i < 50; i++) arr[i] = i * 3;
    
    in1 = arr[1]; in2 = arr[2]; in3 = arr[3]; in4 = arr[4]; in5 = arr[5];
    in6 = arr[6]; in7 = arr[7]; in8 = arr[8]; in9 = arr[9]; in10 = arr[10];
    
    /* Large asm with many operands - maximizes rl->opnum values */
    asm volatile (
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        "movl %[in3], %[out2]\n\t"
        "imull %[in4], %[out2]\n\t"
        "movl %[in5], %[out3]\n\t"
        "andl %[in6], %[out3]\n\t"
        "movl %[in7], %[out4]\n\t"
        "orl %[in8], %[out4]\n\t"
        "movl %[in9], %[out5]\n\t"
        "xorl %[in10], %[out5]\n\t"
        "leal (%[out1], %[out2]), %[out6]\n\t"
        "leal (%[out3], %[out4]), %[out7]\n\t"
        "addl %[out5], %[out6]\n\t"
        "subl %[out7], %[out6]\n\t"
        "movl %[out6], %[out8]\n\t"
        "movl %[out7], %[out9]\n\t"
        "addl $1, %[out10]"
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
        : [in1] "r" (in1),
          [in2] "r" (in2),
          [in3] "r" (in3),
          [in4] "r" (in4),
          [in5] "r" (in5),
          [in6] "r" (in6),
          [in7] "r" (in7),
          [in8] "r" (in8),
          [in9] "r" (in9),
          [in10] "r" (in10),
          "0" (0),  /* out1 initialized to 0 */
          "1" (0),  /* out2 initialized to 0 */
          "2" (0)   /* out3 initialized to 0 */
        : "memory"
    );
    
    printf("Test 4 results: %d %d %d %d %d\n", out1, out6, out8, out9, out10);
}

/* Function 5: __builtin_constant_p in address contexts */
void test_builtin_constant_p(void) {
    volatile int arr[100];
    int i, result;
    void *addr1, *addr2;
    
    for (i = 0; i < 100; i++) arr[i] = i;
    
    /* Use __builtin_constant_p to select address expression */
    int idx = arr[30];
    
    /* Conditional address selection */
    addr1 = __builtin_constant_p(idx) ? &arr[10] : &arr[idx + 20];
    addr2 = __builtin_constant_p(idx + 5) ? &arr[20] : &arr[idx * 2];
    
    /* Use these addresses in asm */
    asm volatile (
        "movl (%[addr1]), %[result]\n\t"
        "addl (%[addr2]), %[result]"
        : [result] "=r" (result)
        : [addr1] "r" (addr1),
          [addr2] "r" (addr2)
        : "memory"
    );
    
    /* Another variation with more complex expression */
    asm volatile (
        ""
        : "=r" (result)
        : "r" (__builtin_constant_p(arr[40]) ? 
                &global_arr[10] : 
                &global_arr[global_arr[40] & 255]),
          "r" (__builtin_constant_p(arr[50] + 100) ?
                &global_long_arr[5] :
                &global_long_arr[(arr[50] + 100) % 128])
        : "memory"
    );
    
    printf("Test 5 result: %d\n", result);
}

/* Function 6: Mixed float/integer via bitcast */
void test_mixed_types(void) {
    volatile float farr[50];
    volatile double darr[50];
    int i;
    uint64_t out1, out2;
    
    /* Initialize */
    for (i = 0; i < 50; i++) {
        farr[i] = i * 1.5f;
        darr[i] = i * 2.5;
    }
    
    /* Use __builtin_bit_cast to treat floats as integers in asm */
    uint32_t float_as_int = __builtin_bit_cast(uint32_t, farr[10]);
    uint64_t double_as_int = __builtin_bit_cast(uint64_t, darr[20]);
    
    asm volatile (
        "mov %[fin], %[out1]\n\t"
        "mov %[din], %[out2]\n\t"
        "add $0x1000, %[out1]\n\t"
        "sub $0x2000, %[out2]"
        : [out1] "=r" (out1),
          [out2] "=r" (out2)
        : [fin] "r" ((uint64_t)float_as_int),
          [din] "r" (double_as_int)
        : "memory"
    );
    
    printf("Test 6 results: 0x%lx 0x%lx\n", out1, out2);
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
    
    /* Use results to prevent optimization */
    volatile int dummy = 0;
    for (int i = 0; i < 256; i++) {
        dummy += global_arr[i];
    }
    
    return dummy == 0 ? 0 : 1;
}
