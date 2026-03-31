/* test_early_remat.c - Target specific coverage for early-remat.cc lines 930-937 */
#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {0x7FFFFFFF, 0x80000000, 0x12345678, 0x9ABCDEF0};
static volatile int sink; /* Prevent dead code elimination */

/* Function A: Loop with invariants and high register pressure */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int *data) {
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = iterations;
    register int r1 asm("ebx") = large_constants[0];
    register int r2 asm("ecx") = large_constants[1];
    int a = 0, b = 1, c = 2, d = 3, e = 4, f = 5, g = 6, h = 7;
    int i = 8, j = 9, k = 10, l = 11, m = 12, n = 13, o = 14, p = 15;
    
    /* Loop invariants used in multiple places */
    const int invariant1 = 0x12345678; /* Non-encodable immediate */
    const int invariant2 = 0x9ABCDEF0;
    const int *invariant_ptr = &global_array[128];
    
    /* Complex loop with many live values */
    for (int idx = 0; idx < iterations; idx += 1) {
        /* Use invariants in address calculations */
        int addr1 = idx + invariant1;
        int addr2 = idx * 2 + invariant2;
        
        /* Overlapping computations keeping many values live */
        a = data[addr1 & 0xFF] + b;
        b = data[addr2 & 0xFF] + c;
        c = a * b + d;
        d = c - e + (invariant_ptr[idx & 0x7F] >> 2);
        e = d * f + g;
        f = e / (h + 1);
        g = f ^ a;
        h = g | b;
        
        /* Use register variables with invariants */
        r0 = r0 + invariant1;
        r1 = r1 * invariant2;
        r2 = r2 ^ invariant1;
        
        /* More variables to increase pressure */
        i = j + k + l;
        j = k * l + m;
        k = l - m + n;
        l = m ^ n ^ o;
        m = n | o | p;
        n = o & p & idx;
        o = p + idx * 2;
        p = idx + invariant2;
        
        /* Conditional to create control flow complexity */
        if (idx % 3 == 0) {
            a = b + c * d;
            e = f - g / h;
        } else if (idx % 3 == 1) {
            i = j * k - l;
            m = n ^ o | p;
        }
    }
    
    /* Combine all results to ensure they're live */
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p + r0 + r1 + r2;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int x, int y) {
    int result1, result2, result3;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %2, %%eax\n\t"
        "movl %3, %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "leal (%%eax, %%ebx, 4), %%ecx\n\t"
        "movl %%ecx, %1\n\t"
        "addl $0x7FFFFFFF, %%edx\n\t"
        : "=&r" (result1), "=&r" (result2), "=&a" (result3)
        : "r" (x), "r" (y)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "cc", "memory"
    );
    
    /* Use results in complex expressions */
    register int r4 asm("esi") = result1;
    register int r5 asm("edi") = result2;
    
    /* Chain of operations keeping hard register values live */
    for (int i = 0; i < 100; i++) {
        r4 = r4 * 0x12345678 + i;  /* Large constant */
        r5 = r5 ^ 0x9ABCDEF0 - i;  /* Another large constant */
        result3 = result3 + (r4 & r5);
        
        /* Use in address calculation */
        global_array[i & 0xFF] = result3 + r4 - r5;
    }
    
    /* Force register variable usage with control flow */
    switch (r4 & 0x3) {
        case 0: r5 = r5 * 2; break;
        case 1: r5 = r5 + large_constants[2]; break;
        case 2: r5 = r5 ^ large_constants[3]; break;
        default: r5 = r5 | 0xF0F0F0F0; break;
    }
    
    return result1 + result2 + result3 + r4 + r5;
}

/* Function C: Complex control flow with register variables */
__attribute__((noinline, noclone))
int func_complex_control(int seed) {
    /* Many scalar temporaries */
    int t0 = seed, t1 = seed * 2, t2 = seed * 3, t3 = seed * 4;
    int t4 = seed * 5, t5 = seed * 6, t6 = seed * 7, t7 = seed * 8;
    int t8 = seed * 9, t9 = seed * 10, t10 = seed * 11, t11 = seed * 12;
    int t12 = seed * 13, t13 = seed * 14, t14 = seed * 15, t15 = seed * 16;
    
    /* Register variables with specific constraints */
    register int reg_a asm("eax") = t0;
    register int reg_b asm("ebx") = t1;
    register int reg_c asm("ecx") = t2;
    
    /* Nested loops with switch inside */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 20; j++) {
            /* Computed goto-like structure using switch */
            switch ((i + j) % 5) {
                case 0:
                    t0 = t1 + t2 * reg_a;
                    t3 = t4 - t5 / reg_b;
                    reg_c = large_constants[0] + i;
                    break;
                case 1:
                    t6 = t7 ^ t8 & reg_c;
                    t9 = t10 | t11 ^ reg_a;
                    reg_b = large_constants[1] - j;
                    break;
                case 2:
                    t12 = t13 * t14 + reg_b;
                    t15 = t0 - t1 * reg_c;
                    reg_a = large_constants[2] * i;
                    break;
                case 3:
                    /* Use target-specific builtin if available */
                    #ifdef __i386__
                    unsigned long long ts = __builtin_ia32_rdtsc();
                    t2 = (ts >> 32) + (ts & 0xFFFFFFFF);
                    #endif
                    t4 = t5 + t6 + reg_a;
                    reg_c = large_constants[3] ^ j;
                    break;
                default:
                    t8 = t9 * t10 - reg_b;
                    t11 = t12 / t13 + reg_c;
                    reg_a = reg_b ^ reg_c;
                    break;
            }
            
            /* Overlapping computations */
            t1 = t2 + t3;
            t5 = t6 * t7;
            t9 = t10 - t11;
            t13 = t14 ^ t15;
            
            /* Use all temporaries to keep them live */
            sink = t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7 +
                   t8 + t9 + t10 + t11 + t12 + t13 + t14 + t15;
        }
        
        /* Conditional with more computations */
        if (i % 7 == 0) {
            reg_a = reg_b * reg_c + 0x7FFFFFFF;
            reg_b = reg_c ^ 0x80000000;
            reg_c = reg_a & 0x12345678;
        }
    }
    
    return t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7 +
           t8 + t9 + t10 + t11 + t12 + t13 + t14 + t15 +
           reg_a + reg_b + reg_c;
}

/* Main function to drive everything */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    /* Get some argument-based values */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int seed = (argc > 2) ? atoi(argv[2]) : 12345;
    
    /* Call all test functions with complex arguments */
    int sum = 0;
    
    /* Function A with loop invariants */
    sum += func_loop_invariants(iterations, global_array);
    
    /* Function B with inline assembly */
    sum += func_asm_clobber(iterations, seed);
    
    /* Function C with complex control flow */
    sum += func_complex_control(seed);
    
    /* Use the result to prevent optimization */
    return sum & 0xFF;
}
