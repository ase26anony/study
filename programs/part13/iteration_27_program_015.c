/* Test program to trigger GCC reload pass uncovered lines */
#include <stdio.h>
#include <stdint.h>

/* Force the compiler to generate reloads by creating register pressure,
   conflicting constraints, and complex live ranges. */

int main(void) {
    /* Volatile variables to prevent optimization and force memory operations */
    volatile int v1 = 0x12345678;
    volatile int v2 = 0x9ABCDEF0;
    volatile int v3 = 0x11111111;
    volatile int v4 = 0x22222222;
    volatile int v5 = 0x33333333;
    volatile int v6 = 0x44444444;
    volatile int v7 = 0x55555555;
    volatile int v8 = 0x66666666;
    
    /* Non-volatile variables with different data types */
    char c1 = 'A';
    short s1 = 0x1234;
    int i1 = 0x56789ABC;
    long l1 = 0xFEDCBA9876543210L;
    float f1 = 3.14159f;
    double d1 = 2.718281828459045;
    
    /* Explicit register variables to pin values to specific registers */
    register int reg_var1 asm ("r12") = 0x11111111;
    register int reg_var2 asm ("r13") = 0x22222222;
    register int reg_var3 asm ("r14") = 0x33333333;
    
    /* Variables for inline assembly results */
    int asm_result1, asm_result2, asm_result3;
    int *ptr1 = &v1;
    int *ptr2 = &v2;
    
    /* Complex arithmetic to create live values */
    v1 = v1 * 2 + 1;
    v2 = v2 / 3 - 1;
    v3 = v3 ^ v4;
    v4 = v4 | v5;
    
    /* BLOCK 1: Create register pressure and conflicting constraints */
block1:
    {
        /* Inline assembly with multiple outputs, inputs, and clobbers */
        __asm__ volatile (
            /* Multiple output operands with earlyclobber to force separate registers */
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movl %[in3], %%ebx\n\t"
            "subl %[in4], %%ebx\n\t"
            "movl %%ebx, %[out2]\n\t"
            /* Complex operation using memory operand */
            "movl (%[mem1]), %%ecx\n\t"
            "imull %%ecx, %%eax\n\t"
            "movl %%eax, %[out3]"
            : [out1] "=&r" (asm_result1),  /* Earlyclobber constraint */
              [out2] "=&r" (asm_result2),  /* Earlyclobber constraint */
              [out3] "=r" (asm_result3)
            : [in1] "r" (v1),
              [in2] "r" (v2),
              [in3] "r" (v3),
              [in4] "r" (v4),
              [mem1] "r" (ptr1)           /* Memory address operand */
            : "rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "cc", "memory"
        );
        
        /* Use results to keep them live */
        v5 = asm_result1 + asm_result2;
        v6 = asm_result3 ^ v5;
    }
    
    /* Modify variables to create new live ranges */
    reg_var1 = reg_var1 * 2;
    reg_var2 = reg_var2 + v1;
    reg_var3 = reg_var3 - v2;
    
    /* BLOCK 2: Mixed data types and mode conflicts */
block2:
    {
        int temp1, temp2;
        float ftemp;
        double dtemp;
        
        /* Inline assembly with different sized operands */
        __asm__ volatile (
            /* Mix 8-bit, 16-bit, and 32-bit operations */
            "movb %[c1], %%al\n\t"
            "movw %[s1], %%bx\n\t"
            "movl %[i1], %%ecx\n\t"
            "addb %%al, %%al\n\t"
            "addw %%bx, %%bx\n\t"
            "addl %%ecx, %%ecx\n\t"
            "movb %%al, %[out8]\n\t"
            "movw %%bx, %[out16]\n\t"
            "movl %%ecx, %[out32]"
            : [out8] "=r" (c1),
              [out16] "=r" (s1),
              [out32] "=r" (temp1)
            : [c1] "r" ((int)c1),
              [s1] "r" ((int)s1),
              [i1] "r" (i1)
            : "rax", "rbx", "rcx", "cc"
        );
        
        /* Floating point operations to use different register sets */
        __asm__ volatile (
            "movss %[f1], %%xmm0\n\t"
            "movsd %[d1], %%xmm1\n\t"
            "addss %%xmm0, %%xmm0\n\t"
            "addsd %%xmm1, %%xmm1\n\t"
            "movss %%xmm0, %[fout]\n\t"
            "movsd %%xmm1, %[dout]"
            : [fout] "=x" (ftemp),
              [dout] "=x" (dtemp)
            : [f1] "x" (f1),
              [d1] "x" (d1)
            : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
        );
        
        f1 = ftemp;
        d1 = dtemp;
        i1 = temp1;
    }
    
    /* BLOCK 3: More complex constraints with address taken variables */
block3:
    {
        int local1 = 0xAAAA;
        int local2 = 0xBBBB;
        int local3 = 0xCCCC;
        int *ptr_local = &local1;
        
        /* Take address of multiple variables */
        int *addr1 = &local1;
        int *addr2 = &local2;
        int *addr3 = &local3;
        
        /* Inline assembly that uses both register and memory constraints
           for the same logical value */
        __asm__ volatile (
            /* Use variable in register */
            "movl %[val1], %%eax\n\t"
            /* Use same variable's address for memory operation */
            "addl $1, (%[mem_addr])\n\t"
            "addl %%eax, %[val2]\n\t"
            "movl %[val2], %%ebx"
            : [val2] "+m" (local2),  /* Read-write memory constraint */
              "=&r" (local3)         /* Earlyclobber output */
            : [val1] "r" (local1),
              [mem_addr] "r" (addr1)
            : "rax", "rbx", "rcx", "memory", "cc"
        );
        
        v7 = local1 + local2 + local3;
    }
    
    /* BLOCK 4: Force spills by using all explicit register variables */
block4:
    {
        /* Inline assembly that clobbers the registers we've pinned variables to */
        __asm__ volatile (
            "movl $0x9999, %%r12\n\t"   /* Clobber reg_var1's register */
            "movl $0x8888, %%r13\n\t"   /* Clobber reg_var2's register */
            "movl $0x7777, %%r14\n\t"   /* Clobber reg_var3's register */
            "addl %%r12d, %%r13d\n\t"
            "movl %%r13d, %[out]"
            : [out] "=r" (v8)
            : /* no inputs */
            : "r12", "r13", "r14", "cc"
        );
        
        /* Force compiler to reload the original values */
        reg_var1 = reg_var1 + 1;
        reg_var2 = reg_var2 * 2;
        reg_var3 = reg_var3 - 3;
    }
    
    /* Create control flow with goto to extend live ranges */
    if (v1 > v2) {
        goto block1;
    } else if (v3 < v4) {
        goto block2;
    } else if (v5 == v6) {
        goto block3;
    } else {
        goto block4;
    }
    
    /* Final aggregation to prevent dead code elimination */
    volatile int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
                          i1 + c1 + s1 + (int)l1 + (int)f1 + (int)d1 +
                          reg_var1 + reg_var2 + reg_var3;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
