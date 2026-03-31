/* Test program to trigger reload.cc uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force many live values across complex control flow */
int main(void) {
    /* Volatile variables to prevent optimization and force spills */
    volatile int v1 = 0x12345678;
    volatile int v2 = 0x9ABCDEF0;
    volatile int v3 = 0x11111111;
    volatile int v4 = 0x22222222;
    volatile int v5 = 0x33333333;
    volatile int v6 = 0x44444444;
    volatile int v7 = 0x55555555;
    volatile int v8 = 0x66666666;
    
    /* Floating point variables to increase register pressure */
    volatile double f1 = 3.14159;
    volatile double f2 = 2.71828;
    volatile float f3 = 1.41421f;
    volatile float f4 = 1.73205f;
    
    /* Different sized variables for mode mismatches */
    volatile char c1 = 'A';
    volatile short s1 = 0x1234;
    volatile long long ll1 = 0x1122334455667788ULL;
    
    /* Explicit register variables to pin registers */
    register int reg_var1 asm ("r12") = 0xAAAA5555;
    register int reg_var2 asm ("r13") = 0xBBBB6666;
    register int reg_var3 asm ("r14") = 0xCCCC7777;
    
    /* Non-volatile variables for arithmetic */
    int nv1 = 100, nv2 = 200, nv3 = 300, nv4 = 400;
    double nvf1 = 1.1, nvf2 = 2.2, nvf3 = 3.3;
    
    /* Variables for address taking */
    int addr_var1 = 999, addr_var2 = 888;
    int *addr_ptr1 = &addr_var1;
    int *addr_ptr2 = &addr_var2;
    
    /* Complex arithmetic to create many live values */
    nv1 = v1 + v2;
    nv2 = v3 * v4;
    nv3 = nv1 ^ nv2;
    nvf1 = f1 * f2;
    nvf2 = f3 + f4;
    
    /* Block 1: Inline assembly with conflicting constraints */
block1:
    {
        int tmp1, tmp2, tmp3;
        /* Complex asm with multiple outputs, inputs, and clobbers */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out2]\n\t"
            "leal (%[in4],%[in5],2), %%ebx\n\t"
            "movl %%ebx, %[out3]"
            : [out1] "=&r" (tmp1),  /* Early clobber to force separate reg */
              [out2] "=r" (tmp2),
              [out3] "=&r" (tmp3)   /* Another early clobber */
            : [in1] "r" (nv1),
              [in2] "r" (nv2),
              [in3] "r" (v5),
              [in4] "r" (v6),
              [in5] "r" (v7)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Use results to keep them live */
        v1 = tmp1 + tmp2 + tmp3;
        nv4 = tmp1 ^ tmp2;
    }
    
    /* Modify variables to keep them live across blocks */
    v2 += v1;
    f1 += 0.1;
    reg_var1 ^= 0xFF00FF00;
    
    /* Block 2: More inline assembly with memory constraints */
block2:
    {
        int mem_tmp;
        /* Mix memory and register constraints */
        asm volatile (
            "movl %[mem_in], %%ecx\n\t"
            "addl $0x10, %%ecx\n\t"
            "movl %%ecx, (%[mem_out])\n\t"
            "movl %%ecx, %[reg_out]"
            : [reg_out] "=r" (mem_tmp)
            : [mem_in] "m" (addr_var1),  /* Memory constraint */
              [mem_out] "r" (addr_ptr2)   /* Register constraint */
            : "rcx", "memory"
        );
        
        /* Force address computation */
        addr_var2 = mem_tmp + (int)addr_ptr1;
    }
    
    /* More arithmetic to keep values live */
    nvf3 = f1 * f2 + f3 - f4;
    s1 = (short)(v3 & 0xFFFF);
    c1 = (char)(v4 & 0xFF);
    
    /* Block 3: Assembly with explicit clobber of pinned registers */
block3:
    {
        long long result;
        /* This asm clobbers registers we've pinned variables to */
        asm volatile (
            "movq %[in_ll], %%rax\n\t"
            "rorq $32, %%rax\n\t"
            "addq %%r12, %%rax\n\t"  /* Use r12 (our reg_var1) */
            "subq %%r13, %%rax\n\t"  /* Use r13 (our reg_var2) */
            "movq %%rax, %[out]"
            : [out] "=r" (result)
            : [in_ll] "r" (ll1)
            : "rax", "r12", "r13", "r14", "cc"
        );
        
        ll1 = result;
        /* Force reload of our register variables */
        reg_var1 = reg_var1 + 1;
        reg_var2 = reg_var2 - 1;
        reg_var3 = reg_var3 ^ 0x5555;
    }
    
    /* Block 4: Mixed data types and modes */
block4:
    {
        char char_out;
        short short_out;
        int int_out;
        
        /* Different sized outputs from same inputs */
        asm volatile (
            "movzbl %[c_in], %%eax\n\t"
            "addw %[s_in], %%ax\n\t"
            "addl %[i_in], %%eax\n\t"
            "movb %%al, %[c_out]\n\t"
            "movw %%ax, %[s_out]\n\t"
            "movl %%eax, %[i_out]"
            : [c_out] "=r" (char_out),
              [s_out] "=r" (short_out),
              [i_out] "=r" (int_out)
            : [c_in] "r" (c1),
              [s_in] "r" (s1),
              [i_in] "r" (v8)
            : "rax", "cc"
        );
        
        v5 = char_out + short_out + int_out;
    }
    
    /* Final computation using all variables */
    volatile int checksum = 0;
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    checksum += c1 + s1 + (int)(ll1 & 0xFFFFFFFF);
    checksum += reg_var1 + reg_var2 + reg_var3;
    checksum += nv1 + nv2 + nv3 + nv4;
    checksum += (int)nvf1 + (int)nvf2 + (int)nvf3;
    checksum += addr_var1 + addr_var2;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0;
}
