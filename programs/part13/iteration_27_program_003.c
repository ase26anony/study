/* reload_test.c - Complex program to trigger GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Force many live values across complex control flow */
int main(void) {
    /* Phase 1: Declare many volatile variables to prevent optimization */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile long vl1 = 100, vl2 = 200, vl3 = 300;
    volatile float vf1 = 1.5f, vf2 = 2.5f, vf3 = 3.5f;
    volatile double vd1 = 10.5, vd2 = 20.5;
    volatile char vc1 = 'a', vc2 = 'b';
    volatile short vs1 = 1000, vs2 = 2000;
    
    /* Non-volatile variables that will span multiple basic blocks */
    int nv1 = 100, nv2 = 200, nv3 = 300, nv4 = 400, nv5 = 500;
    long nvl1 = 1000, nvl2 = 2000;
    float nvf1 = 100.5f, nvf2 = 200.5f;
    double nvd1 = 1000.5;
    
    /* Explicit register variables - pin to specific registers */
    register int reg_var1 asm ("r12") = 0x1234;
    register int reg_var2 asm ("r13") = 0x5678;
    register int reg_var3 asm ("r14") = 0x9ABC;
    
    /* Variables for address-taking and memory constraints */
    int addr_var1 = 999, addr_var2 = 888, addr_var3 = 777;
    int *addr_ptr1 = &addr_var1;
    int *addr_ptr2 = &addr_var2;
    
    /* Result accumulator */
    volatile int checksum = 0;
    
    /* Block 1: Complex inline assembly with conflicting constraints */
block1:
    {
        int temp1, temp2, temp3;
        
        /* Assembly with multiple outputs, early clobber, and memory constraint */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "imull %[in3], %%eax\n\t"
            "movl %%eax, %[out2]\n\t"
            "leal (%[mem], %[in4]), %%ecx\n\t"
            "movl %%ecx, %[out3]"
            : [out1] "=r" (temp1), 
              [out2] "=&r" (temp2),  /* & = early clobber */
              [out3] "=r" (temp3)
            : [in1] "r" (nv1),
              [in2] "r" (nv2),
              [in3] "r" (nv3),
              [in4] "r" (nv4),
              [mem] "m" (addr_var1)  /* Memory constraint */
            : "rax", "rcx", "memory", "cc"
        );
        
        checksum += temp1 + temp2 + temp3;
        nv1 = temp1;  /* Keep nv1 live */
        nv2 = temp2;  /* Keep nv2 live */
    }
    
    /* Force some computation to create more live values */
    v1 = v2 + v3;
    vf1 = vf2 * vf3;
    vd1 = vd2 / 2.0;
    
    /* Block 2: More assembly with clobbered explicit registers */
block2:
    {
        long result1, result2;
        int small1, small2;
        
        /* This assembly clobbers r12, r13, r14 which are pinned above */
        asm volatile (
            "mov %[in_long1], %%rbx\n\t"
            "add %[in_long2], %%rbx\n\t"
            "mov %%rbx, %[out_long1]\n\t"
            "mov %[reg1], %%r12\n\t"      /* Use pinned register */
            "add %[reg2], %%r12\n\t"      /* Modify pinned register */
            "mov %%r12, %[out_small1]\n\t"
            "mov %[in_small1], %%r13\n\t" /* Another pinned register */
            "sub %[in_small2], %%r13\n\t"
            "mov %%r13, %[out_small2]"
            : [out_long1] "=r" (result1),
              [out_small1] "=r" (small1),
              [out_small2] "=r" (small2)
            : [in_long1] "r" (nvl1),
              [in_long2] "r" (nvl2),
              [reg1] "r" (reg_var1),
              [reg2] "r" (reg_var2),
              [in_small1] "r" (vs1),
              [in_small2] "r" (vs2)
            : "rbx", "r12", "r13", "r14", "memory", "cc"
        );
        
        checksum += (int)result1 + small1 + small2;
        reg_var1 = small1;  /* Update pinned register variable */
    }
    
    /* More volatile operations */
    v4 = v5 * 2;
    vc1 = vc2 + 1;
    
    /* Block 3: Mixed data types and addressing modes */
