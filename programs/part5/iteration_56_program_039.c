/* test_early_remat.c - Target coverage for early-remat.cc lines 930-937 */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {
    0x123456789ABCDEF0, 0xFEDCBA9876543210,
    0xDEADBEEFCAFEBABE, 0x0BADF00D12345678
};

/* Prevent optimizations from simplifying our patterns */
#define NOINLINE __attribute__((noinline, noclone))
#define KEEP_ALIVE(expr) asm volatile("" : : "r"(expr))

/* Function A: Loop with invariants and expensive constants */
NOINLINE static uint64_t func_loop_invariants(int iterations, int *data) {
    /* Many local variables with overlapping live ranges */
    register uint64_t a asm("ebx") = large_constants[0];
    register uint64_t b asm("edi") = large_constants[1];
    uint64_t c = large_constants[2];
    uint64_t d = large_constants[3];
    uint64_t e = (uint64_t)global_array + 0x7FFFFFFF;
    uint64_t f = (uint64_t)data + 0x3FFFFFFF;
    uint64_t sum = 0;
    
    /* Loop with invariant address calculations */
    for (int i = 0; i < iterations; i++) {
        /* Use all invariants in complex expressions */
        uint64_t addr1 = e + i * 256;
        uint64_t addr2 = f + i * 128;
        
        /* Multiple uses of invariants in non-adjacent calculations */
        uint64_t val1 = (a ^ c) + (uint64_t)(*(int*)addr1);
        uint64_t val2 = (b & d) + (uint64_t)(*(int*)addr2);
        
        /* More overlapping live ranges */
        uint64_t tmp1 = val1 * 0x987654321;
        uint64_t tmp2 = val2 * 0x123456789;
        uint64_t tmp3 = tmp1 ^ tmp2;
        uint64_t tmp4 = tmp3 + a;
        uint64_t tmp5 = tmp4 - b;
        
        sum += tmp5 + (c >> 32) + (d << 32);
        
        /* Force register pressure with many temporaries */
        KEEP_ALIVE(addr1);
        KEEP_ALIVE(addr2);
        KEEP_ALIVE(val1);
        KEEP_ALIVE(val2);
        KEEP_ALIVE(tmp1);
        KEEP_ALIVE(tmp2);
        KEEP_ALIVE(tmp3);
        KEEP_ALIVE(tmp4);
        KEEP_ALIVE(tmp5);
    }
    
    /* Use register variables in final computation */
    asm volatile("" : "+r"(a), "+r"(b));
    return sum + a + b;
}

/* Function B: Inline assembly with clobbered registers */
NOINLINE static uint64_t func_asm_clobber(uint64_t x, uint64_t y) {
    uint64_t result1, result2, result3;
    
    /* Multi-output inline assembly with many clobbers */
    asm volatile (
        "movl %[x_lo], %%eax\n\t"
        "movl %[x_hi], %%edx\n\t"
        "addl %[y_lo], %%eax\n\t"
        "adcl %[y_hi], %%edx\n\t"
        "movl %%eax, %[r1_lo]\n\t"
        "movl %%edx, %[r1_hi]\n\t"
        "imull $0x1234567, %%eax, %%ecx\n\t"
        "imull $0x89ABCDE, %%edx, %%ebx\n\t"
        : [r1_lo] "=&r" (*(uint32_t*)&result1),
          [r1_hi] "=&r" (*((uint32_t*)&result1 + 1)),
          "=c" (*(uint32_t*)&result2),
          "=b" (*((uint32_t*)&result2 + 1))
        : [x_lo] "rm" ((uint32_t)x),
          [x_hi] "rm" ((uint32_t)(x >> 32)),
          [y_lo] "rm" ((uint32_t)y),
          [y_hi] "rm" ((uint32_t)(y >> 32))
        : "eax", "edx", "memory", "cc"
    );
    
    /* More assembly with different register constraints */
    register uint64_t r3 asm("esi");
    asm volatile (
        "rdtsc\n\t"
        "shlq $32, %%rdx\n\t"
        "orq %%rax, %%rdx\n\t"
        "movq %%rdx, %0\n\t"
        : "=r" (r3)
        : 
        : "rax", "rdx", "cc"
    );
    
    /* Use results in complex expressions */
    result3 = (result1 * 0xFFFFFFFF) + (result2 * 0xAAAAAAAA) + r3;
    
    /* Force spilling with many live values */
    uint64_t t1 = result1 ^ 0x5555555555555555;
    uint64_t t2 = result2 ^ 0xAAAAAAAAAAAAAAAA;
    uint64_t t3 = r3 ^ 0x3333333333333333;
    uint64_t t4 = t1 + t2;
    uint64_t t5 = t3 + t4;
    uint64_t t6 = t5 * 0x123456789ABCDEF;
    
    KEEP_ALIVE(t1);
    KEEP_ALIVE(t2);
    KEEP_ALIVE(t3);
    KEEP_ALIVE(t4);
    KEEP_ALIVE(t5);
    KEEP_ALIVE(t6);
    
    return result3 + t6;
}

