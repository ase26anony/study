/* test_early_remat.c - Program to trigger virtual register creation in early rematerialization */

#include <stdint.h>
#include <stdio.h>

/* Global arrays for address calculations */
static int global_array[1024];
static double global_double[512];
static char global_char[2048];

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int func_loop_invariants(int start, int end, int stride) {
    volatile int result = 0;
    int i, j;
    
    /* Large immediate constants that can't be encoded in single instructions */
    const long long big_const_1 = 0x123456789ABCDEF0LL;
    const long long big_const_2 = 0xFEDCBA9876543210LL;
    const unsigned long large_mask = 0xFFFFFFFF00000000UL;
    
    /* Invariant pointers used in loop */
    int *invariant_ptr1 = &global_array[0];
    double *invariant_ptr2 = &global_double[0];
    char *invariant_ptr3 = &global_char[0];
    
    /* Create many overlapping live ranges */
    int temp1 = start * 3;
    int temp2 = start + 100;
    int temp3 = start - 50;
    long long temp4 = big_const_1;
    long long temp5 = big_const_2;
    unsigned long temp6 = large_mask;
    
    for (i = start; i < end; i += stride) {
        /* Use invariants in multiple places with different calculations */
        int idx1 = (i * 7) % 1024;
        int idx2 = (i * 13) % 512;
        int idx3 = (i * 29) % 2048;
        
        /* Force register pressure by using many temporaries */
        int calc1 = invariant_ptr1[idx1] * temp1;
        int calc2 = (int)(invariant_ptr2[idx2] * temp2);
        int calc3 = invariant_ptr3[idx3] + temp3;
        
        /* Use large constants in calculations */
        long long mix1 = (temp4 * calc1) >> 8;
        long long mix2 = (temp5 * calc2) >> 8;
        unsigned long mix3 = (temp6 & (calc3 * 0x1000UL));
        
        /* Complex condition with invariants */
        if ((mix1 > mix2) && ((mix3 & 0xFF) != 0)) {
            result += calc1 + calc2 + calc3;
        } else {
            result -= calc1 - calc2 + calc3;
        }
        
        /* Modify temporaries to extend live ranges */
        temp1 = (temp1 * 3 + i) & 0xFFF;
        temp2 = (temp2 * 5 - i) & 0xFFF;
        temp3 = (temp3 * 7 + i * 2) & 0xFFF;
        
        /* Nested loop to increase pressure */
        for (j = 0; j < 4; j++) {
            int nested_temp = temp1 + temp2 + temp3 + j;
            double nested_calc = invariant_ptr2[(idx2 + j) % 512] * nested_temp;
            result += (int)nested_calc;
        }
    }
    
    return result;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int a, int b, int c) {
    int result1, result2, result3;
    int temp;
    
    /* Declare register variables to force specific allocation */
    register int reg_var1 asm("eax") = a;
    register int reg_var2 asm("ebx") = b;
    register int reg_var3 asm("ecx") = c;
    
    /* Complex inline asm with multiple outputs and clobbers */
    asm volatile (
        "movl %[reg1], %%eax\n\t"
        "movl %[reg2], %%ebx\n\t"
        "movl %[reg3], %%ecx\n\t"
        "imull %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%%ebx,%%ecx,2), %%edx\n\t"
        "movl %%edx, %[out2]\n\t"
        "xorl %%esi, %%esi\n\t"
        "movl %%esi, %[out3]"
        : [out1] "=r" (result1), [out2] "=r" (result2), [out3] "=r" (result3)
        : [reg1] "r" (reg_var1), [reg2] "r" (reg_var2), [reg3] "r" (reg_var3)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory", "cc"
    );
    
    /* Use results in complex expressions */
    temp = result1 * 3 + result2 * 5 - result3 * 7;
    
    /* Another asm with different clobbers */
    asm volatile (
        "cpuid"
        : "=a" (result1), "=b" (result2), "=c" (result3), "=d" (temp)
        : "a" (0)
        : "memory"
    );
    
    return result1 + result2 + result3 + temp;
}

