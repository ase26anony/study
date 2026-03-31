/* test_mcf_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in GCC's
 * min-cost flow solver (mcf.cc) by creating register allocation
 * scenarios that require fixup graph construction with new exit/entry nodes.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or for more aggressive optimization: gcc -O3 -funroll-loops -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force many overlapping live ranges to create complex conflict graph */
#define FORCE_REGISTER_PRESSURE 1

/* Function with many overlapping live variables to stress IRA */
__attribute__((noinline))
static void complex_live_ranges(int n) {
    /* Declare many integer variables that will have overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, o, p, q, r, s, t;
    
    /* Initialize with computations to prevent optimization */
    a = n * 1;
    b = n * 2;
    c = n * 3;
    d = n * 4;
    e = n * 5;
    f = n * 6;
    g = n * 7;
    h = n * 8;
    i = n * 9;
    j = n * 10;
    k = n * 11;
    l = n * 12;
    m = n * 13;
    o = n * 14;
    p = n * 15;
    q = n * 16;
    r = n * 17;
    s = n * 18;
    t = n * 19;
    
    /* Nested loops to extend live ranges and create complex control flow */
    for (int x = 0; x < 3; x++) {
        /* All variables are live here - creates massive register pressure */
        a += b + c;
        b += d + e;
        c += f + g;
        d += h + i;
        e += j + k;
        
        /* Inner loop with more computations */
        for (int y = 0; y < 2; y++) {
            /* More overlapping live ranges */
            f += l + m;
            g += o + p;
            h += q + r;
            i += s + t;
            j += a + b;
            
            /* Volatile asm to clobber registers and increase pressure */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                : 
                : 
                : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12"
            );
        }
        
        /* More computations keeping variables live */
        k += c + d;
        l += e + f;
        m += g + h;
        o += i + j;
        p += k + l;
    }
    
    /* Final use to prevent dead code elimination */
    volatile int result = a + b + c + d + e + f + g + h + i + j + 
                         k + l + m + o + p + q + r + s + t;
    (void)result;
}

/* Another function with different pattern of live ranges */
__attribute__((noinline))
static void alternating_live_ranges(int iterations) {
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int w1, w2, w3, w4, w5, w6, w7, w8, w9, w10;
    
    /* Initialize */
    v1 = iterations;
    v2 = iterations * 2;
    v3 = iterations * 3;
    v4 = iterations * 4;
    v5 = iterations * 5;
    v6 = iterations * 6;
    v7 = iterations * 7;
    v8 = iterations * 8;
    v9 = iterations * 9;
    v10 = iterations * 10;
    
    w1 = iterations + 1;
    w2 = iterations + 2;
    w3 = iterations + 3;
    w4 = iterations + 4;
    w5 = iterations + 5;
    w6 = iterations + 6;
    w7 = iterations + 7;
    w8 = iterations + 8;
    w9 = iterations + 9;
    w10 = iterations + 10;
    
    /* Complex control flow with many basic blocks */
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            /* Group 1 variables live */
            v1 += w1;
            v2 += w2;
            v3 += w3;
            v4 += w4;
            v5 += w5;
            
            /* Force spill/reload pattern */
            asm volatile ("nop\n\tnop\n\tnop" : : : "memory");
        } else {
            /* Group 2 variables live */
            v6 += w6;
            v7 += w7;
            v8 += w8;
            v9 += w9;
            v10 += w10;
            
            /* Different clobber set */
            asm volatile ("nop" : : : "r0", "r1", "r2", "r3", "r4");
        }
        
        /* All variables live at loop end */
        if (i % 3 == 0) {
            w1 += v1;
            w2 += v2;
            w3 += v3;
        } else if (i % 3 == 1) {
            w4 += v4;
            w5 += v5;
            w6 += v6;
        } else {
            w7 += v7;
            w8 += v8;
            w9 += v9;
            w10 += v10;
        }
        
        /* Cross dependencies */
        v1 = w10 - v1;
        v10 = w1 - v10;
    }
    
    volatile int sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                      w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + w9 + w10;
    (void)sum;
}

/* Function designed to create specific graph structure */
__attribute__((noinline))
static void targeted_graph_structure(void) {
    /* This function tries to create a specific conflict graph structure
     * that might trigger the creation of new_exit_index and new_entry_index
     * nodes in the fixup graph */
    
    /* Arrays to create many pseudos */
    int arr[20];
    int brr[20];
    
    /* Initialize arrays */
    for (int i = 0; i < 20; i++) {
        arr[i] = i;
        brr[i] = i * 2;
    }
    
    /* Complex computation with many simultaneously live values */
    for (int i = 0; i < 10; i++) {
        /* Many values live across loop iterations */
        int t1 = arr[i];
        int t2 = arr[i + 1];
        int t3 = arr[i + 2];
        int t4 = arr[i + 3];
        int t5 = arr[i + 4];
        int t6 = brr[i];
        int t7 = brr[i + 1];
        int t8 = brr[i + 2];
        int t9 = brr[i + 3];
        int t10 = brr[i + 4];
        
        /* Nested loop to extend live ranges */
        for (int j = 0; j < 5; j++) {
            /* All t1-t10 are live here */
            t1 += t6;
            t2 += t7;
            t3 += t8;
            t4 += t9;
            t5 += t10;
            
            /* More variables become live */
            int u1 = t1 * j;
            int u2 = t2 * j;
            int u3 = t3 * j;
            int u4 = t4 * j;
            int u5 = t5 * j;
            
            /* Use them */
            t6 += u1;
            t7 += u2;
            t8 += u3;
            t9 += u4;
            t10 += u5;
            
            /* Clobber many registers */
            asm volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                : 
                : "r" (u1), "r" (u2)
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
            );
        }
        
        /* Write back - creates store pressure */
        arr[i] = t1;
        arr[i + 1] = t2;
        arr[i + 2] = t3;
        arr[i + 3] = t4;
        arr[i + 4] = t5;
        brr[i] = t6;
        brr[i + 1] = t7;
        brr[i + 2] = t8;
        brr[i + 3] = t9;
        brr[i + 4] = t10;
    }
    
    /* Final reduction */
    int total = 0;
    for (int i = 0; i < 20; i++) {
        total += arr[i] + brr[i];
    }
    volatile int final = total;
    (void)final;
}

/* Main test driver */
int main(void) {
    printf("Starting MCF coverage test...\n");
    
    /* Test different scenarios to explore different graph configurations */
    for (int test_case = 0; test_case < 10; test_case++) {
        complex_live_ranges(test_case);
        alternating_live_ranges(test_case + 1);
        targeted_graph_structure();
    }
    
    printf("Test completed.\n");
    return 0;
}
