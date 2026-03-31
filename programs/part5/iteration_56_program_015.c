/* test_early_remat.c - Trigger virtual register creation in early rematerialization */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static double global_doubles[128];
static char global_chars[512];

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int* data) {
    /* Use many local variables with overlapping lifetimes */
    register int r0 asm("eax") = 0x12345678;  /* Non-encodable immediate */
    register int r1 asm("ebx") = 0x89ABCDEF;
    register int r2 asm("ecx") = iterations;
    int sum = 0;
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Loop with invariant address calculations */
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in multiple places */
        t1 = r0 + i;           /* Use expensive constant */
        t2 = r1 - i;           /* Use another expensive constant */
        t3 = global_array[i & 255] * t1;  /* Use global address */
        t4 = data[i] * t2;     /* Use parameter pointer */
        t5 = t1 * t3;
        t6 = t2 * t4;
        t7 = t5 + 0x7FFFFFFF;  /* Large immediate */
        t8 = t6 - 0x80000000;  /* Another large immediate */
        t9 = t7 ^ t8;
        t10 = t9 & 0xFFFFFFFF;
        
        /* More computations to extend live ranges */
        t11 = t10 * r0;
        t12 = t11 / (r1 + 1);
        t13 = t12 << 3;
        t14 = t13 >> 2;
        t15 = t14 | 0x55555555;
        t16 = t15 & 0xAAAAAAAA;
        t17 = t16 + global_array[(i + 1) & 255];
        t18 = t17 - global_array[(i + 2) & 255];
        t19 = t18 * 0x1234567;  /* Expensive constant */
        t20 = t19 % 0x89ABCDE;  /* Another expensive constant */
        
        sum += t20;
        
        /* Force register pressure with overlapping computations */
        r0 = (r0 << 1) | (r0 >> 31);  /* Rotate */
        r1 = (r1 >> 1) ^ (r1 << 31);  /* Another rotate */
    }
    
    /* Use all temporaries one more time to extend live ranges */
    return sum + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10
           + t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int a, int b) {
    int result1, result2, result3;
    register int reg_var1 asm("esi") = a;
    register int reg_var2 asm("edi") = b;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %2, %%eax\n\t"
        "movl %3, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %1\n\t"
        : "=&r" (result1), "=&r" (result2), "=&a" (result3)
        : "r" (reg_var1), "r" (reg_var2)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Use results in complex expressions */
    int x1 = result1 * 0x12345678;
    int x2 = result2 / 0x89ABCDEF;
    int x3 = result3 + 0x55555555;
    int x4 = x1 ^ x2;
    int x5 = x3 & x4;
    int x6 = x5 | 0xAAAAAAAA;
    int x7 = x6 << 3;
    int x8 = x7 >> 2;
    int x9 = x8 * global_array[0];
    int x10 = x9 / (global_array[1] + 1);
    
    /* Another asm with different clobbers */
    asm volatile (
        "rdtsc\n\t"  /* Uses eax and edx */
        : "=a" (x1), "=d" (x2)
        :
        : "ecx"  /* Clobber ecx too */
    );
    
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
}

/* Function C: Complex control flow with switch */
__attribute__((noinline, noclone))
int func_complex_control(int seed, int* data) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    int val = seed;
    int a = 0x11111111, b = 0x22222222, c = 0x33333333;
    int d = 0x44444444, e = 0x55555555, f = 0x66666666;
    int g = 0x77777777, h = 0x88888888, i = 0x99999999;
    int j = 0xAAAAAAAA, k = 0xBBBBBBBB, l = 0xCCCCCCCC;
    
    /* Nested loops with switch inside */
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = 0; inner < 5; inner++) {
            /* Use computed goto for unpredictable control flow */
            goto *labels[(val + outer + inner) % 5];
            
            label0:
                a = b + c;
                d = e * f;
                val += global_array[outer] * 0x1234567;
                break;
            label1:
                g = h - i;
                j = k / (l + 1);
                val += global_doubles[inner] * 0x89ABCDE;
                break;
            label2:
                a = d ^ g;
                b = e | j;
                val += global_chars[outer * inner] * 0x55555555;
                break;
            label3:
                c = f & k;
                h = a << 3;
                val += data[inner] * 0xAAAAAAAA;
                break;
            label4:
                i = b >> 2;
                l = c * 7;
                val += (outer * inner) * 0xCCCCCCCC;
                break;
        }
        
        /* Use all variables to keep them live */
        int temp = a + b + c + d + e + f + g + h + i + j + k + l;
        val = (val * 1103515245 + 12345) ^ temp;
    }
    
    return val;
}

/* Function D: Mixed hard and virtual register usage */
__attribute__((noinline, noclone))
int func_mixed_registers(double* dbl_data, int int_data) {
    /* Use builtins that return in specific registers */
    uint64_t tsc1 = __builtin_ia32_rdtsc();  /* Returns in eax:edx */
    
    /* Force these into computations */
    uint32_t tsc_lo = tsc1 & 0xFFFFFFFF;
    uint32_t tsc_hi = tsc1 >> 32;
    
    /* Many overlapping computations */
    double d1 = dbl_data[0] * 3.14159265358979323846;
    double d2 = dbl_data[1] * 2.71828182845904523536;
    double d3 = d1 + d2;
    double d4 = d1 - d2;
    double d5 = d3 * d4;
    double d6 = d5 / (d3 + 1.0);
    
    int i1 = tsc_lo * int_data;
    int i2 = tsc_hi / (int_data + 1);
    int i3 = i1 ^ i2;
    int i4 = i3 & 0x7FFFFFFF;
    int i5 = i4 | 0x80000000;
    
    /* Use inline asm with specific register constraints */
    register double d7 asm("xmm0") = d6;
    register int i6 asm("eax") = i5;
    
    asm volatile (
        "cvtsd2si %1, %0\n\t"
        : "=r" (i6)
        : "x" (d7)
    );
    
    return i6 + tsc_lo + tsc_hi;
}

/* Main function that calls all test functions */
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
    
    int local_data[100];
    for (int i = 0; i < 100; i++) {
        local_data[i] = i * 7;
    }
    
    double dbl_data[10];
    for (int i = 0; i < 10; i++) {
        dbl_data[i] = i * 2.5;
    }
    
    /* Call all test functions with arguments that create register pressure */
    int result = 0;
    
    /* Use large immediate values as arguments */
    result += func_loop_invariants(100, local_data);
    result += func_asm_clobber(0x12345678, 0x9ABCDEF0);
    result += func_complex_control(0x55555555, local_data);
    result += func_mixed_registers(dbl_data, 0x77777777);
    
    /* Add more calls with different arguments */
    result += func_loop_invariants(50, &local_data[50]);
    result += func_asm_clobber(0x11111111, 0x22222222);
    
    return result & 0xFF;  /* Return non-zero result */
}