/* Function C: Complex control flow with switch and computed goto */
__attribute__((noinline, noclone))
int func_complex_control(int base, int iterations) {
    static void *label_table[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    int i, state = 0;
    int acc = 0;
    
    /* Many local variables with overlapping lives */
    int var1 = base * 2;
    int var2 = base + 100;
    int var3 = base - 50;
    int var4 = base * 3;
    int var5 = base / 2;
    int var6 = base % 7;
    int var7 = base ^ 0x55;
    int var8 = base | 0xAA;
    int var9 = base & 0xF0;
    int var10 = ~base;
    
    for (i = 0; i < iterations; i++) {
        /* Complex switch with many cases */
        switch (state) {
            case 0:
                acc += var1 * var2;
                var1 = (var1 * 3 + i) & 0xFF;
                state = 1;
                break;
            case 1:
                acc += var3 * var4;
                var2 = (var2 * 5 - i) & 0xFF;
                state = 2;
                break;
            case 2:
                acc += var5 * var6;
                var3 = (var3 * 7 + i) & 0xFF;
                state = 3;
                break;
            case 3:
                acc += var7 * var8;
                var4 = (var4 * 11 ^ i) & 0xFF;
                state = 4;
                break;
            case 4:
                acc += var9 * var10;
                var5 = (var5 * 13 | i) & 0xFF;
                state = 0;
                break;
        }
        
        /* Computed goto for additional control flow complexity */
        if (i % 5 == 0) {
            goto *label_table[i % 5];
        }
        
    label0:
        var6 = (var6 + var1) & 0xFF;
        continue;
    label1:
        var7 = (var7 + var2) & 0xFF;
        continue;
    label2:
        var8 = (var8 + var3) & 0xFF;
        continue;
    label3:
        var9 = (var9 + var4) & 0xFF;
        continue;
    label4:
        var10 = (var10 + var5) & 0xFF;
        continue;
    }
    
    /* Use all variables one more time to extend live ranges */
    return acc + var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8 + var9 + var10;
}

/* Function D: Mixed patterns with builtins */
__attribute__((noinline, noclone))
unsigned long long func_builtins_mix(int seed) {
    /* Use timestamp counter builtin (returns in edx:eax on x86) */
    unsigned long long tsc1 = __builtin_ia32_rdtsc();
    
    /* Create many calculations to use the result */
    unsigned int low1 = (unsigned int)tsc1;
    unsigned int high1 = (unsigned int)(tsc1 >> 32);
    
    int i;
    unsigned long long acc = 0;
    
    /* Force many live values */
    unsigned int temp1 = low1 * seed;
    unsigned int temp2 = high1 ^ seed;
    unsigned int temp3 = low1 + high1;
    unsigned int temp4 = low1 - high1;
    unsigned int temp5 = low1 & high1;
    unsigned int temp6 = low1 | high1;
    unsigned int temp7 = ~low1;
    unsigned int temp8 = ~high1;
    
    for (i = 0; i < 100; i++) {
        /* Complex expression using all temporaries */
        unsigned int calc = ((temp1 * temp2) + (temp3 * temp4) - 
                           (temp5 * temp6) ^ (temp7 & temp8)) * i;
        
        acc += calc;
        
        /* Modify temporaries to keep them live */
        temp1 = (temp1 * 3 + i) & 0xFFFF;
        temp2 = (temp2 * 5 - i) & 0xFFFF;
        temp3 = (temp3 * 7 + i) & 0xFFFF;
        temp4 = (temp4 * 11 ^ i) & 0xFFFF;
        temp5 = (temp5 * 13 | i) & 0xFFFF;
        temp6 = (temp6 * 17 + i * 2) & 0xFFFF;
        
        /* Another rdtsc to create more hard register references */
        if (i % 10 == 0) {
            unsigned long long tsc2 = __builtin_ia32_rdtsc();
            temp7 = (unsigned int)tsc2;
            temp8 = (unsigned int)(tsc2 >> 32);
        }
    }
    
    return acc;
}

/* Main function that calls all test functions */
int main(int argc, char **argv) {
    int i;
    int total = 0;
    
    /* Initialize global arrays */
    for (i = 0; i < 1024; i++) {
        global_array[i] = i * 3;
    }
    for (i = 0; i < 512; i++) {
        global_double[i] = i * 1.5;
    }
    for (i = 0; i < 2048; i++) {
        global_char[i] = i & 0xFF;
    }
    
    /* Call all test functions with different arguments */
    total += func_loop_invariants(0, 100, 3);
    total += func_asm_clobber(100, 200, 300);
    total += func_complex_control(500, 50);
    
    unsigned long long builtin_result = func_builtins_mix(1234);
    total += (int)(builtin_result & 0xFFFFFFFF);
    
    /* Additional calls with different parameters to increase coverage */
    for (i = 0; i < 5; i++) {
        total += func_loop_invariants(i * 20, i * 20 + 50, i + 1);
        total += func_complex_control(i * 100, 20);
    }
    
    printf("Total result: %d\n", total);
    return total & 0xFF;
}
