#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural optimization */
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int iter_count) {
    volatile int result = 0;
    
    for (volatile int i = 0; i < iter_count; i++) {
        /* Declare many local variables to create register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        int v31, v32, v33, v34, v35, v36, v37, v38, v39, v40;
        
        /* Complex expression that will be reused - candidate for rematerialization */
        int complex_expr = (a * b) + (c << 2) - (d / 3);
        
        /* First block of independent computations */
        v1 = a + b;
        v2 = b * c;
        v3 = c - d;
        v4 = d ^ a;
        v5 = (a << 3) | b;
        v6 = complex_expr;  /* First use of complex expression */
        v7 = v1 * v2;
        v8 = v3 + v4;
        v9 = v5 ^ v6;
        v10 = v7 - v8;
        
        /* Compiler barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        
        /* Second block with more computations */
        v11 = b + c;
        v12 = c * d;
        v13 = d - a;
        v14 = a ^ b;
        v15 = (b << 2) | c;
        v16 = complex_expr;  /* Second use - candidate for rematerialization */
        v17 = v11 * v12;
        v18 = v13 + v14;
        v19 = v15 ^ v16;
        v20 = v17 - v18;
        
        /* Another compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Third block */
        v21 = c + d;
        v22 = d * a;
        v23 = a - b;
        v24 = b ^ c;
        v25 = (c << 1) | d;
        v26 = complex_expr;  /* Third use */
        v27 = v21 * v22;
        v28 = v23 + v24;
        v29 = v25 ^ v26;
        v30 = v27 - v28;
        
        /* Control flow split based on volatile condition */
        volatile int condition = a & 1;
        if (condition) {
            /* Branch with more computations */
            v31 = v1 + v11;
            v32 = v2 * v12;
            v33 = v3 - v13;
            v34 = v4 ^ v14;
            v35 = complex_expr;  /* Fourth use in branch */
            v36 = v31 * v32;
            v37 = v33 + v34;
            v38 = v35 ^ v6;      /* Mix with earlier use */
            v39 = v36 - v37;
            v40 = v38 | v39;
            
            result += v40;
        } else {
            /* Alternative branch with different computations */
            v31 = v6 + v16;
            v32 = v7 * v17;
            v33 = v8 - v18;
            v34 = v9 ^ v19;
            v35 = complex_expr;  /* Fifth use in else branch */
            v36 = v31 * v32;
            v37 = v33 + v34;
            v38 = v35 ^ v26;     /* Mix with third use */
            v39 = v36 - v37;
            v40 = v38 & v39;
            
            result += v40;
        }
        
        /* More computations after the conditional */
        int t1 = v10 + v20;
        int t2 = v20 * v30;
        int t3 = v30 - v10;
        int t4 = complex_expr;  /* Sixth use */
        int t5 = t1 * t2;
        int t6 = t3 + t4;
        int t7 = t5 ^ t6;
        
        result += t7;
        
        /* Modify inputs slightly for next iteration */
        a ^= i;
        b += 1;
        c -= 1;
        d ^= 0x55;
        
        /* Final compiler barrier in loop */
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    /* Use volatile inputs to prevent constant propagation */
    volatile int a = rand() % 100 + 1;
    volatile int b = rand() % 100 + 1;
    volatile int c = rand() % 100 + 1;
    volatile int d = rand() % 100 + 1;
    volatile int iterations = 10;  /* Small enough to run, large enough for pressure */
    
    printf("Starting computation with a=%d, b=%d, c=%d, d=%d, iter=%d\n",
           a, b, c, d, iterations);
    
    int result = high_pressure_compute(a, b, c, d, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}
