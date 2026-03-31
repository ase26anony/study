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
    return (a << 3) | (b & 0xFF);
}

/* x86-specific: force use of specific registers */
#ifdef __x86_64__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b * c;
}
#endif

/* Function to create artificial register pressure */
void __attribute__((noinline)) use_registers(int *arr, int n) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    /* Prevent elimination */
    asm volatile("" : : "r"(sum));
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
    int v11 = rand() % 100;
    int v12 = rand() % 100;
    int v13 = rand() % 100;
    int v14 = rand() % 100;
    int v15 = rand() % 100;
    float f1 = (float)rand() / RAND_MAX;
    float f2 = (float)rand() / RAND_MAX;
    float f3 = (float)rand() / RAND_MAX;
    double d1 = (double)rand() / RAND_MAX;
    double d2 = (double)rand() / RAND_MAX;
    long l1 = rand();
    long l2 = rand();
    
    /* Volatile variables to inhibit optimizations */
    volatile int vol1 = v1;
    volatile float volf = f1;
    volatile double vold = d1;
    
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = rand() % 100;
    }
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v2 = v1 ^ v4;
        v3 = v5 * v6;
        v4 = v7 - v8;
        v5 = v9 | v10;
        v6 = v11 & v12;
        v7 = v13 + v14;
        v8 = v15 ^ v1;
        v9 = v2 * v3;
        v10 = v4 + v5;
        v11 = v6 - v7;
        v12 = v8 ^ v9;
        v13 = v10 * v11;
        v14 = v12 + v13;
        v15 = v14 ^ v15;
        
        /* Float operations */
        f1 = f2 * 1.5f;
        f2 = f3 + f1;
        f3 = f1 * 0.7f - f2;
        
        /* Double operations */
        d1 = d2 * 1.618;
        d2 = d1 / 2.0 + 1.0;
        
        /* Long operations */
        l1 = l2 << 2;
        l2 = l1 >> 1;
        
        /* CRITICAL: Function call with live values in call-clobbered registers */
        /* v1-v4 are used as arguments, but v5-v8 remain live across the call */
        int result1 = helper1(v1, v2);
        
        /* Immediately use result with variables that were live across the call */
        /* This forces save/restore insertion between the call and this use */
        v5 = result1 + v5;  /* v5 was live in call-clobbered register */
        v6 = v6 * result1;  /* v6 was live in call-clobbered register */
        
        /* Artificial use to prevent optimization */
        asm volatile("" : "+r"(v5), "+r"(v6));
        
        /* Another function call creating more pressure */
        int result2 = helper2(v3, v4, v5);
        
        /* More immediate uses of live variables */
        v7 = v7 + result2;
        v8 = v8 ^ result2;
        
        /* Float function call */
        float fresult = helper3(f1, f2);
        f3 = f3 + fresult;
        
        /* Double function call */
        double dresult = helper4(d1, d2);
        d1 = d1 - dresult;
        
        /* Long function call */
        long lresult = helper5(l1, l2);
        l2 = l2 + lresult;
        
        #ifdef __x86_64__
        /* x86-specific: force regparm calling convention */
        int rresult = helper_regparm(v9, v10, v11);
        v12 = v12 + rresult;
        #endif
        
        /* Function that uses many registers */
        use_registers(arr, 10);
        
        /* More operations to keep variables live */
        v13 = v13 + v14;
        v14 = v14 * v15;
        v15 = v15 ^ v13;
        
        /* Volatile operations to inhibit reordering */
        vol1 = v1;
        volf = f1;
        vold = d1;
        
        /* Create cross-type dependencies */
        v1 = (int)(f1 * 100.0f);
        f2 = (float)v2 / 10.0f;
        v3 = (int)(d1 * 1000.0);
        d2 = (double)v4 / 100.0;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                    v11 + v12 + v13 + v14 + v15 +
                    (long)(f1 * 1000) + (long)(f2 * 1000) + (long)(f3 * 1000) +
                    (long)(d1 * 10000) + (long)(d2 * 10000) +
                    l1 + l2 + vol1;
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
