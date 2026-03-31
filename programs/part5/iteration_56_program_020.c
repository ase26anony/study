/* test_early_remat.c - Target specific patterns to trigger virtual register creation in early rematerialization */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {0x7FFFFFFF, 0x80000000, 0x12345678, 0x9ABCDEF0};

/* Prevent optimizations from simplifying our patterns */
#define NOINLINE __attribute__((noinline, noclone))
#define KEEP_ALIVE asm volatile("" : : "r"(result) : "memory")

/* Function A: Loop with invariants and expensive constants */
NOINLINE static int func_loop_invariants(int iterations, int *data) {
    volatile int result = 0;
    /* Large immediate constants that need rematerialization */
    const long expensive_const1 = 0x123456789ABCDEF0LL;
    const long expensive_const2 = 0xFEDCBA9876543210LL;
    const int *invariant_ptr = &global_array[128]; /* Loop invariant address */
    
    /* Complex loop with overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in multiple places with different expressions */
        int idx1 = (i * expensive_const1) % 256;
        int idx2 = (i * expensive_const2) % 256;
        
        /* Force register pressure with many temporaries */
        int temp1 = data[idx1] + *invariant_ptr;
        int temp2 = data[idx2] - *invariant_ptr;
        int temp3 = temp1 * temp2;
        int temp4 = temp3 / (i + 1);
        int temp5 = temp4 ^ (int)expensive_const1;
        int temp6 = temp5 | (int)expensive_const2;
        int temp7 = temp6 & 0xFFFF;
        int temp8 = temp7 << 4;
        int temp9 = temp8 >> 2;
        int temp10 = temp9 + idx1 - idx2;
        
        result += temp10;
        
        /* More operations to extend live ranges */
        if (i % 3 == 0) {
            temp1 = data[i % 256] + (int)(expensive_const1 >> 32);
            temp3 = temp1 * result;
            result = temp3 % 1000;
        } else if (i % 5 == 0) {
            temp2 = data[(i + 128) % 256] - (int)(expensive_const2 >> 32);
            temp4 = temp2 * result;
            result = temp4 % 1000;
        }
    }
    
    return result;
}

/* Function B: Inline assembly with clobbered registers */
NOINLINE static int func_asm_clobber(int a, int b) {
    register int r1 asm("eax") = a;
    register int r2 asm("ebx") = b;
    register int r3 asm("ecx");
    register int r4 asm("edx");
    int result;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movl $0x12345678, %%ecx\n\t"
        "movl $0x9ABCDEF0, %%edx"
        : "=r" (result), "=&r" (r3), "=&r" (r4)
        : "1" (r1), "2" (r2)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    /* Use the hard register values in subsequent computations */
    asm volatile (
        "addl %%ecx, %0\n\t"
        "subl %%edx, %0"
        : "+r" (result)
        : 
        : "cc"
    );
    
    /* More operations mixing register variables */
    for (int i = 0; i < 100; i++) {
        r1 = result + i;
        r2 = r1 * large_constants[i % 4];
        r3 = r2 / (i + 1);
        r4 = r3 ^ 0xFF;
        result = r4;
        
        /* Clobber many registers periodically */
        if (i % 7 == 0) {
            asm volatile (
                "pushl %%eax\n\t"
                "pushl %%ebx\n\t"
                "pushl %%ecx\n\t"
                "pushl %%edx\n\t"
                "popl %%edx\n\t"
                "popl %%ecx\n\t"
                "popl %%ebx\n\t"
                "popl %%eax"
                : : : "memory"
            );
        }
    }
    
    return result;
}

/* Function C: Complex control flow with register variables */
NOINLINE static int func_complex_flow(int seed) {
    register int r1 asm("esi") = seed;
    register int r2 asm("edi") = seed * 2;
    int result = 0;
    
    /* Labels for computed goto */
    static void *labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4};
    
    /* Nested loops with switch inside */
    for (int i = 0; i < 50; i++) {
        int branch = (r1 + i) % 5;
        
        /* Computed goto creating complex control flow */
        goto *labels[branch];
        
    L0:
        r1 = r1 * 3 + 1;
        r2 = r2 / 2;
        result += r1 - r2;
        continue;
        
    L1:
        r1 = r1 ^ 0xAAAA;
        r2 = r2 | 0x5555;
        result += r1 + r2;
        continue;
        
    L2:
        /* Use target-specific builtins if available */
        {
            uint64_t tsc;
            #ifdef __i386__
            asm volatile ("rdtsc" : "=A" (tsc));
            r1 = (tsc >> 32) & 0xFFFF;
            r2 = tsc & 0xFFFF;
            #endif
            result += r1 * r2;
        }
        continue;
        
    L3:
        /* Switch inside loop with register variables */
        switch (r1 % 4) {
            case 0: r1 = large_constants[0] & 0xFFFFFFFF; break;
            case 1: r1 = large_constants[1] & 0xFFFFFFFF; break;
            case 2: r1 = large_constants[2] & 0xFFFFFFFF; break;
            case 3: r1 = large_constants[3] & 0xFFFFFFFF; break;
        }
        result += r1;
        continue;
        
    L4:
        /* More arithmetic with overlapping live ranges */
        int temp[10];
        for (int j = 0; j < 10; j++) {
            temp[j] = r1 * j + r2;
            result += temp[j];
        }
        r1 = (r1 + 1) & 0xFF;
        r2 = (r2 - 1) & 0xFF;
        continue;
    }
    
    return result;
}

/* Main function that creates register pressure */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 - 128;
    }
    
    int iterations = (argc > 1) ? atoi(argv[1]) : 1000;
    if (iterations < 10) iterations = 10;
    if (iterations > 10000) iterations = 10000;
    
    /* Call all test functions to trigger different patterns */
    int sum = 0;
    
    sum += func_loop_invariants(iterations, global_array);
    
    sum += func_asm_clobber(iterations % 100, (iterations * 7) % 100);
    
    sum += func_complex_flow(iterations % 50);
    
    /* Additional pressure by calling multiple times */
    for (int i = 0; i < 5; i++) {
        sum += func_loop_invariants(50, &global_array[i * 50]);
    }
    
    /* Ensure result is used */
    asm volatile ("" : : "r"(sum));
    
    return sum % 255;
}
