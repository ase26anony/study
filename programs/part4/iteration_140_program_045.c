/* Test program to stress GCC's reload pass and cover various reload types */
#include <stdio.h>
#include <stdint.h>

/* Function 1: Complex alternative constraints and hard register clobbers */
void test_complex_constraints(void) {
    volatile int arr1[256];
    volatile int arr2[256];
    int result1, result2, result3;
    int temp1 = 123, temp2 = 456, temp3 = 789;
    
    /* Force register binding for specific variables */
    register int x asm("r10") = 1000;
    register int y asm("r11") = 2000;
    
    /* Complex asm with alternative constraints and clobbers */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%[in3], %[in4]), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "imull %[x], %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %[out3]"
        : [out1] "=r,m" (result1), 
          [out2] "=r,m" (result2),
          [out3] "=r,m" (result3)
        : [in1] "r,m,i" (temp1),
          [in2] "r,m,i" (temp2),
          [in3] "r,m,i" (temp3),
          [in4] "r,m,i" (x),
          [x] "r" (x)
        : "eax", "ecx", "edx", "r10", "r11", "memory"
    );
    
    printf("Test1 results: %d %d %d\n", result1, result2, result3);
    
    /* Another asm with memory address constraints */
    int idx = 50;
    asm volatile (
        "movl (%[addr]), %%ebx\n\t"
        "addl $1, %%ebx\n\t"
        "movl %%ebx, (%[addr])"
        : 
        : [addr] "r,m" (&arr1[idx + __builtin_constant_p(idx) ? 10 : 20]),
          "m" (arr1[0])
        : "ebx", "memory"
    );
}

/* Function 2: Nested address-of operations on volatile data */
void test_nested_addresses(void) {
    volatile int volatile_arr[256];
    volatile long volatile_long[128];
    int out1, out2, out3;
    
    /* Complex address computation */
    int complex_index = 100;
    volatile int* volatile ptr1 = &volatile_arr[0];
    
    /* Multiple levels of address taking */
    asm volatile (
        "movl (%[addr1]), %%esi\n\t"
        "movl (%[addr2]), %%edi\n\t"
        "addl %%esi, %%edi\n\t"
        "movl %%edi, %[out]"
        : [out] "=r" (out1)
        : [addr1] "r" (&volatile_arr[
                complex_index + 
                __builtin_constant_p(complex_index) ? 5 : 15
            ]),
          [addr2] "r" (&volatile_arr[
                (complex_index * 2) % 256
            ])
        : "esi", "edi", "memory"
    );
    
    /* Address of address computation */
    volatile int** ptr_to_ptr = &ptr1;
    asm volatile (
        "movq %[ptrptr], %%rax\n\t"
        "movq (%%rax), %%rbx\n\t"
        "movl (%%rbx), %%ecx\n\t"
        "movl %%ecx, %[result]"
        : [result] "=r" (out2)
        : [ptrptr] "r,m" (ptr_to_ptr)
        : "rax", "rbx", "rcx", "memory"
    );
    
    /* Mixed type address computation */
    uint64_t addr_as_int = (uint64_t)(&volatile_long[64]);
    asm volatile (
        "movq %[addr], %%r8\n\t"
        "movq (%%r8), %%r9\n\t"
        "movq %%r9, %[out]"
        : [out] "=r" (out3)
        : [addr] "r,m,i" (addr_as_int + 8)
        : "r8", "r9", "memory"
    );
    
    printf("Test2 results: %d %d %d\n", out1, out2, (int)out3);
}

