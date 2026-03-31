#include <stdio.h>
#include <stdint.h>

/* Test 1: Complex inline assembly with multiple constraints and clobbers */
void test1_complex_constraints(void) {
    volatile int arr[256] = {0};
    int input1 = 42, input2 = 100;
    int output1, output2;
    int dummy;
    
    /* Force multiple reload types with alternative constraints */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%[in1], %[in2], 2), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "movl %[mem], %%edx\n\t"
        "addl %%edx, %[out1]"
        : [out1] "=r,m" (output1), 
          [out2] "=r,m" (output2),
          "=m" (arr[10])
        : [in1] "r,i,m" (input1),
          [in2] "r,i,m" (input2),
          [mem] "m,r" (arr[5]),
          "m" (arr[15])
        : "eax", "ecx", "edx", "r8", "r9", "r10", "r11", "memory"
    );
    
    printf("Test1: output1=%d, output2=%d\n", output1, output2);
}

/* Test 2: Nested address computations with volatile */
void test2_nested_addresses(void) {
    volatile int volatile_arr[256];
    volatile long volatile_long[128];
    int index = 50;
    int result;
    uintptr_t addr1, addr2;
    
    /* Complex address calculation requiring multiple reload types */
    asm volatile (
        "movq %[addr1], %%rax\n\t"
        "movq %[addr2], %%rbx\n\t"
        "subq %%rbx, %%rax\n\t"
        "shrq $2, %%rax\n\t"
        "movl %%eax, %[res]"
        : [res] "=r,m" (result)
        : [addr1] "r,m" (&volatile_arr[index + 10]),
          [addr2] "r,m" (&volatile_arr[index * 2]),
          "m" (volatile_arr[0]),
          "m" (volatile_arr[255])
        : "rax", "rbx", "rcx", "memory"
    );
    
    /* Another complex case with pointer arithmetic */
    int *ptr = (int*)&volatile_long[0];
    asm volatile (
        "movq %[ptr], %%r12\n\t"
        "addq $16, %%r12\n\t"
        "movq %%r12, %[out]"
        : [out] "=r,m" (addr1)
        : [ptr] "r,m" (ptr + index / 2),
          "m" (volatile_long[0])
        : "r12", "r13", "memory"
    );
    
    printf("Test2: result=%d, addr1=%lu\n", result, (unsigned long)addr1);
}

