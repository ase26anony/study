#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Simple deterministic pseudo-random number generator */
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
    
    /* Output arrays for each comparison pattern */
    int out_gt[N], out_ge[N], out_lt[N], out_le[N];
    int out_mixed[N];
    float fout_gt[N], fout_le[N];
    
    /* Initialize arrays with deterministic values */
    lcg = SEED;
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int(-100, 100);
        arr2[i] = rand_int(-50, 150);
        arr3[i] = rand_int(0, 200);
        arr4[i] = rand_int(-100, 100);
        uarr[i] = (unsigned int)rand_int(0, 255);
        farr[i] = rand_float(-10.0f, 10.0f);
        darr[i] = (double)rand_float(-5.0f, 5.0f);
    }
    
    /* Loop 1: GT_EXPR pattern with integer comparison */
    int threshold_gt = 25;
    for (int i = 0; i < N; i++) {
        /* Direct ternary with > comparison */
        out_gt[i] = (arr1[i] > threshold_gt) ? arr1[i] : 0;
        
        /* Additional mask-based computation */
        int mask = (arr2[i] > -10);
        out_gt[i] += mask * arr3[i];
    }
    
    /* Loop 2: GE_EXPR pattern with floating-point comparison */
    float limit_ge = 2.5f;
    for (int i = 0; i < N; i++) {
        /* Ternary with >= comparison */
        fout_gt[i] = (farr[i] >= limit_ge) ? farr[i] : limit_ge;
        
        /* Nested conditional with >= */
        if (farr[i] >= -limit_ge && farr[i] <= limit_ge * 2) {
            fout_gt[i] *= 2.0f;
        }
    }
    
    /* Loop 3: LT_EXPR pattern with unsigned comparison */
    unsigned int bound_lt = 128;
    for (int i = 0; i < N; i++) {
        /* Ternary with < comparison on unsigned */
        out_lt[i] = (uarr[i] < bound_lt) ? (int)uarr[i] : (int)bound_lt;
        
        /* Complex predicate with < operator */
        if (arr4[i] < 0 && uarr[i] < 200) {
            out_lt[i] += arr4[i];
        }
    }
    
    /* Loop 4: LE_EXPR pattern with double comparison */
    double cap_le = 3.0;
    for (int i = 0; i < N; i++) {
        /* Ternary with <= comparison */
        fout_le[i] = (darr[i] <= cap_le) ? (float)darr[i] : (float)cap_le;
        
        /* Mask-based assignment with <= */
        float temp = (arr1[i] <= threshold_gt) ? farr[i] : 0.0f;
        fout_le[i] += temp;
    }
    
    /* Loop 5: Mixed comparisons in logical expressions */
    int low = -30, high = 70;
    for (int i = 0; i < N; i++) {
        /* Combined comparisons with && and || */
        if ((arr1[i] > low && arr1[i] < high) || 
            (arr2[i] >= 0 && arr2[i] <= 100)) {
            out_mixed[i] = arr1[i] + arr2[i];
        } else {
            out_mixed[i] = arr3[i];
        }
        
        /* Nested ternary with multiple comparisons */
        int val = (arr4[i] < -50) ? -1 : 
                  (arr4[i] > 50) ? 1 : 
                  (arr4[i] >= -10 && arr4[i] <= 10) ? 0 : arr4[i];
        out_mixed[i] += val;
    }
    
    /* Loop 6: While loop with comparison in condition */
    int while_arr[N];
    int j = 0;
    int while_threshold = 75;
    while (j < N) {
        /* Comparison in while condition and inside body */
        while_arr[j] = (arr3[j] > while_threshold) ? arr3[j] : arr1[j];
        
        /* Additional < comparison inside */
        if (arr2[j] < while_threshold / 2) {
            while_arr[j] += arr2[j];
        }
        j++;
    }
    
    /* Loop 7: For loop with comparison in update and body */
    int complex_out[N];
    for (int i = 0; i < N; i += 2) {
        /* Multiple comparisons in one loop */
        int cmp1 = (arr1[i] >= -20) ? 1 : 0;
        int cmp2 = (arr2[i] <= 100) ? 1 : 0;
        complex_out[i] = cmp1 * arr1[i] + cmp2 * arr2[i];
        
        if (i + 1 < N) {
            /* Different comparison operator */
            complex_out[i+1] = (arr3[i+1] > arr4[i+1]) ? 
                               arr3[i+1] : arr4[i+1];
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += out_gt[i] + out_ge[i] + out_lt[i] + out_le[i];
        checksum += out_mixed[i] + while_arr[i] + complex_out[i];
        checksum += (int)fout_gt[i] + (int)fout_le[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
