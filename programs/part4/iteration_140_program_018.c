/* Test program to trigger various reload types in GCC's reload1.cc */
#include <stdio.h>
#include <stdint.h>

/* Function 1: Complex alternative constraints and hard register clobbers */
void test_complex_constraints(void) {
    volatile int arr1[256];
    volatile int arr2[256];
    int result1, result2, result3;
    
    /* Force register pressure with many variables */
    register int r0 asm("r10") = 100;
    register int r1 asm("r11") = 200;
    register int r2 asm("r12") = 300;
    int x = 400, y = 500, z = 600;
    
    /* Complex asm with alternative constraints and clobbers */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%[in3],%[in4],4), %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        "imull %[in5], %%eax\n\t"
        "movl %%eax, %[out3]"
        : [out1] "=r,m" (result1),
          [out2] "=r,m" (result2),
          [out3] "=r,m" (result3)
        : [in1] "r,m,i" (r0),
          [in2] "r,m,i" (r1),
          [in3] "r,m,i" (r2),
          [in4] "r,m,i" (x),
          [in5] "r,m,i" (y)
        : "eax", "ebx", "r10", "r11", "r12", "memory"
    );
    
    printf("Test1 results: %d %d %d\n", result1, result2, result3);
    
    /* Another asm with address constraints */
    int addr_result;
    asm volatile (
        "movl (%%rsi,%%rdi,4), %%eax\n\t"
        "addl $42, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (addr_result)
        : "S" (&arr1[0]), "D" (r0)
        : "eax", "memory"
    );
    
    printf("Address result: %d\n", addr_result);
}

/* Function 2: Nested address-of operations on volatile data */
void test_nested_addresses(void) {
    volatile int volatile_arr[256];
    volatile long volatile_arr2[128];
    
    /* Initialize arrays to prevent optimization */
    for (int i = 0; i < 256; i++) {
        volatile_arr[i] = i * 3;
    }
    
    int index = 50;
    int offset = 25;
    intptr_t addr1, addr2, addr3;
    int value1, value2;
    
    /* Complex address calculation forcing RELOAD_FOR_INPUT_ADDRESS */
    asm volatile (
        "movl (%[base],%[index],4), %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %[val1]\n\t"
        "leaq (%[base],%[index],4), %[addr1]"
        : [val1] "=r" (value1),
          [addr1] "=r" (addr1)
        : [base] "r" (&volatile_arr[0]),
          [index] "r" (index + offset)
        : "eax", "memory"
    );
    
    /* Nested address with pointer arithmetic - may trigger RELOAD_FOR_INPADDR_ADDRESS */
    int* volatile_ptr = (int*)&volatile_arr[0];
    int complex_index = index * 2 + offset / 2;
    
    asm volatile (
        "movq %[ptr], %%rax\n\t"
        "addq $16, %%rax\n\t"
        "movq %%rax, %[addr2]\n\t"
        "movl (%%rax,%[idx],4), %%ebx\n\t"
        "movl %%ebx, %[val2]"
        : [addr2] "=r" (addr2),
          [val2] "=r" (value2)
        : [ptr] "r" (volatile_ptr + complex_index),
          [idx] "r" (offset)
        : "rax", "ebx", "memory"
    );
    
    /* Using __builtin_constant_p to create dynamic address reloads */
    int dynamic_index = __builtin_constant_p(index) ? 0 : index;
    asm volatile (
        "movl (%[arr],%[idx],4), %%ecx"
        : 
        : [arr] "r" (&volatile_arr[0]),
          [idx] "r" (dynamic_index * 2)
        : "ecx", "memory"
    );
    
    printf("Test2: value1=%d, addr1=%p, value2=%d, addr2=%p\n", 
           value1, (void*)addr1, value2, (void*)addr2);
}

