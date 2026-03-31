#include <stdio.h>
#include <stdint.h>

/* Test 1: Complex inline assembly with multiple constraints and clobbers */
void test_complex_constraints(void) {
    volatile int arr1[256];
    volatile int arr2[256];
    int result1, result2, result3;
    
    /* Force register pressure with many clobbers */
    asm volatile (
        "movl $1, %[res1]\n\t"
        "addl $2, %[res1]\n\t"
        "movl %[res1], %[res2]\n\t"
        "imull $3, %[res2]\n\t"
        "movl %[res2], %[res3]\n\t"
        : [res1] "=r,m" (result1),
          [res2] "=r,m" (result2),
          [res3] "=r,m" (result3)
        : 
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory"
    );
    
    /* Alternative constraints on same operand */
    int temp = 42;
    asm volatile (
        "addl $1, %[t]\n\t"
        : [t] "+r,m" (temp)
        : 
        : "cc"
    );
    
    printf("Test1 results: %d %d %d %d\n", result1, result2, result3, temp);
}

/* Test 2: Nested address computations with volatile */
void test_nested_addresses(void) {
    volatile int volatile_arr[256];
    volatile long volatile_arr2[128];
    int index = 100;
    int *addr1, *addr2;
    long *addr3;
    
    /* Complex address computation that may need reloads */
    asm volatile (
        "leaq %[arr1][%[idx1]], %[a1]\n\t"
        "leaq %[arr2][%[idx2]], %[a2]\n\t"
        : [a1] "=r" (addr1),
          [a2] "=r" (addr2)
        : [arr1] "m" (volatile_arr),
          [idx1] "r" ((long)index * sizeof(int)),
          [arr2] "m" (volatile_arr),
          [idx2] "r" ((long)(index + 50) * sizeof(int))
        : "memory"
    );
    
    /* Address of volatile array element with offset */
    asm volatile (
        ""
        : "=r" (addr3)
        : "0" (&volatile_arr2[index + 25] + 10)
        : "memory"
    );
    
    /* Use __builtin_constant_p to create conditional address reloads */
    int offset = __builtin_constant_p(index) ? 0 : 10;
    asm volatile (
        "movq %[addr], %%rax\n\t"
        :
        : [addr] "r" (&volatile_arr[index + offset])
        : "rax", "memory"
    );
    
    printf("Test2 addresses: %p %p %p\n", (void*)addr1, (void*)addr2, (void*)addr3);
}

/* Test 3: Register variables with explicit binding */
void test_register_variables(void) {
    /* Bind to specific registers */
    register int r10_var asm("r10") = 100;
    register int r11_var asm("r11") = 200;
    register int r12_var asm("r12") = 300;
    volatile int mem_buffer[64];
    int output1, output2;
    
    /* Force spilling of fixed registers */
    asm volatile (
        "movl %[r10], %[out1]\n\t"
        "addl %[r11], %[out1]\n\t"
        "movl %[out1], %[mem]\n\t"
        "movl %[r12], %[out2]\n\t"
        "subl %[mem], %[out2]\n\t"
        : [out1] "=r,m" (output1),
          [out2] "=r,m" (output2),
          [mem] "=m" (mem_buffer[0])
        : [r10] "r" (r10_var),
          [r11] "r" (r11_var),
          [r12] "r" (r12_var)
        : "memory"
    );
    
    /* Complex output address reload */
    int *ptr = &mem_buffer[32];
    asm volatile (
        "movl $999, (%[ptr])\n\t"
        : 
        : [ptr] "r,m" (ptr)
        : "memory"
    );
    
    printf("Test3 results: %d %d mem[32]=%d\n", output1, output2, mem_buffer[32]);
}

/* Test 4: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int o1, o2, o3, o4, o5, o6, o7, o8, o9, o10;
    float f1 = 1.5f;
    uint32_t f1_as_int = __builtin_bit_cast(uint32_t, f1);
    
    /* 15 operands mixing types and constraints */
    asm volatile (
        "movl %[i1], %[o1]\n\t"
        "addl %[i2], %[o1]\n\t"
        "movl %[o1], %[o2]\n\t"
        "imull %[i3], %[o2]\n\t"
        "movl %[o2], %[o3]\n\t"
        "addl %[i4], %[o3]\n\t"
        "movl %[o3], %[o4]\n\t"
        "subl %[i5], %[o4]\n\t"
        "movl %[o4], %[o5]\n\t"
        "andl $0xFF, %[o5]\n\t"
        "movl %[o5], %[o6]\n\t"
        "orl %[float_int], %[o6]\n\t"
        "movl %[o6], %[o7]\n\t"
        "shrl $2, %[o7]\n\t"
        "movl %[o7], %[o8]\n\t"
        "addl $100, %[o8]\n\t"
        "movl %[o8], %[o9]\n\t"
        "xorl $0x55, %[o9]\n\t"
        "movl %[o9], %[o10]\n\t"
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
        : [i1] "r,m" (v1),
          [i2] "r,m" (v2),
          [i3] "r,m" (v3),
          [i4] "r,m" (v4),
          [i5] "r,m" (v5),
          [float_int] "r,m" (f1_as_int)
        : "cc", "memory"
    );
    
    printf("Test4 results: %d %d %d %d %d %d %d %d %d %d\n", 
           o1, o2, o3, o4, o5, o6, o7, o8, o9, o10);
}

/* Test 5: Address reloads with complex indexing */
void test_address_reloads(void) {
    volatile struct {
        int a[32];
        long b[16];
        char c[64];
    } big_struct;
    
    int idx1 = 5, idx2 = 10, idx3 = 15;
    int *p1, *p2;
    long *p3;
    char *p4;
    
    /* Multiple address calculations that may need different reload types */
    asm volatile (
        "leaq %[struct], %%rax\n\t"
        "leaq 0(%%rax, %[idx1], 4), %[ptr1]\n\t"  /* a[idx1] */
        "leaq 128(%%rax, %[idx2], 8), %[ptr2]\n\t" /* b[idx2] */
        "leaq 256(%%rax, %[idx3], 1), %[ptr3]\n\t" /* c[idx3] */
        : [ptr1] "=r" (p1),
          [ptr2] "=r" (p2),
          [ptr3] "=r" (p3)
        : [struct] "m" (big_struct),
          [idx1] "r" ((long)idx1),
          [idx2] "r" ((long)idx2),
          [idx3] "r" ((long)idx3)
        : "rax", "memory"
    );
    
    /* Address of address computation */
    asm volatile (
        "leaq (%[ptr1], %[idx2], 4), %[ptr4]\n\t"
        : [ptr4] "=r" (p4)
        : [ptr1] "r" (p1),
          [idx2] "r" ((long)idx2)
        : "memory"
    );
    
    printf("Test5 addresses: %p %p %p %p\n", 
           (void*)p1, (void*)p2, (void*)p3, (void*)p4);
}

int main(void) {
    printf("Starting reload stress tests...\n");
    
    test_complex_constraints();
    test_nested_addresses();
    test_register_variables();
    test_multi_operand_asm();
    test_address_reloads();
    
    printf("All tests completed.\n");
    return 0;
}
