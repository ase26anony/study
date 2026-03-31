#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define SEED 42

/* Worker function with explicit optimization attribute */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize")))
#endif
static int process_comparisons(const short *a, const short *b, int n) {
    /* Destination arrays for comparison results */
    char gt_res[N], ge_res[N], lt_res[N], le_res[N];
    int i;
    
    /* Loop 1: GT_EXPR (>) */
    for (i = 0; i < n; i++) {
        gt_res[i] = a[i] > b[i];
    }
    
    /* Loop 2: GE_EXPR (>=) */
    for (i = 0; i < n; i++) {
        ge_res[i] = a[i] >= b[i];
    }
    
    /* Loop 3: LT_EXPR (<) */
    for (i = 0; i < n; i++) {
        lt_res[i] = a[i] < b[i];
    }
    
    /* Loop 4: LE_EXPR (<=) */
    for (i = 0; i < n; i++) {
        le_res[i] = a[i] <= b[i];
    }
    
    /* Combine results to prevent elimination */
    int sum = 0;
    for (i = 0; i < n; i++) {
        sum += gt_res[i] + ge_res[i] + lt_res[i] + le_res[i];
    }
    
    return sum;
}

/* Simple pseudo-random generator to avoid constant propagation */
static unsigned simple_rand(unsigned *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

int main(int argc, char **argv) {
    short array1[N], array2[N];
    unsigned state = SEED;
    int i, total = 0;
    
    /* Initialize with non-constant values */
    for (i = 0; i < N; i++) {
        array1[i] = (short)(simple_rand(&state) % 1000);
        array2[i] = (short)(simple_rand(&state) % 1000);
        
        /* Introduce some patterns to ensure all comparison outcomes occur */
        if (i % 4 == 0) array1[i] = array2[i];      /* equal */
        else if (i % 4 == 1) array1[i] = array2[i] + 10; /* greater */
        else if (i % 4 == 2) array1[i] = array2[i] - 10; /* less */
        /* else: random relationship */
    }
    
    /* Call worker function multiple times with different data */
    for (i = 0; i < 10; i++) {
        /* Slightly modify arrays each iteration */
        array1[i % N] ^= 1;
        array2[i % N] ^= 2;
        
        total += process_comparisons(array1, array2, N);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", total);
    
    /* Additional floating-point comparisons for completeness */
    {
        float f1[N], f2[N];
        volatile int fp_result = 0;
        
        for (i = 0; i < N; i++) {
            f1[i] = (float)(simple_rand(&state) % 1000) / 10.0f;
            f2[i] = (float)(simple_rand(&state) % 1000) / 10.0f;
        }
        
        /* Separate floating-point comparison loops */
        for (i = 0; i < N; i++) fp_result += f1[i] > f2[i];
        for (i = 0; i < N; i++) fp_result += f1[i] >= f2[i];
        for (i = 0; i < N; i++) fp_result += f1[i] < f2[i];
        for (i = 0; i < N; i++) fp_result += f1[i] <= f2[i];
        
        printf("FP comparisons: %d\n", fp_result);
    }
    
    return 0;
}
