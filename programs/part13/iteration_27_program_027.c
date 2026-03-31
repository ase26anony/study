/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force compiler to generate reloads through complex inline assembly */
int main(void) {
    /* Declare many volatile variables to prevent optimization */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile long v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    volatile double d1 = 4.4, d2 = 5.5, d3 = 6.6;
    
    /* Non-volatile variables that will be live across blocks */
    int nv1 = 100, nv2 = 200, nv3 = 300, nv4 = 400;
    long nv5 = 500, nv6 = 600;
    float nvf1 = 700.0f, nvf2 = 800.0f;
    double nvd1 = 900.0, nvd2 = 1000.0;
    
    /* Explicit register variables - pin to specific registers */
    register int r12_var asm ("r12") = 0x1234;
    register int r13_var asm ("r13") = 0x5678;
    register int r14_var asm ("r14") = 0x9ABC;
    register int r15_var asm ("r15") = 0xDEF0;
    
    /* Variables for address-taking to create addressing mode conflicts */
    int addr_var1 = 111, addr_var2 = 222, addr_var3 = 333;
    int *addr_ptr1 = &addr_var1, *addr_ptr2 = &addr_var2;
    
    /* Result accumulator */
    volatile int checksum = 0;
    
    /* Block 1: Create live values and force spills */
block1:
    {
        /* Complex arithmetic to create many live values */
        v1 = v2 + v3 * v4 - v5;
        v6 = v7 << 2 | v8 >> 1;
        f1 = f2 * 2.0f + f3;
        d1 = d2 / 1.5 + d3;
        
        /* Inline assembly with many clobbers and constraints */
        asm volatile (
            /* Output operands with earlyclobber to force separate registers */
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out2]\n\t"
            
            /* Memory operand with address conflict */
            "movl (%[mem]), %%ebx\n\t"
            "addl %%ebx, %[out3]\n\t"
            
            /* Use explicit register variables */
            "addl %%r12d, %[out1]\n\t"
            "addl %%r13d, %[out2]\n\t"
            
            /* Different sized operations */
            "movw %w[in4], %%cx\n\t"
            "addw %%cx, %w[out4]\n\t"
            
            : [out1] "=&r" (nv1),      /* Earlyclobber reg constraint */
              [out2] "=&r" (nv2),
              [out3] "+r" (nv3),
              [out4] "+r" (nv4)
            : [in1] "r" (v1),
              [in2] "r" (v2),
              [in3] "r" (v3),
              [in4] "r" ((short)v4),
              [mem] "r" (addr_ptr1)    /* Memory address in register */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", 
              "r11", "cc", "memory"
        );
        
        /* Modify variables to keep them live */
        r12_var += nv1;
        r13_var += nv2;
        checksum += nv1 + nv2 + nv3 + nv4;
    }
    
    /* Block 2: Different constraints and data types */
block2:
    {
        /* More arithmetic */
        v2 = v3 ^ v4 | v5;
        v7 = v8 * v9 / (v10 + 1);
        f2 = f1 + f3 * 2.0f;
        d2 = d1 - d3 / 3.0;
        
        /* Another complex asm with mixed constraints */
        asm volatile (
            /* Floating point operations */
            "movss %[fin1], %%xmm0\n\t"
            "addss %[fin2], %%xmm0\n\t"
            "movss %%xmm0, %[fout1]\n\t"
            
            /* Integer with memory constraint */
            "movl %[in5], %%eax\n\t"
            "addl %%eax, %[out5]\n\t"
            "movl %[in6], %[out6]\n\t"
            
            /* Use the same register for input and output (conflict) */
            "movl %[in7], %%ebx\n\t"
            "leal 1(%%ebx), %%ebx\n\t"
            "movl %%ebx, %[out7]\n\t"
            
            : [fout1] "=x" (nvf1),     /* XMM register constraint */
              [out5] "+rm" (nv5),      /* Register or memory */
              [out6] "=rm" (nv6),
              [out7] "=r" (addr_var3)
            : [fin1] "x" (f1),
              [fin2] "x" (f2),
              [in5] "rm" (v5),         /* Can be reg or mem */
              [in6] "m" (v6),          /* Memory constraint */
              [in7] "r" (addr_var2)    /* Register constraint */
            : "rax", "rbx", "rcx", "xmm0", "xmm1", "xmm2", "xmm3", 
              "xmm4", "xmm5", "memory", "cc"
        );
        
        /* Keep values live */
        r14_var += nv5;
        r15_var += nv6;
        checksum += nv5 + nv6 + addr_var3;
        checksum += (int)nvf1;
    }
    
    /* Block 3: More conflicts with double precision */
block3:
    {
        /* Force many values to be live */
        v3 = v4 + v5 - v1;
        v8 = v9 & v10 | v6;
        f3 = f1 * f2 / 3.14f;
        d3 = d1 + d2 * 1.618;
        
        /* Asm with double and different modes */
        asm volatile (
            /* Double precision */
            "movsd %[din1], %%xmm6\n\t"
            "mulsd %[din2], %%xmm6\n\t"
            "movsd %%xmm6, %[dout1]\n\t"
            
            /* Byte operations (different mode) */
            "movb %b[in8], %%al\n\t"
            "addb $1, %%al\n\t"
            "movb %%al, %b[out8]\n\t"
            
            /* Word operations */
            "movw %w[in9], %%bx\n\t"
            "subw $2, %%bx\n\t"
            "movw %%bx, %w[out9]\n\t"
            
            /* Dword with specific register constraint */
            "movl %[in10], %%ecx\n\t"
            "shrl $1, %%ecx\n\t"
            "movl %%ecx, %[out10]\n\t"
            
            : [dout1] "=x" (nvd1),
              [out8] "+r" (v1),        /* Byte output */
              [out9] "+r" (v2),        /* Word output */
              [out10] "=r" (v3)        /* Dword output */
            : [din1] "x" (d1),
              [din2] "x" (d2),
              [in8] "r" ((char)v4),    /* Byte input */
              [in9] "r" ((short)v5),   /* Word input */
              [in10] "r" (v6)          /* Dword input */
            : "rax", "rbx", "rcx", "xmm6", "xmm7", "cc"
        );
        
        checksum += v1 + v2 + v3;
        checksum += (int)nvd1;
    }
    
    /* Block 4: Final block with goto back to create loop-like live ranges */
block4:
    {
        static int counter = 0;
        counter++;
        
        /* Asm that clobbers explicit register variables */
        asm volatile (
            "addl $100, %%r12d\n\t"
            "subl $50, %%r13d\n\t"
            "xorl %%r14d, %%r15d\n\t"
            "movl %%r15d, %[result]\n\t"
            : [result] "=r" (nv1)
            : /* No inputs */
            : "r12", "r13", "r14", "r15", "cc"
        );
        
        checksum += nv1 + r12_var + r13_var + r14_var + r15_var;
        
        /* Create complex control flow with goto */
        if (counter < 2) {
            /* Jump back to create overlapping live ranges */
            goto block2;
        }
    }
    
    /* Final checksum calculation and output */
    checksum += addr_var1 + addr_var2 + addr_var3;
    checksum += (int)f3 + (int)d3;
    
    printf("Result: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
