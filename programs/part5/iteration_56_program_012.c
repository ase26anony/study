/* test_early_remat.c - Target specific coverage for early-remat.cc lines 930-937 */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static double global_doubles[128];
static volatile int volatile_counter;

/* Prevent optimizations */
#define NOINLINE __attribute__((noinline, noclone))
#define USED __attribute__((used))

/* Function A: Loop with invariants and expensive constants */
NOINLINE static long func_loop_invariants(int iterations, int *data) {
    /* Large immediate constants that need rematerialization */
    const long EXPENSIVE_CONST1 = 0x7FFFFFFFFFFFFFFF;
    const long EXPENSIVE_CONST2 = 0x5555555555555555;
    const long EXPENSIVE_CONST3 = 0xAAAAAAAAAAAAAAAA;
    
    /* Loop invariants with complex addresses */
    int *invariant_ptr1 = &global_array[64];
    int *invariant_ptr2 = &global_array[128];
    double *invariant_ptr3 = &global_doubles[32];
    
    register long r1 asm("ebx") = 0;
    register long r2 asm("esi") = 0;
    register long r3 asm("edi") = 0;
    
    /* Create overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in multiple places */
        long val1 = (long)invariant_ptr1 + EXPENSIVE_CONST1;
        long val2 = (long)invariant_ptr2 + EXPENSIVE_CONST2;
        long val3 = (long)invariant_ptr3 + EXPENSIVE_CONST3;
        
        /* Complex address calculations keeping values live */
        int idx1 = (i * 13 + 7) & 0xFF;
        int idx2 = (i * 17 + 11) & 0xFF;
        int idx3 = (i * 19 + 13) & 0xFF;
        
        /* Use all values to create register pressure */
        r1 += data[idx1] * val1;
        r2 += data[idx2] * val2;
        r3 += data[idx3] * val3;
        
        /* More operations to extend live ranges */
        r1 ^= EXPENSIVE_CONST1;
        r2 ^= EXPENSIVE_CONST2;
        r3 ^= EXPENSIVE_CONST3;
        
        /* Use invariants again */
        volatile_counter = *invariant_ptr1 + *invariant_ptr2;
    }
    
    /* Mix results to prevent dead code elimination */
    return r1 + r2 + r3 + (long)invariant_ptr1 + (long)invariant_ptr2;
}

/* Function B: Inline assembly with clobbered registers */
NOINLINE static int func_asm_clobber(int a, int b, int c) {
    int result1, result2, result3;
    register int r4 asm("eax");
    register int r5 asm("edx");
    register int r6 asm("ecx");
    
    /* Initialize register variables */
    r4 = a * 7;
    r5 = b * 13;
    r6 = c * 19;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %[r4], %%eax\n\t"
        "movl %[r5], %%edx\n\t"
        "movl %[r6], %%ecx\n\t"
        "imull %%edx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %%edx, %[out2]\n\t"
        "movl %%ecx, %[out3]\n\t"
        : [out1] "=&r" (result1), [out2] "=&r" (result2), [out3] "=&r" (result3)
        : [r4] "r" (r4), [r5] "r" (r5), [r6] "r" (r6)
        : "eax", "edx", "ecx", "ebx", "esi", "edi", "memory", "cc"
    );
    
    /* Create more register pressure after asm */
    int temp1 = result1 * 31;
    int temp2 = result2 * 37;
    int temp3 = result3 * 41;
    
    /* Use builtins that return in specific registers */
    unsigned long long tsc;
    tsc = __builtin_ia32_rdtsc();  /* Returns in edx:eax */
    
    /* Use the TSC result to create register dependencies */
    temp1 ^= (tsc >> 32);
    temp2 ^= (tsc & 0xFFFFFFFF);
    
    return temp1 + temp2 + temp3 + (r4 ^ r5 ^ r6);
}

