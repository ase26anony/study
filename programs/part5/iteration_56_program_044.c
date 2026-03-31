/* test_early_remat.c - Target specific coverage for early-remat.cc lines 930-937 */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256] = {0};
static double global_doubles[128] = {0.0};
static char global_chars[512] = {0};

/* Large immediate constants that are expensive to materialize */
#define EXPENSIVE_CONST_1 0xDEADBEEF
#define EXPENSIVE_CONST_2 0xCAFEBABE
#define EXPENSIVE_CONST_3 0x12345678
#define EXPENSIVE_CONST_4 0x87654321

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int* restrict out) {
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = EXPENSIVE_CONST_1;
    register int r1 asm("ebx") = EXPENSIVE_CONST_2;
    int a = EXPENSIVE_CONST_3;
    int b = EXPENSIVE_CONST_4;
    int c = (int)(global_array);
    int d = (int)(global_doubles);
    int e = (int)(global_chars);
    int f = 0x55555555;
    int g = 0xAAAAAAAA;
    
    /* Loop with invariant address calculations */
    for (int i = 0; i < iterations; i++) {
        /* Use all invariants in complex expressions */
        int idx1 = (i * a + b) % 256;
        int idx2 = (i * c + d) % 128;
        int idx3 = (i * e + f) % 512;
        
        /* Multiple uses of invariants in different places */
        global_array[idx1] += r0 + r1;
        global_doubles[idx2 % 128] += (double)(a + b);
        global_chars[idx3] = (char)(c ^ d ^ e ^ f ^ g);
        
        /* More overlapping live ranges */
        int t1 = global_array[idx1] * a;
        int t2 = global_doubles[idx2 % 128] * b;
        int t3 = global_chars[idx3] * c;
        int t4 = t1 + t2 + t3 + d;
        int t5 = t4 * e + f;
        int t6 = t5 / g + r0;
        
        out[i] = t6 + r1;
        
        /* Force register pressure with many temporaries */
        int tmp1 = a * i;
        int tmp2 = b * i;
        int tmp3 = c * i;
        int tmp4 = d * i;
        int tmp5 = e * i;
        int tmp6 = f * i;
        int tmp7 = g * i;
        int tmp8 = r0 * i;
        int tmp9 = r1 * i;
        
        /* Use all temporaries to keep them live */
        out[i] += tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8 + tmp9;
    }
    
    return a + b + c + d + e + f + g + r0 + r1;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int x, int y) {
    int result1, result2, result3;
    
    /* Multi-output inline assembly with many clobbered registers */
    asm volatile (
        "movl %4, %%eax\n\t"
        "movl %5, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "imull $0x123456, %%eax, %%ecx\n\t"
        "movl %%ecx, %1\n\t"
        "leal (%%eax, %%ebx, 4), %%edx\n\t"
        "movl %%edx, %2\n\t"
        : "=r" (result1), "=r" (result2), "=r" (result3)
        : "0" (x), "r" (y)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory", "cc"
    );
    
    /* Use register variables with specific constraints */
    register int r10 asm("esi") = result1;
    register int r11 asm("edi") = result2;
    
    /* Complex control flow with register variables */
    volatile int choice = x % 4;
    switch (choice) {
        case 0:
            r10 = r10 * EXPENSIVE_CONST_1 + r11;
            break;
        case 1:
            r10 = r10 / EXPENSIVE_CONST_2 - r11;
            break;
        case 2:
            r10 = r10 ^ EXPENSIVE_CONST_3 | r11;
            break;
        case 3:
            r10 = r10 & EXPENSIVE_CONST_4 ^ r11;
            break;
    }
    
    /* More inline assembly using the register variables */
    asm volatile (
        "addl %%esi, %%edi\n\t"
        "movl %%edi, %0\n\t"
        : "=r" (result1)
        : 
        : "esi", "edi", "cc"
    );
    
    return result1 + result2 + result3 + r10 + r11;
}

