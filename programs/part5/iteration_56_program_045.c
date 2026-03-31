/* test_early_remat.c - Target coverage for early-remat.cc lines 930-937 */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static double global_doubles[128];
static volatile int global_volatile = 12345;

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int* restrict out) {
    /* Large immediate constants that need rematerialization */
    const long long expensive_const1 = 0x7FFFFFFFFFFFFFFFLL;
    const long long expensive_const2 = 0x5555555555555555LL;
    const double expensive_float = 3.14159265358979323846 * 1e30;
    
    /* Loop invariants from arguments/globals */
    int* invariant_ptr1 = global_array;
    double* invariant_ptr2 = global_doubles;
    int invariant_val = global_volatile;
    
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = 0;
    register int r1 asm("ebx") = 0;
    register int r2 asm("ecx") = 0;
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Complex loop with invariant usage in multiple places */
    for (int i = 0; i < iterations; i += 2) {
        /* Use invariants in address calculations */
        v1 = invariant_ptr1[i] + invariant_val;
        v2 = (int)(*(invariant_ptr2 + (i % 128)) * expensive_float);
        
        /* Use expensive constants in different expressions */
        v3 = (int)(expensive_const1 >> (i % 32));
        v4 = (int)(expensive_const2 & (0xFFFFFFFFLL << (i % 16)));
        
        /* Overlapping live ranges created by sequence of operations */
        v5 = v1 * v2 + v3;
        v6 = v4 - v1 * v3;
        v7 = v5 ^ v6;
        v8 = v7 + (int)expensive_const1;
        v9 = v8 * v2 - v4;
        v10 = v9 / (v1 + 1);
        
        /* More operations to extend live ranges */
        v11 = v10 << (i % 8);
        v12 = v11 | v3;
        v13 = v12 & (int)expensive_const2;
        v14 = v13 + v6;
        v15 = v14 * v7;
        v16 = v15 - v8;
        v17 = v16 ^ v9;
        v18 = v17 + v10;
        v19 = v18 * v11;
        v20 = v19 | v12;
        
        /* Use register variables with hard constraints */
        asm volatile("" : "+r"(r0), "+r"(r1), "+r"(r2) : : "memory");
        r0 += v1; r1 ^= v2; r2 |= v3;
        
        /* Store results using invariant pointer */
        out[i] = v20 + r0 + r1 + r2;
        if (i + 1 < iterations) {
            out[i + 1] = v19 - r0 * r1 + (int)(expensive_const1 >> 16);
        }
        
        /* Conditional branch creating different control flow paths */
        if (i % 7 == 0) {
            /* Different use of invariants */
            v1 = invariant_ptr1[255 - i] + (int)expensive_const2;
            v2 = (int)(*(invariant_ptr2 + ((i + 64) % 128)) * expensive_float * 2.0);
        } else if (i % 13 == 0) {
            v3 = (int)(expensive_const1 << (i % 24));
            v4 = (int)(expensive_const2 | (0xAAAAAAAAAAAAAAAALL));
        }
    }
    
    return r0 + r1 + r2;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int a, int b, int c) {
    int result1, result2, result3;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %5, %%eax\n\t"
        "movl %6, %%ebx\n\t"
        "movl %7, %%ecx\n\t"
        "imull %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movl %%ebx, %1\n\t"
        "movl %%ecx, %2\n\t"
        "leal (%%eax, %%ebx, 4), %3\n\t"
        "leal (%%ecx, %%eax, 2), %4\n\t"
        : "=r"(result1), "=r"(result2), "=r"(result3),
          "=r"(tmp1), "=r"(tmp2)
        : "r"(a), "r"(b), "r"(c)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* More variables to increase register pressure */
    register int reg_var1 asm("esi") = result1;
    register int reg_var2 asm("edi") = result2;
    int x1 = reg_var1 * 1234567;
    int x2 = reg_var2 / 9876543;
    int x3 = x1 + x2 + (int)0xDEADBEEF;
    int x4 = x3 ^ 0xCAFEBABE;
    int x5 = x4 << 3;
    int x6 = x5 >> 1;
    int x7 = x6 | 0x12345678;
    int x8 = x7 & 0xF0F0F0F0;
    int x9 = x8 + 0xAAAAAAAA;
    int x10 = x9 * 0x55555555;
    
    /* Another asm with different clobbers */
    asm volatile (
        "cpuid\n\t"
        : "=a"(tmp3), "=b"(tmp4), "=c"(tmp5), "=d"(tmp6)
        : "a"(0)
        : "memory"
    );
    
    return result1 + result2 + result3 + x10 + tmp3 + tmp4 + tmp5 + tmp6;
}

