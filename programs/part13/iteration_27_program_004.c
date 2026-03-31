/* Test program to trigger reload.cc uncovered block in push_reload */
#include <stdio.h>
#include <stdint.h>

/* Force compiler to generate reloads by creating register pressure */
int main(void) {
    /* Volatile variables to prevent optimization */
    volatile int v1 = 0x12345678;
    volatile int v2 = 0x9ABCDEF0;
    volatile int v3 = 0x11111111;
    volatile int v4 = 0x22222222;
    volatile int v5 = 0x33333333;
    volatile int v6 = 0x44444444;
    volatile int v7 = 0x55555555;
    volatile int v8 = 0x66666666;
    
    /* Non-volatile variables that will need registers */
    int nv1 = 0x77777777;
    int nv2 = 0x88888888;
    int nv3 = 0x99999999;
    int nv4 = 0xAAAAAAAA;
    int nv5 = 0xBBBBBBBB;
    int nv6 = 0xCCCCCCCC;
    int nv7 = 0xDDDDDDDD;
    int nv8 = 0xEEEEEEEE;
    
    /* Explicit register variables to pin registers */
    register int r12_var asm ("r12") = 0xF0F0F0F0;
    register int r13_var asm ("r13") = 0xE1E1E1E1;
    register int r14_var asm ("r14") = 0xD2D2D2D2;
    register int r15_var asm ("r15") = 0xC3C3C3C3;
    
    /* Mixed data types to create mode mismatches */
    char c1 = 'A';
    short s1 = 0x1234;
    long l1 = 0x123456789ABCDEF0L;
    float f1 = 3.14159f;
    double d1 = 2.718281828459045;
    
    /* Variables for address-taking */
    int addr_var1 = 0xDEADBEEF;
    int addr_var2 = 0xCAFEBABE;
    int *addr_ptr1 = &addr_var1;
    int *addr_ptr2 = &addr_var2;
    
    /* Force computations to keep variables live */
    nv1 = v1 + v2;
    nv2 = v3 * v4;
    nv3 = nv1 ^ nv2;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int tmp1, tmp2, tmp3;
        
        /* Inline asm with multiple outputs, early clobber, and memory constraint */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out2]\n\t"
            "leal (%[mem], %[in4]), %%ecx\n\t"
            "movl %%ecx, %[out3]"
            : [out1] "=&r" (tmp1),  /* Early clobber - can't share reg with inputs */
              [out2] "=r" (tmp2),
              [out3] "=r" (tmp3)
            : [in1] "r" (nv1),
              [in2] "r" (nv2),
              [in3] "r" (nv3),
              [in4] "r" (nv4),
              [mem] "m" (*addr_ptr1)  /* Memory constraint forcing address reload */
            : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "memory", "cc"
        );
        
        nv4 = tmp1 + tmp2 + tmp3;
        v1 = nv4;  /* Keep volatile updated */
    }
    
    /* More computations to maintain live ranges */
    nv5 = r12_var + r13_var;
    nv6 = r14_var * r15_var;
    
    /* Block 2: More inline assembly with different constraints */
block2:
    {
        long long_result;
        int int_result;
        short short_result;
        
        /* Mixed mode inline assembly with different sized operands */
        asm volatile (
            "movswl %w[short_in], %%eax\n\t"  /* Sign extend short to long */
            "addl %[int_in], %%eax\n\t"
            "movl %%eax, %[int_out]\n\t"
            "cltq\n\t"                       /* Sign extend to 64-bit */
            "addq %[long_in], %%rax\n\t"
            "movq %%rax, %[long_out]\n\t"
            "movw %w[short_in], %w[short_out]"
            : [int_out] "=r" (int_result),
              [long_out] "=r" (long_result),
              [short_out] "=r" (short_result)
            : [short_in] "r" (s1),
              [int_in] "r" (nv5),
              [long_in] "r" (l1)
            : "rax", "rcx", "rdx", "memory", "cc"
        );
        
        l1 = long_result;
        nv7 = int_result;
        s1 = short_result;
    }
    
    /* Force address computation conflicts */
    int * volatile volatile_ptr = &addr_var2;
    
    /* Block 3: Inline assembly that clobbers explicit register variables */
block3:
    {
        int result1, result2;
        
        /* This asm clobbers r12-r15 which are pinned by our register variables */
        asm volatile (
            "movl $0x1234, %%r12d\n\t"
            "movl $0x5678, %%r13d\n\t"
            "addl %%r12d, %%r13d\n\t"
            "movl %%r13d, %[out1]\n\t"
            "movl $0x9ABC, %%r14d\n\t"
            "movl $0xDEF0, %%r15d\n\t"
            "xorl %%r14d, %%r15d\n\t"
            "movl %%r15d, %[out2]"
            : [out1] "=r" (result1),
              [out2] "=r" (result2)
            : /* no inputs */
            : "r12", "r13", "r14", "r15", "cc"
        );
        
        /* Force reload of original register variable values */
        r12_var = result1;
        r13_var = result2;
    }
    
    /* Block 4: Complex addressing mode with multiple memory constraints */
block4:
    {
        int sum1, sum2;
        
        /* Multiple memory operands with the same base address */
        asm volatile (
            "movl (%[ptr1]), %%eax\n\t"
            "addl 4(%[ptr1]), %%eax\n\t"
            "movl %%eax, %[sum1]\n\t"
            "movl (%[ptr2]), %%ebx\n\t"
            "subl 4(%[ptr2]), %%ebx\n\t"
            "movl %%ebx, %[sum2]"
            : [sum1] "=r" (sum1),
              [sum2] "=r" (sum2)
            : [ptr1] "r" (addr_ptr1),
              [ptr2] "r" (addr_ptr2)
            : "rax", "rbx", "memory", "cc"
        );
        
        nv8 = sum1 + sum2;
    }
    
    /* Floating point operations to add pressure */
    {
        volatile float fv1 = 1.234f;
        volatile float fv2 = 5.678f;
        volatile double dv1 = 9.012;
        volatile double dv2 = 3.456;
        
        f1 = fv1 * fv2;
        d1 = dv1 / dv2;
        
        /* Mixed float/int inline assembly */
        int float_as_int;
        asm volatile (
            "movd %[float_in], %%eax\n\t"
            "movl %%eax, %[int_out]"
            : [int_out] "=r" (float_as_int)
            : [float_in] "x" (f1)
            : "eax", "cc"
        );
        
        v2 = float_as_int;
    }
    
    /* Final computation using all live variables */
    volatile int checksum = 0;
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    checksum += nv1 + nv2 + nv3 + nv4 + nv5 + nv6 + nv7 + nv8;
    checksum += r12_var + r13_var + r14_var + r15_var;
    checksum += c1 + s1 + (int)l1 + (int)f1 + (int)d1;
    checksum += addr_var1 + addr_var2;
    
    printf("Checksum: %d\n", checksum);
    
    /* Use goto to create complex control flow with live values */
    if (checksum > 1000) {
        goto block1;
    } else if (checksum > 500) {
        goto block2;
    } else if (checksum > 250) {
        goto block3;
    } else {
        goto block4;
    }
    
    return 0;
}