/* Function C: Complex control flow with many temporaries */
__attribute__((noinline, noclone))
int func_complex_flow(int n, int seed) {
    /* Many local variables that will have overlapping live ranges */
    int v1 = seed + EXPENSIVE_CONST_1;
    int v2 = seed * EXPENSIVE_CONST_2;
    int v3 = seed ^ EXPENSIVE_CONST_3;
    int v4 = seed | EXPENSIVE_CONST_4;
    int v5 = (int)global_array + seed;
    int v6 = (int)global_doubles + seed;
    int v7 = (int)global_chars + seed;
    int v8 = 0x11111111;
    int v9 = 0x22222222;
    int v10 = 0x33333333;
    
    /* Nested loops with switch inside */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 10; j++) {
            /* Switch with computed goto-like behavior */
            int mod = (i * j + seed) % 8;
            
            /* Many temporaries with overlapping lives */
            int t1 = v1 * i + v2 * j;
            int t2 = v3 * i - v4 * j;
            int t3 = v5 * i ^ v6 * j;
            int t4 = v7 * i | v8 * j;
            int t5 = v9 * i & v10 * j;
            
            switch (mod) {
                case 0:
                    sum += t1 + t2 + global_array[(t3) % 256];
                    break;
                case 1:
                    sum += t2 + t3 + global_array[(t4) % 256];
                    break;
                case 2:
                    sum += t3 + t4 + global_array[(t5) % 256];
                    break;
                case 3:
                    sum += t4 + t5 + global_array[(t1) % 256];
                    break;
                case 4:
                    sum += t5 + t1 + global_array[(t2) % 256];
                    break;
                case 5:
                    sum += t1 * t2 - t3 * t4;
                    break;
                case 6:
                    sum += t2 * t3 - t4 * t5;
                    break;
                case 7:
                    sum += t3 * t4 - t5 * t1;
                    break;
            }
            
            /* Use all variables again to extend live ranges */
            v1 = (v1 + t1) % 1000;
            v2 = (v2 + t2) % 1000;
            v3 = (v3 + t3) % 1000;
            v4 = (v4 + t4) % 1000;
            v5 = (v5 + t5) % 1000;
            v6 = (v6 + t1) % 1000;
            v7 = (v7 + t2) % 1000;
            v8 = (v8 + t3) % 1000;
            v9 = (v9 + t4) % 1000;
            v10 = (v10 + t5) % 1000;
        }
    }
    
    /* Final computation using all variables */
    return sum + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Function D: Using builtins for specific register references */
#ifdef __i386__
__attribute__((noinline, noclone))
uint64_t func_builtin_register(uint32_t a, uint32_t b) {
    /* Use rdtsc which returns in eax:edx */
    uint64_t tsc1 = __builtin_ia32_rdtsc();
    
    /* Use the results in subsequent expressions */
    uint32_t tsc_low = (uint32_t)tsc1;
    uint32_t tsc_high = (uint32_t)(tsc1 >> 32);
    
    /* Create chains of hard register references */
    register uint32_t r_eax asm("eax") = tsc_low + a;
    register uint32_t r_edx asm("edx") = tsc_high + b;
    
    /* Complex expressions using register variables */
    uint32_t x = r_eax * EXPENSIVE_CONST_1;
    uint32_t y = r_edx * EXPENSIVE_CONST_2;
    uint32_t z = x ^ y;
    
    /* Another rdtsc to create more register pressure */
    uint64_t tsc2 = __builtin_ia32_rdtsc();
    
    /* Mix with global address calculations */
    uint32_t addr1 = (uint32_t)global_array + z;
    uint32_t addr2 = (uint32_t)global_doubles + (uint32_t)tsc2;
    uint32_t addr3 = (uint32_t)global_chars + (uint32_t)(tsc2 >> 32);
    
    /* Force many values to be live simultaneously */
    return (uint64_t)x * y * z * addr1 * addr2 * addr3 * tsc1 * tsc2;
}
#endif

/* Main function that calls all test patterns */
int main(int argc, char* argv[]) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    int* output_buffer = malloc(iterations * sizeof(int));
    
    /* Call all test functions to trigger different patterns */
    int result1 = func_loop_invariants(iterations, output_buffer);
    int result2 = func_asm_clobber(iterations, result1);
    int result3 = func_complex_flow(iterations, result2);
    
    #ifdef __i386__
    uint64_t result4 = func_builtin_register(result1, result2);
    result3 += (int)result4;
    #endif
    
    /* Combine results to prevent optimization */
    int final_result = result1 + result2 + result3;
    
    /* Use results to affect global state */
    for (int i = 0; i < iterations && i < 256; i++) {
        global_array[i % 256] += output_buffer[i];
    }
    
    free(output_buffer);
    
    /* Return final result based on global state */
    return final_result + global_array[0] + global_array[255];
}
