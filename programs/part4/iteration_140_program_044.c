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
        /* Multiple outputs with alternative constraints */
        "movl %[x], %[out1]\n\t"
        "addl %[y], %[out1]\n\t"
        "movl %[z], %[out2]\n\t"
        "subl %[x], %[out2]\n\t"
        "leal (%[mem1],%[x],4), %[out3]\n\t"
        : [out1] "=r,m" (result1),   /* Alternative: register or memory */
          [out2] "=r,m" (result2),
          [out3] "=r,m" (result3)
        : [x] "0,r" (x),             /* Tied to output, alternative constraint */
          [y] "r,m" (y),
          [z] "r,m" (z),
          [mem1] "m" (arr1[0])
        : "eax", "ebx", "ecx", "edx", "r10", "r11", "r12", "memory"
    );
    
    printf("Test 1 results: %d, %d, %d\n", result1, result2, result3);
}

/* Function 2: Nested address computations with volatile */
void test_nested_addresses(void) {
    volatile int volatile_arr[512];
    volatile long volatile_long[256];
    int indices[8] = {0, 64, 128, 192, 256, 320, 384, 448};
    
    /* Initialize volatile arrays */
    for (int i = 0; i < 512; i++) {
        volatile_arr[i] = i * 3;
    }
    for (int i = 0; i < 256; i++) {
        volatile_long[i] = i * 5;
    }
    
    int results[4];
    int* ptr_results = results;
    
    /* Multiple asm statements with complex address calculations */
    for (int idx = 0; idx < 4; idx++) {
        int complex_index = indices[idx] + (idx * 16);
        
        /* Nested address-of operations */
        asm volatile (
            "movl %[addr], %[out]\n\t"
            "addl $42, %[out]\n\t"
            : [out] "=r" (results[idx])
            : [addr] "r" (&volatile_arr[complex_index + 8])
            : "memory"
        );
        
        /* More complex: address of address calculation */
        long* addr_ptr;
        asm volatile (
            "leaq %[base], %[ptr]\n\t"
            "addq $16, %[ptr]\n\t"
            : [ptr] "=r" (addr_ptr)
            : [base] "m" (volatile_long[complex_index / 2])
            : "memory"
        );
        
        /* Use the computed pointer */
        asm volatile (
            "movq (%[ptr]), %%rax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=r" (results[idx])
            : [ptr] "r" (addr_ptr)
            : "rax", "memory"
        );
    }
    
    printf("Test 2 results:");
    for (int i = 0; i < 4; i++) {
        printf(" %d", results[i]);
    }
    printf("\n");
}

/* Function 3: Large multi-operand asm statement */
void test_multi_operand_asm(void) {
    /* Mix of different variable types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    long la = 1000, lb = 2000, lc = 3000;
    float fa = 1.5f, fb = 2.5f;
    double da = 3.14159;
    
    /* Convert floats to integers for use in integer asm */
    int fa_int = __builtin_bit_cast(int, fa);
    int fb_int = __builtin_bit_cast(int, fb);
    uint64_t da_int = __builtin_bit_cast(uint64_t, da);
    
    /* Register-bound variables */
    register int r1 asm("r13") = 100;
    register int r2 asm("r14") = 200;
    register int r3 asm("r15") = 300;
    
    /* Output variables */
    int out1, out2, out3, out4, out5;
    long out6, out7, out8;
    int out9, out10;
    uint64_t out11;
    
    /* Large asm statement with many operands */
    asm volatile (
        /* Multiple operations mixing all operands */
        "movl %[a], %[o1]\n\t"
        "addl %[b], %[o1]\n\t"
        "movl %[c], %[o2]\n\t"
        "imull %[d], %[o2]\n\t"
        "movl %[e], %[o3]\n\t"
        "addl %[r1], %[o3]\n\t"
        "movl %[r2], %[o4]\n\t"
        "subl %[r3], %[o4]\n\t"
        "movl %[fa], %[o5]\n\t"
        "xorl %[fb], %[o5]\n\t"
        "movq %[la], %[o6]\n\t"
        "addq %[lb], %[o6]\n\t"
        "movq %[lc], %[o7]\n\t"
        "subq %[la], %[o7]\n\t"
        "movq %[da], %[o11]\n\t"
        "shrq $32, %[o11]\n\t"
        "leal (%[o1],%[o2],2), %[o8]\n\t"
        "leal (%[o3],%[o4],4), %[o9]\n\t"
        "addl %[o8], %[o9]\n\t"
        "movl %[o9], %[o10]\n\t"
        : [o1] "=&r" (out1), [o2] "=&r" (out2), [o3] "=&r" (out3),
          [o4] "=&r" (out4), [o5] "=&r" (out5), [o6] "=&r" (out6),
          [o7] "=&r" (out7), [o8] "=&r" (out8), [o9] "=&r" (out9),
          [o10] "=&r" (out10), [o11] "=&r" (out11)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c), [d] "r" (d), [e] "r" (e),
          [r1] "r" (r1), [r2] "r" (r2), [r3] "r" (r3),
          [fa] "r" (fa_int), [fb] "r" (fb_int),
          [la] "r" (la), [lb] "r" (lb), [lc] "r" (lc),
          [da] "r" (da_int)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory", "cc"
    );
    
    printf("Test 3: out1=%d, out2=%d, out3=%d, out4=%d, out5=%d\n",
           out1, out2, out3, out4, out5);
    printf("       out6=%ld, out7=%ld, out8=%ld, out9=%d, out10=%d, out11=%lu\n",
           out6, out7, out8, out9, out10, (unsigned long)out11);
}

/* Function 4: __builtin_constant_p in address contexts */
void test_builtin_constant_p(void) {
    volatile int data[100];
    int indices[10];
    
    for (int i = 0; i < 100; i++) {
        data[i] = i * 7;
    }
    for (int i = 0; i < 10; i++) {
        indices[i] = i * 10;
    }
    
    int results[5];
    
    for (int i = 0; i < 5; i++) {
        int idx = indices[i];
        
        /* Use __builtin_constant_p to select address expression */
        if (__builtin_constant_p(i)) {
            /* Constant index - simpler address */
            asm volatile (
                "movl %[data], %[out]\n\t"
                : [out] "=r" (results[i])
                : [data] "m" (data[idx])
                : "memory"
            );
        } else {
            /* Non-constant - more complex address calculation */
            int* addr;
            asm volatile (
                "leaq %[base], %[addr]\n\t"
                "addq $4, %[addr]\n\t"
                : [addr] "=r" (addr)
                : [base] "r" (&data[idx])
                : "memory"
            );
            
            asm volatile (
                "movl (%[addr]), %[out]\n\t"
                : [out] "=r" (results[i])
                : [addr] "r" (addr)
                : "memory"
            );
        }
    }
    
    printf("Test 4 results:");
    for (int i = 0; i < 5; i++) {
        printf(" %d", results[i]);
    }
    printf("\n");
}

/* Main function that runs all tests */
int main(void) {
    printf("Starting reload pass coverage tests...\n");
    
    test_complex_constraints();
    test_nested_addresses();
    test_multi_operand_asm();
    test_builtin_constant_p();
    
    printf("All tests completed.\n");
    return 0;
}
