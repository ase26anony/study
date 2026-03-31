/* test_mcf_coverage.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization from merging/simplifying */
#define NOINLINE __attribute__((noinline))
#define OPT_O3 __attribute__((optimize("O3")))

/* Memory barrier to force variables to be live across it */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Function to create extreme register pressure */
NOINLINE OPT_O3
static int high_pressure_function(int seed) {
    /* Declare many volatile variables to prevent optimization */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Initialize with seed to create data dependencies */
    v0 = seed;
    v1 = seed + 1;
    v2 = seed * 2;
    v3 = seed - 1;
    v4 = seed ^ 0x55;
    v5 = seed | 0xAA;
    v6 = seed & 0xFF;
    v7 = ~seed;
    v8 = seed << 2;
    v9 = seed >> 1;
    v10 = seed + 100;
    v11 = seed * 3;
    v12 = seed / 2;
    v13 = seed % 7;
    v14 = seed + 200;
    v15 = seed * 5;
    v16 = seed - 50;
    v17 = seed ^ 0xFF;
    
    MEMORY_BARRIER();
    
    /* Complex nested loops to extend live ranges */
    for (int i = 0; i < 4; i++) {  /* Small constant for potential unrolling */
        /* Switch with multiple cases for complex CFG */
        switch ((seed + i) % 5) {
            case 0:
                /* Chain of dependent operations */
                t0 = v0 + v1;
                t1 = v2 * v3;
                t2 = v4 ^ v5;
                v6 = t0 + t1;
                v7 = t1 - t2;
                v8 = v6 * v7;
                v9 = v8 >> 2;
                v10 = v9 + v0;
                break;
                
            case 1:
                /* Different chain */
                t0 = v11 & v12;
                t1 = v13 | v14;
                t2 = v15 ^ v16;
                v0 = t0 + t1;
                v1 = t1 - t2;
                v2 = v0 * v1;
                v3 = v2 << 1;
                v4 = v3 + v17;
                break;
                
            case 2:
                /* More operations mixing variables */
                t0 = v5 + v6;
                t1 = v7 * v8;
                t2 = v9 ^ v10;
                t3 = v11 & v12;
                v13 = t0 + t1 + t2;
                v14 = t1 - t2 + t3;
                v15 = v13 * v14;
                v16 = v15 >> 3;
                break;
                
            case 3:
                /* Complex expression tree */
                t0 = (v0 * v1) + (v2 * v3);
                t1 = (v4 | v5) ^ (v6 & v7);
                t2 = (v8 << 2) + (v9 >> 1);
                t3 = (v10 % 3) * (v11 % 5);
                t4 = t0 + t1 + t2 + t3;
                v12 = t4;
                v13 = t4 * 2;
                v14 = t4 / 2;
                break;
                
            case 4:
                /* All variables used together */
                t0 = v0 + v1 + v2 + v3 + v4;
                t1 = v5 * v6 * v7 * v8;
                t2 = v9 ^ v10 ^ v11 ^ v12;
                t3 = v13 | v14 | v15 | v16;
                v17 = t0 + t1 + t2 + t3;
                v0 = v17 >> 1;
                v1 = v17 << 1;
                break;
        }
        
        MEMORY_BARRIER();
        
        /* Inner conditional to create more CFG edges */
        if (i & 1) {
            t0 = v0 + v2 + v4 + v6 + v8;
            v1 = t0 * i;
            v3 = t0 / (i + 1);
        } else {
            t0 = v1 + v3 + v5 + v7 + v9;
            v0 = t0 * i;
            v2 = t0 / (i + 1);
        }
        
        /* Another memory barrier */
        MEMORY_BARRIER();
        
        /* Cross-variable updates to maintain liveness */
        v10 = v0 + v1;
        v11 = v2 + v3;
        v12 = v4 + v5;
        v13 = v6 + v7;
        v14 = v8 + v9;
        v15 = v10 + v11;
        v16 = v12 + v13;
        v17 = v14 + v15 + v16;
    }
    
    /* Final complex expression using all variables */
    t0 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    t1 = v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    t2 = v16 + v17;
    
    MEMORY_BARRIER();
    
    /* Return value depending on all computations */
    return t0 + t1 + t2 + seed;
}

/* Second function with different pattern to increase overall complexity */
NOINLINE OPT_O3
static int another_pressure_function(int x) {
    volatile int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r;
    
    a = x;
    b = x * 2;
    c = x + 1;
    d = x - 1;
    e = x ^ 0x33;
    f = x | 0xCC;
    g = x & 0x0F;
    h = ~x;
    i = x << 3;
    j = x >> 2;
    k = x + 1000;
    l = x * 7;
    m = x / 3;
    n = x % 11;
    o = x + 500;
    p = x * 11;
    q = x - 100;
    r = x ^ 0xAA;
    
    MEMORY_BARRIER();
    
    /* Unrolled loop */
    for (int iter = 0; iter < 3; iter++) {
        /* Multiple conditionals in sequence */
        if (iter == 0) {
            a = b + c;
            d = e * f;
            g = h ^ i;
        } else if (iter == 1) {
            j = k + l;
            m = n * o;
            p = q ^ r;
        } else {
            a = a + j;
            d = d + m;
            g = g + p;
        }
        
        MEMORY_BARRIER();
        
        /* Small switch */
        switch ((x + iter) % 3) {
            case 0: b = c + d; e = f * g; break;
            case 1: h = i + j; k = l * m; break;
            case 2: n = o + p; q = r * a; break;
        }
    }
    
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + x;
}

/* Main function to ensure compilation */
int main(void) {
    int result = 0;
    
    /* Call pressure functions multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        result ^= high_pressure_function(i);
        result ^= another_pressure_function(i * 7);
    }
    
    return result & 0xFF;  /* Return non-zero to prevent dead code elimination */
}
