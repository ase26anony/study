/* Test program to trigger early rematerialization virtual register creation */
/* Compile with: gcc -O2 -m32 -fno-optimize-sibling-calls -fdump-rtl-early-remat test.c -o test */

#include <stdint.h>
#include <stdlib.h>

/* Global arrays to create address calculation opportunities */
static int global_array1[1024];
static int global_array2[1024];
static int global_array3[1024];
static long long global_array4[512];

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int func_loop_invariants(int start, int end, int step) {
    /* Large immediate constants that need rematerialization */
    const long long big_const1 = 0x123456789ABCDEF0LL;
    const long long big_const2 = 0xFEDCBA9876543210LL;
    const int large_imm = 0x7FFFFFFF;
    const unsigned int mask = 0xFFFF0000;
    
    /* Invariant pointers used in loop */
    int *invariant_ptr1 = &global_array1[0];
    int *invariant_ptr2 = &global_array2[0];
    long long *invariant_ptr3 = &global_array4[0];
    
    int sum = 0;
    int i, j;
    
    /* Complex loop with multiple invariants used in different places */
    for (i = start; i < end; i += step) {
        /* Use invariants in address calculations */
        int val1 = invariant_ptr1[i & 0x3FF];
        int val2 = invariant_ptr2[(i * 3) & 0x3FF];
        long long val3 = invariant_ptr3[(i / 2) & 0x1FF];
        
        /* Use expensive constants in non-adjacent computations */
        if (i & 1) {
            sum += (val1 & mask) + (int)(big_const1 >> (i & 0x3F));
        } else {
            sum += (val2 | large_imm) - (int)(big_const2 & 0xFFFFFFFF);
        }
        
        /* More computations with overlapping live ranges */
        int temp1 = val1 * val2;
        int temp2 = val1 + val2;
        int temp3 = val1 - val2;
        int temp4 = val1 ^ val2;
        
        /* Force all temporaries to stay live across loop iterations */
        sum += temp1 - temp2 + temp3 - temp4;
        
        /* Nested loop to increase register pressure */
        for (j = 0; j < 4; j++) {
            /* Use invariants again in nested context */
            int nested_val = invariant_ptr1[(i + j) & 0x3FF];
            sum += nested_val * j;
            
            /* More temporaries with overlapping lives */
            int nested_temp1 = nested_val + large_imm;
            int nested_temp2 = nested_val - (int)(big_const1 & 0xFFFF);
            sum += nested_temp1 * nested_temp2;
        }
    }
    
    return sum;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int a, int b, int c) {
    int result1, result2, result3, result4;
    
    /* Declare register variables to force specific allocation */
    register int r1 asm("eax") = a;
    register int r2 asm("ebx") = b;
    register int r3 asm("ecx") = c;
    register int r4 asm("edx") = a * b;
    
    /* Multi-output inline assembly with many clobbered registers */
    asm volatile (
        "movl %[r1], %%eax\n\t"
        "movl %[r2], %%ebx\n\t"
        "movl %[r3], %%ecx\n\t"
        "movl %[r4], %%edx\n\t"
        "addl %%ebx, %%eax\n\t"
        "subl %%ecx, %%edx\n\t"
        "imull %%edx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %%edx, %[out2]\n\t"
        "movl %%ebx, %[out3]\n\t"
        "movl %%ecx, %[out4]\n\t"
        : [out1] "=&r" (result1),
          [out2] "=&r" (result2),
          [out3] "=&r" (result3),
          [out4] "=&r" (result4)
        : [r1] "r" (r1),
          [r2] "r" (r2),
          [r3] "r" (r3),
          [r4] "r" (r4)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory", "cc"
    );
    
    /* Use results in complex expressions to create register pressure */
    int temp1 = result1 + result2;
    int temp2 = result3 - result4;
    int temp3 = result1 * result3;
    int temp4 = result2 / (result4 ? result4 : 1);
    int temp5 = result1 ^ result2 ^ result3 ^ result4;
    int temp6 = ~(result1 & result2);
    int temp7 = result3 | result4;
    int temp8 = result1 << (result2 & 0xF);
    
    /* Force all temporaries to be live simultaneously */
    return temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8;
}

