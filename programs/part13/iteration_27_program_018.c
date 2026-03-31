/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdlib.h>

/* Force specific register usage */
register long reg_var1 asm ("r12");
register long reg_var2 asm ("r13");
register long reg_var3 asm ("r14");

int main(void) {
    /* Volatile variables to prevent optimization */
    volatile int v1 = 0x12345678;
    volatile int v2 = 0x9ABCDEF0;
    volatile int v3 = 0x13579BDF;
    volatile int v4 = 0x2468ACE0;
    volatile long v5 = 0x1122334455667788LL;
    volatile long v6 = 0x99AABBCCDDEEFF00LL;
    volatile float f1 = 3.14159f;
    volatile float f2 = 2.71828f;
    volatile double d1 = 1.41421356;
    volatile double d2 = 1.73205080;
    
    /* Non-volatile variables with different types */
    char c1 = 'A', c2 = 'B', c3 = 'C';
    short s1 = 1000, s2 = 2000, s3 = 3000;
    int i1 = 100000, i2 = 200000, i3 = 300000;
    long l1 = 4000000000L, l2 = 5000000000L, l3 = 6000000000L;
    
    /* Initialize register variables */
    reg_var1 = 0x1111111111111111LL;
    reg_var2 = 0x2222222222222222LL;
    reg_var3 = 0x3333333333333333LL;
    
    /* Force many live values */
    int result1, result2, result3, result4;
    long long_result1, long_result2;
    double double_result;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int temp1 = v1 + v2;
        int temp2 = v3 + v4;
        
        /* Inline assembly with multiple constraints that conflict */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out2]\n\t"
            : [out1] "=r" (result1), [out2] "=r" (result2)
            : [in1] "rm" (temp1), [in2] "rm" (temp2), [in3] "rm" (v1)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Use results to keep them live */
        v1 = result1 + result2;
        goto block2;
    }
    
    /* Unreachable but creates control flow complexity */
    if (0) {
block2:
        /* More complex assembly with memory addressing conflicts */
        long addr_var = (long)&v5;
        
        asm volatile (
            "movq %[addr], %%rbx\n\t"
            "movq (%%rbx), %%rax\n\t"
            "addq %[val], %%rax\n\t"
            "movq %%rax, %[out]\n\t"
            "movq %%rax, (%%rbx)\n\t"
            : [out] "=&r" (long_result1)
            : [addr] "r" (addr_var), [val] "rm" (v6)
            : "rax", "rbx", "rcx", "memory"
        );
        
        /* Mixed type operations */
        double dtemp = d1 + d2;
        float ftemp = f1 * f2;
        
        /* Assembly with different operand sizes */
        asm volatile (
            "mov %w[c1], %%ax\n\t"
            "add %w[c2], %%ax\n\t"
            "movsx %%ax, %0\n\t"
            : "=r" (result3)
            : [c1] "r" ((short)c1), [c2] "r" ((short)c2)
            : "rax", "cc"
        );
        
        goto block3;
    }
    
block3:
    {
        /* Force register variable conflicts */
        long reg_temp = reg_var1;
        
        /* Assembly that clobbers pinned registers */
        asm volatile (
            "movq %[in1], %%r12\n\t"  /* Clobber reg_var1's register */
            "addq %[in2], %%r12\n\t"
            "movq %%r12, %[out]\n\t"
            "xorq %%rcx, %%rcx\n\t"
            "movq %%rcx, %%r13\n\t"   /* Clobber reg_var2's register */
            : [out] "=r" (long_result2)
            : [in1] "rm" (reg_temp), [in2] "rm" (reg_var2)
            : "r12", "r13", "rcx", "cc"
        );
        
        /* Restore register variables */
        reg_var1 = long_result2;
        reg_var2 = 0x2222222222222222LL;
        
        goto block4;
    }
    
block4:
    {
        /* Complex addressing with multiple memory operands */
        int* ptr1 = &v1;
        int* ptr2 = &v2;
        
        asm volatile (
            "movl (%[p1]), %%eax\n\t"
            "addl (%[p2]), %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movl %%eax, (%[p1])\n\t"
            "movl $0x1234, (%[p2])\n\t"
            : [out1] "=r" (result4), [p1] "+r" (ptr1), [p2] "+r" (ptr2)
            :
            : "rax", "memory", "cc"
        );
        
        /* Floating point with integer conversion */
        double_result = (double)result4 + d1;
        
        /* Final assembly with many clobbers */
        asm volatile (
            "movq %[dbl], %%xmm0\n\t"
            "cvttsd2si %%xmm0, %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=rm" (i3)
            : [dbl] "xm" (double_result)
            : "xmm0", "rax", "cc"
        );
    }
    
    /* Final computation using all variables to prevent elimination */
    volatile long checksum = 0;
    checksum += v1 + v2 + v3 + v4;
    checksum += v5 + v6;
    checksum += (long)(f1 * 1000) + (long)(f2 * 1000);
    checksum += (long)(d1 * 1000) + (long)(d2 * 1000);
    checksum += c1 + c2 + c3;
    checksum += s1 + s2 + s3;
    checksum += i1 + i2 + i3 + result1 + result2 + result3 + result4;
    checksum += l1 + l2 + l3;
    checksum += long_result1 + long_result2;
    checksum += reg_var1 + reg_var2 + reg_var3;
    
    printf("Checksum: %ld\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
