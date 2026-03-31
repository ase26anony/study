/* test_early_remat.c - Target specific coverage for early-remat.cc lines 930-937 */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static double global_doubles[128];
static volatile int volatile_counter = 0;

/* Prevent optimizations from eliminating register pressure */
#define KEEP_ALIVE(x) do { volatile_counter += (int)(x); } while(0)

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int* data) {
    /* Large immediate constants that need rematerialization */
    const long long BIG_CONST_1 = 0x7FFFFFFFFFFFFFFFLL;
    const long long BIG_CONST_2 = 0x5555555555555555LL;
    const long long BIG_CONST_3 = 0xAAAAAAAAAAAAAAAALL;
    
    /* Loop invariants with different modes */
    int* invariant_ptr = &global_array[128];
    double* dbl_invariant = &global_doubles[64];
    long long* ll_invariant = (long long*)data;
    
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = iterations;
    register int r1 asm("ebx") = 0;
    register int r2 asm("ecx") = 0;
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    long long ll1, ll2, ll3, ll4, ll5;
    double d1, d2, d3;
    
    /* Complex loop with many live values */
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in multiple places */
        v1 = *invariant_ptr + i;
        v2 = *(invariant_ptr - 64) + (int)BIG_CONST_1;
        v3 = *(invariant_ptr + 32) + (int)(BIG_CONST_2 >> 32);
        
        /* Use expensive constants in non-adjacent instructions */
        ll1 = BIG_CONST_1 - i;
        ll2 = BIG_CONST_2 + *ll_invariant;
        ll3 = BIG_CONST_3 * ll1;
        
        /* Floating point calculations with invariants */
        d1 = *dbl_invariant * 3.141592653589793;
        d2 = d1 + *(dbl_invariant - 32);
        d3 = d2 * *(dbl_invariant + 16);
        
        /* More overlapping live ranges */
        v4 = v1 * v2;
        v5 = v3 + v4;
        v6 = (int)ll1 + (int)ll2;
        v7 = v5 - v6;
        v8 = v7 * (int)d1;
        v9 = v8 / (i + 1);
        v10 = v9 % 256;
        
        /* Use register variables in complex expressions */
        r1 += v10;
        r2 ^= (int)ll3;
        
        /* Force spill/reload pressure */
        if (i % 3 == 0) {
            ll4 = BIG_CONST_1 ^ BIG_CONST_2;
            ll5 = ll4 + (long long)invariant_ptr;
            v1 += (int)ll5;
        } else if (i % 3 == 1) {
            ll4 = BIG_CONST_2 | BIG_CONST_3;
            ll5 = ll4 - (long long)dbl_invariant;
            v2 += (int)ll5;
        } else {
            ll4 = BIG_CONST_3 & BIG_CONST_1;
            ll5 = ll4 * (long long)ll_invariant;
            v3 += (int)ll5;
        }
        
        /* Chain computations to extend live ranges */
        v4 = v1 + v2 + v3;
        v5 = v4 * r1;
        v6 = v5 - r2;
        r0 = v6 + i;
    }
    
    /* Return using register variable */
    asm volatile ("" : "+r"(r0), "+r"(r1), "+r"(r2));
    return r0 + r1 + r2;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int a, int b, int c) {
    int result1, result2, result3;
    int temp1, temp2, temp3, temp4, temp5, temp6;
    
    /* Register variables forced to specific registers */
    register int reg_a asm("edi") = a;
    register int reg_b asm("esi") = b;
    register int reg_c asm("ebp") = c;
    
    /* Multi-output inline assembly creating DF_REF_REAL_LOC references */
    asm volatile (
        "movl %[reg_a], %[res1]\n\t"
        "imull %[reg_b], %[res1]\n\t"
        "movl %[reg_c], %[res2]\n\t"
        "addl %[reg_a], %[res2]\n\t"
        "movl %[res1], %[res3]\n\t"
        "subl %[res2], %[res3]\n\t"
        : [res1] "=&r" (result1),
          [res2] "=&r" (result2),
          [res3] "=&r" (result3)
        : [reg_a] "r" (reg_a),
          [reg_b] "r" (reg_b),
          [reg_c] "r" (reg_c)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    /* Create many temporary values with overlapping lives */
    temp1 = result1 * 0x12345678;  /* Large immediate */
    temp2 = result2 + 0x9ABCDEF0;
    temp3 = result3 ^ 0x55555555;
    temp4 = temp1 & temp2;
    temp5 = temp3 | temp4;
    temp6 = temp5 << 3;
    
    /* More inline asm with different clobbers */
    asm volatile (
        "cpuid\n\t"
        "rdtsc\n\t"
        : "=a" (temp1), "=d" (temp2)
        : "a" (0)
        : "ebx", "ecx", "memory"
    );
    
    /* Use builtins that use specific hard registers */
    unsigned long long tsc = __builtin_ia32_rdtsc();
    temp3 = (int)(tsc >> 32);
    temp4 = (int)tsc;
    
    /* Chain all results together */
    return result1 + result2 + result3 + temp1 + temp2 + temp3 + temp4 + temp6;
}

/* Function C: Complex control flow with switch */
__attribute__((noinline, noclone))
int func_complex_control(int start, int* data) {
    /* Many local variables with register keyword */
    register int r0 asm("eax") = start;
    register int r1 asm("ebx") = 0;
    register int r2 asm("ecx") = 0;
    register int r3 asm("edx") = 0;
    
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    /* Labels for computed goto */
    void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    /* Nested loops with switches inside */
    for (int i = 0; i < 100; i++) {
        /* Use all variables to keep them live */
        v1 += r0;
        v2 += r1;
        v3 += r2;
        v4 += r3;
        
        /* Switch with computed goto creates complex control flow */
        int selector = (i * 17) % 5;
        goto *labels[selector];
        
    label0:
        v5 = v1 * v2 + 0x7FFFFFFF;  /* Large immediate */
        v6 = v3 - v4 - 0x80000000;
        v7 = (v5 ^ v6) | 0x55555555;
        continue;
        
    label1:
        v8 = v2 / (v1 + 1) + 0x12345678;
        v9 = v4 % (v3 + 1) + 0x9ABCDEF;
        v10 = (v8 & v9) ^ 0xAAAAAAAA;
        continue;
        
    label2:
        v11 = v1 << (i % 16);
        v12 = v2 >> (i % 8);
        v13 = v11 | v12;
        v14 = v13 & 0xFFFFFFFF;
        continue;
        
    label3:
        v15 = v3 * v4;
        r0 = v15 + 0xDEADBEEF;
        r1 = r0 - 0xCAFEBABE;
        continue;
        
    label4:
        r2 = v5 ^ v6 ^ v7;
        r3 = r2 * 0x31415926;
        v1 = r3 + i;
        continue;
    }
    
    /* Use all variables in final computation */
    int sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
              v11 + v12 + v13 + v14 + v15 + r0 + r1 + r2 + r3;
    
    /* Conditional based on global address */
    if ((uintptr_t)&global_array[0] > 0x1000) {
        sum += *(int*)((uintptr_t)&global_array[64] + 0x10000000);
    }
    
    return sum;
}

/* Main function to drive everything */
int main(int argc, char** argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 128; i++) {
        global_doubles[i] = i * 1.5;
    }
    
    int iterations = (argc > 1) ? atoi(argv[1]) : 1000;
    if (iterations < 100) iterations = 100;
    
    /* Call all test functions with arguments that create register pressure */
    int result1 = func_loop_invariants(iterations, global_array);
    KEEP_ALIVE(result1);
    
    int result2 = func_asm_clobber(result1, iterations, 0x12345678);
    KEEP_ALIVE(result2);
    
    int result3 = func_complex_control(result2, global_array);
    KEEP_ALIVE(result3);
    
    /* Combine results to prevent dead code elimination */
    int final_result = result1 + result2 + result3 + volatile_counter;
    
    /* Use result in system call to ensure it's live */
    if (final_result > 1000000) {
        return final_result % 256;
    }
    
    return final_result;
}
