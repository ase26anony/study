/* test-early-remat.c */
/* Compile with: gcc -O2 -m32 -fno-optimize-sibling-calls -fdump-rtl-early-remat test-early-remat.c -o test */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {0x12345678, 0x9ABCDEF0, 0x55555555, 0xAAAAAAAA};

/* Function A: Loop with invariants and high register pressure */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int* data) {
    volatile int result = 0;
    
    /* Use many local variables with overlapping lifetimes */
    register int r0 asm("eax") = iterations;
    register int r1 asm("ebx") = large_constants[0] & 0xFFFF;
    register int r2 asm("ecx") = large_constants[1] & 0xFFFF;
    int temp1 = r0 * 12345;      /* Non-encodable immediate */
    int temp2 = r1 * 67890;      /* Another large immediate */
    int temp3 = r2 * 54321;
    
    /* Loop with invariant address calculation using global data */
    for (int i = 0; i < iterations; i++) {
        /* Complex address calculation with invariant base */
        int* addr = &global_array[i & 0xFF];
        
        /* Multiple uses of invariants in different expressions */
        int val1 = *addr + (int)(large_constants[2] >> 16);
        int val2 = *addr * (int)(large_constants[3] >> 16);
        int val3 = val1 * temp1 + val2 * temp2;
        
        /* More temporaries to increase register pressure */
        int t4 = val3 + (i * 98765);     /* Large immediate */
        int t5 = t4 - (int)((uintptr_t)global_array & 0xFFFF);
        int t6 = t5 * (int)((uintptr_t)large_constants & 0xFFFF);
        
        result += t6;
        
        /* Use all temporaries in conditional to keep them live */
        if (t6 > 1000) {
            temp1 = t4;
            temp2 = t5;
        } else {
            temp3 = t6;
        }
    }
    
    /* Force use of all register variables */
    asm volatile("" : : "r"(r0), "r"(r1), "r"(r2));
    return result + temp1 + temp2 + temp3;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int x, int y) {
    int a, b, c, d, e, f, g, h;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "movl %[y], %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[a]\n\t"
        "movl %%ebx, %[b]\n\t"
        "leal (%%eax,%%ebx,2), %%ecx\n\t"
        "movl %%ecx, %[c]"
        : [a] "=&r" (a), [b] "=&r" (b), [c] "=&r" (c)
        : [x] "rm" (x), [y] "rm" (y)
        : "eax", "ebx", "ecx", "memory"
    );
    
    /* Chain of operations using hard register results */
    d = a * 0x123456;      /* Large immediate */
    e = b + 0x89ABCDEF;    /* Another large immediate */
    f = c - (int)((uintptr_t)&global_array[0] & 0xFFFFFF);
    
    /* More inline asm with different clobbers */
    asm volatile (
        "movl %[d], %%esi\n\t"
        "movl %[e], %%edi\n\t"
        "addl %%esi, %%edi\n\t"
        "movl %%edi, %[g]"
        : [g] "=r" (g)
        : [d] "rm" (d), [e] "rm" (e)
        : "esi", "edi", "cc"
    );
    
    h = g * f + (int)(large_constants[0] >> 8);
    
    /* Use computed goto to create complex control flow */
    void* labels[] = {&&L1, &&L2, &&L3};
    goto *labels[h % 3];
    
L1:
    return a + b + c;
L2:
    return d + e + f;
L3:
    return g + h;
}

/* Function C: Complex control flow with many temporaries */
__attribute__((noinline, noclone))
int func_complex_cf(int start, int end) {
    int sum = 0;
    int counter = start;
    
    /* Nested loops with switch inside */
    while (counter < end) {
        int i;
        for (i = 0; i < 10; i++) {
            /* Many temporary variables with overlapping lives */
            int t1 = counter * 0x11111111;
            int t2 = i * 0x22222222;
            int t3 = t1 + t2;
            int t4 = t3 * 0x33333333;
            int t5 = t4 - (int)((uintptr_t)&large_constants[i & 3] & 0xFFFF);
            int t6 = t5 >> 4;
            int t7 = t6 * 0x44444444;
            int t8 = t7 + global_array[i];
            
            /* Switch with different cases using different temporaries */
            switch (t8 & 3) {
                case 0:
                    sum += t1 + t3 + t5;
                    break;
                case 1:
                    sum += t2 + t4 + t6;
                    break;
                case 2:
                    sum += t7 + t8 + (int)large_constants[0];
                    break;
                case 3:
                    sum += t1 * t2 - t3 * t4;
                    break;
            }
            
            /* Force all temporaries to stay live across iterations */
            asm volatile("" : : "r"(t1), "r"(t2), "r"(t3), "r"(t4),
                         "r"(t5), "r"(t6), "r"(t7), "r"(t8));
        }
        
        counter++;
        
        /* Conditional with more temporaries */
        if (counter & 1) {
            int extra1 = sum * 0x55555555;
            int extra2 = extra1 + (int)((uintptr_t)global_array & 0xFF);
            int extra3 = extra2 * counter;
            sum = extra3;
        }
    }
    
    return sum;
}

/* Function D: Using builtins for specific register references */
#ifdef __i386__
__attribute__((noinline, noclone))
uint64_t func_builtins(void) {
    /* Use rdtsc which returns in edx:eax */
    uint64_t tsc1 = __builtin_ia32_rdtsc();
    
    /* Create chain of operations using the result */
    uint32_t low = tsc1 & 0xFFFFFFFF;
    uint32_t high = tsc1 >> 32;
    
    /* Force use in expressions with large immediates */
    uint32_t a = low * 0x12345678;
    uint32_t b = high + 0x9ABCDEF0;
    uint32_t c = (a ^ b) * (uint32_t)((uintptr_t)global_array & 0xFFFFFF);
    
    /* Another rdtsc to create more register pressure */
    uint64_t tsc2 = __builtin_ia32_rdtsc();
    
    /* Mix results from both rdtsc calls */
    uint32_t d = (tsc2 & 0xFFFFFFFF) ^ c;
    uint32_t e = (tsc2 >> 32) + a;
    
    return ((uint64_t)e << 32) | d;
}
#endif

/* Main function to call all test functions */
int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    int result = 0;
    
    /* Call each function with arguments that create register pressure */
    result += func_loop_invariants(100, global_array);
    result += func_asm_clobber(0x1000, 0x2000);
    result += func_complex_cf(0, 50);
    
#ifdef __i386__
    result += (int)func_builtins();
#endif
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(result));
    
    return result & 0xFF;
}
