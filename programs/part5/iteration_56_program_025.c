/* test_early_remat.c - Program to trigger virtual register creation in GCC's early rematerialization pass */

#include <stdint.h>
#include <stdio.h>

/* Global data for address calculations */
static int global_array[256];
static long global_counter = 0;
static volatile int volatile_global = 0;

/* Prevent optimizations from removing our complex patterns */
#define NOINLINE __attribute__((noinline, noclone))
#define KEEP_ALIVE asm volatile("" : : "r"(result) : "memory")

/* Function A: Loop with invariants and expensive constants */
NOINLINE static long func_loop_invariants(int iterations, int *data) {
    /* Large immediate constants that can't be encoded in single instructions */
    const long EXPENSIVE_CONST1 = 0x123456789ABCDEF0ULL;
    const long EXPENSIVE_CONST2 = 0xFEDCBA9876543210ULL;
    const long EXPENSIVE_CONST3 = 0xDEADBEEFCAFEBABEULL;
    
    /* Many local variables with overlapping live ranges */
    register long r1 asm("ebx") = EXPENSIVE_CONST1;
    register long r2 asm("esi") = EXPENSIVE_CONST2;
    register long r3 asm("edi") = EXPENSIVE_CONST3;
    long sum = 0;
    int i, j;
    
    /* Complex loop with invariant calculations */
    for (i = 0; i < iterations; i++) {
        /* Use invariants in multiple places */
        long addr1 = (long)(data + i) * r1;
        long addr2 = (long)(data + i * 2) * r2;
        long addr3 = (long)(data + i * 3) * r3;
        
        /* Nested loop to extend live ranges */
        for (j = 0; j < 8; j++) {
            /* Use all invariants in complex expressions */
            sum += (addr1 >> j) & 1;
            sum += (addr2 >> (j * 2)) & 1;
            sum += (addr3 >> (j * 3)) & 1;
            
            /* More register pressure */
            sum += global_array[(i + j) & 0xFF];
        }
        
        /* Conditional that uses invariants */
        if (i & 1) {
            sum += r1 & 0xFFFF;
        } else {
            sum += r2 & 0xFFFF;
        }
        
        /* Use the third invariant in another computation */
        sum ^= r3 >> (i * 4);
    }
    
    /* Force all register variables to be used at the end */
    asm volatile("" : : "r"(r1), "r"(r2), "r"(r3));
    return sum;
}

