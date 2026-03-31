/* Test program to stress GCC's reload pass and cover specific reload types */
#include <stdio.h>
#include <stdint.h>

/* Function 1: Complex alternative constraints and hard register clobbers */
void test_complex_constraints(void) {
    volatile int arr1[256];
    volatile int arr2[256];
    int result1, result2, result3;
    int temp1 = 123, temp2 = 456, temp3 = 789;
    
    /* Register variables bound to specific hard registers */
    register int x asm("r10") = 1000;
    register int y asm("r11") = 2000;
    register int z asm("r12") = 3000;
    
    /* Complex asm with alternative constraints and multiple clobbers */
    asm volatile (
        "movl %[t1], %%eax\n\t"
        "addl %[t2], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%[x],%[t3]), %%ebx\n\t"
        "movl %%ebx, %[out2]\n\t"
        "movl %[arr1], %%ecx\n\t"
        "addl %[arr2], %%ecx\n\t"
        "movl %%ecx, %[out3]"
        : [out1] "=r,m" (result1),
          [out2] "=r,m" (result2),
          [out3] "=r,m" (result3)
        : [t1] "r,i,m" (temp1),
          [t2] "r,i,m" (temp2),
          [t3] "r,i,m" (temp3),
          [x] "r" (x),
          [arr1] "m" (arr1[10]),
          [arr2] "m" (arr2[20])
        : "eax", "ebx", "ecx", "r10", "r11", "r12", "memory"
    );
    
    printf("Test1 results: %d %d %d\n", result1, result2, result3);
}

/* Function 2: Nested address computations with volatile */
void test_nested_addresses(void) {
    volatile int volatile_arr[256];
    volatile long volatile_long[128];
    int index1 = 50, index2 = 75;
    intptr_t addr1, addr2, addr3;
    int result;
    
    /* Complex address calculations that may need reloads */
    asm volatile (
        "movq %[addr1], %%rax\n\t"
        "addq %[addr2], %%rax\n\t"
        "movq %%rax, %[result]\n\t"
        "movl (%%rax), %%ebx\n\t"
        "addl $1, %%ebx\n\t"
        "movl %%ebx, (%%rax)"
        : [result] "=r" (result)
        : [addr1] "r" (&volatile_arr[index1 + 10]),
          [addr2] "r" (&volatile_long[index2 / 2]),
          "m" (volatile_arr[0]),
          "m" (volatile_long[0])
        : "rax", "rbx", "rcx", "memory"
    );
    
    /* More complex: address of address computation */
    int* ptr1 = (int*)&volatile_arr[0];
    int* ptr2 = (int*)&volatile_arr[100];
    
    asm volatile (
        "subq %[ptr2], %[ptr1]\n\t"
        "movq %[ptr1], %[addr3]"
        : [addr3] "=r" (addr3),
          [ptr1] "+r" (ptr1)
        : [ptr2] "r" (ptr2)
        : "cc"
    );
    
    printf("Test2: addr diff = %ld, result = %d\n", addr3, result);
}

