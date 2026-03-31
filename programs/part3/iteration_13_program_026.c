#include <stdio.h>
#include <stdlib.h>

#define N 1024

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static unsigned int rand_int(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

static float rand_float(void) {
    return (float)rand_int() / (float)(1U << 31);
}

int main(void) {
    /* Declare arrays with different types */
    int arr_int[N];
    unsigned int arr_uint[N];
    float arr_float[N];
    double arr_double[N];
    signed char arr_char[N];
    
    int out1[N], out2[N], out3[N], out4[N];
    float fout1[N], fout2[N];
    double dout[N];
    int checksum = 0;
    
    /* Initialize arrays with deterministic values */
    for (int i = 0; i < N; i++) {
        arr_int[i] = (int)(rand_int() % 1000) - 500;
        arr_uint[i] = rand_int() % 1000;
        arr_float[i] = rand_float() * 1000.0f - 500.0f;
        arr_double[i] = (double)rand_float() * 1000.0 - 500.0;
        arr_char[i] = (signed char)(rand_int() % 256 - 128);
    }
    
    /* Loop 1: GT_EXPR (>) with integer mask selection */
    int threshold1 = 100;
    for (int i = 0; i < N; i++) {
        /* Pattern: result = (data > threshold) ? data : 0 */
        out1[i] = (arr_int[i] > threshold1) ? arr_int[i] : 0;
    }
    
    /* Loop 2: GE_EXPR (>=) with floating-point and logical OR */
    float limit = 250.5f;
    for (int i = 0; i < N; i++) {
        /* Pattern: if (data >= limit) accumulate */
        if (arr_float[i] >= limit) {
            fout1[i] = arr_float[i] * 2.0f;
        } else {
            fout1[i] = arr_float[i];
        }
    }
    
    /* Loop 3: LT_EXPR (<) with unsigned integer and nested condition */
    unsigned int bound = 750;
    for (int i = 0; i < N; i++) {
        /* Pattern: mask-based selection with < operator */
        out2[i] = (arr_uint[i] < bound) ? arr_uint[i] : (int)bound;
        
        /* Additional nested conditional to expose LT_EXPR */
        if (arr_uint[i] < bound && arr_uint[i] > 100) {
            out3[i] = arr_uint[i] * 2;
        } else {
            out3[i] = arr_uint[i];
        }
    }
    
    /* Loop 4: LE_EXPR (<=) with double and complex predicate */
    double cap = 300.75;
    for (int i = 0; i < N; i++) {
        /* Pattern: (data <= cap) ? data : constant with OR condition */
        dout[i] = (arr_double[i] <= cap || arr_double[i] >= -cap) 
                  ? arr_double[i] 
                  : 0.0;
    }
    
    /* Loop 5: Mixed comparisons with signed char */
    signed char low = -50, high = 50;
    for (int i = 0; i < N; i++) {
        /* Using both < and > in same expression */
        out4[i] = (arr_char[i] > low && arr_char[i] < high) 
                  ? (int)arr_char[i] * 3 
                  : (int)arr_char[i];
    }
    
    /* Loop 6: GE_EXPR with floating-point accumulation */
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        /* Pattern: sum += (data >= threshold) ? data : 0 */
        sum += (arr_float[i] >= -200.0f) ? arr_float[i] : 0.0f;
    }
    fout2[0] = sum;  /* Store to prevent elimination */
    
    /* Loop 7: LE_EXPR with integer and bitwise-like pattern */
    int limit2 = 400;
    for (int i = 0; i < N; i++) {
        /* Direct if-statement with <= comparison */
        if (arr_int[i] <= limit2) {
            out1[i] += arr_int[i];  /* Modify existing output */
        }
    }
    
    /* Loop 8: GT_EXPR with while-loop style */
    int j = 0;
    int temp_sum = 0;
    while (j < N) {
        /* Using > in while condition with mask operation */
        temp_sum += (arr_int[j] > -300) ? 1 : 0;
        j++;
    }
    out2[0] += temp_sum;  /* Prevent elimination */
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        checksum += out1[i] + out2[i] + out3[i] + out4[i];
        checksum += (int)fout1[i] + (int)dout[i];
    }
    checksum += (int)fout2[0];
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
