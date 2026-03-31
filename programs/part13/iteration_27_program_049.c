/* Test program to trigger GCC reload pass uncovered block */
#include <stdio.h>
#include <stdint.h>

/* Force many live values across complex control flow */
int main(void) {
    /* Volatile variables to prevent optimization */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile long vl1 = 100, vl2 = 200, vl3 = 300;
    volatile float vf1 = 1.5f, vf2 = 2.5f, vf3 = 3.5f;
    volatile double vd1 = 10.5, vd2 = 20.5;
    
    /* Non-volatile variables with complex live ranges */
    int nv1 = 10, nv2 = 20, nv3 = 30, nv4 = 40, nv5 = 50;
    long nvl1 = 1000, nvl2 = 2000;
    float nvf1 = 100.5f, nvf2 = 200.5f;
    double nvd1 = 1000.5;
    
    /* Explicit register variables - pin to specific registers */
    register int r12_var asm ("r12") = 0x12345678;
    register int r13_var asm ("r13") = 0x87654321;
    register int r14_var asm ("r14") = 0xABCDEF01;
    
    /* Variables for address-taking */
    int addr_var1 = 111, addr_var2 = 222, addr_var3 = 333;
    int *addr_ptr1 = &addr_var1;
    int *addr_ptr2 = &addr_var2;
    
    /* Mixed size variables for mode conflicts */
    char c1 = 'A', c2 = 'B';
    short s1 = 1000, s2 = 2000;
    int i1 = 30000, i2 = 40000;
    long l1 = 50000, l2 = 60000;
    
    /* Complex checksum to prevent dead code elimination */
    volatile int64_t checksum = 0;
    
    /* Block 1: Many live values, complex inline asm */
block1:
    /* Force register pressure with many operations */
    nv1 = v1 + nv2;
    nv2 = v2 * nv3;
    nv3 = v3 - nv4;
    nv4 = v4 / (nv5 ? nv5 : 1);
    nv5 = v5 ^ nv1;
    
    /* Inline asm with conflicting constraints */
    asm volatile (
        /* Output operands with early clobber */
        "mov %[out1], %[in1]\n\t"
        "add %[out2], %[in2]\n\t"
        "xor %[out3], %[in3]\n\t"
        : [out1] "=&r" (nv1),    /* Early clobber - can't share reg with inputs */
          [out2] "=r" (nv2),
          [out3] "=r" (nv3)
        : [in1] "r" (v1),
          [in2] "r" (v2),
          [in3] "r" (v3),
          "m" (*addr_ptr1),      /* Memory constraint - force address reload */
          "m" (*addr_ptr2)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "memory", "cc"
    );
    
    /* Modify address-taken variables */
    addr_var1 = nv1 + nv2;
    addr_var2 = nv3 * nv4;
    
    /* Use explicit register variables */
    r12_var = r12_var ^ nv1;
    r13_var = r13_var + nv2;
    r14_var = r14_var - nv3;
    
    goto block2;
    
    /* Unreachable but creates more live range complexity */
    nv1 = v1 * 2;
    nv2 = v2 * 3;
    
    /* Block 2: Different constraints, clobbers same registers */
block2:
    /* More operations to keep variables live */
    nvl1 = vl1 + nv1;
    nvl2 = vl2 - nv2;
    nvf1 = vf1 * nv3;
    nvf2 = vf2 / (nv4 ? nv4 : 1.0f);
    nvd1 = vd1 + nv5;
    
    /* Inline asm with memory output and register input */
    asm volatile (
        /* Mixed constraints causing reloads */
        "mov %[memout], %[regin]\n\t"
        "lea (%[base],%[index],4), %[addrout]\n\t"
        : [memout] "=m" (addr_var3),     /* Memory output */
          [addrout] "=r" (addr_ptr1)     /* Register output */
        : [regin] "r" (nv1),             /* Register input */
          [base] "r" (addr_ptr2),
          [index] "r" (nv2),
          "m" (addr_var1),               /* Memory input */
          "m" (addr_var2)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15",    /* Clobber explicit register vars */
          "memory", "cc"
    );
    
    /* Mixed data type operations */
    c1 = (char)(nv1 & 0xFF);
    s1 = (short)(nv2 & 0xFFFF);
    i1 = nv3;
    l1 = (long)nv4 * nv5;
    
    /* Block 3: Complex addressing modes */
block3:
    /* Force spills with many live values */
    float temp_f = nvf1 + nvf2;
    double temp_d = nvd1 * 2.0;
    
    /* Inline asm with multiple output constraints */
    asm volatile (
        /* Complex constraints that likely need reloads */
        "imul %[in1], %[out1]\n\t"
        "add %[in2], %[out2]\n\t"
        "mov %[out3], %[in3]\n\t"
        : [out1] "+&r" (nv1),    /* Read-write with early clobber */
          [out2] "=r" (nv2),
          [out3] "=r" (nv3)
        : [in1] "r" (v4),
          [in2] "r" (v5),
          [in3] "r" (vl1),
          "m" (c1),              /* Different sized memory operands */
          "m" (s1),
          "m" (i1),
          "m" (l1)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory", "cc"
    );
    
    /* Final computations */
    checksum = (int64_t)v1 + v2 + v3 + v4 + v5;
    checksum += vl1 + vl2 + vl3;
    checksum += (int64_t)(vf1 * 100) + (int64_t)(vf2 * 100) + (int64_t)(vf3 * 100);
    checksum += (int64_t)(vd1 * 100) + (int64_t)(vd2 * 100);
    checksum += nv1 + nv2 + nv3 + nv4 + nv5;
    checksum += nvl1 + nvl2;
    checksum += (int64_t)(nvf1 * 100) + (int64_t)(nvf2 * 100);
    checksum += (int64_t)(nvd1 * 100);
    checksum += r12_var + r13_var + r14_var;
    checksum += addr_var1 + addr_var2 + addr_var3;
    checksum += c1 + s1 + i1 + l1;
    
    printf("Checksum: %lld\n", (long long)checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
