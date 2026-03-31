/* test_modulo_sched.c
 * 
 * This program creates loops with specific characteristics to trigger
 * GCC's modulo scheduler debug output for dependency edges.
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Prevent optimization of critical variables */
static volatile int sink;

/* Function with multiple loops exhibiting different dependency patterns */
void compute_loops(void) {
    /* Loop 1: Integer carried dependency with arithmetic operations */
    int array1[N];
    int result1 = 1;
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < N; i++) {
        array1[i] = (i % 7) + 1;
    }
    
    /* Core loop with distance-1 dependency and integer multiplication */
    /* result1_i depends on result1_{i-1} creating carried dependency */
    for (int i = 1; i < N; i++) {
        /* Multiple operations to create resource pressure */
        int temp = array1[i] * 3;
        result1 = result1 * 2 + temp;  /* Distance-1 dependency */
        array1[i] = result1 % 1000;    /* Another dependency chain */
    }
    
    sink = result1;  /* Prevent dead code elimination */
    
    /* Loop 2: Floating-point operations with complex dependencies */
    double array2[M];
    double result2 = 1.0;
    double coeff = 3.141592653589793;
    
    /* Initialize with varying values */
    for (int i = 0; i < M; i++) {
        array2[i] = sin(i * 0.1) + 2.0;
    }
    
    /* Loop with floating-point divides and multiplies - high latency ops */
    for (int i = 1; i < M; i++) {
        /* Multiple carried dependencies creating complex scheduling constraints */
        double temp1 = array2[i] / coeff;      /* High latency division */
        double temp2 = temp1 * array2[i-1];    /* Uses previous iteration value */
        result2 = result2 * 0.99 + temp2;      /* Another distance-1 dependency */
        array2[i] = result2 * 1.01;            /* Feedback into array */
        
        /* Additional operations to increase register pressure */
        if (i % 3 == 0) {
            result2 = sqrt(fabs(result2)) + 0.5;  /* High latency sqrt */
        }
    }
    
    sink = (int)result2;
    
    /* Loop 3: Mixed integer/float with nested dependencies */
    float array3[N];
    int result3 = 0;
    
    for (int i = 0; i < N; i++) {
        array3[i] = (i % 13) * 0.5f;
    }
    
    /* Complex dependency web */
    float accum = 1.0f;
    for (int i = 1; i < N; i++) {
        /* Multiple inter-dependent operations */
        float val1 = array3[i] * accum;        /* Uses accum from previous iteration */
        float val2 = val1 / 2.7f;              /* Division operation */
        int ival = (int)val2;
        result3 += ival * i;                   /* Integer multiply-add */
        accum = val2 * 0.9f + array3[i-1];     /* Another distance-1 dependency */
        
        /* Conditional to prevent simple analysis */
        if (i % 5 == 0) {
            accum = accum * 1.1f;
            result3 -= ival;
        }
    }
    
    sink = result3;
}

/* Loop 4: Array recurrence with multiple dependency distances */
void array_recurrence(void) {
    int arr1[N], arr2[N];
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < N; i++) {
        arr1[i] = i + 1;
        arr2[i] = (i * 3) % 17;
    }
    
    /* Multiple interleaved recurrence relations */
    for (int i = 2; i < N; i++) {
        /* Distance-1 and distance-2 dependencies */
        arr1[i] = arr1[i-1] * 3 - arr1[i-2];  /* Distance-2 dependency */
        arr2[i] = arr2[i-1] + arr1[i] * 2;    /* Distance-1 dependency */
        sum += arr1[i] + arr2[i];
        
        /* Additional operations to increase II */
        if (i % 4 == 0) {
            sum = sum * 2 - arr1[i-1];
        }
    }
    
    sink = sum;
}

/* Main function to ensure all loops are executed */
int main(void) {
    int final_result = 0;
    
    compute_loops();
    array_recurrence();
    
    /* Use results to prevent optimization */
    final_result = sink;
    
    printf("Result: %d\n", final_result);
    return 0;
}
