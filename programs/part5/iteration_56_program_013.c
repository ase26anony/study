/* test_early_remat.c - Target early-remat.cc lines 930-937 */
#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {
    0x123456789ABCDEF0, 0xFEDCBA9876543210,
    0x7FFFFFFFFFFFFFFF, 0x8000000000000000,
    0xAAAAAAAAAAAAAAAA, 0x5555555555555555
};

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, noclone))
#define KEEP_ALIVE(expr) asm volatile("" : : "r"(expr))

/* Function A: Loop with invariants and high register pressure */
NOINLINE static uint64_t func_loop_invariants(int iterations, const int* data) {
    /* Many local variables with overlapping live ranges */
    register uint64_t r1 asm("ebx") = large_constants[0];
    register uint64_t r2 asm("esi") = large_constants[1];
    uint64_t a = (uint64_t)data;
    uint64_t b = 0xDEADBEEFCAFEBABE;
    uint64_t c = 0x1234567890ABCDEF;
    uint64_t d = 0xFEDCBA0987654321;
    uint64_t e = 0xAAAAAAAAAAAAAAAAllu;
    uint64_t f = 0x5555555555555555llu;
    uint64_t g = 0x8888888888888888llu;
    uint64_t h = 0x9999999999999999llu;
    
    /* Loop with invariant address calculation */
    for (int i = 0; i < iterations; i++) {
        /* Complex address calculation using invariants */
        uint64_t addr1 = a + (i * 8);
        uint64_t addr2 = b + (i * 16);
        uint64_t addr3 = c + (i * 32);
        
        /* Multiple uses of invariants in different expressions */
        uint64_t val1 = *(const uint64_t*)(addr1 & 0xFFFFFFFF);
        uint64_t val2 = *(const uint64_t*)(addr2 & 0xFFFFFFFF);
        uint64_t val3 = *(const uint64_t*)(addr3 & 0xFFFFFFFF);
        
        /* Overlapping live ranges */
        r1 = r1 * val1 + r2;
        r2 = r2 * val2 + d;
        d = d * val3 + e;
        e = e * r1 + f;
        f = f * r2 + g;
        g = g * d + h;
        h = h * e + a;
        a = a * f + b;
        b = b * g + c;
        c = c * h + r1;
        
        /* Use large immediate in condition */
        if (i % 256 == 0x7FFFFFFF) {  /* Unlikely but large immediate */
            r1 ^= 0xFFFFFFFFFFFFFFFFllu;
        }
    }
    
    /* Combine all values to prevent dead code elimination */
    return r1 + r2 + a + b + c + d + e + f + g + h;
}

/* Function B: Inline assembly with clobbered registers */
NOINLINE static uint64_t func_asm_clobber(uint64_t x, uint64_t y) {
    uint64_t result1, result2, result3;
    
    /* Multi-output inline assembly with many clobbered registers */
    asm volatile (
        "movl %[x1], %%eax\n\t"
        "movl %[x2], %%edx\n\t"
        "addl %%edx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %%edx, %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        "leal (%%eax,%%edx,4), %%ecx\n\t"
        "movl %%ecx, %[out3]"
        : [out1] "=&r" (result1), [out2] "=&r" (result2), [out3] "=&r" (result3)
        : [x1] "r" ((uint32_t)x), [x2] "r" ((uint32_t)y)
        : "eax", "edx", "ecx", "memory", "cc"
    );
    
    /* More register variables with specific constraints */
    register uint32_t r3 asm("ebx") = result1;
    register uint32_t r4 asm("esi") = result2;
    register uint32_t r5 asm("edi") = result3;
    
    /* Complex chain of operations keeping registers live */
    for (int i = 0; i < 100; i++) {
        asm volatile (
            "addl %%ebx, %%esi\n\t"
            "adcl %%edi, %%ebx\n\t"
            "xorl %%esi, %%edi"
            : "+r" (r3), "+r" (r4), "+r" (r5)
            :
            : "cc"
        );
    }
    
    /* Use builtins that return in specific registers */
    uint64_t tsc1, tsc2;
    tsc1 = __builtin_ia32_rdtsc();  /* Returns in eax:edx */
    
    /* Create dependency chain with hard register results */
    asm volatile (
        "addl %%eax, %[r3]\n\t"
        "adcl %%edx, %[r4]"
        : [r3] "+r" (r3), [r4] "+r" (r4)
        :
        : "cc"
    );
    
    tsc2 = __builtin_ia32_rdtsc();
    
    return (uint64_t)r3 + ((uint64_t)r4 << 32) + tsc1 + tsc2;
}

/* Function C: Complex control flow with switch statements */
NOINLINE static uint64_t func_complex_control(int mode, uint64_t seed) {
    /* Many temporaries with overlapping lives */
    uint64_t a = seed * 0x5DEECE66D;
    uint64_t b = seed * 0xBF58476D1CE4E5B9;
    uint64_t c = seed * 0x94D049BB133111EB;
    uint64_t d = seed * 0xAAAAAAAAAAAAAAAB;
    uint64_t e = seed * 0x5555555555555555;
    uint64_t f = seed * 0x3333333333333333;
    uint64_t g = seed * 0x0F0F0F0F0F0F0F0F;
    uint64_t h = seed * 0x00FF00FF00FF00FF;
    
    /* Labels for computed goto */
    void* labels[] = { &&case0, &&case1, &&case2, &&case3, &&case4 };
    
    /* Nested loops with switch inside */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            /* Computed goto creates complex control flow */
            goto *labels[(i * j + mode) % 5];
            
        case0:
            a = a * b + c;
            b = b * c + d;
            c = c * d + e;
            d = d * e + f;
            continue;
            
        case1:
            e = e * f + g;
            f = f * g + h;
            g = g * h + a;
            h = h * a + b;
            continue;
            
        case2:
            a = a ^ b ^ c;
            b = b ^ c ^ d;
            c = c ^ d ^ e;
            d = d ^ e ^ f;
            continue;
            
        case3:
            e = e | f | g;
            f = f | g | h;
            g = g | h | a;
            h = h | a | b;
            continue;
            
        case4:
            a = (a << 3) | (b >> 61);
            b = (b << 7) | (c >> 57);
            c = (c << 13) | (d >> 51);
            d = (d << 19) | (e >> 45);
            continue;
        }
    }
    
    /* Use all variables to keep them live */
    return a + b + c + d + e + f + g + h;
}

/* Main function that creates the necessary conditions */
int main(int argc, char** argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 0x10001;
    }
    
    uint64_t total = 0;
    
    /* Call each function with arguments that encourage rematerialization */
    total += func_loop_invariants(
        argc > 1 ? atoi(argv[1]) : 1000,
        global_array
    );
    
    total += func_asm_clobber(
        0x123456789ABCDEF0llu,
        0xFEDCBA9876543210llu
    );
    
    total += func_complex_control(
        argc > 2 ? atoi(argv[2]) : 3,
        0xDEADBEEFCAFEBABEllu
    );
    
    /* Additional pressure: call functions multiple times */
    for (int i = 0; i < 10; i++) {
        total += func_loop_invariants(100, global_array + i);
    }
    
    /* Ensure result is used */
    KEEP_ALIVE(total);
    
    return (int)(total & 0x7FFFFFFF);
}
