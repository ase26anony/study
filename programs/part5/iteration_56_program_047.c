/* test_early_remat.c - Designed to trigger virtual register creation in GCC's early rematerialization pass */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {0x12345678, 0x9ABCDEF0, 0x55555555, 0xAAAAAAAA};
static volatile int volatile_sink;

/* Prevent optimizations from removing our code */
#define KEEP_ALIVE(expr) do { volatile_sink = (expr); } while(0)

/* Function A: Loop with invariants and high register pressure */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int *data) {
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = iterations;
    register int r1 asm("ebx") = data[0];
    register int r2 asm("ecx") = data[1];
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    long invariant1 = 0x7FFFFFFF;  /* Large immediate */
    long invariant2 = (long)&global_array[128];  /* Symbolic address */
    int *invariant_ptr = &global_array[64];
    const long *invariant_const_ptr = &large_constants[2];
    
    /* Complex loop with many uses of invariants */
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in multiple non-adjacent calculations */
        v1 = (i * invariant1) >> 3;
        v2 = ((int)invariant2 + i) & 0xFF;
        v3 = invariant_ptr[v2] + (int)invariant1;
        v4 = (int)invariant_const_ptr[0] + i;
        v5 = (int)invariant_const_ptr[1] - i;
        
        /* More calculations creating register pressure */
        v6 = v1 + v2 + v3;
        v7 = v4 * v5 / (i + 1);
        v8 = v6 ^ v7;
        v9 = (v8 << 3) | (v8 >> 29);
        v10 = v9 + (int)(invariant2 >> 16);
        
        /* Use register variables */
        r0 = r0 + v10;
        r1 = r1 ^ v9;
        r2 = r2 * v8;
        
        /* More variables to increase pressure */
        v11 = v10 * 2;
        v12 = v11 + 0x123456;  /* Large immediate */
        v13 = v12 - 0x89ABCD;
        v14 = v13 | 0x55555555;
        v15 = v14 & 0xAAAAAAAA;
        v16 = v15 ^ v14;
        v17 = v16 << 1;
        v18 = v17 >> 2;
        v19 = v18 + invariant_ptr[i & 63];
        v20 = v19 - (int)invariant_const_ptr[i & 1];
        
        /* Conditional to create control flow complexity */
        if (i & 1) {
            r0 += v20;
        } else {
            r1 += v20;
        }
        
        /* Use invariants again */
        data[i] = v20 + (int)invariant1;
    }
    
    /* Return using register variables */
    asm volatile ("" : "+r"(r0), "+r"(r1), "+r"(r2));
    return r0 + r1 + r2;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int a, int b, int c) {
    int result1, result2, result3;
    register int reg_var1 asm("esi") = a;
    register int reg_var2 asm("edi") = b;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %[input1], %%eax\n\t"
        "movl %[input2], %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "addl %[input3], %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%%eax, %%ebx, 4), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "xorl %%edx, %%edx\n\t"
        "divl %%ebx\n\t"
        "movl %%edx, %[out3]"
        : [out1] "=&r" (result1), [out2] "=&r" (result2), [out3] "=&r" (result3)
        : [input1] "r" (reg_var1), [input2] "r" (reg_var2), [input3] "rm" (c)
        : "eax", "ebx", "ecx", "edx", "cc", "memory"
    );
    
    /* Use results in complex expressions */
    int x1 = result1 + 0x10000000;  /* Large immediate */
    int x2 = result2 - 0x20000000;
    int x3 = result3 | 0x40000000;
    
    /* Chain of calculations */
    for (int i = 0; i < 8; i++) {
        x1 = (x1 << i) | (x1 >> (32 - i));
        x2 = x2 ^ (0x55555555 * i);
        x3 = x3 + ((int)&global_array[i] & 0xFFFF);
        
        /* Use register variables */
        asm volatile (
            "addl %%esi, %0\n\t"
            "subl %%edi, %1"
            : "+r" (x1), "+r" (x2)
            : "r" (reg_var1), "r" (reg_var2)
            : "cc"
        );
    }
    
    return x1 + x2 + x3;
}

