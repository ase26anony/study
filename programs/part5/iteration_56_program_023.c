/* test-early-remat.c */
#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {
    0x123456789ABCDEF0, 0xFEDCBA9876543210,
    0x5555555555555555, 0xAAAAAAAAAAAAAAAA,
    0x1111111111111111, 0x2222222222222222
};

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, noclone))
#define KEEP_ALIVE asm volatile("" : : "r"(result) : "memory")

/* Function A: Loop with invariants and high register pressure */
NOINLINE static uint64_t func_loop_invariants(int iterations, const int* data) {
    /* Many local variables with overlapping live ranges */
    register uint64_t a asm("eax") = large_constants[0];
    register uint64_t b asm("ebx") = large_constants[1];
    register uint64_t c asm("ecx") = large_constants[2];
    uint64_t d = large_constants[3];
    uint64_t e = large_constants[4];
    uint64_t f = large_constants[5];
    uint64_t g = (uint64_t)global_array;
    uint64_t h = (uint64_t)data;
    
    /* Loop with invariant calculations using non-encodable immediates */
    for (int i = 0; i < iterations; i++) {
        /* Multiple uses of invariants in different expressions */
        uint64_t addr1 = g + (i * 16) + a;
        uint64_t addr2 = h + (i * 8) + b;
        
        /* Complex arithmetic creating register pressure */
        d = (d * c) ^ 0x12345678;
        e = (e + a) | 0x87654321;
        f = (f - b) & 0xF0F0F0F0F0F0F0F0;
        
        /* Use invariants in condition */
        if ((addr1 & 0xFF) > (addr2 & 0xFF)) {
            d += c;
        } else {
            e += a;
        }
        
        /* More overlapping live ranges */
        uint64_t temp1 = d * e;
        uint64_t temp2 = f * g;
        uint64_t temp3 = h + temp1;
        uint64_t temp4 = a + temp2;
        
        /* Force all values to be live */
        asm volatile("" : : "r"(temp1), "r"(temp2), "r"(temp3), "r"(temp4));
    }
    
    /* Combine results to prevent dead code elimination */
    uint64_t result = a ^ b ^ c ^ d ^ e ^ f ^ g ^ h;
    KEEP_ALIVE;
    return result;
}

/* Function B: Inline assembly with clobbered registers */
NOINLINE static uint64_t func_asm_clobber(int x, int y) {
    uint64_t result;
    
    /* Multi-output inline assembly with hard register constraints */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" (x), "r" (y)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Use result in complex expressions */
    register uint64_t r1 asm("eax") = result;
    register uint64_t r2 asm("ebx") = result * 2;
    register uint64_t r3 asm("ecx") = result * 3;
    
    /* Chain of hard register references */
    for (int i = 0; i < 100; i++) {
        asm volatile (
            "addl %%eax, %%ebx\n\t"
            "addl %%ebx, %%ecx\n\t"
            : "+r" (r1), "+r" (r2), "+r" (r3)
            :
            : "cc"
        );
        
        /* Mix with large constants */
        r1 += large_constants[i % 6];
        r2 ^= 0xDEADBEEFCAFEBABE;
        r3 |= 0x7FFFFFFFFFFFFFFF;
    }
    
    result = r1 + r2 + r3;
    KEEP_ALIVE;
    return result;
}

