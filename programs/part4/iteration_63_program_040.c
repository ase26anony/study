/* test_mcf_coverage.c
 * 
 * This program is designed to trigger GCC's min-cost flow solver during
 * register allocation, specifically to exercise the debug dumping code
 * that prints special node labels like "NEW_EXIT" and "NEW_ENTRY".
 * 
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Force many overlapping live ranges to create complex conflict graph */
#define FORCE_REGISTER_PRESSURE 1

/* Function with many overlapping live variables to stress IRA */
__attribute__((noinline))
static int test_ira_conflict(int iterations) {
    /* Declare many integer variables that will have overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z;
    int result = 0;
    
    /* Initialize all variables with different values */
    a = 1; b = 2; c = 3; d = 4; e = 5; f = 6; g = 7; h = 8; i = 9; j = 10;
    k = 11; l = 12; m = 13; n = 14; o = 15; p = 16; q = 17; r = 18; s = 19; t = 20;
    u = 21; v = 22; w = 23; x = 24; y = 25; z = 26;
    
    /* Nested loops to create complex control flow and overlapping live ranges */
    for (int outer = 0; outer < iterations; outer++) {
        /* All variables are live at this point - creating maximum register pressure */
        
        /* Complex computation using all variables to prevent optimization */
        a = b + c;
        b = c + d;
        c = d + e;
        d = e + f;
        e = f + g;
        f = g + h;
        g = h + i;
        h = i + j;
        i = j + k;
        j = k + l;
        k = l + m;
        l = m + n;
        m = n + o;
        n = o + p;
        o = p + q;
        p = q + r;
        q = r + s;
        r = s + t;
        s = t + u;
        t = u + v;
        u = v + w;
        v = w + x;
        w = x + y;
        x = y + z;
        y = z + a;
        z = a + b;
        
        /* Inner loop with more variable usage */
        for (int inner = 0; inner < 5; inner++) {
            /* Use inline asm to clobber many registers and increase pressure */
            __asm__ volatile (
                "# Force register pressure\n"
                : 
                : 
                : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
            );
            
            /* More computations to keep variables live */
            a = a ^ b;
            b = b ^ c;
            c = c ^ d;
            d = d ^ e;
            e = e ^ f;
            f = f ^ g;
            g = g ^ h;
            h = h ^ i;
            i = i ^ j;
            j = j ^ k;
            
            /* Accumulate result to prevent dead code elimination */
            result += a + b + c + d + e + f + g + h + i + j;
        }
        
        /* More computations in outer loop */
        k = k * 2;
        l = l * 3;
        m = m * 4;
        n = n * 5;
        o = o * 6;
        p = p * 7;
        q = q * 8;
        r = r * 9;
        s = s * 10;
        t = t * 11;
        u = u * 12;
        v = v * 13;
        w = w * 14;
        x = x * 15;
        y = y * 16;
        z = z * 17;
        
        result += k + l + m + n + o + p + q + r + s + t + u + v + w + x + y + z;
    }
    
    /* Final computation using all variables */
    result += a + b + c + d + e + f + g + h + i + j + 
              k + l + m + n + o + p + q + r + s + t + 
              u + v + w + x + y + z;
    
    return result;
}

/* Second test function with different variable usage pattern */
__attribute__((noinline))
static int test_ira_conflict2(int seed) {
    /* Variables with different lifetimes */
    int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed * 3;
    int v4 = seed * 4;
    int v5 = seed * 5;
    int v6, v7, v8, v9, v10;
    
    /* Conditional blocks create different live ranges */
    if (seed % 2 == 0) {
        v6 = v1 + v2;
        v7 = v3 + v4;
        /* v1, v2, v3, v4 are live here */
    } else {
        v6 = v2 + v3;
        v7 = v4 + v5;
        /* v2, v3, v4, v5 are live here */
    }
    
    /* Loop with variables that become live at different times */
    for (int i = 0; i < 10; i++) {
        v8 = v6 * i;
        v9 = v7 * (i + 1);
        
        /* Inline asm that uses specific registers */
        __asm__ volatile (
            "mov %[v8], %[v8]\n\t"
            "add %[v9], %[v9], #1\n\t"
            : [v8] "+r" (v8), [v9] "+r" (v9)
            :
            : "cc"
        );
        
        v10 = v8 + v9;
        
        /* Force spilling by using many temporaries */
        int t1 = v10 * 2;
        int t2 = t1 * 3;
        int t3 = t2 * 4;
        int t4 = t3 * 5;
        int t5 = t4 * 6;
        int t6 = t5 * 7;
        int t7 = t6 * 8;
        int t8 = t7 * 9;
        int t9 = t8 * 10;
        int t10 = t9 * 11;
        
        v6 += t10;
    }
    
    /* Switch statement for more control flow complexity */
    switch (seed % 4) {
        case 0: v1 = v6 + v7; break;
        case 1: v2 = v6 - v7; break;
        case 2: v3 = v6 * v7; break;
        case 3: v4 = v6 / (v7 ? v7 : 1); break;
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Third test: Function with parameters forcing register allocation decisions */
__attribute__((noinline))
static int test_ira_conflict3(int p1, int p2, int p3, int p4, int p5,
                              int p6, int p7, int p8, int p9, int p10) {
    /* Many parameters already use registers */
    int sum = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10;
    
    /* Local variables that conflict with parameters */
    int l1 = sum * 2;
    int l2 = sum * 3;
    int l3 = sum * 4;
    int l4 = sum * 5;
    int l5 = sum * 6;
    
    /* Complex expression with many intermediate values */
    for (int i = 0; i < 100; i++) {
        int t1 = l1 + i;
        int t2 = l2 + t1;
        int t3 = l3 + t2;
        int t4 = l4 + t3;
        int t5 = l5 + t4;
        
        /* Use all variables to keep them live */
        p1 = t1 % 256;
        p2 = t2 % 256;
        p3 = t3 % 256;
        p4 = t4 % 256;
        p5 = t5 % 256;
        
        /* Force register pressure with asm that clobbers registers */
        __asm__ volatile (
            "# Clobber many registers\n"
            "mov r0, %0\n"
            "mov r1, %1\n"
            "mov r2, %2\n"
            "mov r3, %3\n"
            "mov r4, %4\n"
            : 
            : "r" (p1), "r" (p2), "r" (p3), "r" (p4), "r" (p5)
            : "r0", "r1", "r2", "r3", "r4", "memory"
        );
        
        sum += p1 + p2 + p3 + p4 + p5 + t1 + t2 + t3 + t4 + t5;
    }
    
    return sum + l1 + l2 + l3 + l4 + l5;
}

/* Main function that exercises different patterns */
int main(int argc, char **argv) {
    int result = 0;
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    
    printf("Testing IRA min-cost flow coverage...\n");
    
    /* Call first test function - many variables with overlapping live ranges */
    result += test_ira_conflict(iterations);
    printf("Test 1 result: %d\n", result);
    
    /* Call second test function - complex control flow */
    result += test_ira_conflict2(result);
    printf("Test 2 result: %d\n", result);
    
    /* Call third test function - many parameters */
    result += test_ira_conflict3(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    printf("Test 3 result: %d\n", result);
    
    /* Additional calls with different parameters to explore more allocation scenarios */
    for (int i = 0; i < 5; i++) {
        result += test_ira_conflict2(result + i);
        result += test_ira_conflict(i, i*2, i*3, i*4, i*5, i*6, i*7, i*8, i*9, i*10);
    }
    
    printf("Final result: %d\n", result);
    return result != 0 ? 0 : 1;
}
