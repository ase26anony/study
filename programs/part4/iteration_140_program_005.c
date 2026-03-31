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
    
    /* Register variables bound to specific hard registers */
    register int x asm("r10");
    register int y asm("r11");
    register int z asm("r12");
    
    x = 100;
    y = 200;
    z = 300;
    
    /* Complex inline asm with alternative constraints and clobbers */
    int result1, result2, result3;
    
    asm volatile (
        /* Multiple alternative constraints forcing reload decisions */
        "movl %[in1], %%eax\n\t"
        "addl %[in2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%[in3], %[in4], 4), %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        "imull %[in5], %%ecx\n\t"
        "movl %%ecx, %[out3]\n\t"
        : [out1] "=r,m" (result1),   /* Alternative: register or memory */
          [out2] "=r,m" (result2),
          [out3] "=r,m" (result3)
        : [in1] "r,i,m" (x),         /* Three alternatives */
          [in2] "r,i,m" (y),
          [in3] "r,i,m" (arr1[10]),
          [in4] "r,i,m" (arr2[20]),
          [in5] "r,i,m" (z)
        : "eax", "ebx", "ecx", "edx", "r10", "r11", "r12", "memory"
    );
    
    printf("Test 1 results: %d %d %d\n", result1, result2, result3);
}

/* Function 2: Nested address computations with volatile */
void test_nested_addresses(void) {
    volatile int buffer[512];
    volatile int* volatile ptr_array[32];
    int i;
    
    /* Initialize data */
    for (i = 0; i < 512; i++) {
        buffer[i] = i * 2;
    }
    for (i = 0; i < 32; i++) {
        ptr_array[i] = &buffer[i * 16];
    }
    
    /* Complex address calculations that may need reloads */
    int sum = 0;
    register int idx asm("r13") = 100;
    
    for (i = 0; i < 10; i++) {
        int temp;
        /* Taking address of volatile array element with complex index */
        volatile int* addr1 = &buffer[idx + i * 3];
        volatile int* addr2 = &ptr_array[i][5];
        
        /* Inline asm using these addresses as inputs */
        asm volatile (
            "movl (%[addr1]), %%eax\n\t"
            "addl (%[addr2]), %%eax\n\t"
            "movl %%eax, %[result]\n\t"
            : [result] "=r" (temp)
            : [addr1] "r" (addr1),   /* May need RELOAD_FOR_INPUT_ADDRESS */
              [addr2] "r" (addr2)    /* May need RELOAD_FOR_INPADDR_ADDRESS */
            : "eax", "memory"
        );
        
        sum += temp;
    }
    
    printf("Test 2 sum: %d\n", sum);
}