/* Function C: Complex control flow with register variables */
NOINLINE static uint64_t func_complex_flow(int selector, int count) {
    /* Many register variables */
    register uint64_t v1 asm("eax");
    register uint64_t v2 asm("ebx");
    register uint64_t v3 asm("ecx");
    register uint64_t v4 asm("edx");
    register uint64_t v5 asm("esi");
    register uint64_t v6 asm("edi");
    
    /* Initialize with different values */
    v1 = large_constants[0];
    v2 = large_constants[1];
    v3 = large_constants[2];
    v4 = large_constants[3];
    v5 = large_constants[4];
    v6 = large_constants[5];
    
    /* Labels for computed goto */
    void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    /* Complex control flow */
    for (int i = 0; i < count; i++) {
        int idx = (selector + i) % 5;
        goto *labels[idx];
        
    label0:
        v1 = (v1 * v2) + 0x1234567890ABCDEF;
        v3 = v4 ^ v5;
        continue;
        
    label1:
        v2 = (v2 - v3) | 0xF0F0F0F0F0F0F0F0;
        v4 = v5 & v6;
        continue;
        
    label2:
        v3 = (v3 + v4) * 0x5555555555555555;
        v5 = v6 | v1;
        continue;
        
    label3:
        v4 = (v4 ^ v5) - 0x1111111111111111;
        v6 = v1 & v2;
        continue;
        
    label4:
        v5 = (v5 | v6) + 0x2222222222222222;
        v1 = v2 ^ v3;
        continue;
    }
    
    /* Switch statement mixing register variables */
    switch (selector % 3) {
        case 0:
            v1 = v2 + v3;
            v4 = v5 * v6;
            break;
        case 1:
            v2 = v3 - v4;
            v5 = v6 / (v1 ? v1 : 1);
            break;
        case 2:
            v3 = v4 ^ v5;
            v6 = v1 | v2;
            break;
    }
    
    uint64_t result = v1 + v2 + v3 + v4 + v5 + v6;
    KEEP_ALIVE;
    return result;
}

/* Function D: Mixed patterns for maximum pressure */
NOINLINE static uint64_t func_mixed_patterns(int* ptr, int n) {
    /* Use target-specific builtins if available */
    #ifdef __i386__
    uint64_t tsc1, tsc2;
    asm volatile("rdtsc" : "=A"(tsc1));
    #endif
    
    /* Many temporaries with overlapping scopes */
    uint64_t accum = 0;
    uint64_t base_addr = (uint64_t)ptr;
    
    for (int i = 0; i < n; i++) {
        /* Multiple invariants used in different places */
        uint64_t inv1 = large_constants[0];
        uint64_t inv2 = large_constants[1];
        uint64_t inv3 = (uint64_t)&global_array[0];
        
        /* Complex address calculations */
        uint64_t addr1 = base_addr + (i * inv1) % 256;
        uint64_t addr2 = inv3 + (i * inv2) % 256;
        
        /* Many intermediate values kept live */
        uint64_t t1 = *(int*)addr1 * inv1;
        uint64_t t2 = *(int*)addr2 * inv2;
        uint64_t t3 = t1 + t2 + inv1;
        uint64_t t4 = t1 * t2 - inv2;
        uint64_t t5 = t3 ^ t4;
        uint64_t t6 = t5 | inv3;
        
        /* Force spilling by using all in expression */
        accum += t1 + t2 + t3 + t4 + t5 + t6;
        
        /* Conditional with more live values */
        if (i % 3 == 0) {
            uint64_t extra1 = accum * inv1;
            uint64_t extra2 = extra1 ^ inv2;
            uint64_t extra3 = extra2 | inv3;
            accum += extra1 + extra2 + extra3;
        }
    }
    
    #ifdef __i386__
    asm volatile("rdtsc" : "=A"(tsc2));
    accum ^= (tsc2 - tsc1);
    #endif
    
    KEEP_ALIVE;
    return accum;
}

/* Main function to drive everything */
int main(int argc, char** argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    /* Get some arguments to prevent constant propagation */
    int iter = argc > 1 ? atoi(argv[1]) : 1000;
    int selector = argc > 2 ? atoi(argv[2]) : 5;
    
    /* Call all test functions to create register pressure */
    uint64_t sum = 0;
    
    sum += func_loop_invariants(iter, global_array);
    sum += func_asm_clobber(iter, selector);
    sum += func_complex_flow(selector, iter / 10);
    sum += func_mixed_patterns(global_array, iter / 2);
    
    /* Return result to prevent optimization */
    return (int)(sum % 1000000);
}
