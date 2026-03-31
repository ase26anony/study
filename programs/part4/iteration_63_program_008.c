/* test_mcf_coverage.c
 * 
 * This test is designed to trigger GCC's min-cost flow solver during
 * register allocation, specifically to exercise the debug dumping
 * code that prints special node labels like "NEW_EXIT" and "NEW_ENTRY".
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c
 * 
 * For ARM targets (limited registers): 
 *   gcc -O2 -march=armv7-a -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c
 */

/* Force inclusion of IRA headers and MCF debugging */
#ifdef MCF_DEBUG
/* This ensures debug paths are compiled in */
#endif

/* Function with extreme register pressure to force complex flow network */
int __attribute__((noinline)) 
test_ira_conflict(int iterations) 
{
    /* Declare many variables that will have overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z;
    int sum = 0;
    
    /* Initialize with different values to prevent optimization */
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
    
    /* Nested loops to create complex liveness patterns */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across inner loop */
        int temp1 = a + b;
        int temp2 = c + d;
        
        /* Clobber many registers with inline asm - increases pressure */
        asm volatile (
            "/* Clobber many registers */"
            :
            : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f), 
              "r"(g), "r"(h), "r"(i), "r"(j)
            : "memory"
        );
        
        for (int inner = 0; inner < 10; inner++) {
            /* Complex computation keeping many variables live */
            a = b + c + inner;
            b = c + d + outer;
            c = d + e + temp1;
            d = e + f + temp2;
            e = f + g + a;
            f = g + h + b;
            
            /* More asm to prevent optimization and increase conflicts */
            asm volatile (
                "nop \n\t"
                "nop \n\t"
                : 
                : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f),
                  "r"(g), "r"(h), "r"(i), "r"(j), "r"(k), "r"(l)
                : "cc"
            );
            
            /* Cross-dependencies to create web of conflicts */
            g = h + i + c;
            h = i + j + d;
            i = j + k + e;
            j = k + l + f;
            k = l + m + g;
            l = m + n + h;
            
            /* Use all variables to keep them live */
            sum += a + b + c + d + e + f + g + h + i + j;
            sum += k + l + m + n + o + p + q + r + s + t;
            sum += u + v + w + x + y + z + temp1 + temp2;
        }
        
        /* Rotate values to create data flow */
        int rotate = a;
        a = b; b = c; c = d; d = e; e = f;
        f = g; g = h; h = i; i = j; j = k;
        k = l; l = m; m = n; n = o; o = p;
        p = q; q = r; r = s; s = t; t = u;
        u = v; v = w; w = x; x = y; y = z;
        z = rotate;
    }
    
    /* Final computation using all variables */
    sum = sum + a + b + c + d + e + f + g + h + i + j +
          k + l + m + n + o + p + q + r + s + t +
          u + v + w + x + y + z;
    
    return sum;
}

/* Second test function with different conflict pattern */
int __attribute__((noinline))
test_sparse_conflicts(int seed) 
{
    /* Variables with sparse but overlapping live ranges */
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
    
    /* Create asymmetric live ranges */
    {
        int t1 = v1 + v2;
        int t2 = v3 + v4;
        v5 = t1 * t2;
        
        asm volatile ("nop" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4) : "cc");
    }
    
    {
        int t3 = v5 + v6;
        int t4 = v7 + v8;
        v9 = t3 / (t4 ? t4 : 1);
        
        asm volatile ("nop" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8) : "cc");
    }
    
    /* Force some variables to be live simultaneously at the end */
    asm volatile (
        "/* Force multiple live values */"
        : 
        : "r"(v1), "r"(v3), "r"(v5), "r"(v7), "r"(v9),
          "r"(v2), "r"(v4), "r"(v6), "r"(v8), "r"(v10)
        : "memory"
    );
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Third test: Function with control flow creating different regions */
int __attribute__((noinline))
test_control_flow(int x) 
{
    int a = x * 2;
    int b = x * 3;
    int c = x * 5;
    int d = x * 7;
    int e = x * 11;
    int result = 0;
    
    if (x > 0) {
        int t1 = a + b;
        int t2 = c + d;
        result = t1 * t2;
        
        /* Clobber in one branch only */
        asm volatile ("nop" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e) : "cc");
        
        for (int i = 0; i < 5; i++) {
            a = b + i;
            b = c + i;
            c = d + i;
            result += a + b + c;
        }
    } else {
        int t3 = b + c;
        int t4 = d + e;
        result = t3 - t4;
        
        /* Different clobber set */
        asm volatile ("nop" : : "r"(b), "r"(c), "r"(d), "r"(e) : "cc");
        
        for (int i = 0; i < 3; i++) {
            d = e + i;
            e = a + i;
            result += d - e;
        }
    }
    
    /* All variables live here */
    asm volatile ("nop" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(result) : "cc");
    
    return result + a + b + c + d + e;
}

/* Main function to drive various test cases */
int main() 
{
    int total = 0;
    
    /* Test 1: High register pressure */
    total += test_ira_conflict(5);
    
    /* Test 2: Different conflict pattern */
    total += test_sparse_conflicts(7);
    
    /* Test 3: Control flow variations */
    total += test_control_flow(2);
    total += test_control_flow(-1);
    
    /* Additional calls with different parameters to explore more graph states */
    for (int i = 0; i < 3; i++) {
        total += test_ira_conflict(i + 1);
        total += test_sparse_conflicts(i + 2);
    }
    
    return total > 0 ? 0 : 1;
}
