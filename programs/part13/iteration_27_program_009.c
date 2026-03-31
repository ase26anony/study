/* reload_test.c - Complex program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills with many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
volatile double d1 = 1.0, d2 = 2.0;

/* Explicit register variables to force conflicts */
register int r12_var asm ("r12") = 100;
register int r13_var asm ("r13") = 200;
register int r14_var asm ("r14") = 300;

int main(void) {
    /* Local variables with mixed types and modes */
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 10000, i2 = 20000, i3 = 30000, i4 = 40000;
    long l1 = 50000L, l2 = 60000L;
    float f_local = 3.14f;
    double d_local = 2.71828;
    
    /* Variables whose addresses we'll take */
    int addr_var1 = 111, addr_var2 = 222;
    int *ptr1 = &addr_var1, *ptr2 = &addr_var2;
    
    /* Force many live values */
    v1 = i1 + c1;
    v2 = s1 * 2;
    f1 = f_local + 1.0f;
    d1 = d_local * 2.0;
    
    /* Use explicit register variables */
    r12_var += v1;
    r13_var += v2;
    r14_var += i1;
    
    /* BLOCK 1: Complex inline asm with conflicting constraints */
block1:
    {
        int in1 = i1, in2 = i2, out1, out2;
        
        /* asm with input/output conflicts and memory addressing */
        asm volatile (
            "movl %[input1], %%eax\n\t"
            "addl %[input2], %%eax\n\t"
            "movl %%eax, %[output1]\n\t"
            "leal (%[mem1], %%eax, 2), %%ebx\n\t"
            "movl %%ebx, %[output2]\n\t"
            : [output1] "=&r" (out1),   /* Early clobber - conflicts with inputs */
              [output2] "=r" (out2)     /* Regular output */
            : [input1] "r" (in1),       /* Input in register */
              [input2] "r" (in2),       /* Another input */
              [mem1] "m" (addr_var1)    /* Memory operand - address taken */
            : "rax", "rbx", "rcx", "memory"  /* Many clobbers */
        );
        
        i3 = out1 + out2;
        v3 = i3;
        
        /* Force addr_var1 to be live and modified */
        addr_var1 = out1;
    }
    
    /* Modify variables to keep them live across blocks */
    c1 += 1;
    s1 += 100;
    i2 = i3 * 2;
    f_local += f1;
    d_local += d1;
    
    /* BLOCK 2: More asm with different data types */
block2:
    {
        short s_in = s1;
        char c_in = c1;
        int i_out;
        
        /* Mixed size operands causing mode issues */
        asm volatile (
            "movswl %w[short_in], %%eax\n\t"   /* Sign extend short to long */
            "movsbl %b[char_in], %%ebx\n\t"    /* Sign extend char to long */
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %[int_out]\n\t"
            : [int_out] "=r" (i_out)
            : [short_in] "r" (s_in),
              [char_in] "r" (c_in)
            : "rax", "rbx", "cc"
        );
        
        i4 = i_out;
        v4 = i4 + r12_var;  /* Use register variable */
        
        /* Clobber explicit register */
        asm volatile (
            "xorl %%r12d, %%r12d\n\t"
            "addl $1, %%r12d\n\t"
            : : : "r12"  /* Explicitly clobber r12 */
        );
        
        /* Now r12_var needs reloading */
        r12_var += 50;
    }
    
    /* BLOCK 3: Floating point and integer mix */
block3:
    {
        float f_in = f_local;
        double d_in = d_local;
        int i_temp;
        float f_out;
        
        /* Type conversion forcing reloads */
        asm volatile (
            "cvttss2si %[float_in], %%eax\n\t"   /* float to int */
            "movl %%eax, %[int_temp]\n\t"
            "cvtsi2sdl %[int_temp], %%xmm0\n\t"  /* int to double */
            "cvtsd2ss %%xmm0, %%xmm1\n\t"        /* double to float */
            "movss %%xmm1, %[float_out]\n\t"
            : [int_temp] "=m" (i_temp),          /* Memory output */
              [float_out] "=x" (f_out)           /* XMM register output */
            : [float_in] "x" (f_in)              /* XMM register input */
            : "rax", "xmm0", "xmm1", "memory"
        );
        
        f2 = f_out;
        v5 = i_temp;
    }
    
    /* BLOCK 4: Complex addressing with many live values */
block4:
    {
        int a = i1, b = i2, c = i3, d = i4;
        int result1, result2;
        
        /* Multiple memory operands with same address */
        asm volatile (
            "movl %[a_in], %%eax\n\t"
            "imull %[b_in], %%eax\n\t"
            "addl (%[ptr1]), %%eax\n\t"      /* Memory access via pointer */
            "movl %%eax, %[r1]\n\t"
            "movl %[c_in], %%ebx\n\t"
            "subl %[d_in], %%ebx\n\t"
            "addl (%[ptr2]), %%ebx\n\t"      /* Another memory access */
            "movl %%ebx, %[r2]\n\t"
            : [r1] "=&r" (result1),          /* Early clobber */
              [r2] "=&r" (result2)           /* Early clobber */
            : [a_in] "r" (a),
              [b_in] "r" (b),
              [c_in] "r" (c),
              [d_in] "r" (d),
              [ptr1] "r" (ptr1),            /* Pointer in register */
              [ptr2] "r" (ptr2)             /* Another pointer */
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        v6 = result1;
        v7 = result2;
        
        /* Modify pointers to force reloads */
        ptr1 = &v6;
        ptr2 = &v7;
    }
    
    /* BLOCK 5: Large clobber list forcing many spills */
block5:
    {
        long l_in = l1;
        int i_in = i1;
        long l_out;
        
        /* Clobber almost all registers */
        asm volatile (
            "movq %[lin], %%rax\n\t"
            "addq $1000, %%rax\n\t"
            "movq %%rax, %[lout]\n\t"
            : [lout] "=r" (l_out)
            : [lin] "r" (l_in)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10",
              "r11", "r12", "r13", "r14", "r15", "memory", "cc"
        );
        
        l2 = l_out + i_in;
        v8 = l2;
        
        /* Use all explicit register variables */
        r13_var += r12_var;
        r14_var += r13_var;
    }
    
    /* Final computation using all variables */
    volatile int checksum = 0;
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += c1 + c2 + s1 + s2;
    checksum += i1 + i2 + i3 + i4;
    checksum += l1 + l2;
    checksum += (int)f1 + (int)f2 + (int)f3;
    checksum += (int)d1 + (int)d2;
    checksum += r12_var + r13_var + r14_var;
    checksum += addr_var1 + addr_var2;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
