/* Test program to trigger various reload types in GCC's reload1.cc */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force reloads by using volatile arrays and complex addressing */
volatile int arr1[256];
volatile long arr2[256];
volatile void* ptr_arr[128];

/* Test 1: Complex inline assembly with multiple constraints */
void test_complex_constraints(void) {
    int result1, result2, result3;
    int input1 = 42, input2 = 100, input3 = 255;
    
    /* Multiple alternative constraints forcing different reload types */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%[in3], %[in1], 4), %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        "imull %%eax, %%ebx\n\t"
        "movl %%ebx, %[out3]"
        : [out1] "=r,m" (result1), 
          [out2] "=r,m" (result2), 
          [out3] "=r,m" (result3)
        : [in1] "r,m,i" (input1), 
          [in2] "r,m,i" (input2), 
          [in3] "r,m,i" (input3)
        : "eax", "ebx", "memory", "cc"
    );
    
    printf("Test1 results: %d %d %d\n", result1, result2, result3);
}

/* Test 2: Nested address computations with volatile */
void test_nested_addresses(void) {
    int idx1 = 10, idx2 = 20, idx3 = 30;
    void *addr1, *addr2, *addr3;
    int temp;
    
    /* Complex address calculations that may need RELOAD_FOR_INPUT_ADDRESS */
    asm volatile (
        "movq %[addr1], %%rax\n\t"
        "movq %[addr2], %%rbx\n\t"
        "subq %%rbx, %%rax\n\t"
        "movl %%eax, %[out]"
        : [out] "=r" (temp)
        : [addr1] "r" (&arr1[idx1 + idx2 * 2]), 
          [addr2] "r" (&arr2[idx3 + 7])
        : "rax", "rbx", "cc"
    );
    
    /* More complex with pointer arithmetic */
    addr1 = &arr1[idx1];
    addr2 = &arr2[idx2];
    
    /* This may trigger RELOAD_FOR_INPADDR_ADDRESS */
    asm volatile (
        "movq %[base], %%rcx\n\t"
        "addq $16, %%rcx\n\t"
        "movq (%%rcx), %%rdx\n\t"
        "movq %%rdx, %[result]"
        : [result] "=r" (addr3)
        : [base] "r" (&ptr_arr[idx3])
        : "rcx", "rdx", "memory"
    );
    
    printf("Test2: temp=%d, addr3=%p\n", temp, addr3);
}

/* Test 3: Register variables with explicit binding */
void test_register_variables(void) {
    /* Bind to specific registers to force conflicts */
    register int r10_var asm("r10") = 1000;
    register int r11_var asm("r11") = 2000;
    register int r12_var asm("r12") = 3000;
    register void* r13_ptr asm("r13") = (void*)&arr1[0];
    
    int output1, output2;
    void* output_ptr;
    
    /* Force output address reloads */
    asm volatile (
        "movl %[r10], %%eax\n\t"
        "addl %[r11], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movq %[r13], %%rbx\n\t"
        "addq $40, %%rbx\n\t"
        "movq %%rbx, %[out2]"
        : [out1] "=m,r" (output1), 
          [out2] "=m,r" (output_ptr)
        : [r10] "r" (r10_var), 
          [r11] "r" (r11_var), 
          [r13] "r" (r13_ptr)
        : "rax", "rbx", "memory", "cc"
    );
    
    /* Use the results to prevent optimization */
    arr1[0] = output1;
    ptr_arr[0] = output_ptr;
    
    printf("Test3: output1=%d, output_ptr=%p\n", output1, output_ptr);
}