/* Function C: Complex control flow with switch and computed goto */
__attribute__((noinline, noclone))
int func_complex_flow(int seed, int* data, int size) {
    static void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    
    int sum = 0;
    int i = 0;
    
    /* Many temporary variables with overlapping lives */
    int t1 = seed * 1103515245 + 12345;
    int t2 = t1 ^ 0x5A5A5A5A;
    int t3 = t2 << 17;
    int t4 = t3 | 0x33333333;
    int t5 = t4 - 0x11111111;
    int t6 = t5 * 0x01010101;
    int t7 = t6 >> 24;
    int t8 = t7 & 0xFF;
    int t9 = t8 + 0x100;
    int t10 = t9 * 0x11;
    
    /* Nested loops with switch inside */
    while (i < size) {
        int j = 0;
        while (j < 8 && i + j < size) {
            /* Use computed goto for unpredictable control flow */
            int idx = data[i + j] % 6;
            goto *labels[idx];
            
        L0:
            sum += t1 + t2;
            t1 = t1 * 3 + 1;
            break;
        L1:
            sum += t3 + t4;
            t3 = t3 ^ t4;
            break;
        L2:
            sum += t5 + t6;
            t5 = t5 - t6;
            break;
        L3:
            sum += t7 + t8;
            t7 = t7 | t8;
            break;
        L4:
            sum += t9 + t10;
            t9 = t9 & t10;
            break;
        L5:
            sum += t1 + t10;
            t10 = t10 << 1;
            break;
        }
        
        /* More operations extending live ranges across loop iterations */
        t2 = t2 + t1;
        t4 = t4 - t3;
        t6 = t6 * t5;
        t8 = t8 ^ t7;
        t10 = t10 | t9;
        
        i += 8;
        
        /* Switch statement creating different basic blocks */
        switch (sum % 5) {
            case 0:
                t1 = t1 + 0x1000;
                t3 = t3 * 2;
                break;
            case 1:
                t5 = t5 - 0x2000;
                t7 = t7 >> 1;
                break;
            case 2:
                t9 = t9 ^ 0x3000;
                t2 = t2 & 0x0F0F0F0F;
                break;
            case 3:
                t4 = t4 | 0x4000;
                t6 = t6 + 0x5000;
                break;
            case 4:
                t8 = t8 * 3;
                t10 = t10 - 0x6000;
                break;
        }
    }
    
    return sum + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
}

/* Main function to drive everything */
int main(int argc, char** argv) {
    int iterations = argc > 1 ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 100;
    
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 7;
    }
    for (int i = 0; i < 128; i++) {
        global_doubles[i] = i * 1.41421356;
    }
    
    int* output_buffer = malloc(iterations * sizeof(int));
    int* data_buffer = malloc(iterations * sizeof(int));
    
    /* Initialize data buffer with pseudo-random values */
    for (int i = 0; i < iterations; i++) {
        data_buffer[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Call all test functions to trigger different patterns */
    int result1 = func_loop_invariants(iterations, output_buffer);
    int result2 = func_asm_clobber(result1, iterations, global_volatile);
    int result3 = func_complex_flow(result2, data_buffer, iterations);
    
    /* Combine results to prevent optimization */
    int final_result = result1 + result2 + result3;
    
    /* Use results to prevent dead code elimination */
    for (int i = 0; i < iterations && i < 10; i++) {
        final_result += output_buffer[i];
    }
    
    free(output_buffer);
    free(data_buffer);
    
    return final_result & 0xFF;
}
