/* caller_save_trigger.c
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-inline -fno-tree-loop-optimize caller_save_trigger.c -o caller_save_trigger
 * For RTL dumps: gcc -O3 -fno-schedule-insns -fdump-rtl-all caller_save_trigger.c
 */

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
    return a + b * 0.333;
}

long __attribute__((noinline)) helper5(long a, long b) {
    return (a << 3) | (b & 0xFF);
}

/* x86-specific regparm attribute */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b * c;
}
#else
int __attribute__((noinline)) helper_regparm(int a, int b, int c) {
    return a + b * c;
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
    int N = (argc > 1) ? atoi(argv[1]) : 1000;
    if (N <= 0) N = 1000;
    
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
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    float f3 = (float)(rand() % 100) / 10.0f;
    double d1 = (double)(rand() % 100) / 10.0;
    double d2 = (double)(rand() % 100) / 10.0;
    long l1 = rand() % 100;
    long l2 = rand() % 100;
    
    /* Array for artificial pressure */
    int pressure_arr[10];
    for (int i = 0; i < 10; i++) {
        pressure_arr[i] = rand() % 50;
    }
    
    /* Main loop with loop-carried dependencies */
    for (int iter = 0; iter < N; iter++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v2 = v1 ^ v4;
        v3 = v2 * v5;
        v4 = v3 - v6;
        v5 = v4 | v7;
        v6 = v5 & v8;
        v7 = v6 + v9;
        v8 = v7 * v10;
        v9 = v8 - v11;
        v10 = v9 ^ v12;
        v11 = v10 + v13;
        v12 = v11 * v14;
        v13 = v12 - v15;
        v14 = v13 | v1;
        v15 = v14 & v2;
        
        /* Float operations */
        f1 = f2 * 1.1f + f3;
        f2 = f1 * 0.9f - f3;
        f3 = f2 + f1 * 0.5f;
        
        /* Double operations */
        d1 = d2 * 1.01 + (double)v1;
        d2 = d1 * 0.99 - (double)v2;
        
        /* Long operations */
        l1 = (l2 << 2) | (v3 & 0xF);
        l2 = (l1 >> 1) + v4;
        
        /* 
         * CRITICAL SECTION: Function call followed immediately by use of 
         * values that were live in call-clobbered registers
         */
        int ret1 = helper1(v1, v2);
        
        /* v3, v4, v5 are live across the call and used immediately after */
        v3 = v3 + ret1;      /* Uses v3 which was live across helper1 call */
        v4 = v4 * ret1;      /* Uses v4 which was live across helper1 call */
        v5 = v5 ^ ret1;      /* Uses v5 which was live across helper1 call */
        
        /* Another call creating more pressure */
        float ret2 = helper3(f1, f2);
        
        /* f3 is live across this call */
        f3 = f3 + ret2;      /* Uses f3 which was live across helper3 call */
        
        /* Call with regparm convention (x86) */
        int ret3 = helper_regparm(v6, v7, v8);
        
        /* v9, v10 are live across this call */
        v9 = v9 + ret3;      /* Uses v9 live across helper_regparm call */
        v10 = v10 - ret3;    /* Uses v10 live across helper_regparm call */
        
        /* Double call */
        double ret4 = helper4(d1, d2);
        d1 = d1 + ret4;      /* d1 was live across helper4 call */
        
        /* Long call */
        long ret5 = helper5(l1, l2);
        l1 = l1 ^ ret5;      /* l1 was live across helper5 call */
        
        /* Complex chain with multiple calls */
        int ret6 = helper2(v11, v12, v13);
        v14 = v14 + ret6;    /* v14 live across helper2 call */
        v15 = v15 - ret6;    /* v15 live across helper2 call */
        
        /* Create artificial register pressure with volatile asm */
        asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3));
        asm volatile("" : "+r"(f1), "+r"(f2));
        
        /* Call that uses array to pressure registers */
        use_registers(pressure_arr, 10);
        
        /* More operations after call - values must be in registers */
        v1 = v1 + pressure_arr[iter % 10];
        v2 = v2 - pressure_arr[(iter + 1) % 10];
        
        /* Another helper call in the middle of computation */
        int ret7 = helper1(v3, v4);
        
        /* Immediate use of live values after call */
        v5 = v5 + ret7 + v3;  /* v3 and v5 live across helper1 call */
        v6 = v6 * ret7 + v4;  /* v4 and v6 live across helper1 call */
        
        /* Final mixing */
        v7 = (v7 ^ v8) + v9;
        v8 = (v8 & v10) | v11;
        v9 = v12 + v13 - v14;
        v10 = v15 * v1;
        
        /* Prevent loop unrolling */
        if (iter % 100 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += v11 + v12 + v13 + v14 + v15;
    checksum += (long long)(f1 * 100) + (long long)(f2 * 100) + (long long)(f3 * 100);
    checksum += (long long)(d1 * 100) + (long long)(d2 * 100);
    checksum += l1 + l2;
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
