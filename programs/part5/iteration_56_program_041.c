/* test-early-remat.c */
/* Compile with: gcc -O2 -m32 -fno-optimize-sibling-calls -fdump-rtl-early-remat test-early-remat.c -o test */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static double global_doubles[128];
static char global_chars[512];

/* Large immediate values that can't be encoded in single instructions */
#define LARGE_CONST_1 0x12345678
#define LARGE_CONST_2 0x89ABCDEF
#define LARGE_CONST_3 0xFEDCBA98
#define LARGE_CONST_4 0x76543210

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int function_a(int iterations, int* data) {
    /* Many local variables with overlapping live ranges */
    register int a asm("eax") = LARGE_CONST_1;
    register int b asm("ebx") = LARGE_CONST_2;
    int c = LARGE_CONST_3;
    int d = LARGE_CONST_4;
    int e = (int)(global_array);
    int f = (int)(global_doubles);
    int g = (int)(global_chars);
    int h = 0;
    
    /* Loop with invariant address calculations */
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in multiple places with different operations */
        int idx1 = (i + a) % 256;      /* Uses invariant 'a' */
        int idx2 = (i * b) % 128;      /* Uses invariant 'b' */
        int idx3 = (i ^ c) % 512;      /* Uses invariant 'c' */
        
        /* Complex address calculations using invariants */
        data[i] = global_array[idx1] 
                 + (int)global_doubles[idx2 % 128]
                 + global_chars[idx3];
        
        /* More uses of invariants in different expressions */
        h += (a * b) - (c ^ d) + (e & f) | (g << 2);
        
        /* Force register pressure by using all temporaries */
        int t1 = a + b;
        int t2 = c - d;
        int t3 = e ^ f;
        int t4 = g & h;
        int t5 = t1 * t2;
        int t6 = t3 / (t4 ? t4 : 1);
        int t7 = t5 ^ t6;
        int t8 = t7 << (i & 7);
        
        data[i] ^= t8;
    }
    
    return h;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int function_b(int x, int y) {
    int result1, result2, result3;
    
    /* Multiple output inline assembly forcing hard register references */
    asm volatile (
        "movl %4, %%eax\n\t"
        "movl %5, %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "leal (%%eax, %%ebx, 4), %%ecx\n\t"
        "movl %%ecx, %1\n\t"
        "xorl %%edx, %%edx\n\t"
        "divl %%ebx\n\t"
        "movl %%edx, %2\n\t"
        : "=&r" (result1), "=&r" (result2), "=&r" (result3)
        : "0" (0), "r" (x), "r" (y)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    /* Use register variables with specific constraints */
    register int r1 asm("esi") = result1;
    register int r2 asm("edi") = result2;
    register int r3 asm("ebp") = result3;
    
    /* Complex sequence using these register variables */
    for (int i = 0; i < 100; i++) {
        r1 = (r1 * LARGE_CONST_1) ^ r2;
        r2 = (r2 + LARGE_CONST_2) | r3;
        r3 = (r3 - LARGE_CONST_3) & r1;
        
        /* Inline asm that clobbers many registers */
        asm volatile (
            "pushl %%eax\n\t"
            "pushl %%ebx\n\t"
            "pushl %%ecx\n\t"
            "pushl %%edx\n\t"
            "movl %0, %%eax\n\t"
            "movl %1, %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %0\n\t"
            "popl %%edx\n\t"
            "popl %%ecx\n\t"
            "popl %%ebx\n\t"
            "popl %%eax\n\t"
            : "+r" (r1)
            : "r" (r2)
            : "memory", "cc"
        );
    }
    
    return r1 + r2 + r3;
}

/* Function C: Complex control flow with switch and computed goto */
__attribute__((noinline, noclone))
int function_c(int mode, int count) {
    /* Many scalar temporaries with overlapping lives */
    int a = LARGE_CONST_1;
    int b = LARGE_CONST_2;
    int c = LARGE_CONST_3;
    int d = LARGE_CONST_4;
    int e = 0, f = 0, g = 0, h = 0;
    int i = 0, j = 0, k = 0, l = 0;
    int m = 0, n = 0, o = 0, p = 0;
    
    /* Labels for computed goto */
    void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    /* Nested loops with switch inside */
    for (int outer = 0; outer < count; outer++) {
        for (int inner = 0; inner < 10; inner++) {
            /* Switch statement with multiple cases using all temporaries */
            switch ((outer + inner) % 5) {
                case 0:
                    a = b + c;
                    e = f * g;
                    i = j ^ k;
                    m = n | o;
                    break;
                case 1:
                    b = c - d;
                    f = g / (h ? h : 1);
                    j = k << 2;
                    n = o >> 1;
                    break;
                case 2:
                    c = d ^ a;
                    g = h & e;
                    k = l | i;
                    o = p ^ m;
                    break;
                case 3:
                    d = a * b;
                    h = e + f;
                    l = i - j;
                    p = m * n;
                    break;
                case 4:
                    /* Computed goto to create complex control flow */
                    goto *labels[inner % 5];
                    label0:
                        a += global_array[outer % 256];
                        continue;
                    label1:
                        b += (int)global_doubles[inner % 128];
                        continue;
                    label2:
                        c += global_chars[(outer + inner) % 512];
                        continue;
                    label3:
                        d ^= LARGE_CONST_1;
                        continue;
                    label4:
                        e |= LARGE_CONST_2;
                        continue;
            }
            
            /* More operations to increase register pressure */
            int t1 = a + b + c + d;
            int t2 = e * f * g * h;
            int t3 = i ^ j ^ k ^ l;
            int t4 = m & n & o & p;
            
            /* Use all temporaries in complex expression */
            global_array[outer % 256] = t1 + t2 - t3 * t4;
        }
    }
    
    return a + b + c + d + e + f + g + h;
}

/* Function D: Using builtins that return in specific registers */
#ifdef __i386__
__attribute__((noinline, noclone))
uint64_t function_d(int iterations) {
    uint64_t total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* rdtsc returns in edx:eax - creates hard register references */
        uint64_t tsc = __builtin_ia32_rdtsc();
        
        /* Use the result in subsequent expressions */
        uint32_t low = (uint32_t)tsc;
        uint32_t high = (uint32_t)(tsc >> 32);
        
        /* Chain of operations using the hard register results */
        low = (low * LARGE_CONST_1) ^ high;
        high = (high + LARGE_CONST_2) | low;
        
        /* More operations to create register pressure */
        for (int j = 0; j < 10; j++) {
            low = (low << j) ^ global_array[j];
            high = (high >> j) + global_chars[j];
            
            /* Use inline asm to force specific register usage */
            asm volatile (
                "addl %%eax, %%ebx\n\t"
                "adcl %%edx, %%ecx\n\t"
                : "+b" (low), "+c" (high)
                : "a" (j), "d" (global_array[j % 256])
                : "cc"
            );
        }
        
        total += ((uint64_t)high << 32) | low;
    }
    
    return total;
}
#endif

/* Main function that calls all test functions */
int main(int argc, char** argv) {
    int iterations = argc > 1 ? atoi(argv[1]) : 100;
    
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
    
    /* Call all test functions to trigger early rematerialization */
    int* data = malloc(iterations * sizeof(int));
    
    int result_a = function_a(iterations, data);
    int result_b = function_b(iterations, iterations * 2);
    int result_c = function_c(iterations % 10, iterations);
    
    #ifdef __i386__
    uint64_t result_d = function_d(iterations / 10);
    int final_result = result_a + result_b + result_c + (int)result_d;
    #else
    int final_result = result_a + result_b + result_c;
    #endif
    
    /* Use results to prevent optimization */
    for (int i = 0; i < iterations && i < 10; i++) {
        final_result ^= data[i];
    }
    
    free(data);
    
    return final_result & 0xFF;
}
