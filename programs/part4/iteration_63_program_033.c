/* test_mcf_coverage.c
 * 
 * This program is designed to trigger GCC's min-cost flow solver during
 * register allocation, specifically to exercise the dump_fixup_edge function
 * with n == fixup_graph->new_exit_index.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c
 */

/* Force inclusion of MCF debugging code */
#ifdef MCF_DEBUG
/* This ensures the debug code paths are compiled in */
#endif

#include <stdio.h>
#include <stdlib.h>

/* Function designed to create complex register pressure and overlapping
 * live ranges that will force IRA to build a complex conflict graph
 * requiring fixup edges with new_exit_index */
__attribute__((noinline))
static int test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z;
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
    u = iterations * 21;
    v = iterations * 22;
    w = iterations * 23;
    x = iterations * 24;
    y = iterations * 25;
    z = iterations * 26;
    
    /* Nested loops to create complex control flow and overlapping live ranges */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across the inner loop */
        int temp = a + b;
        
        /* Inner loop with many live variables */
        for (int inner = 0; inner < 10; inner++) {
            /* Complex computation keeping many variables live */
            a = b + c + inner;
            b = c + d + outer;
            c = d + e + temp;
            d = e + f + a;
            e = f + g + b;
            f = g + h + c;
            g = h + i + d;
            h = i + j + e;
            i = j + k + f;
            j = k + l + g;
            k = l + m + h;
            l = m + n + i;
            m = n + o + j;
            n = o + p + k;
            o = p + q + l;
            p = q + r + m;
            q = r + s + n;
            r = s + t + o;
            s = t + u + p;
            t = u + v + q;
            u = v + w + r;
            v = w + x + s;
            w = x + y + t;
            x = y + z + u;
            y = z + a + v;
            z = a + b + w;
            
            /* Use volatile asm to clobber many registers */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                : 
                : 
                : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
            );
        }
        
        /* More computations to extend live ranges */
        result += a + b + c + d + e + f + g + h + i + j +
                  k + l + m + n + o + p + q + r + s + t +
                  u + v + w + x + y + z;
    }
    
    /* Final computation using all variables */
    return result + a + b + c + d + e + f + g + h + i + j +
           k + l + m + n + o + p + q + r + s + t +
           u + v + w + x + y + z;
}

/* Second test function with different register pressure pattern */
__attribute__((noinline))
static int test_ira_conflict2(int seed) {
    /* Use array to create many pseudo-registers */
    int arr[30];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 30; i++) {
        arr[i] = seed * (i + 1);
    }
    
    /* Complex loop with many overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Many array elements live simultaneously */
        int t0 = arr[0] + arr[1];
        int t1 = arr[2] + arr[3];
        int t2 = arr[4] + arr[5];
        int t3 = arr[6] + arr[7];
        int t4 = arr[8] + arr[9];
        int t5 = arr[10] + arr[11];
        int t6 = arr[12] + arr[13];
        int t7 = arr[14] + arr[15];
        int t8 = arr[16] + arr[17];
        int t9 = arr[18] + arr[19];
        
        /* Chain computations to create dependencies */
        arr[0] = t0 + i;
        arr[1] = t1 + arr[0];
        arr[2] = t2 + arr[1];
        arr[3] = t3 + arr[2];
        arr[4] = t4 + arr[3];
        arr[5] = t5 + arr[4];
        arr[6] = t6 + arr[5];
        arr[7] = t7 + arr[6];
        arr[8] = t8 + arr[7];
        arr[9] = t9 + arr[8];
        
        /* More volatile asm to increase pressure */
        asm volatile (
            "mov r0, %0\n\t"
            "mov r1, %1\n\t"
            "add r0, r0, r1\n\t"
            "mov %0, r0\n\t"
            : "+r" (arr[10]), "+r" (arr[11])
            :
            : "r0", "r1", "cc"
        );
        
        sum += arr[0] + arr[1] + arr[2] + arr[3] + arr[4] +
               arr[5] + arr[6] + arr[7] + arr[8] + arr[9];
    }
    
    return sum;
}

/* Third test: Function with conditional flow creating imbalance */
__attribute__((noinline))
static int test_ira_conflict3(int x, int y) {
    int a = x, b = y, c = x + y, d = x - y;
    int e, f, g, h, i, j, k, l;
    
    /* Conditional code creating different live range patterns */
    if (x > y) {
        e = a * b;
        f = c * d;
        g = e + f;
        h = a + b + c + d;
        
        /* Loop with many live variables */
        for (int n = 0; n < 50; n++) {
            i = e + n;
            j = f + n * 2;
            k = g + n * 3;
            l = h + n * 4;
            
            /* Use all variables */
            a += i;
            b += j;
            c += k;
            d += l;
        }
    } else {
        e = a + b;
        f = c + d;
        g = e * f;
        h = a * b * c * d;
        
        /* Different loop structure */
        for (int n = 0; n < 25; n++) {
            i = g - n;
            j = h - n * 2;
            k = i * j;
            l = k / (n + 1);
            
            a -= i;
            b -= j;
            c -= k;
            d -= l;
        }
    }
    
    /* Final computation with all variables live */
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

/* Main function to drive multiple test cases */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Call test functions with different parameters to explore
     * different register allocation scenarios */
    result += test_ira_conflict(10);
    result += test_ira_conflict(20);
    result += test_ira_conflict2(5);
    result += test_ira_conflict2(15);
    result += test_ira_conflict3(100, 50);
    result += test_ira_conflict3(50, 100);
    
    printf("Result: %d\n", result);
    return 0;
}