/* Function B: Inline assembly with clobbered registers */
NOINLINE static long func_asm_clobber(int a, int b, int c) {
    long result = 0;
    int temp1, temp2, temp3, temp4, temp5, temp6;
    
    /* Many temporaries with overlapping lives */
    temp1 = a * 0x12345678;
    temp2 = b * 0x9ABCDEF0;
    temp3 = c * 0xFEDCBA98;
    
    /* Inline asm with multiple outputs and many clobbers */
    asm volatile (
        "movl %[t1], %%eax\n\t"
        "movl %[t2], %%ebx\n\t"
        "movl %[t3], %%ecx\n\t"
        "imull %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl $0xDEADBEEF, %%edx\n\t"
        "movl %%edx, %[out2]"
        : [out1] "=&r" (temp4), [out2] "=&r" (temp5)
        : [t1] "r" (temp1), [t2] "r" (temp2), [t3] "r" (temp3)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    /* More operations to extend live ranges */
    temp6 = temp4 + temp5;
    
    /* Second asm with different clobbers */
    asm volatile (
        "movl %[in1], %%esi\n\t"
        "movl %[in2], %%edi\n\t"
        "leal (%%esi,%%edi,4), %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=r" (result)
        : [in1] "r" (temp4), [in2] "r" (temp6)
        : "eax", "esi", "edi", "memory"
    );
    
    /* Use all temporaries in final calculation */
    result += temp1 + temp2 + temp3 + temp4 + temp5 + temp6;
    
    /* Clobber more registers */
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp");
    
    return result;
}

/* Function C: Complex control flow with register variables */
NOINLINE static long func_complex_flow(int selector, int count) {
    /* Declare register variables */
    register int rv1 asm("ebx") = selector * 0x11111111;
    register int rv2 asm("esi") = selector * 0x22222222;
    register int rv3 asm("edi") = selector * 0x33333333;
    
    long total = 0;
    int i;
    
    /* Labels for computed goto */
    void *labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    /* Complex loop with switch inside */
    for (i = 0; i < count; i++) {
        int mod = i % 5;
        
        /* Computed goto creates complex control flow */
        goto *labels[mod];
        
    label0:
        total += rv1 + i * 0x44444444;
        total += global_array[i & 0xFF];
        continue;
        
    label1:
        total += rv2 - i * 0x55555555;
        total ^= volatile_global;
        continue;
        
    label2:
        total += rv3 | i * 0x66666666;
        /* Nested conditional */
        if (total & 1) {
            total += rv1 * rv2;
        } else {
            total += rv2 * rv3;
        }
        continue;
        
    label3:
        total += (rv1 ^ rv2 ^ rv3) * i;
        /* Another level of nesting */
        switch (i & 3) {
            case 0: total += 0x77777777; break;
            case 1: total += 0x88888888; break;
            case 2: total += 0x99999999; break;
            case 3: total += 0xAAAAAAAA; break;
        }
        continue;
        
    label4:
        total += rv1 + rv2 + rv3 + i * 0xBBBBBBBB;
        /* Use builtin that returns in specific registers */
        {
            unsigned long long tsc = __builtin_ia32_rdtsc();
            total += (tsc >> 32) + (tsc & 0xFFFFFFFF);
        }
        continue;
    }
    
    /* Force register variables to stay alive */
    asm volatile("" : : "r"(rv1), "r"(rv2), "r"(rv3));
    
    return total;
}

/* Function D: Mixed patterns for maximum pressure */
NOINLINE static long func_mixed_patterns(double *array, int size) {
    /* Many different types of variables */
    const double INVARIANT1 = 3.141592653589793;
    const double INVARIANT2 = 2.718281828459045;
    const long LARGE_CONST = 0x1234567890ABCDEF;
    
    register double r1 asm("xmm0") = INVARIANT1;
    register double r2 asm("xmm1") = INVARIANT2;
    register long r3 asm("ebx") = LARGE_CONST;
    
    double sum_d = 0.0;
    long sum_l = 0;
    int i;
    
    /* Loop with mixed computations */
    for (i = 0; i < size; i++) {
        /* Use invariants in FP calculations */
        double temp1 = array[i] * r1;
        double temp2 = array[i] * r2;
        
        /* Integer calculations with large constant */
        long temp3 = (long)(temp1 * 1000) + r3;
        long temp4 = (long)(temp2 * 1000) ^ r3;
        
        /* Inline asm mixing FP and integer */
        asm volatile (
            "cvtsd2si %[d1], %%eax\n\t"
            "cvtsd2si %[d2], %%ecx\n\t"
            "addl %%ecx, %%eax\n\t"
            "movl %%eax, %[out]"
            : [out] "=r" (sum_l)
            : [d1] "x" (temp1), [d2] "x" (temp2)
            : "eax", "ecx", "memory"
        );
        
        sum_d += temp1 + temp2;
        
        /* Conditional with register variables */
        if (i & 1) {
            sum_l += r3 >> (i & 31);
        } else {
            sum_l -= r3 << (i & 31);
        }
    }
    
    /* Combine results using all register variables */
    long result = (long)sum_d + sum_l + r3;
    
    /* Clobber FP and integer registers */
    asm volatile("" : : : "xmm0", "xmm1", "xmm2", "eax", "ebx", "ecx", "edx");
    
    return result;
}

/* Main function that calls all test patterns */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 0x1234567;
    }
    
    long total_result = 0;
    
    /* Call each function with arguments that create register pressure */
    total_result += func_loop_invariants(
        argc > 1 ? 100 : 50, 
        global_array
    );
    
    total_result += func_asm_clobber(
        0x11111111,
        0x22222222,
        0x33333333
    );
    
    total_result += func_complex_flow(
        argc,
        argc > 1 ? 100 : 75
    );
    
    /* Create array for FP function */
    double fp_array[128];
    for (int i = 0; i < 128; i++) {
        fp_array[i] = i * 0.123456789;
    }
    
    total_result += func_mixed_patterns(fp_array, 128);
    
    /* Use result to prevent dead code elimination */
    volatile_global = (int)(total_result & 0xFFFFFFFF);
    
    printf("Result: %ld\n", total_result);
    return (int)(total_result & 0x7FFFFFFF);
}
