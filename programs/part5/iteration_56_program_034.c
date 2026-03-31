/* test_early_remat.c - Designed to trigger virtual register creation in GCC's early rematerialization pass */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static double global_doubles[128];
static char global_chars[512];

/* Large constants that are expensive to materialize */
#define EXPENSIVE_CONSTANT_1 0xDEADBEEF
#define EXPENSIVE_CONSTANT_2 0xCAFEBABE
#define EXPENSIVE_CONSTANT_3 0x8BADF00D
#define EXPENSIVE_CONSTANT_4 0x1BADB002

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int function_a(int iterations, int* data) {
    /* Use many local variables with overlapping live ranges */
    register int r0 asm("eax") = EXPENSIVE_CONSTANT_1;
    register int r1 asm("ebx") = EXPENSIVE_CONSTANT_2;
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    int sum = 0;
    
    /* Loop invariants - these should be rematerialized */
    const int* array_end = global_array + 256;
    const double* double_ptr = global_doubles;
    const long invariant1 = (long)(global_chars + 256);
    const int invariant2 = EXPENSIVE_CONSTANT_3;
    
    /* Complex loop with many live values */
    for (a = 0; a < iterations; a++) {
        /* Use invariants in multiple places */
        b = *((int*)invariant1 - a);
        c = global_array[a] + invariant2;
        
        /* More operations creating register pressure */
        d = r0 * a + r1;
        e = data[a] * b;
        f = c + d + e;
        
        /* Use invariants again in different expressions */
        g = (array_end - global_array) - a;
        h = (int)((double_ptr - global_doubles) * 2);
        
        /* Chain computations with overlapping live ranges */
        i = f * g;
        j = h * a;
        k = i + j;
        l = k * invariant2;
        m = l / (a + 1);
        n = m + (int)invariant1;
        o = n * global_array[a % 256];
        p = o - data[a];
        
        sum += p;
        
        /* Force all variables to stay live across loop iterations */
        r0 = r0 ^ b;
        r1 = r1 + c;
        data[a] = d + e + f + g + h + i + j + k + l + m + n + o + p;
    }
    
    return sum + r0 + r1;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int function_b(int x, int y) {
    int result1, result2, result3, result4;
    register int reg_var1 asm("esi") = EXPENSIVE_CONSTANT_4;
    register int reg_var2 asm("edi") = x * 2;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %[input1], %%eax\n\t"
        "movl %[input2], %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%%eax, %%ebx, 4), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "addl $0x12345678, %%edx\n\t"
        "movl %%edx, %[out3]\n\t"
        "subl %%esi, %%edi\n\t"
        "movl %%edi, %[out4]"
        : [out1] "=&r" (result1), 
          [out2] "=&r" (result2),
          [out3] "=&r" (result3),
          [out4] "=&r" (result4)
        : [input1] "r" (x), 
          [input2] "r" (y),
          "r" (reg_var1),
          "r" (reg_var2)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory", "cc"
    );
    
    /* Use results in complex expressions */
    int t1 = result1 + EXPENSIVE_CONSTANT_1;
    int t2 = result2 * EXPENSIVE_CONSTANT_2;
    int t3 = result3 / (EXPENSIVE_CONSTANT_3 & 0xFFF);
    int t4 = result4 ^ EXPENSIVE_CONSTANT_4;
    
    /* More register pressure */
    for (int i = 0; i < 16; i++) {
        t1 = t1 * t2 + t3;
        t2 = t2 - t4 * i;
        t3 = t3 ^ (t1 >> (i % 8));
        t4 = t4 + global_array[i] * t2;
    }
    
    return t1 + t2 + t3 + t4 + reg_var1 + reg_var2;
}

