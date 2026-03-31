/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with different calling conventions */
int __attribute__((noinline)) helper1(int a, int b) {
    return (a ^ b) + 1;
}

int __attribute__((noinline)) helper2(int a, int b, int c) {
    return (a * b) - c;
}

float __attribute__((noinline)) helper3(float a, float b) {
    return a * 0.5f + b * 0.5f;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a * 2.0 - b;
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0x7);
}

/* x86-specific: force use of specific registers */
#ifdef __x86_64__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return (a + b) * c;
}
#endif

/* Force register pressure by preventing optimization */
static inline void use_var(int var) {
    asm volatile("" : "+r"(var));
}

static inline void use_var_f(float var) {
    asm volatile("" : "+r"(var));
}

static inline void use_var_d(double var) {
    asm volatile("" : "+r"(var));
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Declare many scalar variables of mixed types */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4, d5;
    long l1, l2, l3, l4;
    
    /* Initialize with non-constant values */
    srand(time(NULL));
    v1 = rand() % 100; v2 = rand() % 100; v3 = rand() % 100;
    v4 = rand() % 100; v5 = rand() % 100; v6 = rand() % 100;
    v7 = rand() % 100; v8 = rand() % 100; v9 = rand() % 100;
    v10 = rand() % 100; v11 = rand() % 100; v12 = rand() % 100;
    v13 = rand() % 100; v14 = rand() % 100; v15 = rand() % 100;
    v16 = rand() % 100; v17 = rand() % 100; v18 = rand() % 100;
    v19 = rand() % 100; v20 = rand() % 100;
    
    f1 = (float)rand() / RAND_MAX; f2 = (float)rand() / RAND_MAX;
    f3 = (float)rand() / RAND_MAX; f4 = (float)rand() / RAND_MAX;
    f5 = (float)rand() / RAND_MAX;
    
    d1 = (double)rand() / RAND_MAX; d2 = (double)rand() / RAND_MAX;
    d3 = (double)rand() / RAND_MAX; d4 = (double)rand() / RAND_MAX;
    d5 = (double)rand() / RAND_MAX;
    
    l1 = rand(); l2 = rand(); l3 = rand(); l4 = rand();
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 ^ v9;
        v10 = v8 | v11;
        v12 = v10 & v13;
        v14 = v12 * v15;
        v16 = v14 + v17;
        v18 = v16 - v19;
        v20 = v18 ^ v1;  /* Circular dependency */
        
        /* Float computations */
        f1 = f2 * 1.1f + f3;
        f4 = f1 - f5 * 0.9f;
        f3 = f4 * 2.0f;
        f2 = f3 + f1;
        f5 = f2 - f4;
        
        /* Double computations */
        d1 = d2 * 1.5 + d3;
        d4 = d1 - d5;
        d3 = d4 * 0.75;
        d2 = d3 + d1;
        d5 = d2 - d4;
        
        /* Long computations */
        l1 = l2 << 2;
        l3 = l1 | l4;
        l2 = l3 >> 1;
        l4 = l2 ^ l1;
        
        /* CRITICAL: Call helper1 with some variables, 
           but keep others live across the call */
        int ret1 = helper1(v1, v2);
        
        /* Immediately use return value with variables that were 
           live across the call (v3, v4, v5 in call-clobbered regs) */
        v3 = ret1 + v3 + v4;  /* v3, v4 were live across helper1 call */
        use_var(v5);  /* v5 was also live */
        
        /* Another call creating more pressure */
        float ret2 = helper3(f1, f2);
        
        /* Use return value with variables live across this call */
        f3 = ret2 * f3 + f4;  /* f3, f4 were live across helper3 call */
        use_var_f(f5);
        
        /* Mix of calls */
        double ret3 = helper4(d1, d2);
        d3 = ret3 * d3 - d4;  /* d3, d4 were live */
        use_var_d(d5);
        
        /* Call with more arguments */
        int ret4 = helper2(v6, v7, v8);
        v9 = ret4 * v9 + v10;  /* v9, v10 were live */
        
        /* Long operation */
        long ret5 = helper5(l1, l2);
        l3 = ret5 ^ l3 ^ l4;  /* l3, l4 were live */
        
        #ifdef __x86_64__
        /* x86-specific: regparm calling convention */
        int ret6 = helper_regparm(v11, v12, v13);
        v14 = ret6 + v14 + v15;  /* v14, v15 were live */
        #endif
        
        /* Create artificial dependencies for next iteration */
        v2 = v20 + i;
        v7 = v3 * i;
        v11 = v9 - i;
        v15 = v14 ^ i;
        v19 = v18 + i;
        
        f2 = f3 + (float)i * 0.1f;
        d2 = d3 + (double)i * 0.01;
        l2 = l3 + i;
        
        /* Force all variables to be used to prevent elimination */
        use_var(v1); use_var(v2); use_var(v3); use_var(v4); use_var(v5);
        use_var(v6); use_var(v7); use_var(v8); use_var(v9); use_var(v10);
        use_var(v11); use_var(v12); use_var(v13); use_var(v14); use_var(v15);
        use_var(v16); use_var(v17); use_var(v18); use_var(v19); use_var(v20);
        use_var_f(f1); use_var_f(f2); use_var_f(f3); use_var_f(f4); use_var_f(f5);
        use_var_d(d1); use_var_d(d2); use_var_d(d3); use_var_d(d4); use_var_d(d5);
        asm volatile("" : "+r"(l1), "+r"(l2), "+r"(l3), "+r"(l4));
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                   (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                   (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5 +
                   (int)l1 + (int)l2 + (int)l3 + (int)l4;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
