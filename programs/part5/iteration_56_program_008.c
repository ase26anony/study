/* test_early_remat.c - Designed to trigger virtual register creation in GCC's early rematerialization pass */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static long global_matrix[16][16];

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
static long func_loop_invariants(int limit, int *data) {
    /* Large immediate constants that are expensive to materialize */
    const long EXPENSIVE_CONST1 = 0x7FFFFFFFFFFFFFFF;
    const long EXPENSIVE_CONST2 = 0x5555555555555555;
    const long EXPENSIVE_CONST3 = 0xAAAAAAAAAAAAAAAA;
    
    /* Invariant pointers that will be used throughout loops */
    int *invariant_ptr1 = &global_array[128];
    int *invariant_ptr2 = data;
    long *invariant_ptr3 = &global_matrix[0][0];
    
    long sum = 0;
    int i, j;
    
    /* Outer loop with multiple invariants in address calculations */
    for (i = 0; i < limit; i++) {
        /* Use invariants in multiple places with expensive constants */
        long temp1 = *invariant_ptr1 * EXPENSIVE_CONST1;
        long temp2 = *invariant_ptr2 * EXPENSIVE_CONST2;
        long temp3 = *invariant_ptr3 * EXPENSIVE_CONST3;
        
        /* Inner loop to extend live ranges */
        for (j = 0; j < 8; j++) {
            /* Complex address calculation using invariants */
            sum += temp1 * (invariant_ptr1[j] + i);
            sum += temp2 * (invariant_ptr2[j] - i);
            sum += temp3 * (invariant_ptr3[j * 16] * j);
            
            /* More operations to increase register pressure */
            sum += (EXPENSIVE_CONST1 >> j) & 0xFF;
            sum += (EXPENSIVE_CONST2 << j) & 0xFF00;
            sum += (EXPENSIVE_CONST3 ^ j) & 0xFFFF;
        }
        
        /* Conditional branch that uses invariants */
        if (sum > EXPENSIVE_CONST1) {
            sum -= *invariant_ptr1 * EXPENSIVE_CONST2;
        } else {
            sum += *invariant_ptr2 * EXPENSIVE_CONST3;
        }
    }
    
    return sum;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
static long func_asm_clobber(int a, int b, int c) {
    /* Register variables to force specific allocation */
    register int r1 asm("eax") = a;
    register int r2 asm("ebx") = b;
    register int r3 asm("ecx") = c;
    register long r4 asm("edx");
    register long r5 asm("esi");
    register long r6 asm("edi");
    
    /* Large immediate for rematerialization */
    const long BIG_CONST = 0x123456789ABCDEF0;
    
    /* Multi-output inline assembly with many clobbered registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "movl %3, %%ecx\n\t"
        "imull %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movq %4, %%rdx\n\t"
        "movq %%rdx, %%rsi\n\t"
        "movq %%rsi, %%rdi"
        : "=r" (r4), "=&r" (r5), "=&r" (r6)
        : "r" (r1), "r" (r2), "r" (r3), "m" (BIG_CONST)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory", "cc"
    );
    
    /* Use results in complex expressions */
    long result = r4;
    result += r5 * BIG_CONST;
    result += r6 / (BIG_CONST >> 32);
    
    /* More operations to extend live ranges */
    for (int i = 0; i < 16; i++) {
        result ^= (BIG_CONST << i);
        result += (BIG_CONST >> i);
    }
    
    return result;
}

/* Function C: Complex control flow with register variables */
__attribute__((noinline, noclone))
static long func_complex_control(int selector, int iterations) {
    /* Many local variables with overlapping live ranges */
    register long a asm("eax");
    register long b asm("ebx");
    register long c asm("ecx");
    register long d asm("edx");
    register long e asm("esi");
    register long f asm("edi");
    
    /* Expensive address constant */
    static const long * const TABLE_BASE = (const long *)&global_matrix[0][0];
    
    /* Initialize with expensive constants */
    a = 0x100000001;
    b = 0x200000002;
    c = 0x300000003;
    d = 0x400000004;
    e = 0x500000005;
    f = 0x600000006;
    
    /* Complex control flow with switch */
    long total = 0;
    for (int i = 0; i < iterations; i++) {
        switch ((selector + i) % 7) {
            case 0:
                total += a * TABLE_BASE[0];
                a = b + c;
                break;
            case 1:
                total += b * TABLE_BASE[1];
                b = c - d;
                break;
            case 2:
                total += c * TABLE_BASE[2];
                c = d ^ e;
                break;
            case 3:
                total += d * TABLE_BASE[3];
                d = e | f;
                break;
            case 4:
                total += e * TABLE_BASE[4];
                e = f & a;
                break;
            case 5:
                total += f * TABLE_BASE[5];
                f = a << 2;
                break;
            case 6:
                total += (a + b + c + d + e + f) * TABLE_BASE[6];
                /* Use builtin for hard register reference */
                {
                    unsigned long long tsc = __builtin_ia32_rdtsc();
                    total += (tsc & 0xFFFFFFFF);
                }
                break;
        }
        
        /* Additional computations to increase pressure */
        if (i % 3 == 0) {
            total += (a * b) / (c + 1);
        } else if (i % 3 == 1) {
            total += (d * e) ^ (f - 1);
        } else {
            total += (a | b) & (c ^ d);
        }
    }
    
    /* Computed goto for unusual control flow */
    void *labels[] = { &&label1, &&label2, &&label3, &&label4 };
    goto *labels[selector % 4];
    
label1:
    return total + a;
label2:
    return total + b;
label3:
    return total + c;
label4:
    return total + d;
}

/* Function D: Mixed patterns for maximum pressure */
__attribute__((noinline, noclone))
static long func_mixed_patterns(double *data, int size) {
    /* Many temporaries with overlapping lives */
    double t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    /* Expensive double constants */
    const double EXP_DBL1 = 3.14159265358979323846;
    const double EXP_DBL2 = 2.71828182845904523536;
    const double EXP_DBL3 = 1.41421356237309504880;
    
    /* Initialize */
    t1 = data[0] * EXP_DBL1;
    t2 = data[1] * EXP_DBL2;
    t3 = data[2] * EXP_DBL3;
    t4 = t1 + t2;
    t5 = t2 + t3;
    t6 = t3 + t1;
    
    /* Long sequence of dependent operations */
    for (int i = 3; i < size; i++) {
        t7 = data[i] * t1;
        t8 = data[i] * t2;
        t9 = data[i] * t3;
        t10 = t7 + t8 + t9;
        
        /* Convert to long with expensive operations */
        l1 = (long)(t1 * 1000000);
        l2 = (long)(t2 * 1000000);
        l3 = (long)(t3 * 1000000);
        l4 = (long)(t4 * 1000000);
        l5 = (long)(t5 * 1000000);
        l6 = (long)(t6 * 1000000);
        l7 = (long)(t7 * 1000000);
        l8 = (long)(t8 * 1000000);
        l9 = (long)(t9 * 1000000);
        l10 = (long)(t10 * 1000000);
        
        /* Use all temporaries in complex expression */
        data[i] = (t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10) / 10.0;
        
        /* Keep long values live across loop iterations */
        t1 = (double)l1 / 1000000.0;
        t2 = (double)l2 / 1000000.0;
        t3 = (double)l3 / 1000000.0;
        t4 = (double)l4 / 1000000.0;
        t5 = (double)l5 / 1000000.0;
        
        /* Conditional with many live values */
        if (i % 2 == 0) {
            t6 = (double)(l6 + l7) / 1000000.0;
            t7 = (double)(l8 + l9) / 1000000.0;
        } else {
            t8 = (double)(l10 * 2) / 1000000.0;
            t9 = (double)(l1 - l2) / 1000000.0;
        }
    }
    
    /* Final computation using all values */
    return (long)(t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10);
}

/* Main function to drive everything */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            global_matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Local array for testing */
    double local_data[64];
    for (int i = 0; i < 64; i++) {
        local_data[i] = i * 1.5;
    }
    
    /* Call test functions with arguments that create register pressure */
    long result = 0;
    
    /* Use command line args or defaults to vary inputs */
    int limit = (argc > 1) ? atoi(argv[1]) : 100;
    int selector = (argc > 2) ? atoi(argv[2]) : 5;
    
    result += func_loop_invariants(limit, global_array);
    result += func_asm_clobber(limit, selector, argc);
    result += func_complex_control(selector, limit / 10);
    result += func_mixed_patterns(local_data, 64);
    
    /* Use result to prevent optimization */
    return (int)(result % 1000000);
}
