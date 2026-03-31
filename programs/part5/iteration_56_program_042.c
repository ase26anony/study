/* test_early_remat.c - Program to trigger virtual register creation in early rematerialization */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[1024];
static double global_doubles[256];
static char global_chars[4096];

/* Large immediate values that are expensive to materialize */
#define LARGE_IMMEDIATE_1 0x12345678
#define LARGE_IMMEDIATE_2 0x89ABCDEF
#define LARGE_IMMEDIATE_3 0xFEDCBA98
#define LARGE_IMMEDIATE_4 0x76543210

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int function_a(int iterations, int* data) {
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = LARGE_IMMEDIATE_1;
    register int r1 asm("ebx") = LARGE_IMMEDIATE_2;
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
    int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0, o = 0, p = 0;
    int q = 0, r = 0, s = 0, t = 0, u = 0, v = 0, w = 0, x = 0;
    
    /* Loop invariants - these should be rematerialized */
    int* invariant_ptr = global_array;
    const int invariant_limit = iterations * 2;
    const long invariant_addr = (long)&global_chars[2048];
    
    /* Complex loop with many overlapping live ranges */
    for (int idx = 0; idx < iterations; idx++) {
        /* Use invariants in multiple places */
        a = invariant_ptr[idx % 1024] + invariant_limit;
        b = data[idx] * (int)(invariant_addr >> 16);
        
        /* Chain of arithmetic operations creating register pressure */
        c = a + b + r0;
        d = c * r1 - LARGE_IMMEDIATE_3;
        e = d >> 4;
        f = e ^ LARGE_IMMEDIATE_4;
        g = f & 0xFFFF;
        h = g | (invariant_limit & 0xFF);
        
        /* More operations to extend live ranges */
        i = h * a;
        j = i + b;
        k = j - c;
        l = k ^ d;
        m = l | e;
        n = m & f;
        o = n + g;
        p = o - h;
        
        /* Use all variables to keep them live */
        q += i + j + k + l;
        r += m + n + o + p;
        s += a * b * c;
        t += d * e * f;
        u += g * h * i;
        v += j * k * l;
        w += m * n * o;
        x += p * q * r;
        
        /* Conditional to create control flow complexity */
        if (idx % 3 == 0) {
            s = t + u;
            v = w + x;
        } else if (idx % 3 == 1) {
            t = u + v;
            w = x + q;
        } else {
            u = v + w;
            x = q + r;
        }
    }
    
    /* Combine all results */
    return a + b + c + d + e + f + g + h + i + j + k + l + m + 
           n + o + p + q + r + s + t + u + v + w + x;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int function_b(int x, int y) {
    int result1, result2, result3, result4;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %2, %%eax\n\t"
        "movl %3, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %1\n\t"
        "movl $0x%c4, %%ecx\n\t"
        "subl %%ecx, %%eax\n\t"
        "movl %%eax, %%edx"
        : "=&r" (result1), "=&r" (result2), "=&a" (result3), "=&b" (result4)
        : "r" (x), "r" (y), "i" (LARGE_IMMEDIATE_1)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "cc", "memory"
    );
    
    /* Use register variables with specific constraints */
    register int r10 asm("esi") = result1;
    register int r11 asm("edi") = result2;
    
    /* Complex expression chain using hard register results */
    int a = r10 * LARGE_IMMEDIATE_2;
    int b = r11 / 256;
    int c = a ^ b;
    int d = c << 3;
    int e = d | 0xFF;
    int f = e & r10;
    int g = f + r11;
    int h = g - a;
    int i = h * b;
    int j = i ^ c;
    int k = j | d;
    int l = k & e;
    int m = l + f;
    int n = m - g;
    int o = n * h;
    int p = o ^ i;
    int q = p | j;
    int r = q & k;
    int s = r + l;
    int t = s - m;
    
    /* Another inline asm to create more register pressure */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "leal (%%eax, %%ebx, 4), %%ecx\n\t"
        "movl %%ecx, %0"
        : "=r" (result3)
        : "r" (t), "r" (o)
        : "eax", "ebx", "ecx"
    );
    
    return result1 + result2 + result3 + a + b + c + d + e + f + g + 
           h + i + j + k + l + m + n + o + p + q + r + s + t;
}

