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
    
    /* Output arrays for each comparison type */
    int out_gt[N], out_ge[N], out_lt[N], out_le[N];
    int out_mixed[N];
    float fout[N];
    
    /* Initialize with deterministic values */
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-100, 100);
        arr2[i] = rand_int(0, 200);
        arr3[i] = rand_int(-50, 50);
        arr4[i] = rand_int(-1000, 1000);
        uarr[i] = (unsigned int)rand_int(0, 255);
        farr[i] = rand_float(-10.0f, 10.0f);
        darr[i] = (double)rand_float(-5.0f, 5.0f);
    }
    
    /* Loop 1: GT_EXPR (>) with integer array */
    int threshold1 = 25;
    for (int i = 0; i < N; i++) {
        /* Conditional mask-based computation */
        out_gt[i] = (arr1[i] > threshold1) ? arr1[i] * 2 : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with unsigned array */
    unsigned int limit = 100;
    for (int i = 0; i < N; i++) {
        /* Using if statement for mask generation */
        if (uarr[i] >= limit) {
            out_ge[i] = uarr[i] + 5;
        } else {
            out_ge[i] = 1;
        }
    }
    
    /* Loop 3: LT_EXPR (<) with floating-point array */
    float bound = 2.5f;
    for (int i = 0; i < N; i++) {
        /* Ternary operator creating mask pattern */
        fout[i] = (farr[i] < bound) ? farr[i] * 3.0f : 0.0f;
    }
    
    /* Loop 4: LE_EXPR (<=) with double array */
    double cap = -1.0;
    int sum_le = 0;
    for (int i = 0; i < N; i++) {
        /* Accumulation with conditional mask */
        sum_le += (darr[i] <= cap) ? (int)(darr[i] * 10) : 0;
    }
    
    /* Loop 5: Mixed comparisons with logical operators */
    int low = -30, high = 30;
    for (int i = 0; i < N; i++) {
        /* Nested condition exposing individual comparisons */
        if (arr3[i] > low && arr3[i] < high) {
            out_mixed[i] = arr3[i] * 3;
        } else if (arr3[i] <= -20 || arr3[i] >= 40) {
            out_mixed[i] = arr3[i] / 2;
        } else {
            out_mixed[i] = 0;
        }
    }
    
    /* Loop 6: Complex predicate with all four operators */
    int a = -10, b = 10, c = 20, d = 40;
    int complex_sum = 0;
    for (int i = 0; i < N; i++) {
        /* Complex condition that may decompose into individual comparisons */
        if ((arr4[i] > a && arr4[i] <= b) || (arr4[i] >= c && arr4[i] < d)) {
            complex_sum += arr4[i];
        }
    }
    
    /* Loop 7: Signed/unsigned mixed comparison */
    signed char scharr[N];
    for (int i = 0; i < N; i++) {
        scharr[i] = (signed char)rand_int(-128, 127);
    }
    
    int char_threshold = 64;
    int char_result[N];
    for (int i = 0; i < N; i++) {
        /* Signed char with > comparison */
        char_result[i] = (scharr[i] > char_threshold) ? scharr[i] * 2 : scharr[i];
    }
    
    /* Loop 8: Another LE_EXPR example with different types */
    int limit2 = 75;
    int out_le2[N];
    for (int i = 0; i < N; i++) {
        /* Direct assignment with conditional */
        out_le2[i] = (arr2[i] <= limit2) ? arr2[i] + 100 : arr2[i] - 50;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out_gt[i] + out_ge[i] + (int)fout[i] + out_mixed[i] + 
                   char_result[i] + out_le2[i];
    }
    checksum += sum_le + complex_sum;
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