block3:
    {
        double dresult;
        float fresult;
        char cresult;
        
        /* Complex addressing with mixed types */
        asm volatile (
            "movsd %[in_double], %%xmm0\n\t"
            "addsd %[v_double], %%xmm0\n\t"
            "movsd %%xmm0, %[out_double]\n\t"
            "movss %[in_float], %%xmm1\n\t"
            "mulss %[v_float], %%xmm1\n\t"
            "movss %%xmm1, %[out_float]\n\t"
            "mov %[addr_var], %%rax\n\t"
            "movb (%%rax), %%al\n\t"
            "addb $1, %%al\n\t"
            "movb %%al, %[out_char]"
            : [out_double] "=m" (dresult),   /* Memory output */
              [out_float] "=m" (fresult),    /* Memory output */
              [out_char] "=r" (cresult)
            : [in_double] "m" (nvd1),        /* Memory input */
              [v_double] "m" (vd1),          /* Memory input */
              [in_float] "m" (nvf1),         /* Memory input */
              [v_float] "m" (vf1),           /* Memory input */
              [addr_var] "r" (&addr_var3)    /* Register containing address */
            : "rax", "xmm0", "xmm1", "memory", "cc"
        );
        
        checksum += (int)dresult + (int)fresult + cresult;
        nvd1 = dresult;
        nvf1 = fresult;
    }
    
    /* Block 4: Large clobber list and many operands */
block4:
    {
        int out1, out2, out3, out4, out5;
        
        /* Assembly with many input/output operands */
        asm volatile (
            "movl %[a1], %%eax\n\t"
            "addl %[a2], %%eax\n\t"
            "movl %%eax, %[o1]\n\t"
            "movl %[a3], %%ebx\n\t"
            "subl %[a4], %%ebx\n\t"
            "movl %%ebx, %[o2]\n\t"
            "movl %[a5], %%ecx\n\t"
            "imull %[a6], %%ecx\n\t"
            "movl %%ecx, %[o3]\n\t"
            "movl %[a7], %%edx\n\t"
            "xorl %[a8], %%edx\n\t"
            "movl %%edx, %[o4]\n\t"
            "movl %[a9], %%esi\n\t"
            "orl %[a10], %%esi\n\t"
            "movl %%esi, %[o5]"
            : [o1] "=r" (out1),
              [o2] "=r" (out2),
              [o3] "=r" (out3),
              [o4] "=r" (out4),
              [o5] "=r" (out5)
            : [a1] "r" (nv1),
              [a2] "r" (nv2),
              [a3] "r" (nv3),
              [a4] "r" (nv4),
              [a5] "r" (nv5),
              [a6] "r" (v1),
              [a7] "r" (v2),
              [a8] "r" (v3),
              [a9] "r" (v4),
              [a10] "r" (v5)
            : "rax", "rbx", "rcx", "rdx", "rsi", "memory", "cc"
        );
        
        checksum += out1 + out2 + out3 + out4 + out5;
    }
    
    /* Final block: Use goto to create complex control flow with live values */
    if (checksum > 0) {
        goto block1;  /* Create loop with many live values */
    }
    
    /* Use all variables to prevent dead code elimination */
    checksum += v1 + v2 + v3 + v4 + v5;
    checksum += vl1 + vl2 + vl3;
    checksum += (int)vf1 + (int)vf2 + (int)vf3;
    checksum += (int)vd1 + (int)vd2;
    checksum += vc1 + vc2;
    checksum += vs1 + vs2;
    checksum += nv1 + nv2 + nv3 + nv4 + nv5;
    checksum += (int)nvl1 + (int)nvl2;
    checksum += (int)nvf1 + (int)nvf2;
    checksum += (int)nvd1;
    checksum += reg_var1 + reg_var2 + reg_var3;
    checksum += addr_var1 + addr_var2 + addr_var3;
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
