/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force compiler to generate reloads through complex inline assembly */
int main(void) {
    /* Volatile variables to prevent optimization and force spills */
    volatile int v1 = 0x12345678;
    volatile int v2 = 0x9ABCDEF0;
    volatile int v3 = 0x13579BDF;
    volatile int v4 = 0x2468ACE0;
    volatile int v5 = 0x11111111;
    volatile int v6 = 0x22222222;
    volatile int v7 = 0x33333333;
    volatile int v8 = 0x44444444;
    volatile float f1 = 3.14159f;
    volatile float f2 = 2.71828f;
    volatile double d1 = 1.41421356;
    volatile double d2 = 1.73205080;
    
    /* Non-volatile variables with different types */
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    long l1 = 0xDEADBEEF, l2 = 0xCAFEBABE;
    int i1, i2, i3, i4, i5, i6, i7, i8;
    float f3, f4;
    double d3, d4;
    
    /* Explicit register variables to pin values */
    register int r12_var asm("r12") = 0x55555555;
    register int r13_var asm("r13") = 0x66666666;
    register int r14_var asm("r14") = 0x77777777;
    register int r15_var asm("r15") = 0x88888888;
    
    /* Variables for address-taking */
    int addr_var1 = 0x11111111;
    int addr_var2 = 0x22222222;
    int *addr_ptr1 = &addr_var1;
    int *addr_ptr2 = &addr_var2;
    
    /* Complex arithmetic to create many live values */
    i1 = v1 + v2;
    i2 = v3 * v4;
    i3 = v5 ^ v6;
    i4 = v7 | v8;
    f3 = f1 * f2;
    d3 = d1 + d2;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int temp1 = i1 + r12_var;
        int temp2 = i2 + r13_var;
        
        /* Inline assembly with multiple outputs, early clobber, and memory operands */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out2]\n\t"
            "leal (%[mem1], %[in4]), %%ebx\n\t"
            "movl %%ebx, %[out3]"
            : [out1] "=&r" (i5),        /* Early clobber output */
              [out2] "=r" (i6),         /* Regular output */
              [out3] "=m" (addr_var1)   /* Memory output */
            : [in1] "r" (temp1),        /* Register input */
              [in2] "r" (temp2),        /* Register input */
              [in3] "rm" (v3),          /* Register or memory */
              [in4] "r" (v4),           /* Register input */
              [mem1] "m" (*addr_ptr1)   /* Memory input */
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Use clobbered explicit register variables */
        r12_var += i5;
        r13_var += i6;
        
        /* Force address computation */
        i7 = *addr_ptr1 + *addr_ptr2;
    }
    
    /* Block 2: More assembly with different data types */
block2:
    {
        /* Mixed type operations */
        s1 = (short)(v1 & 0xFFFF);
        c1 = (char)(v2 & 0xFF);
        
        /* Assembly with floating point and integer mix */
        asm volatile (
            "cvtsi2ssl %[int_in], %%xmm0\n\t"
            "addss %[float_in], %%xmm0\n\t"
            "cvtss2sd %%xmm0, %%xmm1\n\t"
            "addsd %[double_in], %%xmm1\n\t"
            "movq %%xmm1, %[double_out]\n\t"
            "cvttsd2si %%xmm1, %[int_out]"
            : [double_out] "=m" (d4),
              [int_out] "=r" (i8)
            : [int_in] "r" (i7),
              [float_in] "x" (f3),
              [double_in] "xm" (d3)
            : "xmm0", "xmm1", "memory", "cc"
        );
        
        /* Modify live variables */
        v1 = i8;
        f1 = (float)d4;
    }
    
    /* Block 3: Assembly that uses explicit register variables */
block3:
    {
        /* Force conflicts with pinned registers */
        asm volatile (
            "movl %[r12], %%eax\n\t"
            "xorl %[r13], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movl %[r14], %%ebx\n\t"
            "orl %[r15], %%ebx\n\t"
            "movl %%ebx, %[out2]"
            : [out1] "=rm" (l1),   /* Register or memory output */
              [out2] "=rm" (l2)    /* Another register/memory output */
            : [r12] "r" (r12_var),
              [r13] "r" (r13_var),
              [r14] "r" (r14_var),
              [r15] "r" (r15_var)
            : "rax", "rbx", "memory", "cc"
        );
        
        /* Create more live values */
        i1 = l1 ^ l2;
        i2 = r12_var & r13_var;
    }
    
    /* Block 4: Final complex assembly block */
block4:
    {
        int temp_array[4] = {i1, i2, i3, i4};
        
        /* Assembly with multiple memory references and constraints */
        asm volatile (
            "movl (%[ptr]), %%eax\n\t"
            "addl 4(%[ptr]), %%eax\n\t"
            "addl 8(%[ptr]), %%eax\n\t"
            "addl 12(%[ptr]), %%eax\n\t"
            "movl %%eax, (%[out_ptr])\n\t"
            "movl %%eax, %[reg_out]"
            : [reg_out] "=r" (i3),
              "=m" (*addr_ptr2)
            : [ptr] "r" (temp_array),
              [out_ptr] "r" (addr_ptr2)
            : "rax", "memory", "cc"
        );
        
        /* Use all variables to keep them live */
        v2 = i3 + c1 + s1;
        v3 = r14_var - r15_var;
        v4 = (int)(f1 * 100.0f);
        v5 = (int)(d4 * 10.0);
    }
    
    /* Create control flow with goto to extend live ranges */
    if (v1 > 0) {
        goto block2;
    }
    
    if (v2 < 0) {
        goto block3;
    }
    
    /* Final computation using all variables */
    volatile int checksum = 0;
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    checksum += i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8;
    checksum += c1 + c2 + s1 + s2;
    checksum += (int)l1 + (int)l2;
    checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    checksum += (int)d1 + (int)d2 + (int)d3 + (int)d4;
    checksum += r12_var + r13_var + r14_var + r15_var;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0;
}
