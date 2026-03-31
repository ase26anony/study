/* test_mcf_coverage.c
 * 
 * This test is designed to exercise GCC's integrated register allocator (IRA)
 * min-cost flow solver, specifically targeting the debug dumping code that
 * prints special node labels like "NEW_EXIT" and "NEW_ENTRY".
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Force many overlapping live ranges with complex control flow */
int test_ira_conflict(int iterations) {
    /* Declare many integer variables to create pseudo-registers */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int result = 0;
    
    /* Initialize with different values to prevent constant propagation */
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
    
    /* Nested loops with many live variables across iterations */
    for (int outer = 0; outer < iterations; outer++) {
        /* All variables are live here - creating high register pressure */
        a += outer;
        b += a;
        c += b;
        
        /* Inner loop with volatile asm to clobber registers */
        for (int inner = 0; inner < 10; inner++) {
            /* Complex computation keeping many variables live */
            d = a + b + inner;
            e = b + c + inner;
            f = c + d + inner;
            g = d + e + inner;
            h = e + f + inner;
            i = f + g + inner;
            j = g + h + inner;
            
            /* Inline asm that clobbers many registers */
            __asm__ volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                "add r0, r0, r1\n\t"
                "mov %0, r0\n\t"
                : "+r" (k), "+r" (l)
                : 
                : "r0", "r1", "cc", "memory"
            );
            
            /* More computations to extend live ranges */
            m = i + j + k;
            n = j + k + l;
            o = k + l + m;
            p = l + m + n;
            q = m + n + o;
            r = n + o + p;
            s = o + p + q;
            t = p + q + r;
            
            /* Conditional to create control flow complexity */
            if (inner % 3 == 0) {
                /* Different computation path */
                a = b + c + d;
                b = c + d + e;
                result += a + b;
            } else if (inner % 3 == 1) {
                /* Another path */
                c = d + e + f;
                d = e + f + g;
                result += c + d;
            } else {
                /* Yet another path */
                e = f + g + h;
                f = g + h + i;
                result += e + f;
            }
        }
        
        /* More computations after inner loop */
        g = h + i + j;
        h = i + j + k;
        i = j + k + l;
        
        /* Switch statement for additional control flow */
        switch (outer % 4) {
            case 0:
                j = k + l + m;
                k = l + m + n;
                result += j * k;
                break;
            case 1:
                l = m + n + o;
                m = n + o + p;
                result += l * m;
                break;
            case 2:
                n = o + p + q;
                o = p + q + r;
                result += n * o;
                break;
            case 3:
                p = q + r + s;
                q = r + s + t;
                result += p * q;
                break;
        }
    }
    
    /* Final computation using all variables */
    result += a + b + c + d + e + f + g + h + i + j;
    result += k + l + m + n + o + p + q + r + s + t;
    
    return result;
}

/* Second test function with different register pressure pattern */
int test_ira_conflict2(int seed) {
    /* Use arrays to create many temporary values */
    int arr[20];
    int sum = 0;
    
    /* Initialize array with values based on seed */
    for (int i = 0; i < 20; i++) {
        arr[i] = seed * (i + 1);
    }
    
    /* Complex loop with many intermediate calculations */
    for (int i = 0; i < 100; i++) {
        int t1 = arr[0] + arr[1];
        int t2 = arr[2] + arr[3];
        int t3 = arr[4] + arr[5];
        int t4 = arr[6] + arr[7];
        int t5 = arr[8] + arr[9];
        int t6 = arr[10] + arr[11];
        int t7 = arr[12] + arr[13];
        int t8 = arr[14] + arr[15];
        int t9 = arr[16] + arr[17];
        int t10 = arr[18] + arr[19];
        
        /* Chain computations to create long live ranges */
        for (int j = 0; j < 5; j++) {
            t1 = t1 + t2 + j;
            t2 = t2 + t3 + j;
            t3 = t3 + t4 + j;
            t4 = t4 + t5 + j;
            t5 = t5 + t6 + j;
            t6 = t6 + t7 + j;
            t7 = t7 + t8 + j;
            t8 = t8 + t9 + j;
            t9 = t9 + t10 + j;
            t10 = t10 + t1 + j;
            
            /* More inline asm to increase register pressure */
            __asm__ volatile (
                "add %0, %0, %1\n\t"
                "sub %1, %1, %0\n\t"
                : "+r" (t1), "+r" (t2)
                :
                : "cc"
            );
        }
        
        /* Update array elements to prevent dead code elimination */
        arr[i % 20] = t1 + t2 + t3 + t4 + t5;
        sum += arr[i % 20];
    }
    
    return sum;
}

/* Third test: Function with many parameters to force register allocation */
int test_many_params(int p1, int p2, int p3, int p4, int p5,
                     int p6, int p7, int p8, int p9, int p10,
                     int p11, int p12, int p13, int p14, int p15) {
    /* All parameters are live initially */
    int sum = p1 + p2 + p3 + p4 + p5;
    
    /* Create many temporary variables that overlap with parameters */
    int t1 = p6 + p7;
    int t2 = p8 + p9;
    int t3 = p10 + p11;
    int t4 = p12 + p13;
    int t5 = p14 + p15;
    
    /* Loop where all variables are live */
    for (int i = 0; i < 50; i++) {
        /* Rotate values to extend live ranges */
        int tmp = p1;
        p1 = p2 + t1;
        p2 = p3 + t2;
        p3 = p4 + t3;
        p4 = p5 + t4;
        p5 = tmp + t5;
        
        t1 = t2 + p1;
        t2 = t3 + p2;
        t3 = t4 + p3;
        t4 = t5 + p4;
        t5 = tmp + p5;
        
        sum += p1 + p2 + p3 + p4 + p5 + t1 + t2 + t3 + t4 + t5;
    }
    
    return sum;
}

/* Main function to call test cases with different parameters */
int main() {
    int total = 0;
    
    /* Call first test with different iteration counts */
    for (int i = 1; i <= 5; i++) {
        total += test_ira_conflict(i * 10);
    }
    
    /* Call second test with different seeds */
    for (int i = 0; i < 10; i++) {
        total += test_ira_conflict2(i * 100);
    }
    
    /* Call third test with many parameters */
    total += test_many_params(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
    total += test_many_params(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
    
    printf("Total result: %d\n", total);
    return 0;
}
