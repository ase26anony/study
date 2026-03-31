/* test_early_remat.c - Trigger virtual register creation in early rematerialization */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static double global_doubles[128];
static char global_chars[512];

/* Prevent optimizations from simplifying our patterns */
#define NOINLINE __attribute__((noinline, noclone))
#define KEEP_ALIVE asm volatile("" : : "r"(result) : "memory")

/* Function A: Loop with invariants and expensive constants */
NOINLINE
int func_loop_invariants(int iterations, int* data) {
    /* Large immediate constants that need rematerialization */
    const long long BIG_CONST_1 = 0x123456789ABCDEF0LL;
    const long long BIG_CONST_2 = 0xFEDCBA9876543210LL;
    const double PI = 3.14159265358979323846;
    
    /* Many local variables with overlapping live ranges */
    int a = iterations * 2;
    int b = iterations + 0x7FFFFFFF;  /* Large immediate */
    int c = iterations - 0x80000000;  /* Another large immediate */
    long long d = BIG_CONST_1;
    long long e = BIG_CONST_2;
    double f = PI;
    int* ptr1 = global_array;
    double* ptr2 = global_doubles;
    char* ptr3 = global_chars;
    
    int result = 0;
    
    /* Complex loop with invariant values used in multiple places */
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in address calculations */
        int idx1 = (i * a) % 256;
        int idx2 = (i * b) % 128;
        int idx3 = (i * c) % 512;
        
        /* Multiple uses of invariants create register pressure */
        global_array[idx1] = *data + (int)(d >> 32);
        global_doubles[idx2] = f * (double)(e & 0xFFFFFFFF);
        global_chars[idx3] = (char)((ptr1[idx1] + ptr2[idx2]) & 0xFF);
        
        /* More computations keeping many values live */
        a = (a * 1103515245 + 12345) & 0x7FFFFFFF;
        b = (b * 1664525 + 1013904223) & 0x7FFFFFFF;
        c = (c * 214013 + 2531011) & 0x7FFFFFFF;
        
        /* Use all pointers and constants again */
        result += ptr1[idx1] + (int)ptr2[idx2] + ptr3[idx3];
        
        /* Force spill/reload by using many values in conditional */
        if ((i & 3) == 0) {
            d = BIG_CONST_1 + i;  /* Rematerialize BIG_CONST_1 */
            e = BIG_CONST_2 - i;  /* Rematerialize BIG_CONST_2 */
            f = PI * (double)i;   /* Rematerialize PI */
        }
    }
    
    /* Final use of all variables to extend live ranges */
    result += a + b + c + (int)d + (int)e + (int)f;
    return result;
}

/* Function B: Inline assembly with clobbered registers */
NOINLINE
int func_asm_clobber(int x, int y) {
    /* Register variables force hard register allocation */
    register int r1 asm("eax") = x;
    register int r2 asm("ebx") = y;
    register int r3 asm("ecx");
    register int r4 asm("edx");
    register int r5 asm("esi");
    register int r6 asm("edi");
    
    int result = 0;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "movl %[y], %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%%eax, %%ebx, 2), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "leal (%%eax, %%ebx, 4), %%edx\n\t"
        "movl %%edx, %[out3]\n\t"
        "leal (%%eax, %%ebx, 8), %%esi\n\t"
        "movl %%esi, %[out4]"
        : [out1] "=&r" (r3),   /* Early clobber output */
          [out2] "=&r" (r4),   /* Early clobber output */
          [out3] "=&r" (r5),   /* Early clobber output */
          [out4] "=&r" (r6)    /* Early clobber output */
        : [x] "r" (r1),
          [y] "r" (r2)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Use all register variables in complex expressions */
    result = r3 * 0x12345678 + r4 * 0x9ABCDEF0 + r5 * 0x55555555 + r6 * 0xAAAAAAAA;
    
    /* More inline assembly with different clobbers */
    asm volatile (
        "cpuid\n\t"
        : "=a" (r1), "=b" (r2), "=c" (r3), "=d" (r4)
        : "a" (0)
        : "memory"
    );
    
    result ^= r1 ^ r2 ^ r3 ^ r4;
    
    /* Use builtins that return in specific registers */
    uint64_t tsc = __builtin_ia32_rdtsc();
    result += (int)(tsc >> 32) + (int)(tsc & 0xFFFFFFFF);
    
    return result;
}

/* Function C: Complex control flow with switch and computed goto */
NOINLINE
int func_complex_cf(int seed, int* data) {
    /* Many local variables with overlapping scopes */
    int a = seed * 3;
    int b = seed * 5;
    int c = seed * 7;
    int d = seed * 11;
    int e = seed * 13;
    int f = seed * 17;
    int g = seed * 19;
    int h = seed * 23;
    
    /* Labels for computed goto */
    void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5, &&L6, &&L7 };
    
    int result = 0;
    int i = 0;
    
    /* Outer loop */
    while (i < 100) {
        /* Switch inside loop creates complex control flow */
        switch (i & 7) {
            case 0:
                a = b * c + 0x7FFFFFFF;
                d = e * f - 0x80000000;
                break;
            case 1:
                b = c * d + 0x12345678;
                e = f * g - 0x9ABCDEF0;
                break;
            case 2:
                c = d * e + 0x55555555;
                f = g * h - 0xAAAAAAAA;
                break;
            case 3:
                d = e * f + 0x33333333;
                g = h * a - 0xCCCCCCCC;
                break;
            case 4:
                e = f * g + 0x0F0F0F0F;
                h = a * b - 0xF0F0F0F0;
                break;
            case 5:
                f = g * h + 0x00FF00FF;
                a = b * c - 0xFF00FF00;
                break;
            case 6:
                g = h * a + 0x0000FFFF;
                b = c * d - 0xFFFF0000;
                break;
            case 7:
                h = a * b + 0xFFFFFFFF;
                c = d * e - 0x00000001;
                break;
        }
        
        /* Computed goto for unpredictable control flow */
        goto *labels[i & 7];
        
    L0:
        result += a * 0x11111111;
        i++;
        continue;
    L1:
        result += b * 0x22222222;
        i++;
        continue;
    L2:
        result += c * 0x33333333;
        i++;
        continue;
    L3:
        result += d * 0x44444444;
        i++;
        continue;
    L4:
        result += e * 0x55555555;
        i++;
        continue;
    L5:
        result += f * 0x66666666;
        i++;
        continue;
    L6:
        result += g * 0x77777777;
        i++;
        continue;
    L7:
        result += h * 0x88888888;
        i++;
        continue;
    }
    
    /* Final computation using all variables */
    result += a + b + c + d + e + f + g + h;
    return result;
}

/* Main function that calls all test patterns */
int main(int argc, char** argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 128; i++) {
        global_doubles[i] = i * 1.5;
    }
    for (int i = 0; i < 512; i++) {
        global_chars[i] = i & 0xFF;
    }
    
    int iterations = (argc > 1) ? atoi(argv[1]) : 1000;
    if (iterations < 100) iterations = 100;
    
    /* Call all test functions with arguments that create register pressure */
    int result1 = func_loop_invariants(iterations, global_array);
    int result2 = func_asm_clobber(result1, iterations);
    int result3 = func_complex_cf(result2, global_array);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2 + result3;
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(final_result) : "memory");
    
    return final_result & 0xFF;
}
