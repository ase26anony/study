/* Test program to trigger GCC reload pass uncovered lines */
#include <stdio.h>
#include <stdlib.h>

/* Force specific register usage to create conflicts */
register long reg_var1 asm ("r12");
register int reg_var2 asm ("r13");
register short reg_var3 asm ("r14");
register char reg_var4 asm ("r15");

/* Volatile variables to prevent optimization */
volatile int vol_int1, vol_int2, vol_int3, vol_int4, vol_int5;
volatile long vol_long1, vol_long2, vol_long3;
volatile float vol_float1, vol_float2, vol_float3;
volatile double vol_double1, vol_double2;
volatile char vol_char1, vol_char2, vol_char3;
volatile short vol_short1, vol_short2;

int main(void) {
    int i, j, k, l, m, n, o, p, q, r;
    long a, b, c, d, e, f;
    float x, y, z;
    double dx, dy, dz;
    char ch1, ch2, ch3;
    short s1, s2, s3;
    
    /* Initialize many variables to create live ranges */
    vol_int1 = 12345; vol_int2 = 67890; vol_int3 = 13579;
    vol_long1 = 0x12345678L; vol_long2 = 0x87654321L;
    vol_float1 = 3.14159f; vol_float2 = 2.71828f;
    vol_double1 = 1.41421356; vol_double2 = 1.73205080;
    
    i = vol_int1 + 100; j = vol_int2 - 200; k = vol_int3 * 3;
    a = vol_long1 >> 2; b = vol_long2 << 1;
    x = vol_float1 * 2.0f; y = vol_float2 / 2.0f;
    dx = vol_double1 + 1.0; dy = vol_double2 - 0.5;
    
    /* Initialize register variables */
    reg_var1 = a + b;
    reg_var2 = i + j;
    reg_var3 = (short)(k & 0xFFFF);
    reg_var4 = 'A';
    
    /* Block 1: Complex inline assembly with many clobbers */
block1:
    {
        int temp1 = i + j;
        long temp2 = a * b;
        float temp3 = x + y;
        
        /* Inline assembly that clobbers many registers */
        asm volatile (
            "movl %[in1], %%eax\n\t"
            "addl %[in2], %%eax\n\t"
            "movl %%eax, %[out1]\n\t"
            "movq %[in3], %%rbx\n\t"
            "subq %[in4], %%rbx\n\t"
            "movq %%rbx, %[out2]\n\t"
            "movss %[in5], %%xmm0\n\t"
            "mulss %[in6], %%xmm0\n\t"
            "movss %%xmm0, %[out3]"
            : [out1] "=m" (vol_int4), 
              [out2] "=m" (vol_long3),
              [out3] "=m" (vol_float3)
            : [in1] "r" (temp1),
              [in2] "r" (k),
              [in3] "r" (temp2),
              [in4] "r" (reg_var1),  /* Conflicts with r12 */
              [in5] "x" (temp3),
              [in6] "x" (vol_float1)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13",
              "r14", "r15", "xmm0", "xmm1", "xmm2", 
              "xmm3", "xmm4", "xmm5", "memory", "cc"
        );
        
        /* Modify variables to keep them live */
        i = vol_int4 + 1;
        a = vol_long3 - 1;
        x = vol_float3 * 2.0f;
    }
    
    /* Force spill by using many variables */
    ch1 = (char)(i & 0xFF);
    s1 = (short)(j & 0xFFFF);
    l = i * j + k;
    m = j * k - i;
    n = k * i + j;
    
    /* Block 2: More assembly with addressing mode conflicts */
block2:
    {
        int *ptr1 = &vol_int1;
        long *ptr2 = &vol_long1;
        float *ptr3 = &vol_float1;
        
        /* Assembly using both memory and register constraints */
        asm volatile (
            "movl (%[addr1]), %%ecx\n\t"
            "addl %[val1], %%ecx\n\t"
            "movl %%ecx, (%[addr2])\n\t"
            "movq (%[addr3]), %%r8\n\t"
            "subq %[val2], %%r8\n\t"
            "movq %%r8, (%[addr4])"
            : 
            : [addr1] "r" (ptr1),
              [val1] "r" (reg_var2),  /* In r13 */
              [addr2] "r" (&vol_int5),
              [addr3] "r" (ptr2),
              [val2] "r" (reg_var1),  /* In r12 */
              [addr4] "r" (&vol_long2)
            : "rcx", "r8", "r9", "r10", "r11", "r12", "r13",
              "memory", "cc"
        );
        
        /* Use the results */
        o = vol_int5 + vol_int1;
        c = vol_long2 - vol_long1;
    }
    
    /* Block 3: Mixed data types and modes */
block3:
    {
        char ch_temp = ch1 + 1;
        short s_temp = s1 - 1;
        int i_temp = l + m;
        
        /* Assembly with different sized operands */
        asm volatile (
            "movb %[ch_in], %%al\n\t"
            "addb $5, %%al\n\t"
            "movb %%al, %[ch_out]\n\t"
            "movw %[s_in], %%bx\n\t"
            "subw $10, %%bx\n\t"
            "movw %%bx, %[s_out]\n\t"
            "movl %[i_in], %%ecx\n\t"
            "imull $3, %%ecx\n\t"
            "movl %%ecx, %[i_out]"
            : [ch_out] "=m" (vol_char2),
              [s_out] "=m" (vol_short2),
              [i_out] "=m" (vol_int3)
            : [ch_in] "r" ((int)ch_temp),
              [s_in] "r" ((int)s_temp),
              [i_in] "r" (i_temp)
            : "rax", "rbx", "rcx", "rdx", "memory", "cc"
        );
        
        ch2 = vol_char2;
        s2 = vol_short2;
        p = vol_int3;
    }
    
    /* Block 4: Complex constraints with earlyclobber */
block4:
    {
        double d_temp = dx + dy;
        int idx1 = p + o;
        int idx2 = n + m;
        
        /* Use '&' constraint to force early clobber */
        asm volatile (
            "movq %[din], %%xmm6\n\t"
            "addsd %[dadd], %%xmm6\n\t"
            "movq %%xmm6, %[dout]\n\t"
            "leal (%[idx1], %[idx2]), %%eax\n\t"
            "movl %%eax, %[iout]"
            : [dout] "=m" (vol_double2),
              [iout] "=m" (vol_int4)
            : [din] "x" (d_temp),
              [dadd] "x" (vol_double1),
              [idx1] "r" (idx1),
              [idx2] "r" (idx2)
            : "rax", "xmm6", "xmm7", "memory", "cc"
        );
        
        dz = vol_double2;
        q = vol_int4;
    }
    
    /* Block 5: Final assembly with output in specific register */
block5:
    {
        int final_calc = q + p + o + n + m + l;
        
        /* Try to force output into a specific register */
        register int out_reg asm ("r10") = 0;
        
        asm volatile (
            "movl %[in], %%r10d\n\t"
            "addl $0x1234, %%r10d\n\t"
            : "=&r" (out_reg)  /* Earlyclobber */
            : [in] "r" (final_calc)
            : "cc"
        );
        
        r = out_reg;
    }
    
    /* Create a checksum to prevent dead code elimination */
    volatile int checksum = 0;
    checksum += i + j + k + l + m + n + o + p + q + r;
    checksum += (int)a + (int)b + (int)c;
    checksum += (int)x + (int)y;
    checksum += (int)dx + (int)dy + (int)dz;
    checksum += ch1 + ch2 + s1 + s2;
    checksum += reg_var1 + reg_var2 + reg_var3 + reg_var4;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