/* Function C: Complex control flow with register variables */
__attribute__((noinline, noclone))
int func_complex_cf(int selector, int iterations) {
    /* Use register variables with complex control flow */
    register int reg_a asm("eax");
    register int reg_b asm("ebx");
    register int reg_c asm("ecx");
    register int reg_d asm("edx");
    
    /* Labels for computed goto */
    void *labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    int total = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Many temporaries with overlapping live ranges */
        int t1 = i * 3;
        int t2 = i + 0x7FFFFFFF;
        int t3 = i ^ 0x12345678;
        int t4 = i & 0xFFFF0000;
        int t5 = i | 0x0000FFFF;
        int t6 = ~i;
        int t7 = i << 3;
        int t8 = i >> 2;
        
        /* Assign to register variables */
        reg_a = t1 + t2;
        reg_b = t3 - t4;
        reg_c = t5 * t6;
        reg_d = t7 / (t8 ? t8 : 1);
        
        /* Complex switch inside loop */
        switch (selector & 0x3) {
            case 0:
                total += reg_a + reg_b;
                /* Fall through */
            case 1:
                total += reg_c - reg_d;
                /* Use builtin for hard register reference */
                {
                    unsigned long long tsc = __builtin_ia32_rdtsc();
                    total += (int)(tsc & 0xFFFFFFFF);
                }
                break;
            case 2:
                total += reg_a * reg_c;
                /* Computed goto */
                goto *labels[i & 0x3];
                label0:
                    total += reg_b;
                    break;
                label1:
                    total += reg_d;
                    break;
                label2:
                    total += reg_a;
                    break;
                label3:
                    total += reg_c;
                    break;
                label4:
                    total += reg_b + reg_d;
                    break;
            case 3:
                total += reg_b / (reg_d ? reg_d : 1);
                break;
        }
        
        /* More computations keeping temporaries live */
        int u1 = total + t1;
        int u2 = total - t2;
        int u3 = total * t3;
        int u4 = total ^ t4;
        
        total = u1 + u2 + u3 + u4;
        
        /* Nested conditional with more temporaries */
        if (i & 1) {
            int v1 = u1 * 2;
            int v2 = u2 / 2;
            int v3 = u3 + 0x1000;
            int v4 = u4 - 0x1000;
            total += v1 + v2 + v3 + v4;
        } else {
            int w1 = u1 & 0xFF00;
            int w2 = u2 | 0x00FF;
            int w3 = u3 ^ 0xAAAA;
            int w4 = u4 << 1;
            total += w1 + w2 + w3 + w4;
        }
    }
    
    return total;
}

/* Function D: Mixed patterns for maximum coverage */
__attribute__((noinline, noclone))
int func_mixed_patterns(double *data, int count) {
    /* Large immediate floating constants */
    const double big_float = 3.14159265358979323846 * 1e30;
    const double small_float = 2.71828182845904523536 / 1e30;
    
    /* Many local variables with overlapping scopes */
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0, acc4 = 0.0;
    double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
    int i;
    
    for (i = 0; i < count; i++) {
        /* Address calculation with large immediate offset */
        double *ptr1 = data + i;
        double *ptr2 = data + (i ^ 0x7FF);
        double *ptr3 = data + (i & 0x3FF);
        
        /* Load and compute with many temporaries */
        tmp1 = *ptr1 * big_float;
        tmp2 = *ptr2 / small_float;
        tmp3 = *ptr3 + big_float;
        tmp4 = *ptr1 - small_float;
        
        /* Cross-dependent computations */
        tmp5 = tmp1 + tmp2;
        tmp6 = tmp3 - tmp4;
        tmp7 = tmp1 * tmp3;
        tmp8 = tmp2 / (tmp4 != 0.0 ? tmp4 : 1.0);
        
        /* Update accumulators */
        acc1 += tmp5;
        acc2 += tmp6;
        acc3 += tmp7;
        acc4 += tmp8;
        
        /* Conditional with more computations */
        if (i & 1) {
            double extra1 = tmp5 * tmp6;
            double extra2 = tmp7 / tmp8;
            double extra3 = tmp1 + tmp4;
            double extra4 = tmp2 - tmp3;
            
            acc1 += extra1;
            acc2 += extra2;
            acc3 += extra3;
            acc4 += extra4;
        }
    }
    
    /* Final reduction */
    return (int)(acc1 + acc2 + acc3 + acc4);
}

/* Main function to drive all tests */
int main(int argc, char **argv) {
    /* Initialize global arrays */
    for (int i = 0; i < 1024; i++) {
        global_array1[i] = i;
        global_array2[i] = i * 2;
        if (i < 512) {
            global_array4[i] = i * 3LL;
        }
    }
    
    int total = 0;
    
    /* Call each test function with appropriate arguments */
    total += func_loop_invariants(0, 100, 1);
    total += func_asm_clobber(argc, 100, 200);
    total += func_complex_cf(argc, 50);
    
    /* Prepare data for mixed patterns function */
    double data[1024];
    for (int i = 0; i < 1024; i++) {
        data[i] = i * 0.5;
    }
    total += func_mixed_patterns(data, 100);
    
    /* Return combined result to prevent dead code elimination */
    return total & 0xFF;
}
