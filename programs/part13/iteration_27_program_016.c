/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills by using many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
volatile double d1 = 1.0, d2 = 2.0;

/* Explicit register variables to force conflicts */
register int r12_var asm ("r12") = 100;
register int r13_var asm ("r13") = 200;
register int r14_var asm ("r14") = 300;

int main(void) {
    /* Local variables with mixed types to create mode conflicts */
    char c1 = 'a', c2 = 'b';
    short s1 = 1000, s2 = 2000;
    int i1 = 10000, i2 = 20000, i3 = 30000, i4 = 40000;
    long l1 = 50000L, l2 = 60000L;
    float f_local = 3.14f;
    double d_local = 2.71828;
    
    /* Variables for address-taking to create addressing mode conflicts */
    int addr_var1 = 111, addr_var2 = 222;
    int *ptr1 = &addr_var1, *ptr2 = &addr_var2;
    
    /* Force many values to be live */
    int sum = 0;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int in1 = i1 + v1;
        int out1;
        
        /* Assembly with input/output conflicts and many clobbers */
        asm volatile (
            "movl %[input], %%eax\n\t"
            "addl $100, %%eax\n\t"
            "movl %%eax, %[output]\n\t"
            : [output] "=r" (out1)          /* Output in register */
            : [input] "r" (in1),            /* Input in register */
              "m" (addr_var1)               /* Also use memory operand - conflict! */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "r12", "r13", "r14", "r15", "memory", "cc"
        );
        
        i1 = out1 + v2;
        sum += i1;
        
        /* Use explicit register variables that were clobbered */
        r12_var += 1;
        r13_var += 2;
    }
    
    /* Block 2: More assembly with different constraints */
block2:
    {
        long combined = l1 + (long)v3;
        short short_out;
        
        /* Mixed size operands to trigger mode reloads */
        asm volatile (
            "mov %[in], %%ax\n\t"
            "inc %%ax\n\t"
            "mov %%ax, %[out]\n\t"
            : [out] "=r" (short_out)        /* 16-bit output */
            : [in] "r" ((short)combined)    /* 16-bit input */
            : "rax", "rbx", "rcx", "memory"
        );
        
        s1 = short_out + v4;
        sum += s1;
        
        /* Force float/double operations to use different registers */
        f_local = f1 + f2 + v5;
        d_local = d1 * d2 + v6;
    }
    
    /* Block 3: Assembly with '&' earlyclobber constraint */
block3:
    {
        int tmp1 = i2 + v7;
        int tmp2 = i3 + v8;
        int result1, result2;
        
        /* Earlyclobber forces different registers, increasing pressure */
        asm volatile (
            "movl %[a], %%eax\n\t"
            "addl %[b], %%eax\n\t"
            "movl %%eax, %[r1]\n\t"
            "movl %[a], %%ebx\n\t"
            "subl %[b], %%ebx\n\t"
            "movl %%ebx, %[r2]\n\t"
            : [r1] "=&r" (result1),         /* Earlyclobber! */
              [r2] "=&r" (result2)          /* Earlyclobber! */
            : [a] "r" (tmp1),
              [b] "r" (tmp2)
            : "rax", "rbx", "memory", "cc"
        );
        
        i2 = result1;
        i3 = result2;
        sum += i2 + i3;
    }
    
    /* Block 4: Memory constraints and address conflicts */
block4:
    {
        int mem_var = v9 + 100;
        int reg_var;
        
        /* Use same variable in both register and memory constraints */
        asm volatile (
            "movl (%[mem]), %%eax\n\t"
            "addl %%ecx, %%eax\n\t"
            "movl %%eax, %[reg]\n\t"
            : [reg] "=r" (reg_var)
            : [mem] "r" (&mem_var),         /* Address in register */
              "m" (mem_var),                /* Value in memory - conflict! */
              "c" (v10)                     /* v10 in ecx */
            : "rax", "memory", "cc"
        );
        
        addr_var1 = reg_var;
        sum += addr_var1;
    }
    
    /* Block 5: Complex live ranges across goto */
block5:
    {
        /* Many live variables at this point */
        int complex1 = i1 + i2 + i3 + i4;
        int complex2 = l1 + l2 + v1 + v2;
        
        asm volatile (
            "movl %[c1], %%eax\n\t"
            "imull %[c2], %%eax\n\t"
            "movl %%eax, %[c1]\n\t"
            : [c1] "+r" (complex1)          /* Read-write operand */
            : [c2] "r" (complex2)
            : "rax", "rdx", "memory", "cc"  /* imul clobbers rdx too */
        );
        
        /* Modify variables to keep them live */
        i4 = complex1 + v3;
        l1 = complex2 + v4;
        
        /* Use all volatile variables to prevent optimization */
        sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        
        /* Jump back to create loop-like live ranges */
        static int counter = 0;
        if (counter++ < 2) {
            goto block3;  /* Jump back to increase live range complexity */
        }
    }
    
    /* Final block: Use all variables in computation */
    {
        /* Force use of all variables to ensure they're live */
        long final = (long)sum + i1 + i2 + i3 + i4 + s1 + s2 + l1 + l2 
                   + (int)c1 + (int)c2 + (int)f_local + (int)d_local
                   + r12_var + r13_var + r14_var;
        
        /* One more assembly with many clobbers */
        asm volatile (
            "add $1, %[val]\n\t"
            : [val] "+r" (final)
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10",
              "r11", "r12", "r13", "r14", "r15", "memory", "cc"
        );
        
        /* Store to volatile to prevent dead code elimination */
        volatile int checksum = (int)final;
        
        printf("Result: %d\n", checksum);
    }
    
    return 0;
}
