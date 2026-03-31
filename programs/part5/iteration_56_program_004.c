/* test_early_remat.c - Target coverage of early-remat.cc lines 930-937 */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256] = {0};
static long global_matrix[16][16] = {0};

/* Prevent optimizations from simplifying our patterns */
#define NOINLINE __attribute__((noinline, noclone))
#define KEEP_ALIVE asm volatile("" : : "r"(val) : "memory")

/* Function A: Loop with invariants and expensive constants */
NOINLINE static long func_loop_invariants(int iterations, int *data) {
    /* Large immediate constants that need rematerialization */
    const long EXPENSIVE_CONST1 = 0x7FFFFFFFFFFFFFFF;
    const long EXPENSIVE_CONST2 = 0x5555555555555555;
    const long EXPENSIVE_CONST3 = 0xAAAAAAAAAAAAAAAA;
    
    /* Invariant pointers used in loop */
    long *invariant_ptr1 = &global_matrix[0][0];
    long *invariant_ptr2 = &global_matrix[8][8];
    int *invariant_ptr3 = &global_array[128];
    
    register long acc1 asm("ebx") = 0;  /* Force hard register */
    register long acc2 asm("edi") = 0;  /* Force hard register */
    long temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    
    /* Create many overlapping live ranges */
    temp1 = EXPENSIVE_CONST1;
    temp2 = EXPENSIVE_CONST2;
    temp3 = EXPENSIVE_CONST3;
    
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in multiple places */
        long idx1 = (long)(invariant_ptr1 + i);
        long idx2 = (long)(invariant_ptr2 - i);
        int idx3 = *(invariant_ptr3 + i);
        
        /* Complex address calculations with invariants */
        temp4 = temp1 * idx1 + temp2;
        temp5 = temp2 * idx2 + temp3;
        temp6 = temp3 * idx1 + temp1;
        temp7 = temp1 * idx2 + temp2;
        temp8 = temp2 * idx1 + temp3;
        
        /* Use all temporaries to keep them live */
        acc1 += temp4 + temp5 + temp6;
        acc2 += temp7 + temp8 + (long)idx3;
        
        /* More operations to increase register pressure */
        temp1 = (temp1 >> 1) | (temp1 << 63);  /* Rotate */
        temp2 = (temp2 >> 2) | (temp2 << 62);
        temp3 = (temp3 >> 3) | (temp3 << 61);
        
        /* Conditional that uses invariants */
        if (i % 2 == 0) {
            acc1 += (long)invariant_ptr1;
            acc2 += (long)invariant_ptr2;
        } else {
            acc1 += (long)invariant_ptr3;
            acc2 += *data;
            data++;
        }
    }
    
    /* Force all values to be used */
    long result = acc1 ^ acc2 ^ temp1 ^ temp2 ^ temp3;
    KEEP_ALIVE;
    return result;
}

