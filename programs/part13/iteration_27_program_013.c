/* Test program to trigger reload.cc uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force spills with many volatile variables */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
volatile double d1 = 1.0, d2 = 2.0, d3 = 3.0;

/* Explicit register variables to pin registers */
register int r12_var asm ("r12") = 100;
register int r13_var asm ("r13") = 200;
register int r14_var asm ("r14") = 300;
register int r15_var asm ("r15") = 400;

int main(void) {
    /* Local variables with mixed types to create mode mismatches */
    char c1 = 65, c2 = 66;
    short s1 = 1000, s2 = 2000;
    int i1 = 10000, i2 = 20000, i3 = 30000, i4 = 40000;
    long l1 = 50000L, l2 = 60000L;
    float f_local = 5.5f;
    double d_local = 10.5;
    
    /* Force many values to be live */
    int sum = 0;
    volatile int checksum = 0;
    
    /* Block 1: Create register pressure */
block1:
    {
        /* Use all volatile variables to prevent optimization */
        i1 = v1 + v2;
        i2 = v3 * v4;
        f_local = f1 + f2;
        d_local = d1 - d2;
        
        /* Inline assembly with conflicting constraints */
        asm volatile (
            "mov %[in1], %%eax\n\t"
            "add %[in2], %%eax\n\t"
            "mov %%eax, %[out1]\n\t"
            "imul %[in3], %%ebx\n\t"
            : [out1] "=r" (i3), "=&r" (i4)  /* Early clobber on i4 */
            : [in1] "r" (i1), 
              [in2] "r" (i2),
              [in3] "r" (r12_var)           /* Uses pinned register */
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        /* Modify variables to keep them live */
        c1 = i3 & 0xFF;
        s1 = i4 & 0xFFFF;
        sum += i3 + i4;
    }
    
    /* Block 2: More complex assembly with memory addressing */
block2:
    {
        /* Take address and use in assembly */
        int* ptr1 = &i1;
        int* ptr2 = &i2;
        
        /* Assembly with mixed constraints causing address reloads */
        asm volatile (
            "mov (%[mem1]), %%r8d\n\t"
            "add (%[mem2]), %%r8d\n\t"
            "mov %%r8d, %[out1]\n\t"
            "lea (%[reg1], %[reg2]), %%r9d\n\t"
            : [out1] "=r" (l1), "=m" (*ptr1)  /* Output to memory */
            : [mem1] "r" (ptr1),
              [mem2] "r" (ptr2),
              [reg1] "r" (r13_var),
              [reg2] "r" (r14_var)
            : "r8", "r9", "r10", "r11", "memory"
        );
        
        /* Force spill by using many variables */
        i1 = v5 + v6;
        i2 = v7 * v8;
        f_local = f3 * 2.0f;
        d_local = d3 / 2.0;
        sum += l1 + *ptr1;
    }
    
    /* Block 3: Assembly with mode mismatches */
block3:
    {
        /* Different sized operands */
        char c_tmp;
        short s_tmp;
        int i_tmp;
        
        asm volatile (
            "movsx %[char_in], %%eax\n\t"
            "addw %[short_in], %%ax\n\t"
            "movl %%eax, %[int_out]\n\t"
            "movb %%al, %[char_out]\n\t"
            : [int_out] "=r" (i_tmp),
              [char_out] "=m" (c_tmp)
            : [char_in] "m" (c1),
              [short_in] "r" (s1)
            : "rax", "rbx", "cc"
        );
        
        /* Use explicit register variables that get clobbered */
        asm volatile (
            "addl $100, %%r12d\n\t"
            "subl $50, %%r13d\n\t"
            : 
            : 
            : "r12", "r13", "r14", "r15"
        );
        
        sum += i_tmp + c_tmp + r12_var + r13_var;
    }
    
    /* Block 4: Complex constraints with early clobbers */
block4:
    {
        int tmp1, tmp2, tmp3, tmp4;
        
        /* Multiple early clobbers force many reloads */
        asm volatile (
            "mov %[a], %%eax\n\t"
            "mov %[b], %%ebx\n\t"
            "add %%ebx, %%eax\n\t"
            "mov %%eax, %[x]\n\t"
            "mov %[c], %%ecx\n\t"
            "mov %[d], %%edx\n\t"
            "imul %%edx, %%ecx\n\t"
            "mov %%ecx, %[y]\n\t"
            : [x] "=&r" (tmp1),   /* Early clobber */
              [y] "=&r" (tmp2),   /* Early clobber */
              "=&r" (tmp3),       /* Early clobber */
              "=&r" (tmp4)        /* Early clobber */
            : [a] "r" (v9),
              [b] "r" (v10),
              [c] "r" (i1),
              [d] "r" (i2)
            : "rax", "rbx", "rcx", "rdx", 
              "rsi", "rdi", "r8", "r9", "r10", "r11",
              "memory", "cc"
        );
        
        sum += tmp1 + tmp2 + tmp3 + tmp4;
        
        /* Jump back to create complex live ranges */
        static int counter = 0;
        if (counter++ < 2) {
            goto block2;
        }
    }
    
    /* Block 5: Mixed floating point and integer */
block5:
    {
        int int_result;
        float float_result;
        double double_result;
        
        /* Force conversions and mode changes */
        asm volatile (
            "cvtsi2ssl %[int_in], %%xmm0\n\t"
            "addss %[float_in], %%xmm0\n\t"
            "cvtss2sd %%xmm0, %%xmm1\n\t"
            "addsd %[double_in], %%xmm1\n\t"
            "cvttsd2si %%xmm1, %%eax\n\t"
            "mov %%eax, %[int_out]\n\t"
            "movss %%xmm0, %[float_out]\n\t"
            "movsd %%xmm1, %[double_out]\n\t"
            : [int_out] "=r" (int_result),
              [float_out] "=m" (float_result),
              [double_out] "=m" (double_result)
            : [int_in] "r" (sum),
              [float_in] "x" (f_local),
              [double_in] "x" (d_local)
            : "xmm0", "xmm1", "xmm2", "xmm3", 
              "xmm4", "xmm5", "rax", "memory"
        );
        
        sum = int_result;
        f_local = float_result;
        d_local = double_result;
    }
    
    /* Final aggregation to prevent dead code elimination */
    checksum = sum + c1 + s1 + i1 + i2 + (int)l1 + 
               (int)f_local + (int)d_local + 
               r12_var + r13_var + r14_var + r15_var;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
