/* test_early_remat.c - Trigger virtual register creation in early rematerialization */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static double global_doubles[128];
static volatile int volatile_counter = 0;

/* Prevent optimizations from simplifying our patterns */
#define NOINLINE __attribute__((noinline, noclone))
#define KEEP_ALIVE asm volatile("" : : "r"(result) : "memory")

/* Function A: Loop with invariants and expensive constants */
NOINLINE static uint64_t function_a(uint64_t limit, int *array) {
    /* Large immediate constants that need rematerialization */
    const uint64_t LARGE_CONST_A = 0x123456789ABCDEF0ULL;
    const uint64_t LARGE_CONST_B = 0xFEDCBA9876543210ULL;
    const uint64_t LARGE_CONST_C = 0xDEADBEEFCAFEBABEULL;
    
    /* Loop invariants that will be used in multiple places */
    uint64_t *invariant_ptr = (uint64_t *)global_array;
    uint64_t invariant_offset = (uint64_t)(&global_array[128]);
    
    uint64_t result = 0;
    uint64_t i, j, k, l, m, n, o, p; /* Many live variables */
    
    /* Create overlapping live ranges */
    i = limit;
    j = LARGE_CONST_A;
    k = LARGE_CONST_B;
    l = (uint64_t)array;
    
    /* Complex loop with many invariants and constants */
    for (uint64_t counter = 0; counter < limit; counter += 16) {
        /* Use invariants in multiple calculations */
        m = invariant_offset + counter;
        n = *invariant_ptr + LARGE_CONST_C;
        o = (m * n) ^ j;
        p = (o << 5) | (k >> 3);
        
        /* Use all variables to keep them live */
        result += i ^ j ^ k ^ l ^ m ^ n ^ o ^ p;
        
        /* Address calculation with invariant pointer */
        array[(counter >> 2) & 0x3F] = (int)(result & 0xFFFFFFFF);
        
        /* Use the large constants again, non-adjacent */
        if (counter & 1) {
            result += LARGE_CONST_A;
        } else {
            result += LARGE_CONST_B;
        }
        
        /* More operations keeping variables live */
        i = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        j = j ^ (j >> 13);
        k = k * 6364136223846793005ULL + 1;
        
        /* Use invariant in condition */
        if ((uint64_t)invariant_ptr > invariant_offset) {
            l = l + 1;
        }
    }
    
    /* Force all variables to be used at the end */
    result = result + i + j + k + l + m + n + o + p;
    KEEP_ALIVE;
    return result;
}

/* Function B: Inline assembly with clobbered registers */
NOINLINE static uint64_t function_b(uint64_t input) {
    uint64_t a, b, c, d, e, f, g, h;
    
    /* Register variables to encourage specific allocation */
    register uint64_t r1 asm("ebx") = input;
    register uint64_t r2 asm("edi") = input * 3;
    register uint64_t r3 asm("esi") = input * 7;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "movl %%edi, %%ecx\n\t"
        "movl %%esi, %%edx\n\t"
        "addl $0x12345678, %%eax\n\t"
        "addl $0x9ABCDEF0, %%ecx\n\t"
        "addl $0x11223344, %%edx\n\t"
        : "=a"(a), "=c"(c), "=d"(d)
        : "b"(r1), "D"(r2), "S"(r3)
        : "memory"
    );
    
    /* Another asm with many clobbered registers */
    asm volatile (
        "mov %1, %%eax\n\t"
        "mov %2, %%ecx\n\t"
        "imul %%ecx, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r"(b)
        : "r"(a), "r"(c)
        : "eax", "ecx", "edx", "ebx", "edi", "esi", "memory"
    );
    
    /* Use builtins that return in specific registers */
    uint32_t lo, hi;
    asm volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    e = ((uint64_t)hi << 32) | lo;
    
    /* Chain hard register references */
    f = e + a;
    g = f * b;
    h = g ^ c;
    
    /* Complex expression using all variables */
    uint64_t result = (a * b * c * d) + (e ^ f ^ g ^ h) + (r1 * r2 * r3);
    KEEP_ALIVE;
    return result;
}