/* Function C: Complex control flow with switch and computed goto */
__attribute__((noinline, noclone))
int function_c(int mode, int value) {
    static void* jump_table[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    int a = value * 2;
    int b = a + EXPENSIVE_CONSTANT_1;
    int c = b * 3;
    int d = c - EXPENSIVE_CONSTANT_2;
    int e = d / 2;
    int f = e ^ EXPENSIVE_CONSTANT_3;
    int g = f + 0xABCD;
    int h = g * a;
    int i = h - b;
    int j = i + c;
    int k = j * d;
    int l = k / e;
    int m = l ^ f;
    int n = m + g;
    int o = n * h;
    int p = o - i;
    
    /* Use computed goto for unpredictable control flow */
    if (mode >= 0 && mode < 5) {
        goto *jump_table[mode];
    }
    
    /* Default path with all variables live */
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    
label0:
    return a * b + global_array[value % 256];
label1:
    return c - d + global_doubles[value % 128];
label2:
    return e * f * (int)(global_chars[value % 512]);
label3:
    return g + h + i + j;
label4:
    return k * l * m * n * o * p;
}

/* Function D: Mixed hard/soft register usage with builtins */
__attribute__((noinline, noclone))
uint64_t function_d(int seed) {
    /* Use timestamp counter builtin (returns in edx:eax on x86) */
    uint64_t tsc1 = __builtin_ia32_rdtsc();
    
    /* Chain hard register results through computations */
    uint32_t low1 = (uint32_t)tsc1;
    uint32_t high1 = (uint32_t)(tsc1 >> 32);
    
    /* Many intermediate values creating register pressure */
    uint32_t x1 = low1 * EXPENSIVE_CONSTANT_1;
    uint32_t x2 = high1 + EXPENSIVE_CONSTANT_2;
    uint32_t x3 = x1 ^ x2;
    uint32_t x4 = x3 * seed;
    uint32_t x5 = x4 - EXPENSIVE_CONSTANT_3;
    uint32_t x6 = x5 / (seed + 1);
    uint32_t x7 = x6 + EXPENSIVE_CONSTANT_4;
    uint32_t x8 = x7 * low1;
    uint32_t x9 = x8 ^ high1;
    uint32_t x10 = x9 + x1;
    uint32_t x11 = x10 * x2;
    uint32_t x12 = x11 - x3;
    uint32_t x13 = x12 / x4;
    uint32_t x14 = x13 ^ x5;
    uint32_t x15 = x14 + x6;
    uint32_t x16 = x15 * x7;
    
    /* Another timestamp to create more hard register references */
    uint64_t tsc2 = __builtin_ia32_rdtsc();
    uint32_t low2 = (uint32_t)tsc2;
    uint32_t high2 = (uint32_t)(tsc2 >> 32);
    
    /* Mix everything together */
    return (uint64_t)(x16 + low2) * (uint64_t)(high1 + high2 + x8 + x9 + x10 + x11 + x12 + x13 + x14 + x15);
}

/* Main function that exercises all patterns */
int main(int argc, char** argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 - 127;
    }
    for (int i = 0; i < 128; i++) {
        global_doubles[i] = i * 1.5;
    }
    for (int i = 0; i < 512; i++) {
        global_chars[i] = (i % 256) - 128;
    }
    
    int* heap_data = (int*)malloc(1024 * sizeof(int));
    for (int i = 0; i < 1024; i++) {
        heap_data[i] = i - 512;
    }
    
    /* Call all test functions with different patterns */
    int result = 0;
    
    /* Function A: Loop with invariants */
    result += function_a(argc > 1 ? atoi(argv[1]) : 100, heap_data);
    
    /* Function B: Inline assembly */
    result += function_b(EXPENSIVE_CONSTANT_1 & 0xFFF, EXPENSIVE_CONSTANT_2 & 0xFFF);
    
    /* Function C: Complex control flow */
    for (int i = 0; i < 5; i++) {
        result += function_c(i, result & 0xFF);
    }
    
    /* Function D: Builtin usage */
    result += (int)function_d(result);
    
    /* Use result to prevent optimization */
    free(heap_data);
    return result & 0x7FFFFFFF; /* Ensure positive return for exit code */
}