/* Function 3: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    volatile double darr[8];
    volatile int iarr[16];
    int i;
    
    for (i = 0; i < 8; i++) darr[i] = i * 1.5;
    for (i = 0; i < 16; i++) iarr[i] = i * 3;
    
    /* Register variables for fixed registers */
    register int r0 asm("rax");
    register int r1 asm("rbx");
    register int r2 asm("rcx");
    register int r3 asm("rdx");
    register int r4 asm("rsi");
    register int r5 asm("rdi");
    register int r6 asm("r8");
    register int r7 asm("r9");
    register int r8 asm("r14");
    register int r9 asm("r15");
    
    r0 = 1; r1 = 2; r2 = 3; r3 = 4; r4 = 5; r5 = 6; r6 = 7; r7 = 8; r8 = 9; r9 = 10;
    
    /* Results */
    int out0, out1, out2, out3, out4, out5, out6, out7, out8, out9;
    int out10, out11, out12, out13, out14;
    
    /* Large asm with many operands - stresses opnum handling */
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
        "andl %[in7], %%edx\n\t"
        "movl %%edx, %[out3]\n\t"
        "movl %[in8], %%esi\n\t"
        "orl %[in9], %%esi\n\t"
        "movl %%esi, %[out4]\n\t"
        "movq %[din0], %%xmm0\n\t"
        "movq %[din1], %%xmm1\n\t"
        "paddq %%xmm1, %%xmm0\n\t"
        "movq %%xmm0, %[out5]\n\t"
        "leal (%[addr0], %[addr1], 2), %%edi\n\t"
        "movl %%edi, %[out6]\n\t"
        "leal (%[addr2], %[addr3], 4), %%r8d\n\t"
        "movl %%r8d, %[out7]\n\t"
        "movl %[mem0], %%r9d\n\t"
        "addl %[mem1], %%r9d\n\t"
        "movl %%r9d, %[out8]\n\t"
        "movl %[mem2], %%r10d\n\t"
        "subl %[mem3], %%r10d\n\t"
        "movl %%r10d, %[out9]\n\t"
        "movl %[mem4], %%r11d\n\t"
        "imull %[mem5], %%r11d\n\t"
        "movl %%r11d, %[out10]\n\t"
        "movl %[mem6], %%r12d\n\t"
        "andl %[mem7], %%r12d\n\t"
        "movl %%r12d, %[out11]\n\t"
        "movl %[mem8], %%r13d\n\t"
        "orl %[mem9], %%r13d\n\t"
        "movl %%r13d, %[out12]\n\t"
        "movl %[cin0], %%r14d\n\t"
        "addl %[cin1], %%r14d\n\t"
        "movl %%r14d, %[out13]\n\t"
        "movl %[cin2], %%r15d\n\t"
        "subl %[cin3], %%r15d\n\t"
        "movl %%r15d, %[out14]\n\t"
        : [out0] "=r" (out0), [out1] "=r" (out1), [out2] "=r" (out2),
          [out3] "=r" (out3), [out4] "=r" (out4), [out5] "=r" (out5),
          [out6] "=r" (out6), [out7] "=r" (out7), [out8] "=r" (out8),
          [out9] "=r" (out9), [out10] "=r" (out10), [out11] "=r" (out11),
          [out12] "=r" (out12), [out13] "=r" (out13), [out14] "=r" (out14)
        : [in0] "r" (r0), [in1] "r" (r1), [in2] "r" (r2), [in3] "r" (r3),
          [in4] "r" (r4), [in5] "r" (r5), [in6] "r" (r6), [in7] "r" (r7),
          [in8] "r" (r8), [in9] "r" (r9),
          [din0] "x" (__builtin_bit_cast(uint64_t, darr[0])),
          [din1] "x" (__builtin_bit_cast(uint64_t, darr[1])),
          [addr0] "r" (&iarr[0]), [addr1] "r" (&iarr[1]),
          [addr2] "r" (&iarr[2]), [addr3] "r" (&iarr[3]),
          [mem0] "m" (iarr[4]), [mem1] "m" (iarr[5]),
          [mem2] "m" (iarr[6]), [mem3] "m" (iarr[7]),
          [mem4] "m" (iarr[8]), [mem5] "m" (iarr[9]),
          [mem6] "m" (iarr[10]), [mem7] "m" (iarr[11]),
          [mem8] "m" (iarr[12]), [mem9] "m" (iarr[13]),
          [cin0] "i" (100), [cin1] "i" (200),
          [cin2] "i" (300), [cin3] "i" (150)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10",
          "r11", "r12", "r13", "r14", "r15", "xmm0", "xmm1", "memory"
    );
    
    printf("Test 3: Multi-operand asm executed\n");
}

/* Function 4: __builtin_constant_p in address contexts */
void test_builtin_constant_p(void) {
    volatile int data[100];
    int i;
    
    for (i = 0; i < 100; i++) {
        data[i] = i * 7;
    }
    
    int offset = 50;
    int result1, result2;
    
    /* Using __builtin_constant_p to create different address reload scenarios */
    for (i = 0; i < 5; i++) {
        int idx = i * 10;
        
        /* Conditional address expression */
        void* addr = __builtin_constant_p(idx) ? 
                     (void*)&data[idx] : 
                     (void*)&data[offset + i];
        
        /* Inline asm using the address */
        asm volatile (
            "movl (%[addr]), %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %[result]\n\t"
            : [result] "=r" (result1)
            : [addr] "r" (addr)
            : "eax", "memory"
        );
        
        /* Another asm with address computation in constraint */
        asm volatile (
            "leal 4(%[base]), %%ebx\n\t"
            "movl (%%ebx), %%ecx\n\t"
            "movl %%ecx, %[out]\n\t"
            : [out] "=r" (result2)
            : [base] "r" (&data[i * 2])
            : "ebx", "ecx", "memory"
        );
    }
    
    printf("Test 4: __builtin_constant_p tests completed\n");
}

int main(void) {
    printf("Starting reload pass stress tests...\n");
    
    /* Run all test functions */
    test_complex_constraints();
    test_nested_addresses();
    test_multi_operand_asm();
    test_builtin_constant_p();
    
    printf("All tests completed successfully.\n");
    return 0;
}