/* Function C: Complex control flow with register variables */
NOINLINE static uint64_t func_complex_cf(int selector, uint64_t seed) {
    /* Declare many register variables */
    register uint64_t r1 asm("ebx");
    register uint64_t r2 asm("edi");
    register uint64_t r3 asm("esi");
    register uint64_t r4 asm("ebp");
    
    r1 = seed * 0x5A827999;
    r2 = seed * 0x6ED9EBA1;
    r3 = seed * 0x8F1BBCDC;
    r4 = seed * 0xCA62C1D6;
    
    uint64_t result = 0;
    
    /* Nested loops with switch inside */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 50; j++) {
            /* Complex switch with register variables */
            switch ((selector + i + j) & 7) {
                case 0:
                    result += r1 + (uint64_t)&global_array[i];
                    r1 = r1 * 0x9E3779B9 + i;
                    break;
                case 1:
                    result += r2 ^ (uint64_t)&global_array[j];
                    r2 = r2 * 0x9E3779B9 + j;
                    break;
                case 2:
                    result += r3 | (uint64_t)&large_constants[i & 3];
                    r3 = r3 * 0x9E3779B9 + (i ^ j);
                    break;
                case 3:
                    result += r4 & (uint64_t)&large_constants[j & 3];
                    r4 = r4 * 0x9E3779B9 + (i + j);
                    break;
                case 4:
                    result += (r1 << 3) + (r2 >> 5);
                    r1 ^= r2;
                    r2 ^= r1;
                    r1 ^= r2;
                    break;
                case 5:
                    result += (r3 * 13) + (r4 * 17);
                    r3 += 0x12345678;
                    r4 += 0x87654321;
                    break;
                case 6:
                    result += r1 + r2 + r3 + r4;
                    /* Create many temporaries */
                    {
                        uint64_t t1 = r1 * 19;
                        uint64_t t2 = r2 * 23;
                        uint64_t t3 = r3 * 29;
                        uint64_t t4 = r4 * 31;
                        result += t1 + t2 + t3 + t4;
                        KEEP_ALIVE(t1);
                        KEEP_ALIVE(t2);
                        KEEP_ALIVE(t3);
                        KEEP_ALIVE(t4);
                    }
                    break;
                case 7:
                    result += (r1 ^ r2) | (r3 & r4);
                    /* More register pressure */
                    asm volatile("" : "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4));
                    break;
            }
            
            /* Additional computations to extend live ranges */
            if (j & 1) {
                uint64_t tmp = r1 + r2;
                result += tmp * 0x10001;
                KEEP_ALIVE(tmp);
            } else {
                uint64_t tmp = r3 - r4;
                result += tmp * 0x10001;
                KEEP_ALIVE(tmp);
            }
        }
    }
    
    return result;
}

/* Main function that calls all test patterns */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    uint64_t total = 0;
    
    /* Call function A with loop invariants */
    total += func_loop_invariants(
        argc > 1 ? atoi(argv[1]) : 100,
        global_array
    );
    
    /* Call function B with inline assembly */
    total += func_asm_clobber(
        0x123456789ABCDEF0,
        0xFEDCBA9876543210
    );
    
    /* Call function C with complex control flow */
    total += func_complex_cf(
        argc > 2 ? atoi(argv[2]) : 42,
        0xDEADBEEFCAFEBABE
    );
    
    /* Additional calls to increase compilation complexity */
    for (int i = 0; i < 3; i++) {
        total += func_loop_invariants(50 + i * 10, &global_array[i * 64]);
        total += func_asm_clobber(total, total ^ 0xAAAAAAAA);
        total += func_complex_cf(i, total);
    }
    
    /* Return result to prevent dead code elimination */
    return (int)(total & 0x7FFFFFFF);
}
