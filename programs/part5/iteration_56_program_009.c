/* test_early_remat.c - Target early-remat.cc lines 930-937 */
#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {
    0x123456789ABCDEF0, 0xFEDCBA9876543210,
    0xDEADBEEFCAFEBABE, 0x0BADF00D12345678
};

/* Prevent optimizations */
#define NOINLINE __attribute__((noinline, noclone))
#define USED __attribute__((used))

/* Function A: Loop with invariants and high register pressure */
NOINLINE static uint64_t func_loop_invariants(int iterations, int *data) {
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = iterations;
    register int r1 asm("ebx") = data[0];
    register int r2 asm("ecx") = data[1];
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    uint64_t sum = 0;
    
    /* Large immediate constants that need rematerialization */
    const uint64_t const1 = 0x123456789ABCDEF0ULL;
    const uint64_t const2 = 0xFEDCBA9876543210ULL;
    const uint64_t const3 = 0xDEADBEEFCAFEBABEULL;
    
    /* Complex loop with invariant usage */
    for (a = 0; a < iterations; a += 2) {
        /* Use invariants in multiple places */
        b = (a * const1) >> 32;
        c = (b + const2) & 0xFFFFFFFF;
        d = (c ^ const3) | r0;
        
        /* More operations creating register pressure */
        e = data[a % 256] + const1;
        f = data[(a + 1) % 256] * const2;
        g = data[(a + 2) % 256] ^ const3;
        h = data[(a + 3) % 256] | const1;
        
        /* Use all variables to keep them live */
        i = (e + f) * g;
        j = (h - i) / (d + 1);
        k = (j << 3) | (r1 & 0xFF);
        l = (k >> 2) + (r2 * 7);
        m = l ^ (const2 & 0xFFFF);
        n = m * 314159265;
        o = n % 271828182;
        p = o + (const3 >> 48);
        
        sum += a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
        
        /* Conditional branch creating separate basic block */
        if (sum & 1) {
            /* Use invariants again in different context */
            r0 = (const1 >> 32) + a;
            r1 = (const2 & 0xFFFFFFFF) ^ b;
            r2 = (const3 % 65536) * c;
        }
    }
    
    return sum + r0 + r1 + r2;
}

/* Function B: Inline assembly with clobbered registers */
NOINLINE static uint64_t func_asm_clobber(int x, int y) {
    uint64_t result = 0;
    int a, b, c, d, e, f, g, h;
    
    /* Many temporaries with overlapping lives */
    a = x * 0x12345678;
    b = y + 0x9ABCDEF0;
    c = a ^ b;
    d = c * 0x11111111;
    e = d >> 16;
    f = e | 0x22222222;
    g = f & 0x33333333;
    h = g - 0x44444444;
    
    /* Multi-output inline assembly with many clobbers */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "movl %[y], %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal 0x12345(%%eax), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        : [out1] "=&r" (a), [out2] "=&r" (b)
        : [x] "rm" (x), [y] "rm" (y)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory", "cc"
    );
    
    /* Use results in complex expressions */
    c = (a * 0x55555555) + (b * 0x66666666);
    d = (c ^ 0x77777777) | (a & 0x88888888);
    e = (d << 3) + (b >> 2);
    f = e * 0x99999999;
    g = f % 0xAAAAAAAB;
    h = g ^ 0xBBBBBBBB;
    
    /* Another asm with different clobbers */
    register int r1 asm("esi") = h;
    register int r2 asm("edi") = g;
    
    asm volatile (
        "movl %[r1], %%esi\n\t"
        "movl %[r2], %%edi\n\t"
        "addl %%edi, %%esi\n\t"
        "rorl $13, %%esi\n\t"
        : "+&r" (r1), "+&r" (r2)
        :
        : "cc"
    );
    
    result = ((uint64_t)r1 << 32) | r2;
    return result + a + b + c + d + e + f + g + h;
}

/* Function C: Complex control flow with register variables */
NOINLINE static uint64_t func_complex_cf(int selector, int count) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    register int r0 asm("eax") = selector;
    register int r1 asm("ebx") = count;
    register int r2 asm("ecx") = 0;
    int i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
    uint64_t total = 0;
    
    /* Large immediate for rematerialization */
    const uint64_t big_const = 0x1234567890ABCDEFULL;
    
    /* Nested loops with many temporaries */
    for (i = 0; i < count; i++) {
        for (j = 0; j < 8; j++) {
            /* Many calculations creating register pressure */
            k = i * j * (big_const & 0xFFFFFFFF);
            l = (k + (big_const >> 32)) ^ r0;
            m = l * 0x1234567;
            n = m % 0x7654321;
            o = n | r1;
            p = o << 3;
            q = p >> 1;
            r = q ^ 0xF0F0F0F0;
            s = r + r2;
            t = s * 0x80808081;
            u = t >> 7;
            v = u & 0x00FFFFFF;
            w = v * 0x00400001;
            x = w >> 22;
            y = x + global_array[(i + j) % 256];
            z = y * large_constants[j % 4];
            
            total += z;
            
            /* Switch-like computed goto */
            if (j % 4 == 0) goto *labels[selector % 4];
            
            label0:
            r0 = (r0 * 0x5A827999) + 1;
            continue;
            
            label1:
            r1 = (r1 ^ 0x6ED9EBA1) << 1;
            continue;
            
            label2:
            r2 = (r2 + 0x8F1BBCDC) >> 1;
            continue;
            
            label3:
            r0 = r0 ^ r1 ^ r2;
            continue;
        }
    }
    
    /* Use builtins for hard register references */
    {
        uint64_t tsc1, tsc2;
        tsc1 = __builtin_ia32_rdtsc();
        total ^= tsc1;
        
        /* Use the result in calculations */
        r0 = (tsc1 >> 32) + r0;
        r1 = (tsc1 & 0xFFFFFFFF) * r1;
        
        tsc2 = __builtin_ia32_rdtsc();
        r2 = (tsc2 - tsc1) ^ r2;
    }
    
    return total + r0 + r1 + r2;
}

/* Main function to drive everything */
int main(int argc, char **argv) {
    uint64_t result = 0;
    int i;
    
    /* Initialize global data */
    for (i = 0; i < 256; i++) {
        global_array[i] = i * 0x01010101;
    }
    
    /* Call test functions with different patterns */
    result += func_loop_invariants(
        argc > 1 ? atoi(argv[1]) : 1000,
        global_array
    );
    
    result += func_asm_clobber(
        argc > 2 ? atoi(argv[2]) : 0x12345678,
        argc > 3 ? atoi(argv[3]) : 0x9ABCDEF0
    );
    
    result += func_complex_cf(
        argc > 4 ? atoi(argv[4]) : 2,
        argc > 5 ? atoi(argv[5]) : 50
    );
    
    /* Ensure result is used */
    return (int)(result % 0x100000000);
}