/* Function C: Complex control flow with switch statements */
NOINLINE static uint64_t function_c(int mode, uint64_t seed) {
    uint64_t a = seed * 0x5DEECE66DULL + 0xB;
    uint64_t b = seed * 0x6C078965ULL + 0x1;
    uint64_t c = seed * 0x19660DULL + 0x3C6EF35F;
    uint64_t d = seed * 0x343FDULL + 0x269EC3;
    uint64_t e, f, g, h, i, j, k, l;
    
    /* Labels for computed goto */
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3, &&case4 };
    
    e = a; f = b; g = c; h = d;
    
    /* Nested loops with switches inside */
    for (int outer = 0; outer < 100; outer++) {
        for (int inner = 0; inner < 50; inner++) {
            /* Switch creates complex control flow */
            switch ((a + inner + outer) & 3) {
                case 0:
                    i = a * b + 0x12345678;
                    j = c * d + 0x9ABCDEF0;
                    k = e * f + 0x11223344;
                    l = g * h + 0x55667788;
                    break;
                case 1:
                    i = a ^ b ^ 0xF0F0F0F0;
                    j = c ^ d ^ 0x0F0F0F0F;
                    k = e ^ f ^ 0xAAAAAAAA;
                    l = g ^ h ^ 0x55555555;
                    break;
                case 2:
                    i = (a << 3) | (b >> 5);
                    j = (c << 7) | (d >> 1);
                    k = (e << 11) | (f >> 9);
                    l = (g << 13) | (h >> 3);
                    break;
                case 3:
                    i = ~a + 0xDEADBEEF;
                    j = ~b + 0xCAFEBABE;
                    k = ~c + 0xBABEC0DE;
                    l = ~d + 0xFACEB00C;
                    break;
            }
            
            /* Use all variables to keep them live */
            a = (a + i) & 0xFFFFFFFF;
            b = (b + j) ^ 0x12345678;
            c = (c * k) | 0x55555555;
            d = (d ^ l) + 0x9ABCDEF0;
            e = e * 3 + i;
            f = f * 5 + j;
            g = g * 7 + k;
            h = h * 11 + l;
        }
        
        /* Computed goto for additional control flow complexity */
        if (mode < 5) {
            goto *labels[mode];
        }
        
    case0:
        volatile_counter++;
        continue;
    case1:
        a = a ^ 0xAAAAAAAA;
        continue;
    case2:
        b = b * 0xCCCCCCCD;
        continue;
    case3:
        c = c + 0x55555555;
        continue;
    case4:
        d = d | 0xF0F0F0F0;
        continue;
    }
    
    uint64_t result = a + b + c + d + e + f + g + h + i + j + k + l;
    KEEP_ALIVE;
    return result;
}

/* Main function that calls all test patterns */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 7;
    }
    for (int i = 0; i < 128; i++) {
        global_doubles[i] = i * 2.5;
    }
    
    uint64_t result = 0;
    
    /* Call function A with large immediate and pointer */
    result += function_a(1000, global_array);
    
    /* Call function B with different inputs */
    result += function_b(0x12345678);
    result += function_b(0x9ABCDEF0);
    
    /* Call function C with different modes */
    for (int mode = 0; mode < 5; mode++) {
        result += function_c(mode, result + mode);
    }
    
    /* Additional stress: mix all patterns in one more complex function */
    {
        uint64_t temp = result;
        for (int i = 0; i < 100; i++) {
            /* Create many live variables with overlapping ranges */
            uint64_t v1 = temp * 0x5A5A5A5A;
            uint64_t v2 = temp * 0x3C3C3C3C;
            uint64_t v3 = temp * 0x69696969;
            uint64_t v4 = temp * 0x42424242;
            
            /* Use inline assembly with specific register constraints */
            register uint64_t rv1 asm("eax") = v1;
            register uint64_t rv2 asm("ebx") = v2;
            uint64_t out1, out2;
            
            asm volatile (
                "movl %%eax, %%ecx\n\t"
                "movl %%ebx, %%edx\n\t"
                "addl $0x11111111, %%ecx\n\t"
                "addl $0x22222222, %%edx\n\t"
                : "=c"(out1), "=d"(out2)
                : "a"(rv1), "b"(rv2)
                : "memory"
            );
            
            /* Complex expression using all variables */
            temp = (v1 * v2 * v3 * v4) + (out1 ^ out2);
            
            /* Use large immediate constants */
            if (i & 1) {
                temp += 0x123456789ABCDEF0ULL;
            } else {
                temp += 0xFEDCBA9876543210ULL;
            }
        }
        result += temp;
    }
    
    /* Return result to prevent dead code elimination */
    return (int)(result & 0x7FFFFFFF);
}
