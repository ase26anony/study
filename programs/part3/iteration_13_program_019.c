#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static unsigned int rand_int(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

int main(void) {
    /* Declare arrays with different types */
    int arr1[N], arr2[N], arr3[N], arr4[N];
    float farr1[N], farr2[N];
    unsigned int uarr[N];
    char carr[N];
    
    int out1[N], out2[N], out3[N], out4[N];
    float fout1[N], fout2[N];
    unsigned int uout[N];
    char cout[N];
    
    long long checksum = 0;
    
    /* Initialize arrays with deterministic values */
    for (int i = 0; i < N; i++) {
        arr1[i] = rand_int() - 16384;          /* Signed int, range -16384..16383 */
        arr2[i] = rand_int() % 1000;           /* Positive int, range 0..999 */
        arr3[i] = (rand_int() % 200) - 100;    /* Signed int, range -100..99 */
        arr4[i] = rand_int() % 500;            /* Positive int, range 0..499 */
        
        farr1[i] = (float)(rand_int() - 16384) / 100.0f;  /* Float, range -163.84..163.83 */
        farr2[i] = (float)(rand_int() % 1000) / 10.0f;    /* Float, range 0.0..99.9 */
        
        uarr[i] = rand_int();                  /* Unsigned int */
        carr[i] = (char)(rand_int() % 256 - 128); /* Signed char, range -128..127 */
    }
    
    /* Loop 1: GT_EXPR (>) with integer array */
    /* Pattern: result[i] = (data[i] > threshold) ? data[i] : 0 */
    const int threshold1 = 500;
    for (int i = 0; i < N; i++) {
        out1[i] = (arr1[i] > threshold1) ? arr1[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with float array */
    /* Pattern: if (data[i] >= limit) accumulate */
    const float limit = 50.0f;
    for (int i = 0; i < N; i++) {
        fout1[i] = (farr1[i] >= limit) ? farr1[i] : -farr1[i];
    }
    
    /* Loop 3: LT_EXPR (<) with mixed types and nested condition */
    /* Pattern: if (data[i] < low && data[i] > -high) select value */
    const int low = -50;
    const int high = 100;
    for (int i = 0; i < N; i++) {
        if (arr3[i] < low && arr3[i] > -high) {
            out3[i] = arr3[i] * 2;
        } else {
            out3[i] = arr3[i];
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with unsigned array */
    /* Pattern: mask-based selection with unsigned comparison */
    const unsigned int cap = 10000;
    for (int i = 0; i < N; i++) {
        uout[i] = (uarr[i] <= cap) ? uarr[i] : (uarr[i] / 2);
    }
    
    /* Additional loops to ensure all operators are covered */
    
    /* Loop 5: GT_EXPR with char array and logical OR */
    const char char_threshold = 64;
    for (int i = 0; i < N; i++) {
        cout[i] = (carr[i] > char_threshold || carr[i] < -char_threshold) ? 
                  carr[i] : (char)(carr[i] + 32);
    }
    
    /* Loop 6: LE_EXPR with float and complex predicate */
    const float upper_bound = 75.0f;
    const float lower_bound = 25.0f;
    for (int i = 0; i < N; i++) {
        if (farr2[i] <= upper_bound && farr2[i] >= lower_bound) {
            fout2[i] = farr2[i] * 2.0f;
        } else if (farr2[i] < lower_bound) {
            fout2[i] = farr2[i] + 10.0f;
        } else {
            fout2[i] = farr2[i] - 10.0f;
        }
    }
    
    /* Loop 7: GE_EXPR with integer and ternary in expression */
    const int limit2 = 200;
    for (int i = 0; i < N; i++) {
        out2[i] = arr2[i] + ((arr2[i] >= limit2) ? 100 : -100);
    }
    
    /* Loop 8: LT_EXPR with while-style loop simulation */
    int temp[N];
    for (int i = 0; i < N; i++) {
        temp[i] = arr4[i];
    }
    for (int i = 0; i < N; i++) {
        while (temp[i] < 400) {  /* LT_EXPR in loop condition */
            temp[i] += 50;
        }
        out4[i] = temp[i];
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i] + uout[i] + cout[i];
        checksum += (long long)fout1[i] + (long long)fout2[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}
