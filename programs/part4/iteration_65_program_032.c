/* test-mcf-debug.c
 * Test program to trigger MCF_DEBUG output in GCC's min-cost flow solver.
 * Compile with: gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization from merging functions */
#define NOINLINE __attribute__((noinline))

/* Force O3 optimization specifically for the pressure function */
#define HIGH_PRESSURE NOINLINE __attribute__((optimize("O3")))

/* Memory barrier to force liveness across operations */
#define MEMORY_BARRIER asm volatile("" : : : "memory")

/* Create register pressure through volatile variables */
#define DECLARE_VOLATILES \
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9; \
    volatile int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;

/* Use all volatile variables in a complex expression to extend live ranges */
#define USE_ALL_VOLATILES(seed) \
    v0 = (seed) + 1; \
    v1 = v0 * 2; \
    v2 = v1 + v0; \
    v3 = v2 - v1; \
    v4 = v3 * v2; \
    v5 = v4 / (v3 ? v3 : 1); \
    v6 = v5 | v4; \
    v7 = v6 & v5; \
    v8 = v7 ^ v6; \
    v9 = v8 << 2; \
    v10 = v9 >> 1; \
    v11 = v10 + v9; \
    v12 = v11 - v10; \
    v13 = v12 * v11; \
    v14 = v13 % (v12 ? v12 : 1); \
    v15 = v14 | v13; \
    v16 = v15 & v14; \
    v17 = v16 ^ v15; \
    v18 = v17 << 1; \
    v19 = v18 >> 2; \
    MEMORY_BARRIER;

/* Function that creates extreme register pressure */
HIGH_PRESSURE
int create_register_pressure(int selector) {
    DECLARE_VOLATILES;
    int i, j, result = 0;
    
    /* Initialize all volatiles */
    v0 = selector;
    for (i = 1; i < 20; i++) {
        *(&v0 + i) = i * selector;  /* Simulate array-like access pattern */
    }
    MEMORY_BARRIER;
    
    /* Complex nested loops to create many live ranges */
    for (i = 0; i < 4; i++) {  /* Will likely unroll at O3 */
        /* Switch creates complex control flow graph */
        switch ((selector + i) % 5) {
            case 0:
                /* Chain of dependent operations */
                v1 = v2 + v3;
                v4 = v1 * v5;
                v6 = v4 - v7;
                v8 = v6 / (v9 ? v9 : 1);
                result += v8;
                MEMORY_BARRIER;
                break;
                
            case 1:
                /* Different chain */
                v10 = v11 * v12;
                v13 = v10 + v14;
                v15 = v13 - v16;
                v17 = v15 & v18;
                result += v17;
                MEMORY_BARRIER;
                break;
                
            case 2:
                /* Use all variables in complex expression */
                USE_ALL_VOLATILES(i);
                result += v19;
                MEMORY_BARRIER;
                break;
                
            case 3:
                /* More arithmetic chains */
                for (j = 0; j < 3; j++) {  /* Inner loop */
                    v0 = v1 + v2;
                    v3 = v0 * v4;
                    v5 = v3 + v6;
                    v7 = v5 - v8;
                    result += v7 * j;
                }
                MEMORY_BARRIER;
                break;
                
            case 4:
                /* Conditional operations */
                if (v0 > v1) {
                    v2 = v3 * v4;
                    v5 = v6 + v7;
                } else {
                    v8 = v9 - v10;
                    v11 = v12 * v13;
                }
                v14 = v15 | v16;
                v17 = v18 ^ v19;
                result += v14 + v17;
                MEMORY_BARRIER;
                break;
        }
        
        /* Cross-iteration dependencies */
        if (i > 0) {
            v0 = result + v19;
            v1 = v0 * 2;
            v2 = v1 - result;
        }
        
        /* Additional memory barrier to force spills */
        MEMORY_BARRIER;
    }
    
    /* Final computation using all variables */
    int temp1 = v0 + v1 + v2 + v3 + v4;
    int temp2 = v5 + v6 + v7 + v8 + v9;
    int temp3 = v10 + v11 + v12 + v13 + v14;
    int temp4 = v15 + v16 + v17 + v18 + v19;
    
    result = temp1 * temp2 + temp3 - temp4;
    
    /* Force one more barrier */
    MEMORY_BARRIER;
    
    return result;
}

/* Secondary pressure function with different pattern */
NOINLINE
int secondary_pressure(int x) {
    volatile int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
    volatile int b0, b1, b2, b3, b4, b5, b6, b7, b8, b9;
    
    /* Initialize */
    a0 = x; b0 = x + 1;
    a1 = a0 * 2; b1 = b0 * 3;
    a2 = a1 + b1; b2 = a1 - b1;
    a3 = a2 * b2; b3 = a2 / (b2 ? b2 : 1);
    a4 = a3 | b3; b4 = a3 & b3;
    a5 = a4 ^ b4; b5 = a4 << 1;
    a6 = b5 >> 1; b6 = a5 + b5;
    a7 = a6 - b6; b7 = a6 * b6;
    a8 = a7 % 17; b8 = b7 % 23;
    a9 = a8 | b8; b9 = a8 & b8;
    
    MEMORY_BARRIER;
    
    /* Complex conditional */
    int result = 0;
    for (int i = 0; i < 8; i++) {
        if (i & 1) {
            result += a0 + a2 + a4 + a6 + a8;
        } else {
            result += b1 + b3 + b5 + b7 + b9;
        }
        
        /* Rotate values */
        int tmp = a0;
        a0 = a1; a1 = a2; a2 = a3; a3 = a4; a4 = a5;
        a5 = a6; a6 = a7; a7 = a8; a8 = a9; a9 = tmp;
        
        tmp = b0;
        b0 = b1; b1 = b2; b2 = b3; b3 = b4; b4 = b5;
        b5 = b6; b6 = b7; b7 = b8; b8 = b9; b9 = tmp;
        
        MEMORY_BARRIER;
    }
    
    return result;
}

/* Main function to ensure everything gets compiled */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Call pressure functions with different inputs */
    result += create_register_pressure(argc);
    result += secondary_pressure(argc + 1);
    
    /* Additional calls with different parameters */
    for (int i = 0; i < 3; i++) {
        result += create_register_pressure(i);
        result += secondary_pressure(i * 2);
    }
    
    return result % 256;  /* Prevent elimination of computation */
}
