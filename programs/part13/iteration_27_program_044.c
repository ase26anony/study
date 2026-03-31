/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills with many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
volatile double f1 = 1.1, f2 = 2.2, f3 = 3.3, f4 = 4.4;
volatile float f5 = 5.5, f6 = 6.6;

/* Explicit register variables to pin registers */
register int r12_var asm ("r12") = 0x1234;
register int r13_var asm ("r13") = 0x5678;
register int r14_var asm ("r14") = 0x9ABC;
register int r15_var asm ("r15") = 0xDEF0;

int main(void) {
    /* Local variables with mixed types to create mode conflicts */
    char c1 = 'A', c2 = 'B';
    short s1 = 100, s2 = 200;
    int i1 = 1000, i2 = 2000, i3 = 3000, i4 = 4000;
    long l1 = 50000L, l2 = 60000L;
    float f_local1 = 7.7f, f_local2 = 8.8f;
    double d_local1 = 9.9, d_local2 = 10.10;
    
    /* Variables whose addresses will be taken */
    int addr_var1 = 111, addr_var2 = 222;
    int *ptr1 = &addr_var1, *ptr2 = &addr_var2;
    
    /* Force many live values by using all variables */
    int sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    sum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    sum += (int)f5 + (int)f6;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int tmp1 = i1 + i2;
        int tmp2 = i3 + i4;
        
        /* Inline asm with multiple outputs, early clobber, and memory constraint */
        asm volatile (
            "mov %[in1], %%eax\n\t"
            "add %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "imul %[in3], %%eax\n\t"
            "mov %%eax, %[out2]\n\t"
            "lea (%[mem_in], %[in4]), %%ebx\n\t"
            "mov %%ebx, %[out3]"
            : [out1] "=r" (i1),          /* Output in register */
              [out2] "=&r" (i2),         /* Early clobber output */
              [out3] "=m" (addr_var1)    /* Output in memory */
            : [in1] "r" (tmp1),          /* Input in register */
              [in2] "r" (tmp2),
              [in3] "r" (r12_var),       /* Uses pinned register variable */
              [in4] "r" (s1),
              [mem_in] "m" (addr_var2)   /* Memory input */
            : "rax", "rbx", "rcx", "rdx", "r12", "memory", "cc"
        );
        
        /* Modify variables to keep them live */
        c1++;
        s1 += v1;
        r12_var ^= 0xFF;
    }
    
    /* Block 2: More inline assembly with floating point and mixed modes */
block2:
    {
        double d_tmp = d_local1 + d_local2;
        float f_tmp = f_local1 * f_local2;
        
        /* Mixed type inline asm with conflicting constraints */
        asm volatile (
            "cvtsi2sd %[int_in], %%xmm0\n\t"
            "addsd %[double_in], %%xmm0\n\t"
            "movsd %%xmm0, %[double_out]\n\t"
            "cvttsd2si %%xmm0, %%eax\n\t"
            "add %[reg_in], %%eax\n\t"
            "mov %%eax, %[int_out]"
            : [double_out] "=m" (d_local1),  /* Double output to memory */
              [int_out] "=r" (i3)            /* Integer output to register */
            : [int_in] "r" (i4),
              [double_in] "x" (d_tmp),       /* XMM register constraint */
              [reg_in] "r" (r13_var)         /* Uses another pinned register */
            : "rax", "xmm0", "xmm1", "xmm2", "r13", "memory", "cc"
        );
        
        /* Force address computation */
        int * volatile ptr_vol = ptr1;
        *ptr_vol += v2;
        ptr2 = &i4;
    }
    
    /* Block 3: Assembly with many clobbers and live range spanning */
block3:
    {
        long l_tmp = l1 * l2;
        char c_tmp = c1 + c2;
        
        /* Inline asm that clobbers many registers */
        asm volatile (
            "mov %[in_long], %%rax\n\t"
            "mov %[in_char], %%bl\n\t"
            "add %%bl, %%al\n\t"
            "mov %%rax, %[out_long]\n\t"
            "mov %%al, %[out_char]"
            : [out_long] "=r" (l1),
              [out_char] "=m" (c2)
            : [in_long] "r" (l_tmp),
              [in_char] "r" ((int)c_tmp)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "r12", "r13", "r14", "r15", "memory", "cc"
        );
        
        /* Use all pinned register variables to keep them live */
        r14_var += r15_var;
        r15_var ^= r12_var;
    }
    
    /* Block 4: Final assembly with address mode conflicts */
block4:
    {
        /* Take address and use in same asm */
        int local_var = 9999;
        int *addr = &local_var;
        
        /* Conflict: variable used both as memory and register operand */
        asm volatile (
            "mov (%[addr_in]), %%eax\n\t"
            "add %[val_in], %%eax\n\t"
            "mov %%eax, (%[addr_in])\n\t"
            "mov %%eax, %[reg_out]"
            : [reg_out] "=r" (i4)
            : [addr_in] "r" (addr),      /* Address in register */
              [val_in] "r" (v3)          /* Value in register */
            : "rax", "memory", "cc"
        );
        
        /* Force the variable to be live in register too */
        local_var += v4;
        sum += local_var;
    }
    
    /* Create control flow with goto to extend live ranges */
    if (sum > 1000) {
        goto block1;
    } else if (sum > 500) {
        goto block2;
    } else {
        goto block3;
    }
    
    /* Final computation using all variables to prevent optimization */
    volatile int checksum = 
        i1 + i2 + i3 + i4 +
        c1 + c2 + s1 + s2 +
        l1 + l2 +
        (int)f_local1 + (int)f_local2 +
        (int)d_local1 + (int)d_local2 +
        r12_var + r13_var + r14_var + r15_var +
        sum;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
