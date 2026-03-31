/* caller_save_test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with mixed calling conventions */
int __attribute__((noinline)) helper1(int a, int b) { return a + b; }
int __attribute__((noinline)) helper2(int a, int b) { return a - b; }
float __attribute__((noinline)) helper3(float a, float b) { return a * b; }
double __attribute__((noinline)) helper4(double a, double b) { return a / b; }
long __attribute__((noinline)) helper5(long a, long b) { return a & b; }

/* x86-specific: force use of specific registers */
#ifdef __x86_64__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b - c;
}
#endif

/* Force register pressure by preventing optimization */
static void use_var(int v) {
    asm volatile("" : "+r"(v));
}

int main(int argc, char *argv[]) {
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    srand(time(NULL));
    
    /* Declare many scalar variables of mixed types */
    int v1 = rand() % 100;
    int v2 = rand() % 100;
    int v3 = rand() % 100;
    int v4 = rand() % 100;
    int v5 = rand() % 100;
    int v6 = rand() % 100;
    int v7 = rand() % 100;
    int v8 = rand() % 100;
    int v9 = rand() % 100;
    int v10 = rand() % 100;
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    float f4 = (float)rand() / RAND_MAX;
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    long l1 = rand() % 100;
    long l2 = rand() % 100;
    long l3 = rand() % 100;
    
    /* Create artificial register pressure */
    volatile int vol1 = v1;
    volatile float volf1 = f1;
    
    for (int i = 0; i < N; i++) {
        /* Complex web of dependencies - all values live simultaneously */
        v1 = v2 + v3;
        v2 = v4 - v5;
        v3 = v6 * v7;
        v4 = v8 / (v9 + 1);
        v5 = v10 ^ v1;
        v6 = v2 | v3;
        v7 = v4 & v5;
        v8 = v6 + v7;
        v9 = v8 - v1;
        v10 = v9 * v2;
        
        f1 = f2 + f3;
        f2 = f4 * f1;
        f3 = f1 - f4;
        f4 = f2 / (f3 + 1.0f);
        
        d1 = d2 * d3;
        d2 = d1 / (d3 + 1.0);
        d3 = d2 - d1;
        
        l1 = l2 | l3;
        l2 = l1 & v1;
        l3 = l2 ^ v2;
        
        /* CRITICAL: Function call with some live variables as arguments,
           leaving others live in call-clobbered registers */
        int ret1 = helper1(v1, v2);
        
        /* Immediately use return value with variables that were live
           across the call (in call-clobbered registers) */
        v3 = ret1 + v3;  /* v3 was live across helper1 call */
        v4 = v4 * ret1;  /* v4 was live across helper1 call */
        
        /* Force register use to prevent spilling to memory too early */
        use_var(v3);
        use_var(v4);
        
        /* Another function call creating another insertion point */
        float ret2 = helper3(f1, f2);
        
        /* Use return value with variables live across this call */
        f3 = ret2 + f3;  /* f3 was live across helper3 call */
        f4 = f4 - ret2;  /* f4 was live across helper3 call */
        
        /* More calls with different types */
        double ret3 = helper4(d1, d2);
        d3 = d3 * ret3;
        
        long ret4 = helper5(l1, l2);
        l3 = l3 | ret4;
        
        #ifdef __x86_64__
        /* x86-specific: force use of regparm calling convention */
        int ret5 = helper_regparm(v5, v6, v7);
        v8 = v8 + ret5;
        #endif
        
        /* Create loop-carried dependencies */
        v1 = v1 + i;
        v2 = v2 - i;
        f1 = f1 + (float)i;
        d1 = d1 + (double)i;
        l1 = l1 + i;
        
        /* Artificial uses to prevent optimization */
        asm volatile("" : "+r"(v9), "+r"(v10));
        asm volatile("" : "+r"(f2), "+r"(f3));
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (int)f1 + (int)f2 + (int)f3 + (int)f4;
    checksum += (int)d1 + (int)d2 + (int)d3;
    checksum += (int)l1 + (int)l2 + (int)l3;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
