/* test_early_remat.c - Program to trigger virtual register creation in GCC's early rematerialization pass */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static double global_doubles[128];
static char global_chars[512];

/* Large immediate values that are expensive to materialize */
#define LARGE_IMMEDIATE_1 0xDEADBEEF
#define LARGE_IMMEDIATE_2 0xCAFEBABE
#define LARGE_IMMEDIATE_3 0x12345678
#define LARGE_IMMEDIATE_4 0x9ABCDEF0

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int* data) {
    /* Many local variables with overlapping live ranges */
    int a = LARGE_IMMEDIATE_1;
    int b = LARGE_IMMEDIATE_2;
    int c = LARGE_IMMEDIATE_3;
    int d = LARGE_IMMEDIATE_4;
    int e = (int)(global_array);
    int f = (int)(global_doubles);
    int g = (int)(global_chars);
    int h = iterations * 2;
    
    /* Loop with invariant values used in multiple places */
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in address calculations */
        int idx1 = (i + a) % 256;
        int idx2 = (i * b) % 128;
        int idx3 = (i ^ c) % 512;
        
        /* Multiple uses of invariants create register pressure */
        sum += global_array[idx1] * (a & 0xFF);
        sum += (int)global_doubles[idx2] * (b >> 16);
        sum += global_chars[idx3] * (c & 0xFF);
        
        /* More operations to extend live ranges */
        int temp1 = d + e;
        int temp2 = f - g;
        int temp3 = h * i;
        int temp4 = temp1 ^ temp2;
        int temp5 = temp3 | temp4;
        
        sum += temp5;
        
        /* Conditional to create control flow complexity */
        if (i % 3 == 0) {
            sum += a * b;
        } else if (i % 3 == 1) {
            sum += c * d;
        } else {
            sum += e * f;
        }
    }
    
    /* Force all values to be used at the end */
    return sum + a + b + c + d + e + f + g + h;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int x, int y) {
    /* Register variables to force specific allocation */
    register int r1 asm("eax") = x;
    register int r2 asm("ebx") = y;
    register int r3 asm("ecx") = LARGE_IMMEDIATE_1;
    register int r4 asm("edx") = LARGE_IMMEDIATE_2;
    
    int result1, result2, result3, result4;
    
    /* Multi-output inline assembly with many clobbered registers */
    asm volatile (
        "movl %[r1], %%eax\n\t"
        "movl %[r2], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %[r3], %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        "xorl %%ebx, %%eax\n\t"
        "movl %%eax, %[out3]\n\t"
        "leal (%%eax,%%ebx,4), %%ecx\n\t"
        "movl %%ecx, %[out4]"
        : [out1] "=&r" (result1),
          [out2] "=&r" (result2),
          [out3] "=&r" (result3),
          [out4] "=&r" (result4)
        : [r1] "r" (r1),
          [r2] "r" (r2),
          [r3] "r" (r3),
          [r4] "r" (r4)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory", "cc"
    );
    
    /* Use results in complex expressions */
    int complex1 = result1 * result2 + result3;
    int complex2 = result4 ^ result1 | result2;
    int complex3 = (complex1 << 3) + (complex2 >> 2);
    
    /* Chain of operations keeping many values live */
    for (int i = 0; i < 10; i++) {
        complex1 = complex1 * 7 + complex2;
        complex2 = complex2 * 13 + complex3;
        complex3 = complex3 * 31 + complex1;
        
        /* Use register variables in the loop */
        asm volatile (
            "addl %%eax, %0\n\t"
            "subl %%ebx, %1"
            : "+r" (complex1), "+r" (complex2)
            : "a" (r1), "b" (r2)
            : "cc"
        );
    }
    
    return complex1 + complex2 + complex3 + result1 + result2 + result3 + result4;
}

