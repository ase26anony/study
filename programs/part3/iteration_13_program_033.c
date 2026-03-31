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
    char carr[N];
    
    /* Output arrays */
    int out1[N], out2[N], out3[N], out4[N];
    unsigned int uout[N];
    float fout[N];
    double dout[N];
    char cout[N];
    
    /* Initialize with deterministic values */
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-1000, 1000);
        arr2[i] = rand_int(-500, 500);
        arr3[i] = rand_int(0, 1000);
        arr4[i] = rand_int(-200, 200);
        uarr[i] = (unsigned int)rand_int(0, 2000);
        farr[i] = rand_float(-10.0f, 10.0f);
        darr[i] = (double)rand_float(-5.0, 5.0);
        carr[i] = (char)rand_int(-128, 127);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask */
    int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data > threshold) ? data : 0 */
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point mask */
    float limit = 2.5f;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data >= limit) ? data : constant */
        fout[i] = (farr[i] >= limit) ? farr[i] : 1.0f;
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integer */
    unsigned int bound = 1000;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data < bound) ? data : bound */
        uout[i] = (uarr[i] < bound) ? uarr[i] : bound;
    }
    
    /* Loop 4: LE_EXPR (<=) with double precision */
    double cap = 3.0;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data <= cap) ? data : -data */
        dout[i] = (darr[i] <= cap) ? darr[i] : -darr[i];
    }
    
    /* Additional loops with mixed comparisons for more coverage */
    
    /* Loop 5: GT_EXPR with nested conditionals */
    int low = -50, high = 50;
    for (int i = 0; i < N; i++) {
        /* Complex predicate: (arr2[i] > low && arr2[i] < high) */
        if (arr2[i] > low && arr2[i] < high) {
            out2[i] = arr2[i] * 2;
        } else {
            out2[i] = arr2[i];
        }
    }
    
    /* Loop 6: GE_EXPR with logical OR */
    int x = -100, y = 100;
    for (int i = 0; i < N; i++) {
        /* Pattern: (arr3[i] <= x || arr3[i] >= y) */
        if (arr3[i] <= x || arr3[i] >= y) {
            out3[i] = 0;
        } else {
            out3[i] = arr3[i];
        }
    }
    
    /* Loop 7: LT_EXPR with char type */
    char char_threshold = 64;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (carr[i] < threshold) ? carr[i] : threshold */
        cout[i] = (carr[i] < char_threshold) ? carr[i] : char_threshold;
    }
    
    /* Loop 8: LE_EXPR with if-else chain */
    int limit1 = -150, limit2 = 150;
    for (int i = 0; i < N; i++) {
        if (arr4[i] <= limit1) {
            out4[i] = -arr4[i];
        } else if (arr4[i] >= limit2) {
            out4[i] = arr4[i] * 3;
        } else {
            out4[i] = arr4[i];
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (long long)uout[i];
        checksum += (long long)fout[i];
        checksum += (long long)dout[i];
        checksum += cout[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
