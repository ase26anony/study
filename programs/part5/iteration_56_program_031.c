/* test_early_remat.c - Target specific patterns to trigger virtual register creation in early rematerialization */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {0x12345678, 0x9ABCDEF0, 0x11112222, 0x33334444};
static volatile int sink;

/* Prevent optimizations from simplifying our patterns */
#define KEEP_ALIVE(x) do { sink = (x); } while(0)

/* =========================================== */
/* Function A: Loop with expensive invariants  */
/* =========================================== */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int *data) {
    /* Large immediate constants that need rematerialization */
    const long IMM1 = 0x7FFFFFFF12345678UL;
    const long IMM2 = 0xFFFFFFFF87654321UL;
    const long IMM3 = 0x55555555AAAAAAAAUL;
    
    /* Invariant pointers used in loop */
    const int *invariant_ptr1 = &global_array[128];
    const int *invariant_ptr2 = &global_array[64];
    const long *invariant_ptr3 = large_constants;
    
    int sum = 0;
    int i, j;
    
    /* Complex loop with multiple invariants used in different places */
    for (i = 0; i < iterations; i++) {
        /* Use invariants in address calculations */
        int val1 = invariant_ptr1[i % 128];
        int val2 = invariant_ptr2[(i * 3) % 64];
        long val3 = invariant_ptr3[i % 4];
        
        /* Use large immediates in non-adjacent computations */
        if (val1 > (int)(IMM1 & 0xFFFFFFFF)) {
            sum += val2 * (int)(IMM2 >> 32);
        }
        
        /* Another use of the same immediate in different expression */
        if (val3 < (IMM3 >> 16)) {
            sum -= val1 * (int)(IMM1 >> 16);
        }
        
        /* Nested loop to extend live ranges */
        for (j = 0; j < 8; j++) {
            /* Mix all invariants and immediates */
            sum += (int)((IMM2 & 0xFFFF) + (long)invariant_ptr1[j]);
            sum -= (int)((IMM3 & 0xFFFFFFFF) - (long)invariant_ptr3[j % 4]);
        }
        
        /* Use invariant pointer in condition */
        if (invariant_ptr2 != &global_array[0]) {
            sum += (int)(IMM1 >> 8);
        }
    }
    
    return sum;
}

/* =========================================== */
/* Function B: Inline assembly with clobbers   */
/* =========================================== */
__attribute__((noinline, noclone))
int func_asm_clobber(int a, int b, int c) {
    /* Register variables to force hard register allocation */
    register int r1 asm("eax") = a;
    register int r2 asm("ebx") = b;
    register int r3 asm("ecx") = c;
    register int r4 asm("edx");
    register int r5 asm("esi");
    register int r6 asm("edi");
    
    int result;
    
    /* Multi-output inline assembly with many clobbers */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "movl %[in2], %%ebx\n\t"
        "movl %[in3], %%ecx\n\t"
        "imull %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %%ebx, %[out2]\n\t"
        : [out1] "=&r" (r4),  /* Early clobber output */
          [out2] "=&r" (r5)   /* Another early clobber */
        : [in1] "r" (r1),
          [in2] "r" (r2),
          [in3] "r" (r3)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory", "cc"
    );
    
    /* Use the results in complex expressions */
    result = r4 * r5;
    
    /* More inline assembly with different register constraints */
    asm volatile (
        "cpuid\n\t"
        : "=a" (r1), "=b" (r2), "=c" (r3), "=d" (r4)
        : "a" (0)
        : "cc"
    );
    
    /* Chain hard register references through computations */
    result += (r1 ^ r2) | (r3 & r4);
    
    /* Use builtins that return in specific registers */
    uint64_t tsc = __builtin_ia32_rdtsc();
    result += (int)(tsc >> 32) + (int)tsc;
    
    return result;
}

/* =========================================== */
/* Function C: Complex control flow            */
/* =========================================== */
__attribute__((noinline, noclone))
int func_complex_flow(int selector, int count) {
    /* Many local variables with overlapping live ranges */
    int a = selector * 2;
    int b = selector + 1000;
    int c = selector - 500;
    int d = selector | 0xFF00;
    int e = selector & 0x00FF;
    int f = selector ^ 0xAAAA;
    int g = ~selector;
    int h = selector << 3;
    int i = selector >> 2;
    int j = selector % 17;
    
    /* Labels for computed goto */
    void *labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4};
    
    int result = 0;
    int k;
    
    /* Outer loop */
    for (k = 0; k < count; k++) {
        /* Switch inside loop creates complex control flow */
        switch ((selector + k) % 5) {
            case 0:
                a = b + c;
                d = e * f;
                /* Use all variables to keep them live */
                result += a + d + g + h;
                break;
            case 1:
                b = c - d;
                e = f / (g ? g : 1);
                result += b + e + i + j;
                break;
            case 2:
                c = d | e;
                f = g & h;
                result += c + f + a + b;
                break;
            case 3:
                /* Nested loop with register variables */
                {
                    register int r7, r8, r9;
                    for (r7 = 0; r7 < 4; r7++) {
                        r8 = a + r7;
                        r9 = b - r7;
                        result += r8 * r9;
                    }
                }
                break;
            case 4:
                /* Computed goto */
                goto *labels[k % 5];
                L0: result += 1000; continue;
                L1: result += 2000; continue;
                L2: result += 3000; continue;
                L3: result += 4000; continue;
                L4: result += 5000; continue;
        }
        
        /* More operations extending live ranges */
        if (k % 3 == 0) {
            a = (a << 1) | (b >> 1);
            b = (b << 2) | (c >> 2);
            c = (c << 3) | (d >> 3);
        } else if (k % 3 == 1) {
            d = (d << 4) ^ (e >> 4);
            e = (e << 5) ^ (f >> 5);
            f = (f << 6) ^ (g >> 6);
        } else {
            g = (g << 7) + (h >> 7);
            h = (h << 8) + (i >> 8);
            i = (i << 9) + (j >> 9);
        }
        
        /* Use large immediate */
        result += 0x12345678 >> (k % 32);
    }
    
    return result;
}

/* =========================================== */
/* Main function to drive all patterns         */
/* =========================================== */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    
    /* Call all test functions with arguments that create register pressure */
    int sum = 0;
    
    /* Function A: Loop with invariants */
    sum += func_loop_invariants(iterations, global_array);
    
    /* Function B: Inline assembly */
    sum += func_asm_clobber(0x11111111, 0x22222222, 0x33333333);
    
    /* Function C: Complex control flow */
    sum += func_complex_flow(iterations % 100, iterations / 2);
    
    /* Mix in some direct large immediate usage */
    sum += (int)(0x7FFFFFFFFFFFFFFFLL >> 32);
    sum += (int)(0xAAAAAAAA55555555LL & 0xFFFFFFFF);
    
    /* Ensure results are used */
    KEEP_ALIVE(sum);
    
    return sum % 256;
}