/* Function C: Complex control flow with switch and computed goto */
__attribute__((noinline, noclone))
int function_c(int mode, int iterations) {
    /* Many temporary variables with overlapping lives */
    int t1 = LARGE_IMMEDIATE_1, t2 = LARGE_IMMEDIATE_2;
    int t3 = LARGE_IMMEDIATE_3, t4 = LARGE_IMMEDIATE_4;
    int t5 = 0, t6 = 0, t7 = 0, t8 = 0, t9 = 0, t10 = 0;
    int t11 = 0, t12 = 0, t13 = 0, t14 = 0, t15 = 0, t16 = 0;
    
    /* Labels for computed goto */
    void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    /* Nested loops with switch inside */
    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < 5; j++) {
            /* Switch creates complex control flow */
            switch ((i + j) % 5) {
                case 0:
                    t5 = t1 + t2;
                    t6 = t3 * t4;
                    t7 = (t5 << 3) | (t6 >> 5);
                    t8 = t7 ^ (int)((long)&global_array[512]);
                    break;
                case 1:
                    t9 = t2 - t3;
                    t10 = t4 / 2;
                    t11 = t9 & t10;
                    t12 = t11 | (int)((long)&global_doubles[128]);
                    break;
                case 2:
                    t13 = t1 * t3;
                    t14 = t2 + t4;
                    t15 = t13 ^ t14;
                    t16 = t15 & 0xFFFFFFFF;
                    break;
                case 3:
                    /* Use computed goto */
                    goto *labels[j];
                case 4:
                    t5 = t6 + t7;
                    t8 = t9 - t10;
                    t11 = t12 * t13;
                    t14 = t15 ^ t16;
                    break;
            }
            
            /* Keep all temporaries live across iterations */
            t1 += t5;
            t2 += t6;
            t3 += t7;
            t4 += t8;
            t5 += t9;
            t6 += t10;
            t7 += t11;
            t8 += t12;
            t9 += t13;
            t10 += t14;
            t11 += t15;
            t12 += t16;
        }
        
        label0:
            t13 = t1 * 2;
            continue;
        label1:
            t14 = t2 / 2;
            continue;
        label2:
            t15 = t3 ^ t4;
            continue;
        label3:
            t16 = t5 | t6;
            continue;
        label4:
            t1 = t7 & t8;
            continue;
    }
    
    /* Use builtins that use specific hard registers (x86 specific) */
    unsigned long long tsc1 = __builtin_ia32_rdtsc();
    unsigned long long tsc2 = __builtin_ia32_rdtsc();
    
    /* Use the TSC results in expressions */
    int ts1 = (int)(tsc1 & 0xFFFFFFFF);
    int ts2 = (int)(tsc2 >> 32);
    
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + 
           t11 + t12 + t13 + t14 + t15 + t16 + ts1 + ts2;
}

/* Function D: Mixed patterns for maximum coverage */
__attribute__((noinline, noclone))
int function_d(int* ptr, int size) {
    /* Register variables with specific constraints */
    register int reg_a asm("eax");
    register int reg_b asm("ebx");
    register int reg_c asm("ecx");
    register int reg_d asm("edx");
    
    reg_a = LARGE_IMMEDIATE_1;
    reg_b = LARGE_IMMEDIATE_2;
    reg_c = size;
    reg_d = (int)((long)ptr >> 2);
    
    int sum = 0;
    int prod = 1;
    
    /* Loop with address calculations using invariants */
    for (int i = 0; i < size; i++) {
        /* Complex address calculation that might need rematerialization */
        int* addr1 = &global_array[(reg_a + i) % 1024];
        int* addr2 = &global_array[(reg_b + i * 2) % 1024];
        char* addr3 = &global_chars[(reg_c * i) % 4096];
        
        /* Operations using the addresses */
        int val1 = *addr1 + reg_d;
        int val2 = *addr2 - reg_d;
        char val3 = *addr3;
        
        /* Chain of dependent operations */
        int tmp1 = val1 * val2;
        int tmp2 = tmp1 + (int)val3;
        int tmp3 = tmp2 ^ reg_a;
        int tmp4 = tmp3 | reg_b;
        int tmp5 = tmp4 & reg_c;
        int tmp6 = tmp5 - reg_d;
        
        /* Use inline asm to force specific register usage */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0"
            : "=r" (tmp6)
            : "r" (tmp6), "r" (i)
            : "eax"
        );
        
        sum += tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6;
        prod *= (tmp1 & 0xFF) + 1;
        
        /* Conditional that creates divergent control flow */
        if (sum % 7 == 0) {
            reg_a = (reg_a << 1) | 1;
        } else if (sum % 7 == 1) {
            reg_b = (reg_b >> 1) ^ reg_a;
        } else if (sum % 7 == 2) {
            reg_c = reg_c + reg_b;
        } else {
            reg_d = reg_d - reg_c;
        }
    }
    
    return sum + prod + reg_a + reg_b + reg_c + reg_d;
}

/* Main function that calls all test functions */
int main(int argc, char** argv) {
    /* Initialize global data */
    for (int i = 0; i < 1024; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 256; i++) {
        global_doubles[i] = i * 1.5;
    }
    for (int i = 0; i < 4096; i++) {
        global_chars[i] = (char)(i % 256);
    }
    
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    int result = 0;
    
    /* Call all test functions with different patterns */
    result += function_a(iterations, global_array);
    result += function_b(iterations, iterations * 2);
    result += function_c(iterations % 5, iterations);
    result += function_d(global_array, iterations % 1024);
    
    /* Call functions multiple times to increase optimization opportunities */
    for (int i = 0; i < 3; i++) {
        result += function_a(iterations / 2, &global_array[256]);
        result += function_b(i, iterations - i);
        result += function_c(i, iterations / 3);
        result += function_d(&global_array[i * 128], 64);
    }
    
    return result & 0x7FFFFFFF; /* Ensure positive result */
}