/* Test 3: Register variables with explicit binding */
void test3_register_variables(void) {
    register int r10_var asm("r10") = 1000;
    register int r11_var asm("r11") = 2000;
    volatile int mem_array[100];
    int output;
    uintptr_t addr_out;
    
    /* Force reloads for output addresses */
    asm volatile (
        "movl %[r10], %%eax\n\t"
        "addl %[r11], %%eax\n\t"
        "movl %%eax, %[out]\n\t"
        "leaq %[mem], %%rbx\n\t"
        "addq $40, %%rbx\n\t"
        "movq %%rbx, %[addr]"
        : [out] "=m,r" (mem_array[30]),
          [addr] "=r,m" (addr_out)
        : [r10] "r,m" (r10_var),
          [r11] "r,m" (r11_var),
          [mem] "m,r" (mem_array[0])
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    printf("Test3: mem[30]=%d, addr_out=%lu\n", mem_array[30], (unsigned long)addr_out);
}

/* Test 4: Large multi-operand asm statement */
void test4_multi_operand_asm(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int o1, o2, o3, o4, o5, o6, o7, o8, o9, o10;
    float f1 = 1.5f, f2 = 2.5f;
    uint32_t f1_bits = __builtin_bit_cast(uint32_t, f1);
    uint32_t f2_bits = __builtin_bit_cast(uint32_t, f2);
    uint32_t fout_bits;
    
    /* Large asm with many operands to stress reload pass */
    asm volatile (
        "movl %[i1], %%eax\n\t"
        "addl %[i2], %%eax\n\t"
        "movl %%eax, %[o1]\n\t"
        "movl %[i3], %%ebx\n\t"
        "subl %[i4], %%ebx\n\t"
        "movl %%ebx, %[o2]\n\t"
        "movl %[i5], %%ecx\n\t"
        "imull %[i6], %%ecx\n\t"
        "movl %%ecx, %[o3]\n\t"
        "movl %[f1], %%edx\n\t"
        "addl %[f2], %%edx\n\t"
        "movl %%edx, %[fo]\n\t"
        "leaq %[mem1], %%r8\n\t"
        "addq $4, %%r8\n\t"
        "movq %%r8, %[o4]\n\t"
        "movq %[mem2], %%r9\n\t"
        "addq $8, %%r9\n\t"
        "movq %%r9, %[o5]"
        : [o1] "=r,m" (o1),
          [o2] "=r,m" (o2),
          [o3] "=r,m" (o3),
          [fo] "=r,m" (fout_bits),
          [o4] "=r,m" (o4),
          [o5] "=r,m" (o5),
          "=m" (v1),
          "=m" (v2)
        : [i1] "r,i,m" (v1),
          [i2] "r,i,m" (v2),
          [i3] "r,i,m" (v3),
          [i4] "r,i,m" (v4),
          [i5] "r,i,m" (v5),
          [i6] "r,i,m" (10),
          [f1] "r,m" (f1_bits),
          [f2] "r,m" (f2_bits),
          [mem1] "m,r" (v1),
          [mem2] "m,r" (v2),
          "m" (v3),
          "m" (v4),
          "m" (v5)
        : "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11", 
          "r12", "r13", "r14", "r15", "memory"
    );
    
    float fout = __builtin_bit_cast(float, fout_bits);
    printf("Test4: o1=%d o2=%d o3=%d fout=%.2f\n", o1, o2, o3, fout);
}

/* Test 5: __builtin_constant_p in address contexts */
void test5_constant_p_address(void) {
    volatile int data[100];
    int index = 30;
    int result;
    uintptr_t addr;
    
    /* Use __builtin_constant_p to create different address expressions */
    int offset = __builtin_constant_p(index) ? 4 : 8;
    
    asm volatile (
        "movq %[addr], %%rax\n\t"
        "addq $16, %%rax\n\t"
        "movq %%rax, %[out]"
        : [out] "=r,m" (addr)
        : [addr] "r,m" (&data[index + offset]),
          "m" (data[0]),
          "m" (data[99])
        : "rax", "rbx", "memory"
    );
    
    /* Another variant with conditional address */
    const int const_idx = 10;
    int* ptr = __builtin_constant_p(1) ? 
               &data[const_idx] : 
               &data[index];
    
    asm volatile (
        "movl (%[ptr]), %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %[res]"
        : [res] "=r,m" (result)
        : [ptr] "r,m" (ptr),
          "m" (data[0])
        : "rax", "memory"
    );
    
    printf("Test5: addr=%lu result=%d\n", (unsigned long)addr, result);
}

/* Test 6: Mixed types and complex constraints */
void test6_mixed_types(void) {
    volatile short sarr[200];
    volatile char carr[300];
    volatile double darr[50];
    long double ld = 3.14L;
    int result_int;
    double result_double;
    uintptr_t ptr_result;
    
    /* Mixed type accesses with complex addressing */
    asm volatile (
        "movzwl %[sptr], %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %[ri]\n\t"
        "movsd %[dptr], %%xmm0\n\t"
        "addsd %%xmm1, %%xmm0\n\t"
        "movsd %%xmm0, %[rd]"
        : [ri] "=r,m" (result_int),
          [rd] "=m,r" (result_double)
        : [sptr] "m,r" (sarr[50]),
          [dptr] "m,r" (darr[10]),
          "m" (carr[0]),
          "m" (carr[299])
        : "rax", "xmm0", "xmm1", "xmm2", "memory"
    );
    
    printf("Test6: int=%d double=%.2f\n", result_int, result_double);
}

int main(void) {
    printf("Starting reload pass stress tests...\n");
    
    test1_complex_constraints();
    test2_nested_addresses();
    test3_register_variables();
    test4_multi_operand_asm();
    test5_constant_p_address();
    test6_mixed_types();
    
    printf("All tests completed.\n");
    return 0;
}
