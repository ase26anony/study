/* Test program to trigger reload.cc uncovered block in push_reload */
#include <stdio.h>
#include <stdint.h>

/* Force many live values across complex control flow */
int main(void) {
    /* Volatile variables to prevent optimization and force spills */
    volatile int v1 = 0x12345678;
    volatile int v2 = 0x9ABCDEF0;
    volatile int v3 = 0x13579BDF;
    volatile int v4 = 0x2468ACE0;
    volatile double v5 = 3.141592653589793;
    volatile float v6 = 2.718281828459045f;
    volatile char v7 = 'X';
    volatile short v8 = 0x7FFF;
    volatile long v9 = 0x1122334455667788L;
    
    /* Non-volatile variables with complex live ranges */
    int nv1 = 0x11111111;
    int nv2 = 0x22222222;
    int nv3 = 0x33333333;
    int nv4 = 0x44444444;
    int nv5 = 0x55555555;
    int nv6 = 0x66666666;
    int nv7 = 0x77777777;
    int nv8 = 0x88888888;
    int nv9 = 0x99999999;
    int nv10 = 0xAAAAAAAA;
    
    /* Explicit register variables to pin values */
    register int reg1 asm ("r12") = 0xBBBBBBBB;
    register int reg2 asm ("r13") = 0xCCCCCCCC;
    register int reg3 asm ("r14") = 0xDDDDDDDD;
    
    /* Variables for address-taking */
    int addr_var1 = 0x11111111;
    int addr_var2 = 0x22222222;
    int *addr_ptr1 = &addr_var1;
    int *addr_ptr2 = &addr_var2;
    
    /* Complex arithmetic to create many live values */
    nv1 = v1 + nv1;
    nv2 = v2 * nv2;
    nv3 = nv1 ^ nv2;
    nv4 = nv3 | reg1;
    
    /* Block 1: Inline assembly with conflicting constraints */
block1:
    {
        int tmp1 = nv1;
        int tmp2 = nv2;
        int tmp3 = nv3;
        
        /* Complex inline assembly with many clobbers and constraints */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out2]\n\t"
            : [out1] "=r" (tmp1), [out2] "=r" (tmp2)
            : [in1] "r" (tmp1), [in2] "r" (tmp2), [in3] "rm" (tmp3)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        nv1 = tmp1;
        nv2 = tmp2;
        v1 = nv1 + v1;
    }
    
    /* Modify variables to keep them live */
    nv5 = nv1 + nv2;
    nv6 = nv3 * reg2;
    
    /* Block 2: More inline assembly with different constraints */
block2:
    {
        double dtmp = v5;
        float ftmp = v6;
        short stmp = v8;
        
        /* Mixed data types and addressing modes */
        asm volatile (
            "cvtsi2ssl %[short_in], %%xmm0\n\t"
            "cvtss2sd %%xmm0, %%xmm1\n\t"
            "addsd %[double_in], %%xmm1\n\t"
            "movsd %%xmm1, %[double_out]\n\t"
            : [double_out] "=m" (dtmp)
            : [short_in] "r" (stmp), [double_in] "xm" (dtmp)
            : "xmm0", "xmm1", "xmm2", "xmm3", "memory", "cc"
        );
        
        v5 = dtmp;
        v6 = ftmp + 1.0f;
    }
    
    /* Force address conflicts */
    int *forced_addr = &nv7;
    int addr_val;
    
    /* Block 3: Address conflicts and explicit register clobbering */
block3:
    {
        /* Use explicit register variable in assembly that clobbers it */
        asm volatile (
            "movl $0xAAAAAAAA, %%r12d\n\t"  /* Clobber reg1's register */
            "movl %[addr_val], %%ebx\n\t"
            "addl %%r12d, %%ebx\n\t"
            "movl %%ebx, %[result]\n\t"
            : [result] "=r" (addr_val)
            : [addr_val] "m" (*forced_addr)
            : "rbx", "r12", "memory", "cc"
        );
        
        nv7 = addr_val;
        reg1 = nv7;  /* Restore register variable */
    }
    
    /* Block 4: Complex constraints with earlyclobber */
block4:
    {
        int in1 = nv8;
        int in2 = nv9;
        int out1, out2;
        
        /* Earlyclobber constraint forces different register */
        asm volatile (
            "leal (%[a], %[b]), %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "imull $2, %%eax, %%ebx\n\t"
            "movl %%ebx, %[out2]\n\t"
            : [out1] "=&r" (out1), [out2] "=r" (out2)
            : [a] "r" (in1), [b] "r" (in2)
            : "rax", "rbx", "rcx", "memory", "cc"
        );
        
        nv8 = out1;
        nv9 = out2;
    }
    
    /* Block 5: Multiple output constraints with memory operands */
block5:
    {
        long ltmp = v9;
        int itmp1 = nv4;
        int itmp2 = nv5;
        
        asm volatile (
            "movl %[in1], %%ecx\n\t"
            "addl %[in2], %%ecx\n\t"
            "movl %%ecx, %[out1]\n\t"
            "addq %%rcx, %[out2]\n\t"
            : [out1] "=r" (itmp1), [out2] "+m" (ltmp)
            : [in1] "r" (itmp1), [in2] "r" (itmp2)
            : "rcx", "rdx", "memory", "cc"
        );
        
        nv4 = itmp1;
        v9 = ltmp;
    }
    
    /* Create control flow with many live values */
    if (nv1 > 0) {
        goto block2;
    }
    
    /* Final block: Use all variables to prevent elimination */
    volatile int checksum = 0;
    checksum += v1 + v2 + v3 + v4;
    checksum += (int)v5 + (int)v6 + v7 + v8;
    checksum += (int)(v9 & 0xFFFFFFFF);
    checksum += nv1 + nv2 + nv3 + nv4 + nv5;
    checksum += nv6 + nv7 + nv8 + nv9 + nv10;
    checksum += reg1 + reg2 + reg3;
    checksum += *addr_ptr1 + *addr_ptr2;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
