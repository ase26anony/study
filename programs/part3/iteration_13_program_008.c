#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Simple deterministic pseudo-random generator */
static unsigned int lcg = SEED;
static inline int rand_int(int min, int max) {
    lcg = lcg * 1103515245 + 12345;
    return min + (lcg % (max - min + 1));
}

static inline float rand_float(float min, float max) {
    lcg = lcg * 1103515245 + 12345;
    return min + ((float)(lcg % 10001) / 10000.0f) * (max - min);
}

int main() {
    /* Declare arrays with different types */
    int arr1[N], arr2[N], arr3[N], arr4[N];
    unsigned int uarr[N];
    float farr[N];
    double darr[N];
    
    /* Output arrays */
    int out1[N], out2[N], out3[N], out4[N];
    unsigned int uout[N];
    float fout[N];
    double dout[N];
    
    /* Initialize with deterministic values */
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-100, 100);
        arr2[i] = rand_int(-50, 150);
        arr3[i] = rand_int(-200, 200);
        arr4[i] = rand_int(0, 255);
        uarr[i] = (unsigned int)rand_int(0, 1000);
        farr[i] = rand_float(-10.0f, 10.0f);
        darr[i] = (double)rand_float(-5.0, 5.0);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask */
    int threshold1 = 25;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data > threshold) ? data : 0 */
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask */
    float limit = 2.5f;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data >= limit) ? data : constant */
        fout[i] = (farr[i] >= limit) ? farr[i] : -1.0f;
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integer */
    unsigned int bound = 500;
    for (int i = 0; i < N; i++) {
        /* Pattern using if statement */
        if (uarr[i] < bound) {
            uout[i] = uarr[i] * 2;
        } else {
            uout[i] = uarr[i];
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision */
    double cap = 1.0;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data <= cap) ? data : scaled */
        dout[i] = (darr[i] <= cap) ? darr[i] : darr[i] * 0.5;
    }
    
    /* Additional loops with mixed patterns to ensure coverage */
    
    /* GT_EXPR with nested conditionals */
    int low = -10, high = 10;
    for (int i = 0; i < N; i++) {
        /* Complex predicate: if (arr2[i] > low && arr2[i] < high) */
        if (arr2[i] > low && arr2[i] < high) {
            out2[i] = arr2[i] * 3;
        } else {
            out2[i] = arr2[i];
        }
    }
    
    /* GE_EXPR with logical OR */
    int x = 0, y = 100;
    for (int i = 0; i < N; i++) {
        /* if (arr3[i] <= x || arr3[i] >= y) */
        if (arr3[i] <= x || arr3[i] >= y) {
            out3[i] = 0;
        } else {
            out3[i] = arr3[i];
        }
    }
    
    /* LT_EXPR with signed char (promoted to int) */
    signed char carr[N];
    char cout[N];
    for (int i = 0; i < N; i++) {
        carr[i] = (signed char)(arr4[i] - 128);
        /* Pattern: result = (data < 0) ? -data : data */
        cout[i] = (carr[i] < 0) ? -carr[i] : carr[i];
    }
    
    /* LE_EXPR with accumulation */
    int sum = 0;
    int limit2 = 50;
    for (int i = 0; i < N; i++) {
        /* sum += (arr4[i] <= limit2) ? arr4[i] : 0 */
        sum += (arr4[i] <= limit2) ? arr4[i] : 0;
    }
    
    /* Compute checksums to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (long long)uout[i];
        checksum += (long long)fout[i];
        checksum += (long long)dout[i];
        checksum += cout[i];
    }
    checksum += sum;
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