/* Function C: Complex control flow with switch and computed goto */
__attribute__((noinline, noclone))
int func_complex_control(int seed, int *output) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    int a = seed * 0x12345679;
    int b = seed + 0x9ABCDEF;
    int c = seed ^ 0x55555555;
    int d = seed | 0xAAAAAAAA;
    int e = seed & 0x33333333;
    int f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    register int reg1 asm("ebp") = a;
    register int reg2 asm("esp") = b;
    
    /* Complex switch with many cases */
    switch (seed & 7) {
        case 0:
            f = a + b;
            g = c - d;
            goto *labels[seed % 5];
        case 1:
            f = a * b;
            g = c ^ d;
            break;
        case 2:
            f = a | b;
            g = c & d;
            break;
        case 3:
            f = a ^ b;
            g = c | d;
            break;
        default:
            f = a & b;
            g = c * d;
    }
    
    /* Computed goto */
    void *target = labels[(seed * 0x12345679) % 5];
    goto *target;
    
label0:
    h = f + 0x1000;
    i = g - 0x2000;
    goto merge;
    
label1:
    h = f * 3;
    i = g / 2;
    goto merge;
    
label2:
    h = f << 4;
    i = g >> 4;
    goto merge;
    
label3:
    h = f ^ 0xFFFF;
    i = g | 0xAAAA;
    goto merge;
    
label4:
    h = f & 0xCCCC;
    i = g + 0x1111;
    /* fall through */
    
merge:
    /* Nested loops with many temporaries */
    for (int x = 0; x < 16; x++) {
        j = h + x * 0x100;
        k = i - x * 0x200;
        
        for (int y = 0; y < 8; y++) {
            l = j * y;
            m = k / (y + 1);
            n = l ^ m;
            o = n << y;
            p = o >> (8 - y);
            q = p + (int)&global_array[x * 16 + y];
            r = q - large_constants[y & 3];
            s = r * reg1;
            t = s + reg2;
            
            output[x * 8 + y] = t;
        }
    }
    
    return h + i + reg1 + reg2;
}

/* Function D: Builtin usage for specific hard registers */
__attribute__((noinline, noclone))
uint64_t func_builtin_usage(int iterations) {
    uint64_t total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Use rdtsc which uses eax and edx */
        uint64_t tsc1 = __builtin_ia32_rdtsc();
        
        /* Chain of operations on the result */
        uint32_t low = tsc1 & 0xFFFFFFFF;
        uint32_t high = tsc1 >> 32;
        
        /* Many intermediate calculations */
        uint32_t a = low * 0x12345679;
        uint32_t b = high ^ 0x9ABCDEF0;
        uint32_t c = (a + b) | 0x55555555;
        uint32_t d = (a - b) & 0xAAAAAAAA;
        uint32_t e = c ^ d;
        uint32_t f = e << 3;
        uint32_t g = f >> 5;
        uint32_t h = g + (uint32_t)&global_array[i & 255];
        
        /* Use another builtin-like operation */
        uint64_t tsc2 = __builtin_ia32_rdtsc();
        uint64_t diff = tsc2 - tsc1;
        
        total += diff + h + (uint64_t)large_constants[i & 3];
    }
    
    return total;
}

/* Main function that calls all test patterns */
int main(int argc, char *argv[]) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 0x12345679;
    }
    
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    int *data = (int*)malloc(iterations * sizeof(int));
    int *output = (int*)malloc(256 * sizeof(int));
    
    /* Call all test functions to trigger different patterns */
    int result1 = func_loop_invariants(iterations, data);
    KEEP_ALIVE(result1);
    
    int result2 = func_asm_clobber(result1, iterations, data[0]);
    KEEP_ALIVE(result2);
    
    int result3 = func_complex_control(result2, output);
    KEEP_ALIVE(result3);
    
    uint64_t result4 = func_builtin_usage(iterations / 10);
    KEEP_ALIVE((int)result4);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2 + result3 + (int)result4;
    
    /* Use output to prevent optimization */
    for (int i = 0; i < 16; i++) {
        final_result += output[i * 16];
    }
    
    free(data);
    free(output);
    
    return final_result & 0xFF;
}