/* Function C: Complex control flow with switch statements */
__attribute__((noinline, noclone))
int func_complex_control(int start, int count) {
    /* Many temporaries with overlapping lives */
    int t1 = start;
    int t2 = LARGE_IMMEDIATE_1;
    int t3 = LARGE_IMMEDIATE_2;
    int t4 = (int)global_array;
    int t5 = (int)global_doubles;
    int t6 = count * 3;
    int t7 = t1 ^ t2;
    int t8 = t3 & t4;
    int t9 = t5 | t6;
    int t10 = t7 + t8;
    
    int total = 0;
    
    /* Nested loops with switch inside */
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 5; j++) {
            /* Switch creates complex control flow */
            switch ((i + j) % 7) {
                case 0:
                    total += t1 * t2;
                    t1 = t1 + 1;
                    break;
                case 1:
                    total += t3 * t4;
                    t2 = t2 - 1;
                    break;
                case 2:
                    total += t5 * t6;
                    t3 = t3 ^ t4;
                    break;
                case 3:
                    total += t7 * t8;
                    t4 = t4 | t5;
                    break;
                case 4:
                    total += t9 * t10;
                    t5 = t5 & t6;
                    break;
                case 5:
                    total += (t1 + t2) * (t3 + t4);
                    t6 = t6 << 1;
                    break;
                case 6:
                    total += (t5 + t6) * (t7 + t8);
                    t7 = t7 >> 1;
                    break;
            }
            
            /* More operations to keep values live across the switch */
            t8 = t8 * 3 + t9;
            t9 = t9 * 5 + t10;
            t10 = t10 * 7 + t1;
        }
        
        /* Conditional with computed goto (labels as values) */
        static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
        
        if (i % 4 == 0) {
            goto *labels[0];
        } else if (i % 4 == 1) {
            goto *labels[1];
        } else if (i % 4 == 2) {
            goto *labels[2];
        } else {
            goto *labels[3];
        }
        
    label0:
        t1 = t1 + t2;
        continue;
    label1:
        t2 = t2 + t3;
        continue;
    label2:
        t3 = t3 + t4;
        continue;
    label3:
        t4 = t4 + t5;
        continue;
    }
    
    /* Use all temporaries at the end */
    return total + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
}

/* Function D: Using builtins for specific register references */
#ifdef __i386__
__attribute__((noinline, noclone))
uint64_t func_builtin_registers(void) {
    /* Use rdtsc which returns in edx:eax */
    uint64_t tsc1 = __builtin_ia32_rdtsc();
    
    /* Create dependency chain on the result */
    uint32_t low1 = (uint32_t)tsc1;
    uint32_t high1 = (uint32_t)(tsc1 >> 32);
    
    /* Many operations to create register pressure */
    uint32_t a = low1 * 0x12345678;
    uint32_t b = high1 * 0x9ABCDEF0;
    uint32_t c = a ^ b;
    uint32_t d = (a + b) * c;
    uint32_t e = (b - a) | d;
    uint32_t f = c * d * e;
    
    /* Another rdtsc */
    uint64_t tsc2 = __builtin_ia32_rdtsc();
    uint32_t low2 = (uint32_t)tsc2;
    uint32_t high2 = (uint32_t)(tsc2 >> 32);
    
    /* Mix with previous values */
    uint32_t g = low2 * a + high2 * b;
    uint32_t h = (low2 ^ high2) * c;
    uint32_t i = d * e * f;
    uint32_t j = g | h | i;
    
    /* Complex expression using all values */
    uint64_t result = ((uint64_t)a << 32) | b;
    result += ((uint64_t)c << 32) | d;
    result += ((uint64_t)e << 32) | f;
    result += ((uint64_t)g << 32) | h;
    result += ((uint64_t)i << 32) | j;
    result += tsc1 + tsc2;
    
    return result;
}
#endif

/* Main function that calls all test functions */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 128; i++) {
        global_doubles[i] = i * 1.5;
    }
    for (int i = 0; i < 512; i++) {
        global_chars[i] = i % 256;
    }
    
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    /* Call all test functions to trigger different patterns */
    int result1 = func_loop_invariants(iterations, global_array);
    int result2 = func_asm_clobber(result1, iterations);
    int result3 = func_complex_control(result2 % 50, iterations % 20 + 5);
    
    uint64_t result4 = 0;
#ifdef __i386__
    result4 = func_builtin_registers();
#endif
    
    /* Combine results to prevent optimization */
    int final_result = result1 + result2 + result3 + (int)result4 + (int)(result4 >> 32);
    
    return final_result % 256; /* Return small value for exit code */
}
