/* caller-save-trigger.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Non-inlineable helper functions with different calling conventions */
int __attribute__((noinline)) helper1(int a, int b) {
    return (a ^ b) + 1;
}

int __attribute__((noinline)) helper2(int a, int b, int c) {
    return (a + b) * c;
}

float __attribute__((noinline)) helper3(float a, float b) {
    return a * b - 0.5f;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a / (b + 1.0);
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0xFF);
}

/* x86-specific: force use of specific registers */
#ifdef __x86_64__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b - c;
}
#endif

/* Force register pressure by preventing optimization */
static void use_var(int *var) {
    asm volatile("" : "+r"(*var));
}

int main(int argc, char **argv) {
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
    
    long l1 = rand() * rand();
    long l2 = rand() * rand();
    long l3 = rand() * rand();
    
    /* Create artificial dependencies between variables */
    for (int i = 0; i < N; i++) {
        /* Complex web of dependencies - all variables stay live */
        v1 = v2 + v3;
        v2 = v1 ^ v4;
        v3 = v5 * v6;
        v4 = v7 - v8;
        v5 = v9 & v10;
        
        /* Use volatile asm to prevent optimization */
        use_var(&v6);
        use_var(&v7);
        
        /* CRITICAL: Function call with some live variables as arguments,
           followed immediately by use of other live variables */
        int ret1 = helper1(v1, v2);
        /* v3, v4, v5, v6, v7 are LIVE across the call in call-clobbered regs */
        v8 = v3 + ret1;  /* Uses v3 which was live across call */
        v9 = v4 * ret1;  /* Uses v4 which was live across call */
        
        /* Another call creating more pressure */
        float ret2 = helper3(f1, f2);
        /* f3, f4 are live across this call */
        f1 = f3 + ret2;
        f2 = f4 * ret2;
        
        /* More dependencies */
        v10 = helper2(v5, v6, v7);
        v6 = v8 ^ v9;
        v7 = v10 + i;
        
        /* Double precision operations */
        double ret3 = helper4(d1, d2);
        d1 = d3 * ret3;
        d2 = d1 + ret3;
        
        /* Long operations */
        long ret4 = helper5(l1, l2);
        l1 = l3 ^ ret4;
        l2 = l1 + i;
        l3 = l2 * 3;
        
        #ifdef __x86_64__
        /* Force specific register usage on x86 */
        int ret5 = helper_regparm(v1, v2, v3);
        v4 = v5 + ret5;
        #endif
        
        /* Create loop-carried dependencies */
        f3 = f1 * 0.9f + f2 * 0.1f;
        f4 = f3 - f1;
        d3 = d1 / (d2 + 1.0);
        
        /* More volatile usage to prevent register elimination */
        asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3));
        asm volatile("" : "+r"(f1), "+r"(f2));
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (long)(f1 * 1000) + (long)(f2 * 1000) + 
                (long)(f3 * 1000) + (long)(f4 * 1000);
    checksum += (long)(d1 * 1000) + (long)(d2 * 1000) + (long)(d3 * 1000);
    checksum += l1 + l2 + l3;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