/* Function 3: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    volatile double darr[8];
    volatile int iarr[16];
    
    /* Initialize */
    for (int i = 0; i < 8; i++) darr[i] = i * 1.5;
    for (int i = 0; i < 16; i++) iarr[i] = i * 2;
    
    /* Register-bound variables for additional pressure */
    register int reg_a asm("r13") = 1000;
    register int reg_b asm("r14") = 2000;
    register int reg_c asm("r15") = 3000;
    
    int out1, out2, out3, out4, out5, out6, out7, out8, out9, out10;
    double dout1, dout2;
    
    /* Convert double to integer for use in integer constraints */
    uint64_t dbl_as_int1 = __builtin_bit_cast(uint64_t, darr[0]);
    uint64_t dbl_as_int2 = __builtin_bit_cast(uint64_t, darr[1]);
    
    /* Large asm with many operands - forces many rl->opnum values */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %[in3], %%ebx\n\t"
        "subl %[in4], %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        "imull %[in5], %%eax\n\t"
        "movl %%eax, %[out3]\n\t"
        "movq %[in6], %%xmm0\n\t"
        "movq %[in7], %%xmm1\n\t"
        "addsd %%xmm1, %%xmm0\n\t"
        "movq %%xmm0, %[out4]\n\t"
        "leaq (%[in8],%[in9],2), %%rcx\n\t"
        "movq %%rcx, %[out5]\n\t"
        "movl (%%rcx), %%edx\n\t"
        "movl %%edx, %[out6]\n\t"
        "movl %[in10], %%esi\n\t"
        "addl $1, %%esi\n\t"
        "movl %%esi, %[out7]\n\t"
        "movl %[in11], %%edi\n\t"
        "addl $2, %%edi\n\t"
        "movl %%edi, %[out8]\n\t"
        "movl %[in12], %%r8d\n\t"
        "addl $3, %%r8d\n\t"
        "movl %%r8d, %[out9]\n\t"
        "movl %[in13], %%r9d\n\t"
        "addl $4, %%r9d\n\t"
        "movl %%r9d, %[out10]"
        : [out1] "=r" (out1),
          [out2] "=r" (out2),
          [out3] "=r" (out3),
          [out4] "=r" (dout1),
          [out5] "=r" (out4),
          [out6] "=r" (out5),
          [out7] "=r" (out6),
          [out8] "=r" (out7),
          [out9] "=r" (out8),
          [out10] "=r" (out9)
        : [in1] "r" (reg_a),
          [in2] "r" (reg_b),
          [in3] "r" (iarr[0]),
          [in4] "r" (iarr[1]),
          [in5] "r" (reg_c),
          [in6] "r" (dbl_as_int1),
          [in7] "r" (dbl_as_int2),
          [in8] "r" (&iarr[0]),
          [in9] "r" (reg_a),
          [in10] "r" (iarr[2]),
          [in11] "r" (iarr[3]),
          [in12] "r" (iarr[4]),
          [in13] "r" (iarr[5])
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "r8", "r9",
          "xmm0", "xmm1", "r13", "r14", "r15", "memory"
    );
    
    /* Force output address reloads */
    int* output_ptr = &out10;
    asm volatile (
        "movl $999, (%[ptr])"
        :
        : [ptr] "r" (output_ptr)
        : "memory"
    );
    
    printf("Test3: out1=%d out2=%d out3=%d out4=%d out5=%d out6=%d "
           "out7=%d out8=%d out9=%d out10=%d dout1=%f\n",
           out1, out2, out3, out4, out5, out6, out7, out8, out9, out10, dout1);
}

/* Function 4: Mixed address types for output reloads */
void test_output_address_reloads(void) {
    volatile int output_buffer[32];
    int input_values[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Force RELOAD_FOR_OUTPUT_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
    for (int i = 0; i < 8; i++) {
        int* dest_ptr = &output_buffer[i * 2];
        int src_val = input_values[i] * 10;
        
        asm volatile (
            "movl %[src], %%eax\n\t"
            "addl $1000, %%eax\n\t"
            "movl %%eax, (%[dest])"
            :
            : [src] "r" (src_val),
              [dest] "r" (dest_ptr)
            : "eax", "memory"
        );
    }
    
    /* Complex output with address computation */
    int* base_ptr = &output_buffer[16];
    register int idx_reg asm("rbx") = 4;
    
    asm volatile (
        "movl $0xDEADBEEF, (%[base],%[idx],4)"
        :
        : [base] "r" (base_ptr),
          [idx] "r" (idx_reg)
        : "memory"
    );
    
    printf("Test4: output_buffer[8]=%d output_buffer[32]=%d\n",
           output_buffer[8], output_buffer[32]);
}

int main(void) {
    printf("Starting reload type coverage tests...\n");
    
    test_complex_constraints();
    test_nested_addresses();
    test_multi_operand_asm();
    test_output_address_reloads();
    
    printf("All tests completed.\n");
    return 0;
}
