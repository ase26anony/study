/* test_mcf_coverage.c
 * 
 * This program is designed to trigger GCC's min-cost flow solver during
 * register allocation, specifically to exercise the dump_fixup_edge function
 * with the new_exit_index and new_entry_index special cases.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

/* Force many register conflicts by using many variables with overlapping live ranges */
#define FORCE_REGISTER_CONFLICT() \
    asm volatile ("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
                         "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15")

/* Function designed to create complex register pressure and overlapping live ranges */
int test_ira_conflict(int iterations) {
    /* Declare many integer variables that will have overlapping live ranges */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    int q = 17, r = 18, s = 19, t = 20, u = 21, v = 22, w = 23, x = 24;
    
    int result = 0;
    
    /* Nested loops to create complex control flow and overlapping live ranges */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across loop iterations */
        a = b + c;
        b = c + d;
        c = d + e;
        d = e + f;
        
        /* Inner loop with more variable usage */
        for (int inner = 0; inner < 10; inner++) {
            /* Complex expression with many live variables */
            e = f + g + h;
            f = g + h + i;
            g = h + i + j;
            h = i + j + k;
            
            /* More computations to increase register pressure */
            i = j + k + l + m;
            j = k + l + m + n;
            k = l + m + n + o;
            l = m + n + o + p;
            
            /* Use volatile asm to clobber many registers */
            FORCE_REGISTER_CONFLICT();
            
            /* Continue using variables to keep them live */
            m = n + o + p + q;
            n = o + p + q + r;
            o = p + q + r + s;
            p = q + r + s + t;
            
            /* More computations */
            q = r + s + t + u;
            r = s + t + u + v;
            s = t + u + v + w;
            t = u + v + w + x;
            
            /* Accumulate result using many variables */
            result += a + b + c + d + e + f + g + h + 
                     i + j + k + l + m + n + o + p +
                     q + r + s + t + u + v + w + x;
        }
        
        /* More computations between outer loop iterations */
        u = v + w + x;
        v = w + x + a;
        w = x + a + b;
        x = a + b + c;
        
        /* Another volatile asm to force register spills */
        FORCE_REGISTER_CONFLICT();
    }
    
    /* Final complex expression using all variables */
    return result + a + b + c + d + e + f + g + h + 
                  i + j + k + l + m + n + o + p +
                  q + r + s + t + u + v + w + x;
}

/* Second test function with different variable usage patterns */
int test_ira_conflict2(int seed) {
    /* Different set of variables */
    int v1 = seed, v2 = seed * 2, v3 = seed * 3, v4 = seed * 4;
    int v5 = seed * 5, v6 = seed * 6, v7 = seed * 7, v8 = seed * 8;
    int v9 = seed * 9, v10 = seed * 10, v11 = seed * 11, v12 = seed * 12;
    int v13 = seed * 13, v14 = seed * 14, v15 = seed * 15, v16 = seed * 16;
    
    /* Unrolled loop to create more register pressure */
    for (int i = 0; i < 100; i++) {
        /* Chain of dependencies to force specific allocation */
        v1 = v2 + v3;
        v2 = v3 + v4;
        v3 = v4 + v5;
        v4 = v5 + v6;
        v5 = v6 + v7;
        v6 = v7 + v8;
        v7 = v8 + v9;
        v8 = v9 + v10;
        v9 = v10 + v11;
        v10 = v11 + v12;
        v11 = v12 + v13;
        v12 = v13 + v14;
        v13 = v14 + v15;
        v14 = v15 + v16;
        v15 = v16 + v1;
        v16 = v1 + v2;
        
        /* Conditional to create control flow complexity */
        if (i % 3 == 0) {
            FORCE_REGISTER_CONFLICT();
            v1 = v16 + v15;
            v2 = v15 + v14;
        } else if (i % 3 == 1) {
            v3 = v14 + v13;
            v4 = v13 + v12;
            FORCE_REGISTER_CONFLICT();
        } else {
            v5 = v12 + v11;
            v6 = v11 + v10;
            v7 = v10 + v9;
            FORCE_REGISTER_CONFLICT();
        }
    }
    
    /* Return using all variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
           v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
}

/* Third test with array accesses to create memory pressure too */
int test_ira_conflict3(int size) {
    int arr[32];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 32; i++) {
        arr[i] = i * size;
    }
    
    /* Complex loop with array accesses and many temporaries */
    for (int i = 0; i < size; i++) {
        int t1 = arr[i % 32];
        int t2 = arr[(i + 1) % 32];
        int t3 = arr[(i + 2) % 32];
        int t4 = arr[(i + 3) % 32];
        int t5 = arr[(i + 4) % 32];
        int t6 = arr[(i + 5) % 32];
        int t7 = arr[(i + 6) % 32];
        int t8 = arr[(i + 7) % 32];
        
        /* Many computations with temporaries */
        for (int j = 0; j < 8; j++) {
            t1 = t2 + t3;
            t2 = t3 + t4;
            t3 = t4 + t5;
            t4 = t5 + t6;
            t5 = t6 + t7;
            t6 = t7 + t8;
            t7 = t8 + t1;
            t8 = t1 + t2;
            
            /* Force register clobbering */
            if (j % 4 == 0) {
                FORCE_REGISTER_CONFLICT();
            }
        }
        
        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
        
        /* Update array to create anti-dependencies */
        arr[i % 32] = t1;
    }
    
    return sum;
}

/* Main function to call all test cases with different parameters */
int main() {
    int total = 0;
    
    /* Call first test - creates many overlapping live ranges */
    total += test_ira_conflict(50);
    
    /* Call second test - different pattern */
    total += test_ira_conflict2(42);
    
    /* Call third test - array-based pattern */
    total += test_ira_conflict3(100);
    
    /* Return something based on all computations */
    return total % 256;
}