/* Function C: Complex control flow with switch */
NOINLINE static long func_complex_control(int mode, int count) {
    /* Many local variables with overlapping lifetimes */
    long v1 = 0x123456789ABCDEF0;
    long v2 = 0xFEDCBA9876543210;
    long v3 = 0x5555555555555555;
    long v4 = 0xAAAAAAAAAAAAAAAA;
    long v5 = 0x3333333333333333;
    long v6 = 0xCCCCCCCCCCCCCCCC;
    long v7 = 0x0F0F0F0F0F0F0F0F;
    long v8 = 0xF0F0F0F0F0F0F0F0;
    
    /* Labels for computed goto */
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    
    register long accumulator asm("ebx") = 0;
    
    for (int i = 0; i < count; i++) {
        /* Switch with multiple cases creating complex control flow */
        switch ((mode + i) % 5) {
            case 0:
                accumulator += v1 * v2;
                v1 ^= v3;
                v2 ^= v4;
                break;
            case 1:
                accumulator += v3 * v4;
                v3 ^= v5;
                v4 ^= v6;
                break;
            case 2:
                accumulator += v5 * v6;
                v5 ^= v7;
                v6 ^= v8;
                break;
            case 3:
                accumulator += v7 * v8;
                v7 ^= v1;
                v8 ^= v2;
                break;
            case 4:
                /* Computed goto */
                goto *labels[i % 5];
                L0: accumulator += v1;
                L1: accumulator += v2;
                L2: accumulator += v3;
                L3: accumulator += v4;
                L4: accumulator += v5;
                break;
        }
        
        /* More operations extending live ranges */
        v1 += i;
        v2 -= i;
        v3 ^= i;
        v4 |= i;
        v5 &= ~i;
        v6 <<= (i & 3);
        v7 >>= (i & 3);
        v8 = (v8 << 1) | (v8 >> 63);
    }
    
    /* Use all variables to prevent optimization */
    return accumulator + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

/* Function D: Mixed patterns for maximum pressure */
NOINLINE static long func_mixed_patterns(int *data, int size) {
    long total = 0;
    
    /* Large immediate that needs rematerialization */
    const long BIG_CONST = 0x7FFFFFFFFFFFFFFF;
    
    for (int i = 0; i < size; i++) {
        /* Multiple independent chains of computation */
        long chain1 = data[i];
        long chain2 = data[(i + 1) % size];
        long chain3 = data[(i + 2) % size];
        
        /* Each chain uses BIG_CONST at different points */
        for (int j = 0; j < 4; j++) {
            chain1 = (chain1 * 6364136223846793005ULL) ^ BIG_CONST;
            chain2 = (chain2 * 6364136223846793005ULL) ^ BIG_CONST;
            chain3 = (chain3 * 6364136223846793005ULL) ^ BIG_CONST;
            
            /* Inline asm with clobber in the middle */
            if ((j & 1) == 0) {
                asm volatile (
                    "cpuid\n\t"
                    : : : "eax", "ebx", "ecx", "edx", "memory"
                );
            }
        }
        
        total += chain1 + chain2 + chain3;
    }
    
    return total;
}

/* Main function that calls all test patterns */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 7;
    }
    for (int i = 0; i < 128; i++) {
        global_doubles[i] = i * 1.5;
    }
    
    /* Use command line args or defaults */
    int iterations = argc > 1 ? atoi(argv[1]) : 1000;
    int mode = argc > 2 ? atoi(argv[2]) : 3;
    
    /* Call all test functions to create various patterns */
    long result1 = func_loop_invariants(iterations, global_array);
    int result2 = func_asm_clobber(iterations, mode, iterations ^ mode);
    long result3 = func_complex_control(mode, iterations / 10);
    long result4 = func_mixed_patterns(global_array, 64);
    
    /* Combine results to prevent dead code elimination */
    volatile long final_result = 
        result1 + result2 + result3 + result4 + 
        (long)&global_array[0] + (long)&global_doubles[0];
    
    return (int)(final_result & 0x7FFFFFFF);
}
