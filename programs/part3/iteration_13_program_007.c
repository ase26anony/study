#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Simple deterministic pseudo-random generator */
static unsigned seed = 123456789;
static unsigned rand_int(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned)(seed / 65536) % 32768;
}

int main(void) {
    /* Declare arrays with different types */
    int arr1[N], arr2[N], arr3[N], arr4[N];
    float farr1[N], farr2[N];
    unsigned int uarr[N];
    char carr[N];
    
    /* Output arrays for results */
    int out1[N], out2[N], out3[N], out4[N];
    float fout1[N], fout2[N];
    int out_mixed[N];
    
    /* Initialize arrays with deterministic values */
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int() % 1000 - 500;      /* Signed int, range -500..499 */
        arr2[i] = rand_int() % 1000 - 500;
        arr3[i] = rand_int() % 1000 - 500;
        arr4[i] = rand_int() % 1000 - 500;
        farr1[i] = (rand_int() % 2000 - 1000) / 10.0f;  /* Float, range -100..100 */
        farr2[i] = (rand_int() % 2000 - 1000) / 10.0f;
        uarr[i] = rand_int() % 1000;            /* Unsigned int, range 0..999 */
        carr[i] = (char)(rand_int() % 256 - 128); /* Signed char, range -128..127 */
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask selection */
    int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data > threshold) ? data : 0 */
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask */
    float limit = 50.0f;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data >= limit) ? data : constant */
        fout1[i] = (farr1[i] >= limit) ? farr1[i] : -1.0f;
    }
    
    /* Loop 3: LT_EXPR (<) with nested condition */
    int low = -200, high = 200;
    for (int i = 0; i < N; i++) {
        /* Complex predicate with LT_EXPR and GT_EXPR */
        if (arr2[i] < high && arr2[i] > low) {
            out2[i] = arr2[i];
        } else {
            out2[i] = 0;
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with unsigned integers */
    unsigned int cap = 500;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data <= cap) ? data : cap */
        out3[i] = (uarr[i] <= cap) ? (int)uarr[i] : (int)cap;
    }
    
    /* Loop 5: Mixed comparisons with signed char */
    char lower = -50, upper = 50;
    for (int i = 0; i < N; i++) {
        /* Using both <= and >= in logical OR */
        if (carr[i] <= lower || carr[i] >= upper) {
            out4[i] = carr[i];
        } else {
            out4[i] = 0;
        }
    }
    
    /* Loop 6: GE_EXPR with floating-point and accumulation */
    float sum = 0.0f;
    float fthreshold = 25.5f;
    for (int i = 0; i < N; i++) {
        /* Accumulation based on comparison mask */
        sum += (farr2[i] >= fthreshold) ? farr2[i] : 0.0f;
    }
    
    /* Loop 7: GT_EXPR with while loop style */
    int j = 0;
    int bound = 800;
    while (j < N) {
        /* Using > in while condition and mask operation */
        out_mixed[j] = (arr3[j] > bound) ? arr3[j] : -arr3[j];
        j++;
    }
    
    /* Loop 8: LE_EXPR with nested ternary */
    int limit2 = 300;
    for (int i = 0; i < N; i++) {
        /* Nested ternary using <= */
        out_mixed[i] = (arr4[i] <= limit2) ? 
                      ((arr4[i] <= limit2/2) ? arr4[i] * 2 : arr4[i]) : 
                      -arr4[i];
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum1 = 0, checksum2 = 0;
    float fchecksum = 0.0f;
    
    for (int i = 0; i < N; i++) {
        checksum1 += out1[i] + out2[i] + out3[i] + out4[i] + out_mixed[i];
    }
    
    for (int i = 0; i < N; i++) {
        fchecksum += fout1[i];
    }
    
    checksum2 = (long long)sum;
    
    printf("Checksum1: %lld\n", checksum1);
    printf("Checksum2: %lld\n", checksum2);
    printf("Floating checksum: %f\n", fchecksum);
    
    return 0;
}
