/* test_early_remat.c - Target early-remat.cc lines 930-937 */
#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[1024];
static const long large_constants[] = {
    0x123456789ABCDEF0, 0xFEDCBA9876543210,
    0xDEADBEEFCAFEBABE, 0xBAADF00D12345678
};

/* Function A: Loop with invariants and high register pressure */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int* data) {
    /* Many local variables with overlapping lifetimes */
    register int r0 asm("eax") = iterations;
    register int r1 asm("ebx") = data[0];
    register int r2 asm("ecx") = data[1];
    int a = r0 * 2;
    int b = r1 + 0x7FFFFFFF;  /* Large immediate */
    int c = r2 - 0x80000000;  /* Another large immediate */
    int d = a ^ b;
    int e = c | d;
    int f = 0x12345678;  /* Non-encodable immediate */
    int g = 0x9ABCDEF0;
    
    /* Loop with invariant address calculation */
    const int* invariant_ptr = &global_array[512];
    const long invariant_const = large_constants[0];
    
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in multiple places */
        int idx1 = (i * invariant_const) % 256;
        int idx2 = (i + (invariant_const >> 32)) % 256;
        
        /* Complex address calculations with invariants */
        int val1 = invariant_ptr[idx1] + (invariant_const & 0xFFFFFFFF);
        int val2 = invariant_ptr[idx2] ^ (invariant_const >> 32);
        
        /* Many overlapping live ranges */
        a = a + val1 + f;  /* f is large immediate used repeatedly */
        b = b ^ val2 ^ g;  /* g is large immediate used repeatedly */
        c = c * a * 0x55555555;  /* Another large immediate */
        d = d / (b + 1);
        e = e % (c + 1);
        
        /* Use all variables to keep them live */
        data[i % 16] = a + b + c + d + e + val1 + val2;
    }
    
    /* Force all values to be used */
    return a + b + c + d + e + f + g;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
long func_asm_clobber(long x, long y) {
    long result1, result2, result3;
    
    /* Multi-output inline assembly with many clobbers */
    asm volatile (
        "movl %[x1], %%eax\n\t"
        "movl %[x2], %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "movl %[y1], %%ecx\n\t"
        "movl %[y2], %%edx\n\t"
        "addl %%ecx, %%edx\n\t"
        "movl %%edx, %[out2]\n\t"
        "leal (%%eax,%%edx,2), %%esi\n\t"
        "movl %%esi, %[out3]"
        : [out1] "=&r" (result1),  /* Early clobber */
          [out2] "=&r" (result2),
          [out3] "=&r" (result3)
        : [x1] "rm" ((int)(x & 0xFFFFFFFF)),
          [x2] "rm" ((int)(x >> 32)),
          [y1] "rm" ((int)(y & 0xFFFFFFFF)),
          [y2] "rm" ((int)(y >> 32))
        : "eax", "ebx", "ecx", "edx", "esi", "edi",
          "memory", "cc"
    );
    
    /* Use register variables with hard constraints */
    register long r10 asm("esi") = result1;
    register long r11 asm("edi") = result2;
    register long r12 asm("ebx") = result3;
    
    /* Complex operations keeping all registers live */
    for (int i = 0; i < 32; i++) {
        r10 = (r10 << i) | (r11 >> (32 - i));
        r11 = (r11 ^ r12) + 0xDEADBEEF;
        r12 = (r12 * 0x12345679) - r10;
        
        /* Use builtins that return in specific registers */
        unsigned long long tsc = __builtin_ia32_rdtsc();
        r10 ^= (tsc & 0xFFFFFFFF);
        r11 ^= (tsc >> 32);
    }
    
    return r10 + r11 + r12;
}

/* Function C: Complex control flow with switch */
__attribute__((noinline, noclone))
int func_complex_flow(int seed, int* output) {
    /* Many temporary variables with overlapping lives */
    int t1 = seed + 0x11111111;
    int t2 = seed * 0x22222222;
    int t3 = seed ^ 0x33333333;
    int t4 = seed | 0x44444444;
    int t5 = seed & 0x55555555;
    int t6 = seed - 0x66666666;
    int t7 = seed / 7;
    int t8 = seed % 9;
    int t9 = ~seed;
    int t10 = seed << 3;
    
    /* Labels for computed goto */
    void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
    
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Switch with many cases creates complex CFG */
        switch (i % 5) {
            case 0:
                t1 = t2 + t3;
                t4 = t5 * 0x77777777;  /* Large immediate */
                goto *labels[i % 5];
            case 1:
                t2 = t3 ^ t4;
                t5 = t6 + 0x88888888;
                /* Fall through */
            case 2:
                t3 = t4 | t5;
                t6 = t7 - 0x99999999;
                break;
            case 3:
                t4 = t5 & t6;
                t7 = t8 * 0xAAAAAAAA;
                continue;  /* Loop continues */
            case 4:
                t5 = t6 << 2;
                t8 = t9 / 3;
                break;
        }
        
    L0:
        t6 = t7 >> 1;
    L1:
        t7 = t8 ^ t9;
    L2:
        t8 = t9 + t10;
    L3:
        t9 = t10 * 2;
    L4:
        t10 = t1 % 17;
        
        /* Use all temporaries to keep them live */
        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
        
        /* Conditional branch with many live values */
        if (sum & 1) {
            output[i % 16] = t1 + t3 + t5 + t7 + t9;
        } else {
            output[i % 16] = t2 + t4 + t6 + t8 + t10;
        }
    }
    
    return sum;
}

/* Main function to drive everything */
int main(int argc, char** argv) {
    /* Initialize global data */
    for (int i = 0; i < 1024; i++) {
        global_array[i] = i * 3;
    }
    
    int local_data[32];
    for (int i = 0; i < 32; i++) {
        local_data[i] = i * 5;
    }
    
    /* Call all test functions with appropriate arguments */
    int result1 = func_loop_invariants(
        argc > 1 ? atoi(argv[1]) : 100,
        local_data
    );
    
    long result2 = func_asm_clobber(
        large_constants[0],
        large_constants[1]
    );
    
    int output_buf[32];
    int result3 = func_complex_flow(argc, output_buf);
    
    /* Combine results to prevent optimization */
    int final_result = result1 + (result2 & 0xFFFFFFFF) + result3;
    
    /* Use the result */
    return final_result > 0 ? 0 : 1;
}
