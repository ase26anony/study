/* test_mcf_coverage.c
 * 
 * This program is designed to trigger GCC's min-cost flow solver during
 * register allocation, specifically to exercise the debug dumping code
 * path that prints "NEW_EXIT" when n == fixup_graph->new_exit_index.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

/* Force inclusion of mcf debugging by defining MCF_DEBUG if not already defined */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

/* Function with complex register pressure to trigger fixup graph creation */
__attribute__((noinline))
static int test_ira_conflict(int iterations) {
    /* Declare many variables with overlapping live ranges */
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
    
    /* Nested loops to create complex liveness intervals */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across the inner loop */
        int temp1 = a + b + c + d;
        int temp2 = e + f + g + h;
        
        /* Inner loop with register pressure */
        for (int inner = 0; inner < 10; inner++) {
            /* Force all variables to be used in computation */
            int compute1 = temp1 + i + j + k + l;
            int compute2 = temp2 + m + n + o + p;
            
            /* More computations to increase register pressure */
            int compute3 = compute1 + q + r + s + t;
            int compute4 = compute2 + u + v + w + x;
            
            /* Final computation using all variables */
            int final_compute = compute3 + compute4 + y + z + inner;
            
            /* Use asm volatile to clobber many registers and prevent optimization */
            asm volatile (
                "/* Clobber many registers to increase pressure */"
                :
                : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e), "r" (f),
                  "r" (g), "r" (h), "r" (i), "r" (j), "r" (k), "r" (l),
                  "r" (m), "r" (n), "r" (o), "r" (p), "r" (q), "r" (r),
                  "r" (s), "r" (t), "r" (u), "r" (v), "r" (w), "r" (x),
                  "r" (y), "r" (z), "r" (final_compute)
                : "memory", "cc"
            );
            
            result += final_compute;
        }
        
        /* Modify variables to create different live ranges */
        a += 1; b += 2; c += 3; d += 4;
        e += 5; f += 6; g += 7; h += 8;
        i += 9; j += 10; k += 11; l += 12;
        m += 13; n += 14; o += 15; p += 16;
        q += 17; r += 18; s += 19; t += 20;
        u += 21; v += 22; w += 23; x += 24;
        y += 25; z += 26;
    }
    
    /* Final computation using all variables */
    result += a + b + c + d + e + f + g + h + i + j + k + l + m + 
              n + o + p + q + r + s + t + u + v + w + x + y + z;
    
    return result;
}

/* Second test function with different register pressure pattern */
__attribute__((noinline))
static int test_ira_conflict2(int seed) {
    /* Use array to create many pseudo-registers */
    int arr[30];
    int sum = 0;
    
    /* Initialize array with values */
    for (int i = 0; i < 30; i++) {
        arr[i] = seed * (i + 1);
    }
    
    /* Complex loop with many live variables */
    for (int i = 0; i < 20; i++) {
        /* Many array elements live simultaneously */
        int t1 = arr[i] + arr[i+1] + arr[i+2];
        int t2 = arr[i+3] + arr[i+4] + arr[i+5];
        int t3 = arr[i+6] + arr[i+7] + arr[i+8];
        int t4 = arr[i+9] + arr[i+10] + arr[i+11];
        
        /* Nested computation */
        for (int j = 0; j < 5; j++) {
            int inner1 = t1 * j + t2;
            int inner2 = t3 * j + t4;
            int inner3 = inner1 + inner2 + arr[i+12] + arr[i+13];
            
            /* Force register pressure with asm */
            asm volatile (
                "/* More register clobbering */"
                :
                : "r" (t1), "r" (t2), "r" (t3), "r" (t4),
                  "r" (inner1), "r" (inner2), "r" (inner3)
                : "memory", "cc"
            );
            
            sum += inner3;
        }
        
        /* Modify array elements to prevent optimization */
        for (int j = 0; j < 15; j++) {
            arr[i+j] += j;
        }
    }
    
    return sum;
}

/* Third test with switch-case to create complex control flow */
__attribute__((noinline))
static int test_ira_conflict3(int value) {
    int r1 = value * 1, r2 = value * 2, r3 = value * 3;
    int r4 = value * 4, r5 = value * 5, r6 = value * 6;
    int r7 = value * 7, r8 = value * 8, r9 = value * 9;
    int r10 = value * 10, r11 = value * 11, r12 = value * 12;
    
    /* Switch with many cases to create complex CFG */
    switch (value % 8) {
        case 0: r1 = r2 + r3; r4 = r5 + r6; break;
        case 1: r2 = r3 + r4; r5 = r6 + r7; break;
        case 2: r3 = r4 + r5; r6 = r7 + r8; break;
        case 3: r4 = r5 + r6; r7 = r8 + r9; break;
        case 4: r5 = r6 + r7; r8 = r9 + r10; break;
        case 5: r6 = r7 + r8; r9 = r10 + r11; break;
        case 6: r7 = r8 + r9; r10 = r11 + r12; break;
        case 7: r8 = r9 + r10; r11 = r12 + r1; break;
    }
    
    /* Loop with all registers live */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        int temp = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12;
        
        asm volatile (
            "/* Clobber all registers */"
            :
            : "r" (r1), "r" (r2), "r" (r3), "r" (r4), "r" (r5), "r" (r6),
              "r" (r7), "r" (r8), "r" (r9), "r" (r10), "r" (r11), "r" (r12),
              "r" (temp)
            : "memory", "cc"
        );
        
        sum += temp;
        r1++; r2++; r3++; r4++; r5++; r6++;
        r7++; r8++; r9++; r10++; r11++; r12++;
    }
    
    return sum;
}

/* Main function to call test functions with different parameters */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Call test functions multiple times with different inputs
     * to explore different register allocation scenarios */
    for (int i = 0; i < 10; i++) {
        result += test_ira_conflict(i + 1);
        result += test_ira_conflict2(i + 100);
        result += test_ira_conflict3(i + 1000);
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r" (result) : "memory");
    
    return result % 256; /* Return small value to avoid overflow */
}
