/* test_mcf_coverage.c
 * 
 * This test is designed to trigger GCC's min-cost flow solver during
 * register allocation, specifically to exercise the debug dumping
 * code that prints special node labels like "NEW_EXIT".
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c
 */

/* Force inclusion of the MCF debugging code */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

/* Function with complex register pressure to trigger fixup graph creation */
__attribute__((noinline))
static int test_ira_conflict(int iterations) {
    /* Create many overlapping live ranges */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int k = 11, l = 12, m = 13, n = 14, o = 15;
    int p = 16, q = 17, r = 18, s = 19, t = 20;
    
    volatile int result = 0;
    
    /* Nested loops to create complex liveness patterns */
    for (int outer = 0; outer < iterations; outer++) {
        /* All variables are live at the start of the loop */
        a += b;
        b += c;
        c += d;
        d += e;
        e += f;
        
        /* Inner loop with more live variables */
        for (int inner = 0; inner < 10; inner++) {
            /* Force many registers to be live simultaneously */
            f += g;
            g += h;
            h += i;
            i += j;
            j += k;
            
            /* Use volatile asm to clobber registers */
            asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
                               "r"(f), "r"(g), "r"(h), "r"(i), "r"(j));
            
            k += l;
            l += m;
            m += n;
            n += o;
            o += p;
            
            /* More register pressure */
            asm volatile ("" : : "r"(k), "r"(l), "r"(m), "r"(n), "r"(o),
                               "r"(p), "r"(q), "r"(r), "r"(s), "r"(t));
            
            p += q;
            q += r;
            r += s;
            s += t;
            t += a;  /* Circular dependency */
        }
        
        /* Mix all variables together */
        result += a + b + c + d + e + f + g + h + i + j +
                  k + l + m + n + o + p + q + r + s + t;
        
        /* Force spilling by using many temporaries */
        int t1 = a * b;
        int t2 = c * d;
        int t3 = e * f;
        int t4 = g * h;
        int t5 = i * j;
        int t6 = k * l;
        int t7 = m * n;
        int t8 = o * p;
        int t9 = q * r;
        int t10 = s * t;
        
        result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
        
        /* Another asm that clobbers many registers */
        asm volatile (""
            : 
            : "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5),
              "r"(t6), "r"(t7), "r"(t8), "r"(t9), "r"(t10)
            : "memory");
    }
    
    return result;
}

/* Second test function with different register pressure pattern */
__attribute__((noinline))
static int test_ira_conflict2(int seed) {
    /* Create variables with different lifetimes */
    int v1 = seed;
    int v2 = v1 * 2;
    int v3 = v2 * 3;
    int v4 = v3 * 4;
    int v5 = v4 * 5;
    int v6 = v5 * 6;
    int v7 = v6 * 7;
    int v8 = v7 * 8;
    int v9 = v8 * 9;
    int v10 = v9 * 10;
    int v11 = v10 * 11;
    int v12 = v11 * 12;
    int v13 = v12 * 13;
    int v14 = v13 * 14;
    int v15 = v14 * 15;
    
    /* Conditional blocks create different control flow paths */
    if (seed & 1) {
        v1 = v15;
        v2 = v14;
        v3 = v13;
        
        /* Force many live ranges in this path */
        asm volatile ("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
                           "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10));
    } else {
        v11 = v1;
        v12 = v2;
        v13 = v3;
        
        /* Different set of live registers */
        asm volatile ("" : : "r"(v11), "r"(v12), "r"(v13), "r"(v14), "r"(v15));
    }
    
    /* All variables come together at the end */
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
               v11 + v12 + v13 + v14 + v15;
        
        /* Rotate values to create data dependencies */
        int temp = v1;
        v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6;
        v6 = v7; v7 = v8; v8 = v9; v9 = v10; v10 = v11;
        v11 = v12; v12 = v13; v13 = v14; v14 = v15; v15 = temp;
    }
    
    return sum;
}

/* Third test: Function with array accesses that force register spilling */
__attribute__((noinline))
static int test_ira_conflict3(int size) {
    int arr[20];
    int sum = 0;
    
    /* Initialize array with values */
    for (int i = 0; i < 20; i++) {
        arr[i] = i * size;
    }
    
    /* Complex computation with many array elements live */
    for (int i = 0; i < size; i++) {
        /* Load many array elements into "registers" (compiler will use pseudos) */
        int a0 = arr[0] + i;
        int a1 = arr[1] * i;
        int a2 = arr[2] - i;
        int a3 = arr[3] / (i + 1);
        int a4 = arr[4] ^ i;
        int a5 = arr[5] | i;
        int a6 = arr[6] & i;
        int a7 = arr[7] << (i & 3);
        int a8 = arr[8] >> (i & 3);
        int a9 = arr[9] + a0;
        
        /* Use them all in a computation */
        sum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
        
        /* Update array elements creating anti-dependencies */
        arr[0] = a9;
        arr[1] = a8;
        arr[2] = a7;
        arr[3] = a6;
        arr[4] = a5;
        
        /* Clobber to force register reloading */
        asm volatile ("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4),
                           "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9));
    }
    
    return sum;
}

/* Main function that exercises all test cases */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Call test functions with different parameters to explore
     * different register allocation scenarios */
    result += test_ira_conflict(100);
    result += test_ira_conflict2(42);
    result += test_ira_conflict3(50);
    
    /* Additional calls with different parameters */
    if (argc > 1) {
        result += test_ira_conflict(atoi(argv[1]));
    }
    
    return result & 0xFF;  /* Return non-zero but bounded value */
}
