/* test_early_remat.c - Target specific coverage for early-remat.cc lines 930-937 */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static double global_doubles[128];
static volatile int volatile_counter;

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int* data) {
    /* Large immediate constants that need rematerialization */
    const long long BIG_CONST_1 = 0x7FFFFFFFFFFFFFFFLL;
    const long long BIG_CONST_2 = 0x123456789ABCDEF0LL;
    const double PI = 3.14159265358979323846;
    
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = iterations;
    register int r1 asm("ebx") = 0;
    register int r2 asm("ecx") = 0;
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    double dtemp1, dtemp2, dtemp3;
    long long ltemp1, ltemp2;
    
    /* Loop with invariant address calculations */
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in multiple places */
        temp1 = data[i] + (int)(BIG_CONST_1 & 0xFFFFFFFF);
        temp2 = data[i + 1] * (int)(BIG_CONST_2 >> 32);
        
        /* Complex address calculation with invariant */
        temp3 = global_array[(i * 13) & 0xFF] + 
                (int)(BIG_CONST_1 >> 32);
        
        /* Use invariant in condition */
        if (temp3 > (int)(BIG_CONST_2 & 0xFFFFFFFF)) {
            temp4 = temp1 * temp2;
        } else {
            temp4 = temp2 / (temp1 ? temp1 : 1);
        }
        
        /* More variables to increase register pressure */
        temp5 = temp4 ^ (int)BIG_CONST_1;
        temp6 = temp5 << ((int)BIG_CONST_2 & 0x1F);
        temp7 = temp6 | r0;
        temp8 = temp7 & r1;
        
        /* Floating point with invariant */
        dtemp1 = PI * i;
        dtemp2 = dtemp1 + global_doubles[i & 0x7F];
        dtemp3 = dtemp2 * PI;  /* PI used again, non-adjacent */
        
        /* Long long operations */
        ltemp1 = BIG_CONST_1 - i;
        ltemp2 = BIG_CONST_2 + ltemp1;
        
        /* Update register variables */
        r1 += temp8;
        r2 ^= (int)ltemp2;
        
        /* Force spill/reload boundaries */
        volatile_counter = i;
    }
    
    /* Mix all results */
    return r0 + r1 + r2 + temp8 + (int)dtemp3 + (int)ltemp2;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int a, int b) {
    int result1, result2, result3;
    register int reg_var1 asm("esi") = a;
    register int reg_var2 asm("edi") = b;
    
    /* Multi-output inline assembly with many clobbers */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "movl %[in2], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[out2]\n\t"
        "subl $0x7FFFFFFF, %%eax\n\t"
        "movl %%eax, %[out3]"
        : [out1] "=&r" (result1), 
          [out2] "=&r" (result2),
          [out3] "=&r" (result3)
        : [in1] "r" (reg_var1),
          [in2] "r" (reg_var2)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    /* Use results in complex expressions */
    int x = result1 * 0x12345678;
    int y = result2 / 0x9ABCDEF0;
    int z = result3 ^ 0xFFFFFFFF;
    
    /* More inline asm with different constraints */
    int final;
    asm volatile (
        "leal (%1, %2, 4), %0\n\t"
        "addl $0x55555555, %0"
        : "=r" (final)
        : "r" (x), "r" (y)
        : "cc"
    );
    
    return final + z + reg_var1 + reg_var2;
}

/* Function C: Complex control flow with register variables */
__attribute__((noinline, noclone))
int func_complex_flow(int seed, int* data) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    register int rval asm("ebp") = seed;
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
    int i = 0, j = 0, k = 0, l = 0, m = 0, n = 0, o = 0, p = 0;
    
    /* Nested loops with many temporaries */
    for (int outer = 0; outer < 100; outer++) {
        a = data[outer % 256];
        b = a * 0x1234567;
        c = b + rval;
        
        for (int inner = 0; inner < 50; inner++) {
            d = c ^ inner;
            e = d << (inner & 0xF);
            f = e | a;
            g = f & b;
            
            /* Switch inside loop creates complex control flow */
            switch ((d + inner) % 5) {
                case 0:
                    h = g * 3;
                    i = h + 0x7FFFFFFF;  /* Large immediate */
                    break;
                case 1:
                    j = g / 5;
                    k = j - 0x80000000;  /* Another large immediate */
                    break;
                case 2:
                    l = g ^ 0xFFFFFFFF;
                    m = l | 0x55555555;
                    break;
                case 3:
                    n = g & 0xAAAAAAAA;
                    o = n << 1;
                    break;
                case 4:
                    p = g >> 2;
                    /* Use computed goto */
                    goto *labels[inner % 5];
            }
            
            label0: rval += h;
            label1: rval ^= i;
            label2: rval |= j;
            label3: rval &= k;
            label4: rval -= l;
            
            /* Force register pressure */
            volatile_counter = inner;
        }
        
        /* Use all variables to keep them live */
        m += n + o + p;
    }
    
    /* Complex return expression */
    return rval + a + b + c + d + e + f + g + h + i + j + k + l + m;
}

/* Function D: Target-specific builtins and hard registers */
#ifdef __i386__
__attribute__((noinline, noclone))
uint64_t func_builtins(int iterations) {
    uint64_t total = 0;
    register uint32_t low asm("eax");
    register uint32_t high asm("edx");
    
    for (int i = 0; i < iterations; i++) {
        /* Use rdtsc which returns in eax:edx */
        asm volatile ("rdtsc" : "=a" (low), "=d" (high));
        
        /* Chain hard register results through computations */
        uint32_t x = low * 0x12345678;
        uint32_t y = high / 0x9ABCDEF;
        
        /* More operations keeping values live */
        uint32_t z = (x ^ y) + i;
        uint32_t w = (z << 8) | (z >> 24);
        
        /* Use in address calculation */
        total += global_array[w % 256];
        total += (uint64_t)low << 32 | high;
        
        /* Force spills with many live values */
        volatile_counter = x + y + z + w;
    }
    
    return total;
}
#endif

/* Main function to drive everything */
int main(int argc, char** argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 128; i++) {
        global_doubles[i] = i * 1.5;
    }
    
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    int* data = (int*)malloc(iterations * sizeof(int));
    for (int i = 0; i < iterations; i++) {
        data[i] = i * 7;
    }
    
    /* Call all test functions to trigger early remat */
    int result1 = func_loop_invariants(iterations, data);
    int result2 = func_asm_clobber(result1, iterations);
    int result3 = func_complex_flow(result2, data);
    
    uint64_t result4 = 0;
#ifdef __i386__
    result4 = func_builtins(iterations / 10);
#endif
    
    /* Combine results to prevent optimization */
    int final_result = result1 + result2 + result3 + (int)result4;
    
    free(data);
    
    /* Return non-zero to indicate execution */
    return (final_result != 0) ? 0 : 1;
}
