/* Test program to trigger reload.cc push_reload block */
#include <stdio.h>

/* Force many live values and complex register allocation */
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
    
    /* Floating point volatiles for mode mixing */
    volatile double f1 = 3.14159;
    volatile double f2 = 2.71828;
    volatile float f3 = 1.41421f;
    volatile float f4 = 1.73205f;
    
    /* Non-volatile variables that will be live across blocks */
    int nv1 = 0xAAAAAAAA;
    int nv2 = 0xBBBBBBBB;
    int nv3 = 0xCCCCCCCC;
    int nv4 = 0xDDDDDDDD;
    int nv5 = 0xEEEEEEEE;
    
    /* Explicit register variables to pin registers */
    register int regvar1 asm ("r12") = 0x11112222;
    register int regvar2 asm ("r13") = 0x33334444;
    register int regvar3 asm ("r14") = 0x55556666;
    
    /* Variables for address-taking */
    int addr_var1 = 0x77777777;
    int addr_var2 = 0x88888888;
    int *addr_ptr1 = &addr_var1;
    int *addr_ptr2 = &addr_var2;
    
    /* Mixed size variables */
    char c1 = 'A';
    short s1 = 0x1234;
    long long ll1 = 0x1122334455667788LL;
    
    /* Force many live values before first asm */
    v1 = v2 + v3;
    v4 = v5 * v6;
    f1 = f2 * 2.0;
    f3 = f4 / 2.0f;
    
    /* Start of complex control flow with live values */
    block1:
    {
        /* Inline asm with conflicting constraints */
        /* Input in memory, output in register, clobbers many regs */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (nv1)          /* Output in any register */
            : "m" (v1)            /* Input from memory */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
        );
        
        /* Use the explicit register variables */
        regvar1 = regvar1 * 2;
        regvar2 = regvar2 + nv1;
        
        /* Force spill by using many variables */
        v2 = v3 + v4 + v5 + v6 + v7 + v8;
    }
    
    /* Modify variables to keep them live */
    nv2 = nv1 + 1;
    nv3 = nv2 * 2;
    
    goto block2;
    
    /* Unreachable code to create more live ranges */
    v1 = v2;  /* Keep compiler guessing about live ranges */
    
    block2:
    {
        /* Another asm with address constraint conflicts */
        /* Use both the address and value of variables */
        int temp1, temp2;
        
        asm volatile (
            "movq %[addr], %%rax\n\t"
            "movl (%%rax), %%ebx\n\t"
            "addl %%ebx, %[val1]\n\t"
            "movl %[val1], %[out1]\n\t"
            "movl %[val2], %[out2]\n\t"
            : [out1] "=r" (temp1), [out2] "=r" (temp2)
            : [addr] "m" (addr_ptr1), [val1] "0" (addr_var1), 
              [val2] "r" (addr_var2)
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        addr_var1 = temp1;
        addr_var2 = temp2;
        
        /* Clobber the explicit register variables */
        asm volatile (
            "xor %%r12, %%r12\n\t"
            "xor %%r13, %%r13\n\t"
            : : : "r12", "r13"
        );
        
        /* Force reload of regvar1 and regvar2 */
        regvar1 = 0x1000;
        regvar2 = 0x2000;
    }
    
    /* More variable modifications */
    nv4 = nv3 + regvar1;
    nv5 = nv4 + regvar2;
    
    goto block3;
    
    block3:
    {
        /* Asm with mixed data types and modes */
        /* char, short, int, long long in same asm */
        long long result;
        int int_result;
        short short_result;
        
        asm volatile (
            "movsbl %[cval], %%eax\n\t"
            "movswl %[sval], %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movslq %%eax, %%rax\n\t"
            "addq %[llval], %%rax\n\t"
            "movq %%rax, %[llout]\n\t"
            "movl %%eax, %[intout]\n\t"
            "movw %%ax, %[shortout]\n\t"
            : [llout] "=r" (result), [intout] "=r" (int_result),
              [shortout] "=r" (short_result)
            : [cval] "m" (c1), [sval] "m" (s1), [llval] "r" (ll1)
            : "rax", "rbx", "rcx", "memory"
        );
        
        ll1 = result;
        nv1 = int_result;
        s1 = short_result;
        
        /* Complex floating point asm with many clobbers */
        double dbl_result;
        asm volatile (
            "movsd %[fin1], %%xmm0\n\t"
            "movsd %[fin2], %%xmm1\n\t"
            "addsd %%xmm1, %%xmm0\n\t"
            "mulsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %[out]\n\t"
            : [out] "=m" (dbl_result)
            : [fin1] "m" (f1), [fin2] "m" (f2)
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory"
        );
        
        f1 = dbl_result;
    }
    
    /* Final computations using all variables */
    volatile int checksum = 0;
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    checksum += nv1 + nv2 + nv3 + nv4 + nv5;
    checksum += regvar1 + regvar2 + regvar3;
    checksum += addr_var1 + addr_var2;
    checksum += c1 + s1 + (int)ll1;
    checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0;
}
