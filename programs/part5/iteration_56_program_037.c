/* test_early_remat.c - Target coverage for early-remat.cc lines 930-937 */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static long global_matrix[16][16];
static volatile int global_volatile = 12345;

/* Prevent optimizations from simplifying our patterns */
#define NOINLINE __attribute__((noinline, noclone))
#define KEEP_ALIVE asm volatile("" : : "r"(result) : "memory")

/* Function A: Loop with invariants and expensive constants */
NOINLINE static long func_loop_invariants(int start, int end, int *data) {
    /* Large immediate constants that need rematerialization */
    const long EXPENSIVE_CONST1 = 0x7FFFFFFFFFFFFFFF;
    const long EXPENSIVE_CONST2 = 0x5555555555555555;
    const long EXPENSIVE_CONST3 = 0xAAAAAAAAAAAAAAAA;
    
    /* Invariant pointers used in loop */
    int *invariant_ptr1 = &global_array[0];
    int *invariant_ptr2 = &global_array[128];
    long *matrix_ptr = &global_matrix[0][0];
    
    long result = 0;
    int i, j;
    
    /* Complex loop with multiple invariants and constants */
    for (i = start; i < end; i++) {
        /* Use invariants in address calculations */
        int val1 = invariant_ptr1[i & 0x7F];
        int val2 = invariant_ptr2[i & 0x7F];
        
        /* Use expensive constants in non-adjacent calculations */
        if (i & 1) {
            result += (long)val1 * EXPENSIVE_CONST1;
            result ^= EXPENSIVE_CONST2;
        } else {
            result += (long)val2 * EXPENSIVE_CONST3;
            result ^= EXPENSIVE_CONST1;
        }
        
        /* Nested loop to increase register pressure */
        for (j = 0; j < 8; j++) {
            /* More uses of invariants and constants */
            long matrix_val = matrix_ptr[(i * 8 + j) & 0xFF];
            result += matrix_val * (EXPENSIVE_CONST2 >> j);
            result ^= EXPENSIVE_CONST3 << (j & 0x3F);
        }
        
        /* Conditional with more constant usage */
        result += (result > 0) ? EXPENSIVE_CONST1 : EXPENSIVE_CONST3;
    }
    
    KEEP_ALIVE;
    return result;
}

/* Function B: Inline assembly with clobbered registers */
NOINLINE static uint64_t func_asm_clobber(int a, int b, int c) {
    /* Register variables to force hard register allocation */
    register int r1 asm("eax") = a;
    register int r2 asm("ebx") = b;
    register int r3 asm("ecx") = c;
    register uint64_t result asm("edx");
    
    /* Multi-output inline assembly with many clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "movl %3, %%ecx\n\t"
        "imull %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %%edx\n\t"
        "shrl $16, %%edx\n\t"
        "xorl %%eax, %%edx\n\t"
        : "=&a" (r1), "=&b" (r2), "=&c" (r3), "=&d" (result)
        : "1" (r1), "2" (r2), "3" (r3)
        : "memory", "cc", "esi", "edi", "ebp"
    );
    
    /* Chain of operations using hard register results */
    uint64_t temp1, temp2, temp3;
    
    /* More inline asm to create register pressure */
    asm volatile (
        "rdtsc\n\t"
        : "=a" (temp1), "=d" (temp2)
    );
    
    /* Use results in complex expressions */
    temp3 = (temp1 << 32) | temp2;
    result = result ^ temp3;
    result = result * 0x9E3779B97F4A7C15ULL;
    
    /* Another asm with different clobbers */
    asm volatile (
        "movq %1, %%rax\n\t"
        "rorq $17, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (result)
        : "r" (result)
        : "rax", "rdx", "cc"
    );
    
    KEEP_ALIVE;
    return result;
}

/* Function C: Complex control flow with register variables */
NOINLINE static int func_complex_flow(int seed, int iterations) {
    /* Many local variables with overlapping live ranges */
    int a = seed * 1103515245 + 12345;
    int b = a ^ 0xDEADBEEF;
    int c = b * 1664525 + 1013904223;
    int d = c ^ 0xCAFEBABE;
    int e = d * 1103515245 + 12345;
    int f = e ^ 0xBAADF00D;
    int g = f * 1664525 + 1013904223;
    int h = g ^ 0xFEEDFACE;
    
    /* Register variables in switch */
    register int r1 asm("esi") = a;
    register int r2 asm("edi") = b;
    register int r3 asm("ebp") = c;
    
    int result = 0;
    int i;
    
    /* Labels for computed goto */
    void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    
    for (i = 0; i < iterations; i++) {
        /* Complex switch with register variables */
        switch (i & 0x7) {
            case 0:
                result += r1 * r2;
                r1 = (r1 << 1) | (r1 >> 31);
                break;
            case 1:
                result += r2 * r3;
                r2 = (r2 << 2) | (r2 >> 30);
                break;
            case 2:
                result += r3 * r1;
                r3 = (r3 << 3) | (r3 >> 29);
                break;
            case 3:
                result += r1 ^ r2 ^ r3;
                r1 += global_volatile;
                break;
            case 4:
                result += (r1 << r2) | (r3 >> r1);
                r2 -= global_volatile;
                break;
            case 5:
                result += r1 * r2 * r3;
                r3 ^= global_volatile;
                break;
            case 6:
                /* Computed goto to create unpredictable flow */
                goto *labels[i % 6];
            L0:
                result += a + b;
                a = b ^ c;
                break;
            L1:
                result += c + d;
                b = c ^ d;
                break;
            L2:
                result += e + f;
                c = d ^ e;
                break;
            L3:
                result += g + h;
                d = e ^ f;
                break;
            L4:
                result += a * c * e * g;
                e = f ^ g;
                break;
            L5:
                result += b * d * f * h;
                f = g ^ h;
                break;
            default:
                result += i;
        }
        
        /* More overlapping live ranges */
        int t1 = a + b;
        int t2 = c + d;
        int t3 = e + f;
        int t4 = g + h;
        
        if (i & 1) {
            result += t1 * t2;
            a = t3 ^ t4;
        } else {
            result += t3 * t4;
            b = t1 ^ t2;
        }
        
        /* Chain calculations to extend live ranges */
        c = (a * b) + (c * d);
        d = (e * f) + (g * h);
        e = (t1 * t2) + (t3 * t4);
        f = (a * c * e) ^ (b * d * f);
        g = (t1 + t2 + t3 + t4) * result;
        h = (a | b | c | d | e | f | g) & 0x7FFFFFFF;
    }
    
    KEEP_ALIVE;
    return result;
}

/* Main function to drive all patterns */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 1103515245 + 12345;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            global_matrix[i][j] = (i * 16 + j) * 1664525 + 1013904223;
        }
    }
    
    /* Call functions with different patterns */
    long result1 = func_loop_invariants(0, 100, global_array);
    uint64_t result2 = func_asm_clobber(argc, 42, 0x12345678);
    int result3 = func_complex_flow(argc, 50);
    
    /* Combine results to prevent dead code elimination */
    int final_result = (result1 & 0xFFFFFFFF) 
                     ^ (result2 & 0xFFFFFFFF) 
                     ^ (result3 & 0xFFFFFFFF);
    
    /* Use result to affect control flow */
    if (final_result > 0) {
        return final_result % 256;
    } else {
        return (-final_result) % 256;
    }
}
