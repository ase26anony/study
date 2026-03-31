/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with different calling conventions */
int __attribute__((noinline)) helper1(int a, int b) { return a ^ b; }
int __attribute__((noinline)) helper2(int a, int b) { return a + b; }
float __attribute__((noinline)) helper3(float a, float b) { return a * b; }
double __attribute__((noinline)) helper4(double a, double b) { return a - b; }
long __attribute__((noinline)) helper5(long a, long b) { return a | b; }

/* x86-specific: force use of specific registers */
#ifdef __x86_64__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b - c;
}
#endif

/* Prevent optimization of critical variables */
#define PRESSURE(var) asm volatile("" : "+r"(var))

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
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    double d3 = (double)rand() / RAND_MAX;
    long l1 = rand() * rand();
    long l2 = rand() * rand();
    long l3 = rand() * rand();
    long l4 = rand() * rand();
    
    /* Create artificial register pressure */
    volatile int vol1 = v1;
    volatile int vol2 = v2;
    
    for (int i = 0; i < N; i++) {
        /* Complex web of dependencies - all values live simultaneously */
        v1 = v2 + v3;
        v2 = v1 ^ v4;
        v3 = v2 * v5;
        v4 = v3 | v6;
        v5 = v4 - v7;
        v6 = v5 + v8;
        v7 = v6 ^ v9;
        v8 = v7 * v10;
        v9 = v8 | v1;
        v10 = v9 - v2;
        
        f1 = f2 * f3;
        f2 = f1 + 1.0f;
        f3 = f2 / 2.0f;
        
        d1 = d2 - d3;
        d2 = d1 * 1.5;
        d3 = d2 + 0.25;
        
        l1 = l2 | l3;
        l2 = l1 ^ l4;
        l3 = l2 & l1;
        l4 = l3 | 0xFF;
        
        /* CRITICAL: Function call followed immediately by use of live values
           that were NOT passed to the function */
        int ret1 = helper1(v1, v2);
        
        /* v3, v4, v5 are live across the call in call-clobbered registers */
        v3 = ret1 + v3;  /* Uses v3 which was live across helper1 call */
        PRESSURE(v3);    /* Force v3 to stay in register */
        
        /* Another call creating another insertion point */
        float ret2 = helper3(f1, f2);
        
        /* f3 is live across this call */
        f3 = ret2 * f3;  /* Uses f3 which was live across helper3 call */
        PRESSURE(f3);
        
        /* Mix of calls with different return types */
        double ret3 = helper4(d1, d2);
        d3 = ret3 + d3;  /* d3 live across call */
        
        long ret4 = helper5(l1, l2);
        l3 = ret4 ^ l3;  /* l3 live across call */
        
        #ifdef __x86_64__
        /* Force specific register usage on x86 */
        int ret5 = helper_regparm(v6, v7, v8);
        v9 = ret5 + v9;  /* v9 live across call */
        #endif
        
        /* More complex dependency chain */
        v4 = helper2(v3, v4);  /* v4 used both as argument and live across */
        v5 = v4 + v5;          /* v5 live across helper2 call */
        
        /* Create loop-carried dependencies */
        v1 = v1 + i;
        v2 = v2 - i;
        f1 = f1 + (float)i;
        d1 = d1 + (double)i;
        l1 = l1 + i;
        
        /* Use volatile to prevent reordering */
        vol1 = v1;
        vol2 = v2;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                  + (int)f1 + (int)f2 + (int)f3
                  + (int)d1 + (int)d2 + (int)d3
                  + l1 + l2 + l3 + l4
                  + vol1 + vol2;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
