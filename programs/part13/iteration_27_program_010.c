/* Test program to trigger reload.cc uncovered block in push_reload() */
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
    
    /* Non-volatile variables with complex live ranges */
    int nv1 = 0x77777777;
    int nv2 = 0x88888888;
    int nv3 = 0x99999999;
    int nv4 = 0xAAAAAAAA;
    int nv5 = 0xBBBBBBBB;
    int nv6 = 0xCCCCCCCC;
    int nv7 = 0xDDDDDDDD;
    int nv8 = 0xEEEEEEEE;
    
    /* Mixed data types to trigger mode mismatches */
    char c1 = 'A';
    short s1 = 0x1234;
    long l1 = 0x123456789ABCDEF0L;
    float f1 = 3.14159f;
    double d1 = 2.718281828459045;
    
    /* Explicit register variables to pin specific registers */
    register int reg_var1 asm ("r12") = 0xF0F0F0F0;
    register int reg_var2 asm ("r13") = 0xE1E1E1E1;
    register int reg_var3 asm ("r14") = 0xD2D2D2D2;
    
    /* Variables for address-taking to create addressing mode conflicts */
    int addr_var1 = 0x11111111;
    int addr_var2 = 0x22222222;
    int *addr_ptr1 = &addr_var1;
    int *addr_ptr2 = &addr_var2;
    
    /* Complex arithmetic to create many live values */
    v1 = v1 + v2;
    v3 = v4 - v5;
    nv1 = nv2 * nv3;
    nv4 = nv5 / (nv6 + 1);
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int temp1, temp2, temp3;
        
        /* Inline assembly with multiple outputs, inputs, and clobbers */
        asm volatile (
            /* Output operands with earlyclobber to force separate registers */
            "movl %[in1], %[out1]\n\t"
            "addl %[in2], %[out1]\n\t"
            "movl %[out1], %[out2]\n\t"
            "imull %[in3], %[out2]\n\t"
            /* Memory operand to force addressing mode */
            "movl %[out2], %[mem1]\n\t"
            /* Clobber many registers to force spills */
            :
            [out1] "=&r" (temp1),      /* Earlyclobber register */
            [out2] "=&r" (temp2)       /* Another earlyclobber */
            :
            [in1] "r" (v1),            /* Input in register */
            [in2] "r" (v2),
            [in3] "r" (nv1),
            [mem1] "m" (v3)            /* Memory operand - conflicts with v3 usage */
            : 
            /* Long clobber list to force many reloads */
            "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
            "r8", "r9", "r10", "r11", "r15",
            "xmm0", "xmm1", "xmm2", "xmm3",
            "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15",
            "memory", "cc"
        );
        
        /* Use results to keep them live */
        nv7 = temp1 + temp2;
        v4 = v4 ^ temp1;
    }
    
    /* Modify variables to extend live ranges */
    v2 = v2 + c1;
    s1 = s1 * 2;
    l1 = l1 >> 4;
    f1 = f1 * 1.5f;
    d1 = d1 / 1.1;
    
    /* Block 2: More inline assembly with different constraints */
block2:
    {
        long temp_l;
        double temp_d;
        
        /* Mixed type inline assembly */
        asm volatile (
            /* Different sized operations */
            "mov %[in_l], %%rax\n\t"
            "shl $8, %%rax\n\t"
            "mov %%rax, %[out_l]\n\t"
            /* Floating point operation */
            "movsd %[in_d], %%xmm0\n\t"
            "addsd %%xmm0, %%xmm0\n\t"
            "movsd %%xmm0, %[out_d]\n\t"
            :
            [out_l] "=r" (temp_l),
            [out_d] "=x" (temp_d)
            :
            [in_l] "r" (l1),
            [in_d] "x" (d1)
            :
            "rax", "xmm0", "xmm1", "xmm2", "xmm3",
            "memory", "cc"
        );
        
        l1 = temp_l;
        d1 = temp_d;
    }
    
    /* Force register variable conflicts */
    {
        int conflict_var;
        
        /* Inline assembly that clobbers explicit register variables */
        asm volatile (
            "movl $0xDEADBEEF, %%r12\n\t"  /* Clobber reg_var1's register */
            "movl $0xCAFEBABE, %%r13\n\t"  /* Clobber reg_var2's register */
            "movl %%r12, %[out]\n\t"
            :
            [out] "=r" (conflict_var)
            :
            :
            "r12", "r13", "r14",           /* Explicit clobber of pinned registers */
            "memory", "cc"
        );
        
        /* Use the clobbered register variables - forces reload */
        reg_var1 = reg_var1 + 1;
        reg_var2 = reg_var2 - 1;
        nv8 = reg_var1 + reg_var2 + reg_var3;
    }
    
    /* Block 3: Addressing mode conflicts */
block3:
    {
        int temp_addr;
        
        /* Take address and use in memory constraint while also using variable */
        asm volatile (
            "movl (%[addr]), %%eax\n\t"
            "addl $0x100, %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            "movl %%eax, (%[addr])\n\t"  /* Store back - creates conflict */
            :
            [out] "=r" (temp_addr)
            :
            [addr] "r" (addr_ptr1)       /* Pointer in register */
            :
            "rax", "memory", "cc"
        );
        
        /* Use both the variable and its address */
        addr_var1 = addr_var1 + temp_addr;
        *addr_ptr1 = *addr_ptr1 * 2;
    }
    
    /* Block 4: Complex constraints with multiple memory operands */
block4:
    {
        int temp_mem1, temp_mem2;
        
        asm volatile (
            "movl %[mem_in1], %%eax\n\t"
            "movl %[mem_in2], %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %[mem_out1]\n\t"
            "movl %%eax, %[mem_out2]\n\t"
            :
            [mem_out1] "=m" (v5),        /* Output to memory */
            [mem_out2] "=m" (v6)
            :
            [mem_in1] "m" (v7),          /* Input from memory */
            [mem_in2] "m" (v8)
            :
            "rax", "rbx", "memory", "cc"
        );
    }
    
    /* Final computations using all variables */
    int checksum = 0;
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    checksum += nv1 + nv2 + nv3 + nv4 + nv5 + nv6 + nv7 + nv8;
    checksum += c1 + s1;
    checksum += (int)l1;
    checksum += (int)f1;
    checksum += (int)d1;
    checksum += reg_var1 + reg_var2 + reg_var3;
    checksum += addr_var1 + addr_var2;
    
    /* Volatile store to prevent dead code elimination */
    volatile int final_result = checksum;
    
    printf("Result: %d\n", final_result);
    
    /* Use goto to create complex control flow with live values */
    if (final_result > 0) {
        goto block1;
    } else if (final_result < 1000) {
        goto block2;
    } else {
        goto block3;
    }
    
    return 0;
}
