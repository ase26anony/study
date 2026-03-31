/* test_mcf_coverage.c
 * 
 * This program is designed to trigger GCC's min-cost flow solver during
 * register allocation, specifically to exercise the debug dumping code
 * that prints special node labels like "NEW_EXIT" and "NEW_ENTRY".
 * 
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

/* Force inclusion of MCF debugging code */
#ifdef MCF_DEBUG
/* This ensures the debug code is compiled in */
#else
/* Define it if not already defined by compiler flags */
#define MCF_DEBUG 1
#endif

#include <stdio.h>
#include <stdlib.h>

/* Function with complex live ranges to create register pressure */
static int __attribute__((noinline)) 
complex_live_ranges(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int result = 0;
    
    /* Initialize all variables with different values to prevent optimization */
    a = iterations * 1;
    b = iterations * 2;
    c = iterations * 3;
    d = iterations * 4;
    e = iterations * 5;
    f = iterations * 6;
    g = iterations * 7;
    h = iterations * 8;
    i = iterations * 9;
    j = iterations * 10;
    k = iterations * 11;
    l = iterations * 12;
    m = iterations * 13;
    n = iterations * 14;
    o = iterations * 15;
    p = iterations * 16;
    q = iterations * 17;
    r = iterations * 18;
    s = iterations * 19;
    t = iterations * 20;
    
    /* Nested loops to create complex control flow and extended live ranges */
    for (int outer = 0; outer < iterations; outer++) {
        /* All variables are live at this point - creating high register pressure */
        
        /* Use volatile assembly to clobber registers and force spills */
        asm volatile (
            "/* Clobber many registers to increase pressure */"
            :
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r14", "memory"
        );
        
        for (int inner = 0; inner < 10; inner++) {
            /* Complex computation keeping many variables live */
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
            s = t + a;
            t = a + b;
            
            /* More volatile asm to prevent optimization and clobber registers */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                :
                :
                : "memory"
            );
        }
        
        /* Use all variables in result calculation to keep them live */
        result += a + b + c + d + e + f + g + h + i + j +
                  k + l + m + n + o + p + q + r + s + t;
    }
    
    return result;
}

/* Second function with different conflict pattern */
static int __attribute__((noinline))
alternate_conflict_pattern(int seed) {
    /* Create variables with partial overlapping live ranges */
    int v1 = seed * 2;
    int v2 = seed * 3;
    int v3 = seed * 5;
    int v4 = seed * 7;
    int v5 = seed * 11;
    int v6 = seed * 13;
    int v7 = seed * 17;
    int v8 = seed * 19;
    int v9 = seed * 23;
    int v10 = seed * 29;
    
    /* Create diamond control flow to split live ranges */
    if (seed & 1) {
        v1 = v2 + v3;
        v4 = v5 + v6;
        /* v2, v3, v5, v6 die here */
    } else {
        v7 = v8 + v9;
        v10 = v1 + v4;
        /* v8, v9 die here */
    }
    
    /* All remaining variables are live here */
    int sum = v1 + v4 + v7 + v10;
    
    /* Loop with varying register usage */
    for (int i = 0; i < 5; i++) {
        sum += (v1 * i) + (v4 * (i + 1)) + (v7 * (i + 2)) + (v10 * (i + 3));
        
        /* Force register shuffling */
        asm volatile (
            "/* Force register moves */"
            :
            :
            : "memory"
        );
    }
    
    return sum;
}

/* Function that creates imbalance in defs/uses to trigger fixup edges */
static int __attribute__((noinline))
imbalanced_def_use(int base) {
    /* Create many uses of a single variable with few definitions */
    int heavily_used = base;
    int temp1, temp2, temp3, temp4, temp5;
    
    /* Single definition, many uses */
    temp1 = heavily_used + 1;
    temp2 = heavily_used + temp1;
    temp3 = heavily_used + temp2;
    temp4 = heavily_used + temp3;
    temp5 = heavily_used + temp4;
    
    /* Chain of uses */
    for (int i = 0; i < 8; i++) {
        heavily_used = heavily_used + temp1 + temp2 + temp3 + temp4 + temp5;
        temp1 = heavily_used / 2;
        temp2 = heavily_used / 3;
        temp3 = heavily_used / 4;
        temp4 = heavily_used / 5;
        temp5 = heavily_used / 6;
    }
    
    /* Many parallel live ranges at the end */
    return heavily_used + temp1 * 2 + temp2 * 3 + temp3 * 4 + temp4 * 5 + temp5 * 6;
}

/* Main test function that exercises different conflict patterns */
void test_ira_conflict(void) {
    int total = 0;
    
    printf("Testing IRA conflict patterns...\n");
    
    /* Test 1: Many overlapping live ranges */
    total += complex_live_ranges(3);
    
    /* Test 2: Different conflict pattern */
    total += alternate_conflict_pattern(total);
    
    /* Test 3: Imbalanced def/use pattern */
    total += imbalanced_def_use(total);
    
    /* Test 4: Varying iteration counts to create different graph sizes */
    for (int scale = 1; scale <= 4; scale++) {
        total += complex_live_ranges(scale);
        total += alternate_conflict_pattern(total + scale);
    }
    
    /* Use the result to prevent dead code elimination */
    volatile int sink = total;
    (void)sink;
}

/* Additional test with specific architecture constraints in mind */
#ifdef __arm__
__attribute__((target("arch=armv7-a")))
void test_arm_register_pressure(void) {
    /* ARM has only 16 general purpose registers, so pressure is high */
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
    
    /* Initialize all to different values */
    r0 = 1; r1 = 2; r2 = 3; r3 = 4; r4 = 5; r5 = 6; r6 = 7; r7 = 8;
    r8 = 9; r9 = 10; r10 = 11; r11 = 12; r12 = 13; r13 = 14; r14 = 15; r15 = 16;
    
    /* Use them all in a complex computation */
    for (int i = 0; i < 100; i++) {
        r0 = r1 + r2;
        r1 = r3 + r4;
        r2 = r5 + r6;
        r3 = r7 + r8;
        r4 = r9 + r10;
        r5 = r11 + r12;
        r6 = r13 + r14;
        r7 = r15 + r0;
        r8 = r1 + r2;
        r9 = r3 + r4;
        r10 = r5 + r6;
        r11 = r7 + r8;
        r12 = r9 + r10;
        r13 = r11 + r12;
        r14 = r13 + r14;
        r15 = r15 + 1;
        
        /* Clobber caller-saved registers */
        asm volatile (
            "mov r0, %0\n\t"
            "mov r1, %1\n\t"
            :
            : "r" (r0), "r" (r1)
            : "r0", "r1"
        );
    }
    
    volatile int result = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 +
                         r8 + r9 + r10 + r11 + r12 + r13 + r14 + r15;
    (void)result;
}
#endif

int main(void) {
    /* Call the test function multiple times with different parameters
     * to explore different flow network configurations */
    for (int run = 0; run < 5; run++) {
        test_ira_conflict();
        
        #ifdef __arm__
        test_arm_register_pressure();
        #endif
        
        /* Small allocation to potentially change stack layout */
        char *buffer = malloc(64);
        if (buffer) {
            buffer[0] = run;
            free(buffer);
        }
    }
    
    return 0;
}
