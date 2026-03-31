/* test_early_remat.c - Designed to trigger virtual register creation in GCC's early rematerialization pass */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {0x7FFFFFFF, 0x80000000, 0x12345678, 0x9ABCDEF0};
static volatile int sink; /* Prevent optimizations */

/* Function A: Loop with invariants and high register pressure */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int* data) {
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = iterations;
    register int r1 asm("ebx") = data[0];
    register int r2 asm("ecx") = data[1];
    int a = 0x12345678;  /* Non-encodable immediate */
    int b = 0x9ABCDEF0;  /* Another large immediate */
    int c = 0x55555555;
    int d = 0xAAAAAAAA;
    int e = 0x33333333;
    int f = 0xCCCCCCCC;
    int g = 0x0F0F0F0F;
    int h = 0xF0F0F0F0;
    
    /* Loop with invariant address calculation using multiple large immediates */
    for (int i = 0; i < iterations; i++) {
        /* Complex address calculation using invariants */
        int idx = (i * a + b) & 0xFF;  /* Uses a, b (invariants) */
        int val1 = data[idx] * c;      /* Uses c (invariant) */
        int val2 = data[255 - idx] * d; /* Uses d (invariant) */
        
        /* More operations creating register pressure */
        int tmp1 = val1 * e + val2 * f;
        int tmp2 = val1 * g + val2 * h;
        
        /* Use register variables in computation */
        r0 = (r0 * tmp1) >> 3;
        r1 = (r1 + tmp2) & 0xFFFF;
        r2 = (r2 ^ tmp1) | tmp2;
        
        /* Conditional that uses invariants */
        if (idx > (a & 0xFF)) {
            r0 += b & 0xFF;
        }
        
        /* Another invariant use */
        sink = c + d;  /* Volatile write to prevent elimination */
    }
    
    /* Force all values to be live at return */
    return r0 + r1 + r2 + a + b + c + d + e + f + g + h;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int x, int y) {
    int result1, result2, result3;
    
    /* Multi-output inline assembly with many clobbered registers */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "movl %[y], %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%%eax, %%ebx, 2), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "xorl %%edx, %%edx\n\t"
        "divl %%ebx\n\t"
        "movl %%edx, %[out3]\n\t"
        : [out1] "=&r" (result1),  /* Early clobber */
          [out2] "=&r" (result2),  /* Early clobber */
          [out3] "=&r" (result3)   /* Early clobber */
        : [x] "rm" (x),
          [y] "rm" (y)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    /* Create register pressure after asm */
    register int r4 asm("esi") = result1;
    register int r5 asm("edi") = result2;
    
    /* Use large immediates that need rematerialization */
    int imm1 = 0x7FFFFFFF;
    int imm2 = 0x80000000;
    int imm3 = 0x12345678;
    
    /* Complex control flow to extend live ranges */
    for (int i = 0; i < 10; i++) {
        r4 = (r4 * imm1) >> (i & 3);
        r5 = (r5 + imm2) ^ imm3;
        
        /* Inline asm that references specific registers */
        asm volatile (
            "addl %%esi, %%edi\n\t"
            "subl %[imm], %%edi\n\t"
            : "+r" (r5)
            : [imm] "i" (0x100), "r" (r4)
            : "cc"
        );
    }
    
    return r4 + r5 + result3 + imm1 + imm2 + imm3;
}

/* Function C: Complex control flow with switch and computed goto */
__attribute__((noinline, noclone))
int func_complex_flow(int seed, int* data) {
    static void* jump_table[] = { &&label0, &&label1, &&label2, &&label3 };
    
    /* Many temporaries with overlapping lives */
    int t1 = seed * 0x11111111;
    int t2 = seed * 0x22222222;
    int t3 = seed * 0x33333333;
    int t4 = seed * 0x44444444;
    int t5 = seed * 0x55555555;
    int t6 = seed * 0x66666666;
    int t7 = seed * 0x77777777;
    int t8 = seed * 0x88888888;
    
    int result = 0;
    
    /* Nested loops with switch inside */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 10; j++) {
            /* Use all temporaries in computation */
            int idx = (i * t1 + j * t2) & 3;
            
            /* Computed goto creates complex control flow */
            goto *jump_table[idx];
            
        label0:
            result += data[0] * t3 + t4;
            t1 = (t1 ^ t5) + t6;
            continue;
            
        label1:
            result += data[1] * t7 + t8;
            t2 = (t2 ^ t6) + t5;
            continue;
            
        label2:
            result += data[2] * t4 + t3;
            t3 = (t3 ^ t7) + t8;
            continue;
            
        label3:
            result += data[3] * t8 + t7;
            t4 = (t4 ^ t1) + t2;
            continue;
        }
        
        /* Use invariants from outer scope */
        sink = t5 + t6 + t7 + t8;
    }
    
    /* Force all values to be live */
    return result + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
}

/* Function D: Mixed patterns for maximum coverage */
__attribute__((noinline, noclone))
int func_mixed_patterns(void) {
    /* Use target-specific builtins if available */
    uint64_t tsc;
    #ifdef __i386__
    asm volatile ("rdtsc" : "=a" (((uint32_t*)&tsc)[0]), "=d" (((uint32_t*)&tsc)[1]));
    #else
    tsc = 0;
    #endif
    
    /* Register variables with hard constraints */
    register uint32_t low asm("eax") = tsc & 0xFFFFFFFF;
    register uint32_t high asm("edx") = tsc >> 32;
    
    /* Many large immediates */
    const uint32_t imms[] = {
        0xDEADBEEF, 0xCAFEBABE, 0xBAADF00D, 0xFEEDFACE,
        0x8BADF00D, 0xC00010FF, 0x1337C0DE, 0xACC01ADE
    };
    
    uint32_t sum = 0;
    
    /* Loop that uses all patterns */
    for (int i = 0; i < 8; i++) {
        /* Use hard register results */
        uint32_t tmp = low + high;
        
        /* Switch with register variables */
        switch (i & 3) {
            case 0:
                sum += tmp * imms[i];
                low = (low << 3) | (high >> 29);
                break;
            case 1:
                sum += tmp ^ imms[i];
                high = (high << 5) | (low >> 27);
                break;
            case 2:
                sum += tmp + imms[i];
                low = low ^ imms[(i + 1) & 7];
                break;
            case 3:
                sum += tmp - imms[i];
                high = high ^ imms[(i + 2) & 7];
                break;
        }
        
        /* Inline asm with clobbers */
        asm volatile (
            "movl %[low], %%eax\n\t"
            "movl %[high], %%edx\n\t"
            "addl %%edx, %%eax\n\t"
            "movl %%eax, %[sum]\n\t"
            : [sum] "+rm" (sum)
            : [low] "rm" (low), [high] "rm" (high)
            : "eax", "edx", "cc"
        );
    }
    
    return sum + low + high;
}

/* Main function that calls all test patterns */
int main(int argc, char** argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    int result = 0;
    
    /* Call each function with arguments that create register pressure */
    result += func_loop_invariants(
        argc > 1 ? atoi(argv[1]) : 100,
        global_array
    );
    
    result += func_asm_clobber(0x123456, 0x789ABC);
    
    result += func_complex_flow(
        argc > 2 ? atoi(argv[2]) : 42,
        global_array
    );
    
    result += func_mixed_patterns();
    
    /* Use result to prevent optimization */
    return result & 0xFF;
}
