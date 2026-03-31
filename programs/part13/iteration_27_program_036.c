/* Test program to trigger GCC reload pass uncovered block in reload.cc */
#include <stdio.h>

/* Force many live values across complex control flow */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile float f1 = 1.0f, f2 = 2.0f, f3 = 3.0f;
volatile double d1 = 1.0, d2 = 2.0;
volatile long l1 = 100, l2 = 200, l3 = 300;
volatile char c1 = 'a', c2 = 'b';
volatile short s1 = 10, s2 = 20;

/* Explicit register variables to pin specific registers */
register int reg_var1 asm ("r12") = 0x1234;
register int reg_var2 asm ("r13") = 0x5678;
register int reg_var3 asm ("r14") = 0x9abc;

int main(void) {
    /* Local volatile variables to force memory operations */
    volatile int local_v1, local_v2, local_v3, local_v4;
    volatile float local_f1, local_f2;
    volatile double local_d1;
    volatile long local_l1, local_l2;
    volatile char local_c1;
    volatile short local_s1;
    
    /* Non-volatile variables that will be live across blocks */
    int nv1 = 1000, nv2 = 2000, nv3 = 3000, nv4 = 4000;
    float nvf1 = 1000.0f, nvf2 = 2000.0f;
    double nvd1 = 3000.0;
    long nvl1 = 5000, nvl2 = 6000;
    
    /* Complex addressing mode setup */
    int array[100];
    int *ptr1 = &array[10];
    int *ptr2 = &array[20];
    int *ptr3 = &array[30];
    
    /* Initialize with arithmetic to create dependencies */
    local_v1 = v1 + v2 + v3;
    local_v2 = v4 * v5;
    local_f1 = f1 + f2 + f3;
    local_d1 = d1 * d2;
    local_l1 = l1 + l2 + l3;
    local_c1 = c1 + c2;
    local_s1 = s1 + s2;
    
    /* Block 1: Complex inline assembly with many clobbers */
block1:
    {
        int temp1, temp2, temp3;
        float ftemp;
        double dtemp;
        
        /* Inline assembly with conflicting constraints */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movq %[in3], %%rbx\n\t"
            "addq %[in4], %%rbx\n\t"
            "movq %%rbx, %[out2]\n\t"
            "movss %[in5], %%xmm0\n\t"
            "addss %[in6], %%xmm0\n\t"
            "movss %%xmm0, %[out3]\n\t"
            : [out1] "=r" (temp1), 
              [out2] "=r" (temp2), 
              [out3] "=r" (ftemp)
            : [in1] "r" (local_v1),
              [in2] "r" (local_v2),
              [in3] "r" (local_l1),
              [in4] "r" (l2),
              [in5] "r" (local_f1),
              [in6] "r" (f2)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
              "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
              "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc"
        );
        
        /* Use results to keep them live */
        nv1 += temp1;
        nvl1 += temp2;
        nvf1 += ftemp;
        
        /* Force spill by using all explicit register variables */
        reg_var1 = nv1;
        reg_var2 = nv2;
        reg_var3 = nv3;
    }
    
    /* Block 2: More inline assembly with memory constraints */
block2:
    {
        int out_val;
        long out_long;
        float out_float;
        
        /* Mixed constraints with address taken variables */
        asm volatile (
            "movl (%[mem_in]), %%eax\n\t"
            "imull %[reg_in], %%eax\n\t"
            "movl %%eax, %[reg_out]\n\t"
            "movq (%[mem_in2]), %%rbx\n\t"
            "addq %[reg_in2], %%rbx\n\t"
            "movq %%rbx, %[reg_out2]\n\t"
            : [reg_out] "=&r" (out_val),    /* Early clobber */
              [reg_out2] "=&r" (out_long)   /* Early clobber */
            : [mem_in] "m" (*ptr1),
              [reg_in] "r" (nv1),
              [mem_in2] "m" (*ptr2),
              [reg_in2] "r" (nvl1)
            : "rax", "rbx", "rcx", "memory", "cc"
        );
        
        /* Another asm with floating point and different modes */
        asm volatile (
            "movd %[in_char], %%xmm0\n\t"
            "cvtdq2ps %%xmm0, %%xmm0\n\t"
            "addss %[in_float], %%xmm0\n\t"
            "movss %%xmm0, %[out_float]\n\t"
            : [out_float] "=r" (out_float)
            : [in_char] "r" (local_c1),
              [in_float] "r" (nvf1)
            : "xmm0", "xmm1", "cc"
        );
        
        nv2 += out_val;
        nvl2 += out_long;
        nvf2 += out_float;
    }
    
    /* Block 3: Inline assembly that clobbers explicit register variables */
block3:
    {
        int result1, result2;
        
        /* This asm clobbers r12, r13, r14 which are used by reg_var1-3 */
        asm volatile (
            "movl $0x1111, %%r12d\n\t"
            "movl $0x2222, %%r13d\n\t"
            "movl $0x3333, %%r14d\n\t"
            "addl %%r12d, %%r13d\n\t"
            "addl %%r13d, %%r14d\n\t"
            "movl %%r12d, %0\n\t"
            "movl %%r14d, %1\n\t"
            : "=r" (result1), "=r" (result2)
            :
            : "r12", "r13", "r14", "cc"
        );
        
        /* Force compiler to reload the original register variables */
        nv3 += reg_var1 + reg_var2 + reg_var3;
        nv4 += result1 + result2;
    }
    
    /* Block 4: Complex addressing with different data types */
block4:
    {
        short out_short;
        char out_char;
        double out_double;
        
        /* Mixed size operands */
        asm volatile (
            "movswl %[in_short], %%eax\n\t"
            "addl %[in_int], %%eax\n\t"
            "movw %%ax, %[out_short]\n\t"
            "movsbl %[in_char], %%ebx\n\t"
            "addl %%eax, %%ebx\n\t"
            "movb %%bl, %[out_char]\n\t"
            : [out_short] "=r" (out_short),
              [out_char] "=r" (out_char)
            : [in_short] "r" (local_s1),
              [in_int] "r" (nv4),
              [in_char] "r" (local_c1)
            : "rax", "rbx", "cc"
        );
        
        /* Double precision with memory constraint */
        asm volatile (
            "movsd %[in_double], %%xmm0\n\t"
            "addsd %[in_double2], %%xmm0\n\t"
            "movsd %%xmm0, %[out_double]\n\t"
            : [out_double] "=r" (out_double)
            : [in_double] "m" (local_d1),
              [in_double2] "r" (nvd1)
            : "xmm0", "xmm1", "memory"
        );
        
        local_s1 = out_short;
        local_c1 = out_char;
        nvd1 = out_double;
    }
    
    /* Create control flow with goto to keep many values live */
    if (nv1 > 0) {
        goto block1;
    } else if (nv2 > 100) {
        goto block2;
    } else if (nv3 > 200) {
        goto block3;
    } else {
        goto block4;
    }
    
    /* Final computation to prevent dead code elimination */
    volatile int checksum = 0;
    checksum += nv1 + nv2 + nv3 + nv4;
    checksum += (int)nvf1 + (int)nvf2;
    checksum += (int)nvd1;
    checksum += (int)nvl1 + (int)nvl2;
    checksum += local_v1 + local_v2;
    checksum += (int)local_f1 + (int)local_d1;
    checksum += local_l1 + local_c1 + local_s1;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0;
}