/* Function B: Inline assembly with clobbered registers */
NOINLINE static int func_asm_clobber(int a, int b, int c) {
    int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    
    /* Force specific hard register allocation with register vars */
    register int reg_a asm("eax") = a;
    register int reg_b asm("ebx") = b;
    register int reg_c asm("ecx") = c;
    
    /* Multi-output inline assembly that creates DF_REF_REAL_LOC references */
    asm volatile (
        "movl %[a], %%eax\n\t"
        "movl %[b], %%ebx\n\t"
        "movl %[c], %%ecx\n\t"
        "addl %%ebx, %%eax\n\t"
        "imull %%ecx, %%eax\n\t"
        "movl %%eax, %[r1]\n\t"
        "movl %%ebx, %[r2]\n\t"
        "movl %%ecx, %[r3]"
        : [r1] "=&r" (r1), [r2] "=&r" (r2), [r3] "=&r" (r3)
        : [a] "r" (reg_a), [b] "r" (reg_b), [c] "r" (reg_c)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Create many overlapping live ranges after asm */
    t1 = r1 * 0x12345678;  /* Large immediate */
    t2 = r2 * 0x9ABCDEF0;
    t3 = r3 * 0xFEDCBA98;
    t4 = t1 + t2;
    t5 = t2 + t3;
    t6 = t3 + t1;
    t7 = t4 * t5;
    t8 = t5 * t6;
    t9 = t6 * t4;
    t10 = t7 + t8 + t9;
    
    /* Second asm with different clobbers */
    asm volatile (
        "rdtsc\n\t"  /* Uses eax, edx - creates hard register refs */
        "movl %%eax, %[r4]\n\t"
        "movl %%edx, %[r5]"
        : [r4] "=r" (r4), [r5] "=r" (r5)
        :
        : "eax", "edx", "memory"
    );
    
    /* Mix results */
    r6 = r4 + t10;
    r7 = r5 + t9;
    r8 = r6 * r7;
    r9 = r8 ^ 0xFFFFFFFF;
    r10 = r9 >> 16;
    
    /* Complex control flow to extend live ranges */
    switch (r10 & 0x7) {
        case 0: r1 += r2; break;
        case 1: r2 += r3; break;
        case 2: r3 += r4; break;
        case 3: r4 += r5; break;
        case 4: r5 += r6; break;
        case 5: r6 += r7; break;
        case 6: r7 += r8; break;
        case 7: r8 += r9; break;
    }
    
    int result = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    KEEP_ALIVE;
    return result;
}

/* Function C: Complex control flow with switch and computed goto */
NOINLINE static long func_complex_control(int seed, int *data, int size) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5, &&L6, &&L7 };
    
    /* Many local variables with overlapping lives */
    long v1 = seed * 0x11111111;
    long v2 = seed * 0x22222222;
    long v3 = seed * 0x33333333;
    long v4 = seed * 0x44444444;
    long v5 = seed * 0x55555555;
    long v6 = seed * 0x66666666;
    long v7 = seed * 0x77777777;
    long v8 = seed * 0x88888888;
    long v9 = seed * 0x99999999;
    long v10 = seed * 0xAAAAAAAA;
    
    register long rv1 asm("esi") = v1;
    register long rv2 asm("edi") = v2;
    
    long result = 0;
    int i = 0;
    
    /* Outer loop */
    while (i < size) {
        /* Inner loop with switch */
        for (int j = 0; j < 8 && i < size; j++, i++) {
            int idx = data[i] & 0x7;
            
            /* Computed goto - creates complex control flow */
            goto *labels[idx];
            
        L0:
            v1 = v2 + v3;
            v4 = v5 * 0x12345678;
            continue;
        L1:
            v2 = v3 + v4;
            v5 = v6 * 0x23456789;
            continue;
        L2:
            v3 = v4 + v5;
            v6 = v7 * 0x3456789A;
            continue;
        L3:
            v4 = v5 + v6;
            v7 = v8 * 0x456789AB;
            continue;
        L4:
            v5 = v6 + v7;
            v8 = v9 * 0x56789ABC;
            continue;
        L5:
            v6 = v7 + v8;
            v9 = v10 * 0x6789ABCD;
            continue;
        L6:
            v7 = v8 + v9;
            v10 = v1 * 0x789ABCDE;
            continue;
        L7:
            v8 = v9 + v10;
            v1 = v2 * 0x89ABCDEF;
            continue;
        }
        
        /* Use register variables in computation */
        rv1 = rv1 + v1 + v3 + v5 + v7 + v9;
        rv2 = rv2 + v2 + v4 + v6 + v8 + v10;
        
        /* Conditional that uses all variables */
        if (i % 3 == 0) {
            result += rv1 * 0x13579BDF;
        } else if (i % 3 == 1) {
            result += rv2 * 0x2468ACE0;
        } else {
            result += (rv1 ^ rv2) * 0xFDB97531;
        }
    }
    
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    KEEP_ALIVE;
    return result;
}

/* Main function that calls all test patterns */
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
    
    /* Create test data */
    int test_data[100];
    for (int i = 0; i < 100; i++) {
        test_data[i] = i * 7;
    }
    
    long total = 0;
    
    /* Call function A with loop invariants */
    total += func_loop_invariants(50, test_data);
    
    /* Call function B with inline assembly */
    total += func_asm_clobber(0x11111111, 0x22222222, 0x33333333);
    
    /* Call function C with complex control flow */
    total += func_complex_control(42, test_data, 100);
    
    /* Additional calls to increase optimization opportunities */
    for (int i = 0; i < 10; i++) {
        total += func_loop_invariants(10 + i, test_data + i);
        total += func_asm_clobber(i, i * 2, i * 3);
    }
    
    /* Ensure result is used */
    return (int)(total & 0x7FFFFFFF);
}