/* Function 3: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    volatile double darr[64];
    volatile float farr[128];
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int o1, o2, o3, o4, o5, o6, o7, o8, o9, o10;
    double d1 = 1.5, d2 = 2.5;
    float f1 = 3.14f, f2 = 2.71f;
    
    /* Convert floats/doubles to integers for use in integer asm */
    uint64_t d1_int = __builtin_bit_cast(uint64_t, d1);
    uint64_t d2_int = __builtin_bit_cast(uint64_t, d2);
    uint32_t f1_int = __builtin_bit_cast(uint32_t, f1);
    uint32_t f2_int = __builtin_bit_cast(uint32_t, f2);
    
    /* Register variables for additional pressure */
    register int r1 asm("r13") = 100;
    register int r2 asm("r14") = 200;
    register int r3 asm("r15") = 300;
    
    /* Large asm with many operands - forces many reloads */
    asm volatile (
        "movl %[i1], %%eax\n\t"
        "addl %[i2], %%eax\n\t"
        "movl %%eax, %[o1]\n\t"
        "movl %[i3], %%ebx\n\t"
        "imull %[i4], %%ebx\n\t"
        "movl %%ebx, %[o2]\n\t"
        "movq %[d1], %%rcx\n\t"
        "addq %[d2], %%rcx\n\t"
        "movq %%rcx, %[o3]\n\t"
        "movl %[f1], %%edx\n\t"
        "addl %[f2], %%edx\n\t"
        "movl %%edx, %[o4]\n\t"
        "leal (%[r1],%[i5]), %%esi\n\t"
        "movl %%esi, %[o5]\n\t"
        "movl %[arr1], %%edi\n\t"
        "addl %[arr2], %%edi\n\t"
        "movl %%edi, %[o6]\n\t"
        "movl %[r2], %%r8d\n\t"
        "subl %[r3], %%r8d\n\t"
        "movl %%r8d, %[o7]\n\t"
        "movl %[i1], %%r9d\n\t"
        "andl %[i2], %%r9d\n\t"
        "movl %%r9d, %[o8]\n\t"
        "movl %[i3], %%r10d\n\t"
        "orl %[i4], %%r10d\n\t"
        "movl %%r10d, %[o9]\n\t"
        "movl %[i5], %%r11d\n\t"
        "xorl $0xFF, %%r11d\n\t"
        "movl %%r11d, %[o10]"
        : [o1] "=r,m" (o1),
          [o2] "=r,m" (o2),
          [o3] "=r,m" (o3),
          [o4] "=r,m" (o4),
          [o5] "=r,m" (o5),
          [o6] "=r,m" (o6),
          [o7] "=r,m" (o7),
          [o8] "=r,m" (o8),
          [o9] "=r,m" (o9),
          [o10] "=r,m" (o10)
        : [i1] "r,i,m" (i1),
          [i2] "r,i,m" (i2),
          [i3] "r,i,m" (i3),
          [i4] "r,i,m" (i4),
          [i5] "r,i,m" (i5),
          [d1] "r,i,m" (d1_int),
          [d2] "r,i,m" (d2_int),
          [f1] "r,i,m" (f1_int),
          [f2] "r,i,m" (f2_int),
          [r1] "r" (r1),
          [r2] "r" (r2),
          [r3] "r" (r3),
          [arr1] "m" (darr[0]),
          [arr2] "m" (farr[0])
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r13", "r14", "r15", "cc", "memory"
    );
    
    printf("Test3: %d %d %ld %d %d %d %d %d %d %d\n", 
           o1, o2, (long)o3, o4, o5, o6, o7, o8, o9, o10);
}

/* Function 4: __builtin_constant_p in address contexts */
void test_builtin_constant_p(void) {
    volatile int varr[100];
    int idx = 42;
    int result1, result2;
    void* addr;
    
    /* Use __builtin_constant_p to choose between constant and non-constant */
    if (__builtin_constant_p(idx)) {
        addr = &varr[10];
    } else {
        addr = &varr[idx];
    }
    
    /* Force reload for operand address */
    asm volatile (
        "movl (%[addr]), %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %%eax, (%[addr])"
        : [out1] "=r" (result1)
        : [addr] "r" (addr)
        : "rax", "memory"
    );
    
    /* Another variation with output address reload */
    int output;
    asm volatile (
        "movl $999, %%eax\n\t"
        "movl %%eax, %[out2]"
        : [out2] "=m" (output)
        :
        : "rax"
    );
    
    printf("Test4: result1=%d, output=%d\n", result1, output);
}

/* Function 5: Mixed types and complex expressions */
void test_mixed_expressions(void) {
    volatile struct {
        int a;
        double b;
        char c[16];
    } s1, s2;
    
    int offset = 8;
    double result_d;
    int result_i;
    
    /* Complex expression involving structure addresses */
    asm volatile (
        "movq %[s1], %%rax\n\t"
        "addq %[off], %%rax\n\t"
        "movq (%%rax), %%xmm0\n\t"
        "movq %[s2], %%rbx\n\t"
        "addq %[off], %%rbx\n\t"
        "addsd (%%rbx), %%xmm0\n\t"
        "movq %%xmm0, %[outd]"
        : [outd] "=m" (result_d)
        : [s1] "r" (&s1),
          [s2] "r" (&s2),
          [off] "r" (offset)
        : "rax", "rbx", "xmm0", "memory"
    );
    
    /* Address computation that might need RELOAD_FOR_OTHER_ADDRESS */
    char* ptr = s1.c;
    asm volatile (
        "movb $65, (%[ptr],%[idx])"
        :
        : [ptr] "r" (ptr),
          [idx] "r" (5)
        : "memory"
    );
    
    printf("Test5: result_d=%f\n", result_d);
}

int main(void) {
    printf("Starting reload pass stress tests...\n");
    
    test_complex_constraints();
    test_nested_addresses();
    test_multi_operand_asm();
    test_builtin_constant_p();
    test_mixed_expressions();
    
    printf("All tests completed.\n");
    return 0;
}
