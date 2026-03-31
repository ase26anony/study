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

/* x86-specific regparm attribute */
#ifdef __i386__
int __attribute__((regparm(3), noinline)) helper_regparm(int a, int b, int c) {
    return a + b - c;
}
#else
int __attribute__((noinline)) helper_regparm(int a, int b, int c) {
    return a + b - c;
}
#endif

/* Function to create artificial register pressure */
void use_registers(int *arr, int n) {
    volatile int tmp;
    for (int i = 0; i < n; i++) {
        tmp = arr[i];
        asm volatile("" : "+r"(tmp));
        arr[i] = tmp + 1;
    }
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
    
    float f1 = (float)(rand() % 100) / 10.0f;
    float f2 = (float)(rand() % 100) / 10.0f;
    float f3 = (float)(rand() % 100) / 10.0f;
    float f4 = (float)(rand() % 100) / 10.0f;
    
    double d1 = (double)(rand() % 100) / 10.0;
    double d2 = (double)(rand() % 100) / 10.0;
    double d3 = (double)(rand() % 100) / 10.0;
    
    long l1 = rand() % 100;
    long l2 = rand() % 100;
    long l3 = rand() % 100;
    long l4 = rand() % 100;
    
    /* Array to create additional pressure */
    int arr[10];
    for (int i = 0; i < 10; i++) arr[i] = rand() % 100;
    
    /* Main loop with loop-carried dependencies */
    for (int i = 0; i < N; i++) {
        /* Create dense web of data dependencies */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 ^ v9;
        v10 = v8 | v2;
        
        f1 = f2 * 1.1f;
        f3 = f1 + f4;
        f2 = f3 - 0.5f;
        f4 = f2 / 2.0f;
        
        d1 = d2 + 0.25;
        d3 = d1 * 1.5;
        d2 = d3 / (d1 + 1.0);
        
        l1 = l2 << 2;
        l3 = l1 | l4;
        l2 = l3 + 1;
        l4 = l2 ^ 0xAA;
        
        /* CRITICAL: Function call followed immediately by use of live values
         * This creates the scenario where caller-save must be inserted
         * between the call and the next instruction */
        int ret1 = helper1(v1, v2);
        
        /* Immediate use of variables live across the call */
        v3 = ret1 + v4;  /* v4 was live in call-clobbered register */
        v5 = v3 * v6;    /* v6 was live in call-clobbered register */
        
        /* Use asm volatile to prevent optimizations */
        asm volatile("" : "+r"(v3), "+r"(v5));
        
        /* Another function call with different register types */
        float ret2 = helper3(f1, f2);
        
        /* Immediate use of float variables live across call */
        f3 = ret2 + f4;  /* f4 was live in call-clobbered register */
        asm volatile("" : "+r"(f3));
        
        /* Call with mixed arguments */
        int ret3 = helper2(v5, v6, v7);
        
        /* Complex dependency chain across calls */
        v8 = ret3 + v9 + v10;
        v9 = v8 ^ v1;
        
        /* Call using regparm convention (x86) */
        int ret4 = helper_regparm(v2, v3, v4);
        v10 = ret4 * v5;
        
        /* Double precision call */
        double ret5 = helper4(d1, d2);
        d3 = ret5 + d1;  /* d1 was live across call */
        
        /* Long integer call */
        long ret6 = helper5(l1, l2);
        l3 = ret6 + l4;  /* l4 was live across call */
        
        /* Create artificial array pressure periodically */
        if (i % 10 == 0) {
            use_registers(arr, 10);
            v1 += arr[0];
            v2 += arr[1];
        }
        
        /* Loop-carried updates */
        v2 = v1 + i;
        v7 = v6 - i;
        v9 = v8 ^ i;
        
        f2 = f1 + (float)i;
        f4 = f3 * (float)i;
        
        d2 = d1 + (double)i;
        
        l2 = l1 + i;
        l4 = l3 ^ i;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += (long)(f1 + f2 + f3 + f4);
    checksum += (long)(d1 + d2 + d3);
    checksum += l1 + l2 + l3 + l4;
    
    for (int i = 0; i < 10; i++) checksum += arr[i];
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}
