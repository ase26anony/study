/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force many live values and complex register allocation */
int main(void) {
    /* Volatile variables to prevent optimization */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile long vl1 = 100, vl2 = 200, vl3 = 300;
    volatile float vf1 = 1.5f, vf2 = 2.5f, vf3 = 3.5f;
    volatile double vd1 = 10.5, vd2 = 20.5;
    volatile char vc1 = 'a', vc2 = 'b';
    volatile short vs1 = 1000, vs2 = 2000;
    
    /* Non-volatile variables for register pressure */
    int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    long l1, l2, l3, l4, l5;
    float f1, f2, f3;
    double d1, d2;
    
    /* Explicit register variables - pin to specific registers */
    register int reg_a asm ("rax") = 0x12345678;
    register int reg_b asm ("rbx") = 0x87654321;
    register int reg_c asm ("r12") = 0xABCDEF01;
    register int reg_d asm ("r13") = 0xFEDCBA09;
    
    /* Initialize non-volatile variables */
    r1 = v1 + 1; r2 = v2 + 2; r3 = v3 + 3; r4 = v4 + 4; r5 = v5 + 5;
    r6 = r1 * 2; r7 = r2 * 3; r8 = r3 * 4; r9 = r4 * 5; r10 = r5 * 6;
    l1 = vl1 * 2; l2 = vl2 * 3; l3 = vl3 * 4; l4 = l1 + l2; l5 = l3 - l4;
    f1 = vf1 * 2.0f; f2 = vf2 * 3.0f; f3 = vf3 * 4.0f;
    d1 = vd1 * 2.0; d2 = vd2 * 3.0;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int temp1 = r1 + r2;
        int temp2 = r3 + r4;
        int *ptr1 = &r5;
        int *ptr2 = &r6;
        
        /* Inline asm with memory and register constraints on same variable */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out2]\n\t"
            : [out1] "=m" (*ptr1), [out2] "=r" (temp2)
            : [in1] "r" (temp1), [in2] "m" (r7), [in3] "r" (r8)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        r1 = temp1;
        r2 = temp2;
        v1 = *ptr1 + *ptr2;
    }
    
    /* Force spill by using all explicit register variables */
    reg_a = reg_a + reg_b;
    reg_c = reg_d - reg_a;
    
    /* Block 2: Mixed data types and addressing mode conflicts */
block2:
    {
        short s_temp = vs1;
        char c_temp = vc1;
        float f_temp = vf1;
        
        /* Inline asm with different sized operands */
        asm volatile (
            "movw %w[in_s], %%ax\n\t"
            "movsbl %b[in_c], %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %[out_i]\n\t"
            "cvtsi2ss %%eax, %%xmm0\n\t"
            "addss %[in_f], %%xmm0\n\t"
            "movss %%xmm0, %[out_f]\n\t"
            : [out_i] "=r" (r3), [out_f] "=m" (f_temp)
            : [in_s] "r" (s_temp), [in_c] "r" (c_temp), [in_f] "m" (vf2)
            : "rax", "rbx", "xmm0", "xmm1", "memory", "cc"
        );
        
        vs1 = s_temp + 1;
        vc1 = c_temp + 1;
        vf1 = f_temp;
    }
    
    /* Create more register pressure */
    l1 = l1 + reg_a;
    l2 = l2 + reg_b;
    l3 = l3 + reg_c;
    l4 = l4 + reg_d;
    
    /* Block 3: Multiple output operands with early clobber */
block3:
    {
        long long1 = vl1;
        long long2 = vl2;
        double double1 = vd1;
        
        /* & means early clobber - can't share registers with inputs */
        asm volatile (
            "movq %[in_l1], %%rax\n\t"
            "addq %[in_l2], %%rax\n\t"
            "movq %%rax, %[out_l]\n\t"
            "cvtsi2sdq %%rax, %%xmm0\n\t"
            "addsd %[in_d], %%xmm0\n\t"
            "movsd %%xmm0, %[out_d]\n\t"
            : [out_l] "=&r" (long1), [out_d] "=m" (double1)
            : [in_l1] "r" (vl1), [in_l2] "m" (long2), [in_d] "m" (vd2)
            : "rax", "xmm0", "xmm1", "xmm2", "memory", "cc"
        );
        
        vl1 = long1;
        vd1 = double1;
    }
    
    /* Block 4: Complex constraints with '+' modifier */
block4:
    {
        int inout1 = r9;
        int inout2 = r10;
        
        /* '+' means read-write operand */
        asm volatile (
            "movl %[io1], %%eax\n\t"
            "addl %[io2], %%eax\n\t"
            "movl %%eax, %[io1]\n\t"
            "imull %%eax, %%eax\n\t"
            "movl %%eax, %[io2]\n\t"
            : [io1] "+r" (inout1), [io2] "+r" (inout2)
            :
            : "rax", "rdx", "memory", "cc"
        );
        
        r9 = inout1;
        r10 = inout2;
    }
    
    /* Block 5: Force reloads with control flow */
block5:
    {
        int sum = 0;
        int *ptr_arr[5] = {&r1, &r2, &r3, &r4, &r5};
        
        /* Complex asm with indirect addressing */
        for (int i = 0; i < 5; i++) {
            asm volatile (
                "movl (%[ptr]), %%eax\n\t"
                "addl %%eax, %[sum]\n\t"
                : [sum] "+r" (sum)
                : [ptr] "r" (ptr_arr[i])
                : "rax", "memory", "cc"
            );
        }
        
        v2 = sum;
    }
    
    /* Final calculations using all live variables */
    int final_sum = 0;
    final_sum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    final_sum += l1 + l2 + l3 + l4 + l5;
    final_sum += (int)f1 + (int)f2 + (int)f3;
    final_sum += (int)d1 + (int)d2;
    final_sum += reg_a + reg_b + reg_c + reg_d;
    final_sum += v1 + v2 + v3 + v4 + v5;
    final_sum += vl1 + vl2 + vl3;
    final_sum += (int)vf1 + (int)vf2 + (int)vf3;
    final_sum += (int)vd1 + (int)vd2;
    final_sum += vc1 + vc2;
    final_sum += vs1 + vs2;
    
    /* Use goto to create complex control flow with live values */
    if (final_sum > 1000)
        goto block1;
    else if (final_sum > 500)
        goto block3;
    
    /* Final volatile store to prevent dead code elimination */
    volatile int checksum = final_sum;
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
