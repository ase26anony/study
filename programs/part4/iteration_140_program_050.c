#include <stdio.h>
#include <stdint.h>

/* Test 1: Complex inline assembly with multiple constraints and clobbers */
void test1_complex_constraints(void) {
    volatile int arr[256];
    int result1, result2, result3;
    int x = 42, y = 100, z = 255;
    
    /* Force multiple reload types with alternative constraints */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "addl %[y], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%[arr],%[idx],4), %%ebx\n\t"
        "movl (%%ebx), %%ecx\n\t"
        "addl %%ecx, %[out2]\n\t"
        "imull %[imm], %[out3]"
        : [out1] "=r,m" (result1), 
          [out2] "=r,m" (result2),
          [out3] "=r,m" (result3)
        : [x] "r,i,m" (x),
          [y] "r,i,m" (y),
          [arr] "r,m" (arr),
          [idx] "r,i" (z),
          [imm] "i,r" (8)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    printf("Test1 results: %d %d %d\n", result1, result2, result3);
}

/* Test 2: Nested address computations with volatile */
void test2_nested_addresses(void) {
    volatile int volatile_arr[256];
    volatile long volatile_long[128];
    int out1, out2;
    int idx = 64;
    
    /* Complex address calculation that may need RELOAD_FOR_INPUT_ADDRESS */
    asm volatile (
        "movl %[addr1], %%eax\n\t"
        "movl (%%eax), %%ebx\n\t"
        "movl %%ebx, %[out1]\n\t"
        "movl %[addr2], %%ecx\n\t"
        "movl (%%ecx), %%edx\n\t"
        "addl %%edx, %[out2]"
        : [out1] "=r" (out1),
          [out2] "=r" (out2)
        : [addr1] "r" (&volatile_arr[idx + 32]),  /* May trigger RELOAD_FOR_INPADDR_ADDRESS */
          [addr2] "r" (&volatile_long[idx / 2])   /* Complex address calculation */
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    printf("Test2 results: %d %d\n", out1, out2);
}

/* Test 3: Register variables with explicit binding */
void test3_register_variables(void) {
    volatile int arr[256];
    register int r10_var asm("r10") = 100;
    register int r11_var asm("r11") = 200;
    register int r12_var asm("r12") = 300;
    int out1, out2, out3;
    
    /* Force output address reloads with register-bound variables */
    asm volatile (
        "movl %[r10], %%eax\n\t"
        "addl %[r11], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%[arr],%[r12],4), %%ebx\n\t"
        "movl (%%ebx), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "movl %[r12], %[out3]"
        : [out1] "=m,r" (out1),    /* Alternative constraint for output */
          [out2] "=m,r" (out2),
          [out3] "=m,r" (out3)
        : [r10] "r" (r10_var),
          [r11] "r" (r11_var),
          [r12] "r" (r12_var),
          [arr] "r" (arr)
        : "eax", "ebx", "ecx", "memory"
    );
    
    printf("Test3 results: %d %d %d\n", out1, out2, out3);
}

/* Test 4: Large multi-operand asm statement */
void test4_multi_operand(void) {
    volatile int varr[256];
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int out1, out2, out3, out4, out5;
    int out6, out7, out8, out9, out10;
    
    /* 15 operands total - stresses operand reload handling */
    asm volatile (
        "movl %[a], %%eax\n\t"
        "addl %[b], %%eax\n\t"
        "movl %%eax, %[o1]\n\t"
        "movl %[c], %%ebx\n\t"
        "addl %[d], %%ebx\n\t"
        "movl %%ebx, %[o2]\n\t"
        "movl %[e], %%ecx\n\t"
        "addl %[f], %%ecx\n\t"
        "movl %%ecx, %[o3]\n\t"
        "movl %[g], %%edx\n\t"
        "addl %[h], %%edx\n\t"
        "movl %%edx, %[o4]\n\t"
        "movl %[i], %%esi\n\t"
        "addl %[j], %%esi\n\t"
        "movl %%esi, %[o5]\n\t"
        "leal (%[arr],%%eax,4), %%edi\n\t"
        "movl (%%edi), %%eax\n\t"
        "movl %%eax, %[o6]\n\t"
        "addl %%ebx, %[o7]\n\t"
        "addl %%ecx, %[o8]\n\t"
        "addl %%edx, %[o9]\n\t"
        "addl %%esi, %[o10]"
        : [o1] "=r,m" (out1),
          [o2] "=r,m" (out2),
          [o3] "=r,m" (out3),
          [o4] "=r,m" (out4),
          [o5] "=r,m" (out5),
          [o6] "=r,m" (out6),
          [o7] "=r,m" (out7),
          [o8] "=r,m" (out8),
          [o9] "=r,m" (out9),
          [o10] "=r,m" (out10)
        : [a] "r,i,m" (a),
          [b] "r,i,m" (b),
          [c] "r,i,m" (c),
          [d] "r,i,m" (d),
          [e] "r,i,m" (e),
          [f] "r,i,m" (f),
          [g] "r,i,m" (g),
          [h] "r,i,m" (h),
          [i] "r,i,m" (i),
          [j] "r,i,m" (j),
          [arr] "r,m" (varr)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    printf("Test4 results: %d %d %d %d %d\n", out1, out2, out3, out4, out5);
    printf("              %d %d %d %d %d\n", out6, out7, out8, out9, out10);
}

/* Test 5: __builtin_constant_p in address contexts */
void test5_builtin_constant(void) {
    volatile int arr[256];
    int idx = 128;
    int result;
    
    /* Dynamic address selection based on constantness */
    void* addr = __builtin_constant_p(idx) 
                 ? (void*)&arr[64]  /* Constant address */
                 : (void*)&arr[idx]; /* Non-constant address */
    
    /* Force RELOAD_FOR_OPERAND_ADDRESS */
    asm volatile (
        "movl (%[addr]), %%eax\n\t"
        "movl %%eax, %[result]"
        : [result] "=r" (result)
        : [addr] "r" (addr)
        : "eax", "memory"
    );
    
    printf("Test5 result: %d (addr: %p)\n", result, addr);
}

/* Test 6: Mixed float/integer via bitcast (for x86_64) */
void test6_mixed_types(void) {
    volatile float farr[256];
    float fval = 3.14159f;
    uint32_t ival;
    int result;
    
    /* Convert float to int via bitcast for use in asm */
    ival = __builtin_bit_cast(uint32_t, fval);
    
    /* Mixed type constraints */
    asm volatile (
        "movd %[ival], %%xmm0\n\t"
        "movd %%xmm0, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %[result]"
        : [result] "=r" (result)
        : [ival] "r" (ival)
        : "eax", "xmm0", "memory"
    );
    
    printf("Test6 result: %d (float bits: 0x%08x)\n", result, ival);
}

int main(void) {
    printf("Starting reload pass stress tests...\n");
    
    test1_complex_constraints();
    test2_nested_addresses();
    test3_register_variables();
    test4_multi_operand();
    test5_builtin_constant();
    test6_mixed_types();
    
    printf("All tests completed.\n");
    return 0;
}