/* Test 4: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    int in1 = 1, in2 = 2, in3 = 3, in4 = 4, in5 = 5;
    int in6 = 6, in7 = 7, in8 = 8, in9 = 9, in10 = 10;
    int out1, out2, out3, out4, out5;
    int out6, out7, out8, out9, out10;
    
    /* 10 inputs, 10 outputs - maximum register pressure */
    asm volatile (
        "movl %[i1], %%eax\n\t"
        "addl %[i2], %%eax\n\t"
        "movl %%eax, %[o1]\n\t"
        "movl %[i3], %%ebx\n\t"
        "imull %[i4], %%ebx\n\t"
        "movl %%ebx, %[o2]\n\t"
        "movl %[i5], %%ecx\n\t"
        "subl %[i6], %%ecx\n\t"
        "movl %%ecx, %[o3]\n\t"
        "movl %[i7], %%edx\n\t"
        "andl %[i8], %%edx\n\t"
        "movl %%edx, %[o4]\n\t"
        "movl %[i9], %%esi\n\t"
        "orl %[i10], %%esi\n\t"
        "movl %%esi, %[o5]\n\t"
        "leal (%%eax, %%ebx, 2), %%edi\n\t"
        "movl %%edi, %[o6]\n\t"
        "leal (%%ecx, %%edx, 4), %%r8d\n\t"
        "movl %%r8d, %[o7]\n\t"
        "leal (%%esi, %%edi, 1), %%r9d\n\t"
        "movl %%r9d, %[o8]\n\t"
        "leal (%%r8d, %%r9d, 2), %%r10d\n\t"
        "movl %%r10d, %[o9]\n\t"
        "leal (%%eax, %%r10d, 1), %%r11d\n\t"
        "movl %%r11d, %[o10]"
        : [o1] "=r,m" (out1), [o2] "=r,m" (out2), [o3] "=r,m" (out3),
          [o4] "=r,m" (out4), [o5] "=r,m" (out5), [o6] "=r,m" (out6),
          [o7] "=r,m" (out7), [o8] "=r,m" (out8), [o9] "=r,m" (out9),
          [o10] "=r,m" (out10)
        : [i1] "r,m,i" (in1), [i2] "r,m,i" (in2), [i3] "r,m,i" (in3),
          [i4] "r,m,i" (in4), [i5] "r,m,i" (in5), [i6] "r,m,i" (in6),
          [i7] "r,m,i" (in7), [i8] "r,m,i" (in8), [i9] "r,m,i" (in9),
          [i10] "r,m,i" (in10)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "memory", "cc"
    );
    
    printf("Test4: out1=%d out10=%d\n", out1, out10);
}

/* Test 5: __builtin_constant_p in address contexts */
void test_builtin_constant_address(void) {
    int idx = 64;
    void* addr;
    int result;
    
    /* Conditional address expression */
    addr = __builtin_constant_p(idx) 
           ? (void*)&arr1[16]  /* constant address */
           : (void*)&arr1[idx]; /* non-constant address */
    
    /* Force reload for operand address */
    asm volatile (
        "movq %[addr], %%rax\n\t"
        "movl (%%rax), %%ebx\n\t"
        "addl $1, %%ebx\n\t"
        "movl %%ebx, %[out]"
        : [out] "=r" (result)
        : [addr] "r" (addr)
        : "rax", "rbx", "memory"
    );
    
    /* Another variation with more complexity */
    int offset = __builtin_constant_p(result) ? 8 : 16;
    
    asm volatile (
        "movq %[base], %%rcx\n\t"
        "addq %[off], %%rcx\n\t"
        "movl (%%rcx), %%edx\n\t"
        "movl %%edx, %[out2]"
        : [out2] "=r" (result)
        : [base] "r" (&arr2[0]), [off] "r" (offset)
        : "rcx", "rdx", "memory"
    );
    
    printf("Test5: result=%d\n", result);
}

/* Test 6: Mixed float/integer via bitcast (triggers different constraints) */
void test_mixed_types(void) {
    float f1 = 3.14f, f2 = 2.71f;
    int i1, i2;
    
    /* Convert floats to ints via bitcast for integer asm */
    i1 = __builtin_bit_cast(int, f1);
    i2 = __builtin_bit_cast(int, f2);
    
    int out_int;
    float out_float;
    
    /* Mixed constraints - integer register but float value */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "xorl %[in2], %%eax\n\t"
        "movl %%eax, %[out_i]\n\t"
        "movd %[in1], %%xmm0\n\t"
        "addss %[in2], %%xmm0\n\t"
        "movd %%xmm0, %[out_f]"
        : [out_i] "=r,m" (out_int), 
          [out_f] "=r,m" (out_float)
        : [in1] "r,m" (i1), 
          [in2] "r,m" (i2)
        : "rax", "xmm0", "memory"
    );
    
    printf("Test6: out_int=%d, out_float=%f\n", out_int, out_float);
}

/* Main function that runs all tests */
int main(void) {
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        if (i < 128) ptr_arr[i] = (void*)(uintptr_t)i;
    }
    
    printf("Starting reload stress tests...\n");
    
    test_complex_constraints();
    test_nested_addresses();
    test_register_variables();
    test_multi_operand_asm();
    test_builtin_constant_address();
    test_mixed_types();
    
    printf("All tests completed.\n");
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += arr1[i];
    }
    
    return sum == 0 ? 0 : 0;  /* Always return 0 */
}
