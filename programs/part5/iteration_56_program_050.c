/* test_early_remat.c - Target specific patterns for early rematerialization */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static double global_doubles[128];
static volatile int global_volatile = 12345;

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int function_a(int iterations, int* data) {
    /* Large immediate constants that need rematerialization */
    const long long expensive_const1 = 0x7FFFFFFFFFFFFFFFLL;
    const long long expensive_const2 = 0x123456789ABCDEF0LL;
    const double expensive_float = 3.14159265358979323846;
    
    /* Invariant pointers used in loop */
    int* invariant_ptr1 = &global_array[0];
    int* invariant_ptr2 = &global_array[128];
    double* invariant_ptr3 = &global_doubles[0];
    
    int sum = 0;
    long long accumulator = 0;
    
    /* Complex loop with many overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in multiple calculations */
        int val1 = invariant_ptr1[i & 255];
        int val2 = invariant_ptr2[(i + 64) & 255];
        double val3 = invariant_ptr3[i & 127];
        
        /* Use expensive constants in non-adjacent operations */
        if (i & 1) {
            accumulator += expensive_const1 - val1;
        } else {
            accumulator += expensive_const2 + val2;
        }
        
        /* More operations creating register pressure */
        sum += (int)(val3 * expensive_float);
        sum += data[i] * (i & 31);
        
        /* Nested conditionals extend live ranges */
        if (i % 3 == 0) {
            sum += (int)(expensive_const1 >> 32);
        } else if (i % 3 == 1) {
            sum += (int)(expensive_const2 & 0xFFFFFFFF);
        }
        
        /* Address calculations using invariants */
        int* temp_ptr = invariant_ptr1 + (i & 15);
        sum += *temp_ptr;
    }
    
    return sum + (int)(accumulator >> 32);
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int function_b(int x, int y) {
    /* Register variables forcing specific allocation */
    register int r1 asm("eax") = x;
    register int r2 asm("ebx") = y;
    register int r3 asm("ecx");
    register int r4 asm("edx");
    
    int result1, result2, result3;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %[r1], %%eax\n\t"
        "movl %[r2], %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%%eax, %%ebx, 4), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "xorl %%edx, %%edx\n\t"
        "divl %%ebx\n\t"
        "movl %%edx, %[out3]\n\t"
        : [out1] "=r" (result1), [out2] "=r" (result2), [out3] "=r" (result3)
        : [r1] "r" (r1), [r2] "r" (r2)
        : "eax", "ebx", "ecx", "edx", "cc", "memory"
    );
    
    /* Use results in complex expressions */
    int a = result1 * 0x12345678;
    int b = result2 + 0x9ABCDEF0;
    int c = result3 | 0x55555555;
    
    /* More inline assembly with different clobbers */
    asm volatile (
        "cpuid\n\t"
        : "=a" (a), "=b" (b), "=c" (c), "=d" (r4)
        : "a" (0), "c" (0)
        : "memory"
    );
    
    return a + b + c + r4;
}

/* Function C: Complex control flow with switch and computed goto */
__attribute__((noinline, noclone))
int function_c(int mode, int count) {
    /* Many local variables with overlapping lifetimes */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    /* Labels for computed goto */
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    int result = 0;
    
    /* Nested loops with switch inside */
    for (int iter = 0; iter < count; iter++) {
        switch (mode) {
            case 0:
                a = b * c + d;
                b = e - f * g;
                c = h / i + j;
                d = k | l & m;
                e = n ^ o | p;
                break;
            case 1:
                f = a + b + c + d;
                g = e * f - g;
                h = i / j * k;
                i = l & m | n;
                j = o ^ p & a;
                break;
            case 2:
                k = a * b - c;
                l = d + e * f;
                m = g / h + i;
                n = j | k & l;
                o = m ^ n | p;
                break;
            default:
                p = a + b - c + d - e + f - g + h;
                break;
        }
        
        /* Computed goto creating complex control flow */
        if (iter % 4 == 0) {
            goto *labels[iter & 3];
        }
        
        /* Continue with more operations */
        result += a + b + c + d + e + f + g + h;
        result += i + j + k + l + m + n + o + p;
        
        /* Backward jump label */
        label0:
        a += global_volatile;
        continue;
        
        label1:
        b -= global_volatile;
        continue;
        
        label2:
        c *= global_volatile;
        continue;
        
        label3:
        d /= (global_volatile | 1);
        continue;
    }
    
    /* Final complex expression with all variables */
    return result + a - b + c - d + e - f + g - h +
           i - j + k - l + m - n + o - p;
}

/* Function D: Mixed patterns for maximum pressure */
__attribute__((noinline, noclone))
int function_d(int seed) {
    /* Use builtins that return in specific registers */
    unsigned long long tsc1 = __builtin_ia32_rdtsc();
    unsigned int eax1 = tsc1 & 0xFFFFFFFF;
    unsigned int edx1 = tsc1 >> 32;
    
    /* Chain of operations on hard register results */
    int x = eax1 * 0x12345678 + edx1;
    int y = (eax1 ^ edx1) | 0x55555555;
    
    /* More builtins */
    unsigned int cpuid_eax, cpuid_ebx, cpuid_ecx, cpuid_edx;
    __cpuid(0, cpuid_eax, cpuid_ebx, cpuid_ecx, cpuid_edx);
    
    /* Complex expression mixing all results */
    int result = (x * cpuid_eax) + (y * cpuid_ebx) -
                 (eax1 * cpuid_ecx) + (edx1 * cpuid_edx);
    
    /* Loop with many temporaries */
    for (int i = 0; i < 100; i++) {
        int t1 = result + i;
        int t2 = t1 * cpuid_eax;
        int t3 = t2 - cpuid_ebx;
        int t4 = t3 | cpuid_ecx;
        int t5 = t4 ^ cpuid_edx;
        int t6 = t5 + eax1;
        int t7 = t6 - edx1;
        int t8 = t7 * x;
        int t9 = t8 / (y | 1);
        
        result = t9 + (i & 255);
    }
    
    return result;
}

/* Main function to drive all patterns */
int main(int argc, char** argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    for (int i = 0; i < 128; i++) {
        global_doubles[i] = i * 1.5;
    }
    
    /* Local array for function_a */
    int local_data[100];
    for (int i = 0; i < 100; i++) {
        local_data[i] = i * 2;
    }
    
    /* Call all test functions with appropriate arguments */
    int result = 0;
    
    /* Large iteration count for register pressure */
    result += function_a(1000, local_data);
    
    /* Use large immediate arguments */
    result += function_b(0x7FFFFFFF, 0x12345678);
    
    /* Complex control flow with many iterations */
    result += function_c(argc > 1 ? atoi(argv[1]) : 2, 500);
    
    /* Mixed patterns */
    result += function_d(result);
    
    /* Ensure result is used */
    return result & 0xFF;
}
