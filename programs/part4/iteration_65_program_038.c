/* test_mcf_coverage.c
 * Designed to trigger debug dumps in GCC's min-cost flow solver
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization from removing the function */
#define NOINLINE __attribute__((noinline))

/* Force O3 optimization specifically for the high-pressure function */
#define HIGH_PRESSURE NOINLINE __attribute__((optimize("O3")))

/* Memory barrier to force variables to be live across it */
#define MEMORY_BARRIER asm volatile("" : : : "memory")

/* Create register clobbering for x86 (portable fallback) */
#if defined(__i386__) || defined(__x86_64__)
#define CLOBBER_REGS asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi")
#else
#define CLOBBER_REGS asm volatile("" : : : "memory")
#endif

/* High register pressure function */
HIGH_PRESSURE
static int create_register_pressure(int seed) {
    /* Declare many volatile variables to prevent optimization */
    volatile int v0 = seed + 1;
    volatile int v1 = seed + 2;
    volatile int v2 = seed + 3;
    volatile int v3 = seed + 4;
    volatile int v4 = seed + 5;
    volatile int v5 = seed + 6;
    volatile int v6 = seed + 7;
    volatile int v7 = seed + 8;
    volatile int v8 = seed + 9;
    volatile int v9 = seed + 10;
    volatile int v10 = seed + 11;
    volatile int v11 = seed + 12;
    volatile int v12 = seed + 13;
    volatile int v13 = seed + 14;
    volatile int v14 = seed + 15;
    volatile int v15 = seed + 16;
    volatile int v16 = seed + 17;
    volatile int v17 = seed + 18;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Complex control flow with switch */
    int result = 0;
    switch (seed % 7) {
        case 0:
            v0 = v1 + v2;
            v3 = v4 * v5;
            v6 = v7 - v8;
            result = v0 + v3 + v6;
            break;
        case 1:
            v9 = v10 / (v11 + 1);
            v12 = v13 | v14;
            v15 = v16 & v17;
            result = v9 ^ v12 ^ v15;
            break;
        case 2:
            v1 = v2 << v3;
            v4 = v5 >> v6;
            v7 = v8 ^ v9;
            result = v1 | v4 | v7;
            break;
        case 3:
            v10 = v11 * v12;
            v13 = v14 + v15;
            v16 = v17 - v0;
            result = v10 + v13 + v16;
            break;
        case 4:
            v2 = v3 & v4;
            v5 = v6 | v7;
            v8 = v9 ^ v10;
            result = v2 + v5 + v8;
            break;
        case 5:
            v11 = v12 * v13;
            v14 = v15 / (v16 + 1);
            v17 = v0 - v1;
            result = v11 + v14 + v17;
            break;
        default:
            v3 = v4 + v5;
            v6 = v7 * v8;
            v9 = v10 - v11;
            result = v3 + v6 + v9;
            break;
    }
    
    MEMORY_BARRIER;
    
    /* Nested loops to extend live ranges and encourage unrolling */
    for (int i = 0; i < 4; i++) {
        /* Chain dependencies to create data flow */
        t0 = v0 + v1;
        t1 = v2 * v3;
        t2 = v4 - v5;
        t3 = v6 | v7;
        t4 = v8 & v9;
        
        /* More operations with dependencies */
        v10 = t0 + t1;
        v11 = t2 * t3;
        v12 = t4 ^ v10;
        v13 = v11 - v12;
        
        /* Conditional inside loop */
        if (i % 2 == 0) {
            v14 = v15 + v16;
            v17 = v13 * v14;
            result += v17;
        } else {
            v15 = v16 - v17;
            v0 = v13 / (v15 + 1);
            result -= v0;
        }
        
        CLOBBER_REGS;
        
        /* Another memory barrier */
        MEMORY_BARRIER;
        
        /* Inner loop with small iteration count */
        for (int j = 0; j < 2; j++) {
            /* More variable mixing */
            t0 = v1 + v2 + j;
            t1 = v3 * v4 * (j + 1);
            v5 = t0 - t1;
            v6 = v5 + v7;
            v8 = v6 * v9;
            
            result += v8 >> (j + 1);
        }
    }
    
    /* Final complex expression using all variables */
    result = ((v0 + v1) * (v2 - v3)) |
             ((v4 & v5) ^ (v6 | v7)) +
             ((v8 * v9) / (v10 + 1)) -
             ((v11 << 2) | (v12 >> 1)) +
             ((v13 + v14) * (v15 - v16)) ^ v17;
    
    return result;
}

/* Another high-pressure function with different pattern */
NOINLINE __attribute__((optimize("O2")))
static int more_pressure(int x) {
    volatile int a = x * 2;
    volatile int b = x + 3;
    volatile int c = x - 4;
    volatile int d = x / 2;
    volatile int e = x % 7;
    volatile int f = x << 1;
    volatile int g = x >> 2;
    volatile int h = x ^ 0xFF;
    volatile int i = x | 0xAA;
    volatile int j = x & 0x55;
    
    int sum = 0;
    
    /* Loop with if-else chain */
    for (int k = 0; k < 8; k++) {
        if (k % 3 == 0) {
            a = b + c;
            d = e * f;
            sum += a + d;
        } else if (k % 3 == 1) {
            g = h - i;
            j = a * b;
            sum += g - j;
        } else {
            c = d | e;
            f = g & h;
            sum += c ^ f;
        }
        
        MEMORY_BARRIER;
        
        /* Nested switch */
        switch (k % 4) {
            case 0: a = b + 1; break;
            case 1: c = d - 2; break;
            case 2: e = f * 3; break;
            case 3: g = h / 4; break;
        }
    }
    
    return sum;
}

/* Main function to ensure everything gets compiled */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Call high-pressure functions with different seeds */
    for (int i = 0; i < 10; i++) {
        result ^= create_register_pressure(i);
        result += more_pressure(i);
    }
    
    /* Use result to prevent dead code elimination */
    volatile int sink = result;
    
    return sink != 0;
}
