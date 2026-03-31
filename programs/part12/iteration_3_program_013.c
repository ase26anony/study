/* caller-save-trigger.c */
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
    return a + b * 0.5f;
}

double __attribute__((noinline)) helper4(double a, double b) {
    return a - b * 0.25;
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0xFF);
}

/* x86-specific regparm attribute to force register parameter passing */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return (a + b) * c;
}
#endif

/* Function to create artificial register pressure */
void __attribute__((noinline)) use_registers(int *arr, int n) {
    volatile int tmp = 0;
    for (int i = 0; i < n; i++) {
        tmp += arr[i];
    }
    /* Prevent elimination */
    asm volatile("" : "+r"(tmp));
}

int main(int argc, char *argv[]) {
    /* Non-constant loop bound to prevent unrolling */
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
    /* Seed RNG to prevent constant propagation */
    srand((unsigned int)time(NULL));
    
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
    
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    float f3 = (float)(rand() % 100) / 10.0f;
    float f4 = (float)(rand() % 100) / 10.0f;
    
    double d1 = (double)(rand() % 100) / 5.0;
    double d2 = (double)(rand() % 100) / 5.0;
    double d3 = (double)(rand() % 100) / 5.0;
    
    long l1 = (long)rand() * rand();
    long l2 = (long)rand() * rand();
    long l3 = (long)rand() * rand();
    
    /* Array for additional pressure */
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = rand() % 50;
    }
    
    /* Main loop with loop-carried dependencies */
    for (int iter = 0; iter < N; iter++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v2 = v1 ^ v4;
        v3 = v5 - v6;
        v4 = v7 * v8;
        v5 = v9 / (v10 + 1);
        v6 = v2 | v3;
        v7 = v4 & v5;
        v8 = v6 ^ v7;
        v9 = v8 + v1;
        v10 = v9 - v2;
        
        f1 = f2 + f3;
        f2 = f1 * 1.1f;
        f3 = f4 - 0.5f;
        f4 = f2 / (f3 + 0.1f);
        
        d1 = d2 * 1.01;
        d2 = d3 + 0.25;
        d3 = d1 - d2;
        
        l1 = l2 << 2;
        l2 = l3 >> 1;
        l3 = l1 | l2;
        
        /* CRITICAL: Function call followed immediately by use of 
           live variables in call-clobbered registers */
        int ret1 = helper1(v1, v2);
        
        /* This use of v3 must happen immediately after the call,
           forcing potential caller-save insertion between call and use */
        v3 = v3 + ret1;  /* v3 was live across the call */
        
        /* Another call with different register types */
        float ret2 = helper3(f1, f2);
        
        /* Immediate use of f3 which was live across the call */
        f3 = f3 * ret2;
        
        /* Call with double arguments */
        double ret3 = helper4(d1, d2);
        
        /* Immediate use of d3 */
        d3 = d3 + ret3;
        
        /* Call with long arguments */
        long ret4 = helper5(l1, l2);
        
        /* Immediate use of l3 */
        l3 = l3 ^ ret4;
        
        /* Mixed-type call */
        int ret5 = helper2(v4, v5, v6);
        
        /* Multiple live variables used immediately after call */
        v7 = v7 + ret5;  /* v7 was live */
        v8 = v8 - ret5;  /* v8 was live */
        
        /* Use asm volatile to prevent reordering/elimination */
        asm volatile("" : "+r"(v9), "+r"(v10));
        asm volatile("" : "+r"(f4));
        asm volatile("" : "+r"(d3));
        
        /* Create additional register pressure */
        use_registers(arr, 10);
        
        /* More arithmetic to keep variables live */
        v1 = v1 + iter;
        v2 = v2 - iter;
        f1 = f1 + (float)iter * 0.01f;
        d1 = d1 + (double)iter * 0.001;
        l1 = l1 + iter;
        
        #ifdef __i386__
        /* x86-specific: regparm calling convention */
        int ret6 = helper_regparm(v1, v2, v3);
        v4 = v4 + ret6;  /* v4 was live across call */
        #endif
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (long)(f1 + f2 + f3 + f4);
    checksum += (long)(d1 + d2 + d3);
    checksum += l1 + l2 + l3;
    
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
