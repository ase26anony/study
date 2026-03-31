/* test_early_remat.c - Target specific patterns for early rematerialization */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {0x7FFFFFFF, 0x80000000, 0x12345678, 0x9ABCDEF0};

/* Prevent optimizations from simplifying our patterns */
#define NOINLINE __attribute__((noinline, noclone))
#define KEEP_ALIVE(expr) asm volatile("" : : "r"(expr))

/* Function A: Loop with invariants and expensive constants */
NOINLINE static long func_loop_invariants(int iterations, int *data) {
    /* Many local variables with overlapping live ranges */
    register long r0 asm("eax") = iterations;
    register long r1 asm("ebx") = (long)data;
    register long r2 asm("ecx") = large_constants[0];
    register long r3 asm("edx") = large_constants[1];
    long sum = 0;
    
    /* Loop with invariant address calculation using expensive constants */
    for (int i = 0; i < iterations; i++) {
        /* Multiple uses of invariants in non-adjacent calculations */
        long addr1 = r1 + i * sizeof(int) + r2;
        long addr2 = r1 + (i * 2) * sizeof(int) + r3;
        
        /* Complex arithmetic creating register pressure */
        long val1 = *(int *)addr1 * r2;
        long val2 = *(int *)addr2 / (r3 & 0xFFFF);
        long val3 = (r2 >> 16) * (r3 << 8);
        long val4 = (0x123456789ABCDEF0LL >> i) & 0xFFFFFFFF;
        
        /* Overlapping live ranges */
        sum += val1 + val2 + val3 + val4;
        
        /* More register-intensive operations */
        r0 = (r0 * 1103515245 + 12345) & 0x7FFFFFFF;
        r1 = (r1 ^ r2) + (r3 << 3);
    }
    
    /* Use all register variables in final computation */
    return sum + r0 + r1 + r2 + r3;
}

/* Function B: Inline assembly with clobbered registers */
NOINLINE static int func_asm_clobber(int a, int b) {
    int result1, result2, result3;
    
    /* Multi-output inline assembly forcing hard register allocation */
    asm volatile (
        "movl %[a], %%eax\n\t"
        "movl %[b], %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%%eax, %%ebx, 4), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "xorl %%edx, %%edx\n\t"
        "divl %%ebx\n\t"
        "movl %%edx, %[out3]\n\t"
        : [out1] "=&r" (result1), 
          [out2] "=&r" (result2), 
          [out3] "=&r" (result3)
        : [a] "rm" (a), 
          [b] "rm" (b)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    /* More inline asm with different clobbers */
    register int r4 asm("esi"), r5 asm("edi");
    asm volatile (
        "cpuid\n\t"
        : "=a"(r4), "=d"(r5)
        : "a"(0)
        : "ebx", "ecx"
    );
    
    /* Use results in complex expressions */
    int x = (result1 * large_constants[2]) + (result2 / large_constants[3]);
    int y = (r4 ^ r5) * 0xDEADBEEF;
    
    /* Control flow to extend live ranges */
    switch (x & 3) {
        case 0: return y + result3;
        case 1: return y - result3;
        case 2: return y * result3;
        default: return y ^ result3;
    }
}

/* Function C: Complex control flow with register variables */
NOINLINE static long func_complex_flow(int n, const int *arr) {
    /* Many register variables with 'register' keyword */
    register long acc0 asm("eax") = 0;
    register long acc1 asm("ebx") = 0;
    register long acc2 asm("ecx") = 0;
    register long acc3 asm("edx") = 0;
    
    /* Labels for computed goto */
    static void *labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4};
    
    for (int i = 0; i < n; i++) {
        /* Complex switch inside loop */
        switch (i % 5) {
            case 0:
                acc0 += arr[i] * 0x100000001LL;
                acc1 -= arr[i] * 0x200000002LL;
                break;
            case 1:
                acc2 ^= arr[i] | 0x400000004LL;
                acc3 &= arr[i] ^ 0x800000008LL;
                break;
            case 2:
                /* Nested control flow */
                if (arr[i] > 0) {
                    acc0 = (acc0 << 2) | (acc1 >> 30);
                    acc1 = (acc1 << 3) ^ (acc2 >> 29);
                } else {
                    acc2 = (acc2 << 4) & (acc3 >> 28);
                    acc3 = (acc3 << 5) + (acc0 >> 27);
                }
                break;
            case 3:
                /* Computed goto */
                goto *labels[arr[i] % 5];
            L0:
                acc0 += 1;
                continue;
            L1:
                acc1 += 2;
                continue;
            L2:
                acc2 += 3;
                continue;
            L3:
                acc3 += 4;
                continue;
            L4:
                /* Fall through */
            default:
                /* More arithmetic with large constants */
                acc0 = (acc0 * 6364136223846793005LL) + 1442695040888963407LL;
                acc1 = (acc1 * 6364136223846793005LL) + 1442695040888963407LL;
                acc2 = (acc2 * 6364136223846793005LL) + 1442695040888963407LL;
                acc3 = (acc3 * 6364136223846793005LL) + 1442695040888963407LL;
        }
        
        /* Cross-basic-block register usage */
        if (i % 7 == 0) {
            long temp = acc0 + acc1 + acc2 + acc3;
            acc0 = temp ^ 0xAAAAAAAA;
            acc1 = temp ^ 0x55555555;
        }
    }
    
    return acc0 + acc1 + acc2 + acc3;
}

/* Function D: Mixed patterns for maximum pressure */
NOINLINE static long func_mixed_patterns(void) {
    /* Use builtins that return in specific registers */
    unsigned long long tsc1, tsc2;
    tsc1 = __builtin_ia32_rdtsc();
    
    /* Create many temporaries with overlapping lives */
    long v1 = global_array[0] * 0x12345678;
    long v2 = global_array[1] * 0x9ABCDEF0;
    long v3 = v1 + v2;
    long v4 = v1 - v2;
    long v5 = v3 * v4;
    long v6 = v3 / (v4 | 1);
    long v7 = v5 ^ v6;
    long v8 = v5 & v6;
    long v9 = v7 | v8;
    long v10 = v7 ^ v8;
    
    /* More builtin usage */
    tsc2 = __builtin_ia32_rdtsc();
    
    /* Force all values to be live simultaneously */
    asm volatile (
        "addl %[v1], %[v2]\n\t"
        "subl %[v3], %[v4]\n\t"
        "imull %[v5], %[v6]\n\t"
        : [v2] "+r" (v2), [v4] "+r" (v4), [v6] "+r" (v6)
        : [v1] "r" (v1), [v3] "r" (v3), [v5] "r" (v5)
        : "cc"
    );
    
    return v2 + v4 + v6 + v9 + v10 + (tsc2 - tsc1);
}

/* Main function to drive all patterns */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    long total = 0;
    
    /* Call each function with arguments that create register pressure */
    total += func_loop_invariants(
        argc > 1 ? atoi(argv[1]) : 100,
        global_array
    );
    
    total += func_asm_clobber(
        large_constants[0],
        large_constants[1]
    );
    
    total += func_complex_flow(
        argc > 2 ? atoi(argv[2]) : 50,
        global_array
    );
    
    total += func_mixed_patterns();
    
    /* Ensure results are used */
    KEEP_ALIVE(total);
    
    return (int)(total & 0x7FFFFFFF);
}