/* Function 3: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    /* Declare many register-bound variables */
    register int r0 asm("r12") = 1;
    register int r1 asm("r13") = 2;
    register int r2 asm("r14") = 3;
    register int r3 asm("r15") = 4;
    
    /* Stack variables */
    int s0 = 10, s1 = 20, s2 = 30, s3 = 40, s4 = 50;
    int s5 = 60, s6 = 70, s7 = 80, s8 = 90, s9 = 100;
    
    /* Output variables */
    int o0, o1, o2, o3, o4, o5, o6, o7, o8, o9;
    
    /* Large asm with many operands - will stress operand reload logic */
    asm volatile (
        "movl %[in0], %%eax\n\t"
        "addl %[in1], %%eax\n\t"
        "movl %%eax, %[out0]\n\t"
        
        "movl %[in2], %%ebx\n\t"
        "subl %[in3], %%ebx\n\t"
        "movl %%ebx, %[out1]\n\t"
        
        "movl %[in4], %%ecx\n\t"
        "imull %[in5], %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        
        "movl %[in6], %%edx\n\t"
        "xorl %[in7], %%edx\n\t"
        "movl %%edx, %[out3]\n\t"
        
        "leal (%[in8], %[in9]), %%esi\n\t"
        "movl %%esi, %[out4]\n\t"
        
        "movl %[r0], %%edi\n\t"
        "addl %[r1], %%edi\n\t"
        "movl %%edi, %[out5]\n\t"
        
        "movl %[r2], %%r8d\n\t"
        "subl %[r3], %%r8d\n\t"
        "movl %%r8d, %[out6]\n\t"
        
        "movl %[in0], %%r9d\n\t"
        "addl %[in4], %%r9d\n\t"
        "movl %%r9d, %[out7]\n\t"
        
        "movl %[in1], %%r10d\n\t"
        "imull %[in5], %%r10d\n\t"
        "movl %%r10d, %[out8]\n\t"
        
        "movl %[in2], %%r11d\n\t"
        "addl %[in6], %%r11d\n\t"
        "movl %%r11d, %[out9]"
        
        : [out0] "=r,m" (o0),
          [out1] "=r,m" (o1),
          [out2] "=r,m" (o2),
          [out3] "=r,m" (o3),
          [out4] "=r,m" (o4),
          [out5] "=r,m" (o5),
          [out6] "=r,m" (o6),
          [out7] "=r,m" (o7),
          [out8] "=r,m" (o8),
          [out9] "=r,m" (o9)
        
        : [in0] "r,m,i" (s0),
          [in1] "r,m,i" (s1),
          [in2] "r,m,i" (s2),
          [in3] "r,m,i" (s3),
          [in4] "r,m,i" (s4),
          [in5] "r,m,i" (s5),
          [in6] "r,m,i" (s6),
          [in7] "r,m,i" (s7),
          [in8] "r,m,i" (s8),
          [in9] "r,m,i" (s9),
          [r0] "r" (r0),
          [r1] "r" (r1),
          [r2] "r" (r2),
          [r3] "r" (r3)
        
        : "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory"
    );
    
    printf("Test3 results: %d %d %d %d %d %d %d %d %d %d\n",
           o0, o1, o2, o3, o4, o5, o6, o7, o8, o9);
}

/* Function 4: Address reloads with output operands */
void test_output_address_reloads(void) {
    volatile int data[100];
    int* ptr_array[10];
    int result;
    
    /* Force output address reloads */
    register int* out_ptr asm("rbx");
    
    asm volatile (
        "leaq %[data], %%rax\n\t"
        "addq $40, %%rax\n\t"
        "movq %%rax, %[out]"
        : [out] "=r,m" (out_ptr)
        : [data] "m" (data)
        : "rax"
    );
    
    /* Use the computed pointer */
    *out_ptr = 999;
    
    /* Complex output address with indexing */
    int index = 25;
    asm volatile (
        "leaq %[base], %%rcx\n\t"
        "movslq %[idx], %%rdx\n\t"
        "leaq (%%rcx, %%rdx, 4), %%rsi\n\t"
        "movq %%rsi, %[outaddr]"
        : [outaddr] "=r,m" (ptr_array[0])
        : [base] "m" (data),
          [idx] "r,m,i" (index)
        : "rcx", "rdx", "rsi", "memory"
    );
    
    result = *ptr_array[0];
    printf("Test4 result: %d\n", result);
}

/* Function 5: Mixed float/integer via bitcast for additional pressure */
void test_mixed_types(void) {
    volatile float farr[64];
    int int_result;
    float float_result;
    
    /* Use bitcast to treat float as integer in asm */
    uint32_t float_as_int = __builtin_bit_cast(uint32_t, 3.14f);
    
    asm volatile (
        "movl %[fval], %%eax\n\t"
        "addl $0x40000000, %%eax\n\t"  /* Add 2.0 in float representation */
        "movl %%eax, %[out]"
        : [out] "=r,m" (int_result)
        : [fval] "r,m,i" (float_as_int)
        : "eax"
    );
    
    /* Convert back to float */
    float_result = __builtin_bit_cast(float, (uint32_t)int_result);
    
    /* Use in address computation */
    int idx = (int)float_result;
    asm volatile (
        "movl (%[base], %[idx], 4), %%ecx\n\t"
        "movl %%ecx, %[out]"
        : [out] "=r" (int_result)
        : [base] "r" (farr),
          [idx] "r,m,i" (idx & 63)
        : "ecx", "memory"
    );
    
    printf("Test5 result: %d (from float %f)\n", int_result, float_result);
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
