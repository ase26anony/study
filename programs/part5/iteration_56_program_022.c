/* test-early-remat.c */
#include <stdint.h>
#include <stdio.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {
    0x123456789ABCDEF0, 0xFEDCBA9876543210,
    0xDEADBEEFCAFEBABE, 0xBAADF00D12345678
};

/* Prevent optimizations from simplifying our patterns */
#define NOINLINE __attribute__((noinline, noclone))

/* Function A: Loop with invariants and high register pressure */
NOINLINE static uint64_t func_loop_invariants(int iterations, const int* data) {
    /* Many local variables with overlapping live ranges */
    register uint64_t a asm("eax") = large_constants[0];
    register uint64_t b asm("ebx") = large_constants[1];
    register uint64_t c asm("ecx") = large_constants[2];
    register uint64_t d asm("edx") = large_constants[3];
    uint64_t e, f, g, h, i, j, k, l, m, n, o, p;
    
    /* Loop with invariant address calculation */
    const int* invariant_ptr = data;
    const uint64_t invariant_const = 0xFFFFFFFFFFFFFFFF;
    
    for (int idx = 0; idx < iterations; idx++) {
        /* Complex address calculation using invariants multiple times */
        int offset = (idx * 7 + 13) % 256;
        
        /* Use invariants in multiple non-adjacent expressions */
        e = a + (uint64_t)invariant_ptr;
        f = b + invariant_const;
        g = c + (uint64_t)(invariant_ptr + offset);
        h = d + invariant_const - idx;
        
        /* More operations to extend live ranges */
        i = e * f;
        j = g * h;
        k = i + j;
        l = k * invariant_const;
        m = l + (uint64_t)invariant_ptr;
        n = m * a;
        o = n / (b + 1);
        p = o ^ invariant_const;
        
        /* Use results to prevent dead code elimination */
        a = b + p;
        b = c + a;
        c = d + b;
        d = a + c;
    }
    
    /* Return complex expression to keep all values live */
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

/* Function B: Inline assembly with clobbered registers */
NOINLINE static uint64_t func_asm_clobber(uint64_t x, uint64_t y) {
    uint64_t result1, result2, result3;
    
    /* Multi-output inline assembly with specific register constraints */
    asm volatile (
        "movl %[x1], %%eax\n\t"
        "movl %[x2], %%edx\n\t"
        "addl %%eax, %%edx\n\t"
        "movl %%edx, %[out1]\n\t"
        "imull %[y1], %%eax\n\t"
        "movl %%eax, %[out2]"
        : [out1] "=&r" (result1), [out2] "=&r" (result2)
        : [x1] "r" ((uint32_t)x), [x2] "r" ((uint32_t)(x >> 32)),
          [y1] "r" ((uint32_t)y)
        : "eax", "edx", "ecx", "ebx", "esi", "edi", "memory", "cc"
    );
    
    /* More register variables with hard constraints */
    register uint32_t r1 asm("eax") = result1;
    register uint32_t r2 asm("ebx") = result2;
    register uint32_t r3 asm("ecx") = y;
    register uint32_t r4 asm("edx") = x;
    
    /* Complex expression chain using register variables */
    asm volatile (
        "addl %%ebx, %%eax\n\t"
        "subl %%ecx, %%edx\n\t"
        "imull %%edx, %%eax"
        : "+r" (r1), "+r" (r4)
        : "r" (r2), "r" (r3)
        : "cc"
    );
    
    /* Use results in conditionals to create control flow */
    if (r1 > r4) {
        result3 = r1 * large_constants[0];
    } else {
        result3 = r4 * large_constants[1];
    }
    
    return result1 + result2 + result3 + r1 + r4;
}

/* Function C: Complex control flow with switch and computed goto */
NOINLINE static uint64_t func_complex_flow(int selector, int iterations) {
    static void* jump_table[] = {
        &&label0, &&label1, &&label2, &&label3,
        &&label4, &&label5, &&label6, &&label7
    };
    
    /* Many temporaries with overlapping lives */
    uint64_t t1 = 0x1111111111111111;
    uint64_t t2 = 0x2222222222222222;
    uint64_t t3 = 0x3333333333333333;
    uint64_t t4 = 0x4444444444444444;
    uint64_t t5, t6, t7, t8, t9, t10;
    
    /* Nested loops with switch inside */
    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < 8; j++) {
            /* Computed goto creates complex control flow */
            goto *jump_table[(selector + i + j) % 8];
            
        label0:
            t5 = t1 + t2;
            t6 = t3 * large_constants[0];
            continue;
            
        label1:
            t7 = t2 - t3;
            t8 = t4 / (i + 1);
            continue;
            
        label2:
            t9 = t1 * t3;
            t10 = t2 + t4;
            continue;
            
        label3:
            t1 = t5 + t6;
            t2 = t7 * t8;
            continue;
            
        label4:
            t3 = t9 - t10;
            t4 = t1 ^ t2;
            continue;
            
        label5:
            t5 = t3 | t4;
            t6 = t1 & t2;
            continue;
            
        label6:
            t7 = t5 << 3;
            t8 = t6 >> 2;
            continue;
            
        label7:
            t9 = t7 + t8;
            t10 = t5 - t6;
            
            /* Use builtins that use specific registers */
            uint64_t ts = __builtin_ia32_rdtsc();
            t1 = t1 ^ ts;
            t2 = t2 + (ts >> 32);
            continue;
        }
    }
    
    /* Return value using all temporaries */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
}

/* Function D: Mixed patterns for maximum pressure */
NOINLINE static uint64_t func_mixed_patterns(const int* data, int size) {
    uint64_t accum = 0;
    
    /* Unroll loops manually to create many live values */
    for (int i = 0; i < size; i += 4) {
        /* Load with complex address calculation */
        uint64_t val1 = data[i] + (uint64_t)&global_array[i];
        uint64_t val2 = data[i + 1] * large_constants[i % 4];
        uint64_t val3 = data[i + 2] ^ (uint64_t)&large_constants[0];
        uint64_t val4 = data[i + 3] | 0x9876543210ABCDEF;
        
        /* Chain computations keeping many values live */
        uint64_t tmp1 = val1 * val2;
        uint64_t tmp2 = val3 + val4;
        uint64_t tmp3 = tmp1 - tmp2;
        uint64_t tmp4 = (val1 << 5) | (val2 >> 3);
        uint64_t tmp5 = tmp3 ^ tmp4;
        uint64_t tmp6 = tmp5 * 0x12345678;
        uint64_t tmp7 = tmp6 + (uint64_t)data;
        uint64_t tmp8 = tmp7 / (i + 1);
        
        /* Use inline asm to force register constraints */
        register uint64_t r1 asm("eax") = tmp1;
        register uint64_t r2 asm("ebx") = tmp2;
        uint64_t r3;
        
        asm volatile (
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %0"
            : "=r" (r3)
            : "a" (r1), "b" (r2)
            : "cc"
        );
        
        accum += tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8 + r3;
    }
    
    return accum;
}

int main() {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 7;
    }
    
    uint64_t total = 0;
    
    /* Call functions with arguments that can't be easily optimized */
    total += func_loop_invariants(100, global_array);
    total += func_asm_clobber(0x123456789ABCDEF0, 0xFEDCBA9876543210);
    total += func_complex_flow(3, 50);
    total += func_mixed_patterns(global_array, 256);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %llu\n", (unsigned long long)total);
    return (int)(total % 256);
}
