#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024

/* Prevent compiler from optimizing away the loop */
volatile int offset = 1;

int main(int argc, char *argv[]) {
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE] = {0};
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (i * 3.14159) / (argc + 1);
        array_b[i] = (i * 2.71828) / (argc + 2);
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Variable to create conditional dependency across iterations */
    double prev_condition = 0.0;
    
    /* Target loop for modulo scheduling */
    for (int i = 0; i < SIZE; i++) {
        /* High latency operation: square root */
        double x = sqrt(array_a[i] + 1.0);
        
        /* Another high latency operation: division */
        double y = array_b[i] / (x + 0.5);
        
        /* Loop-carried reduction with recurrence */
        sum = sum * 0.999 + x * y;
        
        /* Access with non-constant offset (creates complex addressing) */
        int idx = i + offset;
        if (idx < SIZE) {
            /* Memory access with potential aliasing */
            double temp = array_a[idx] * 0.5;
            
            /* Conditional store with cross-iteration dependency */
            if (i > 0 && prev_condition > 0.0) {
                array_c[i] = x + temp;
            }
            
            /* Update condition for next iteration */
            prev_condition = array_b[i] * 0.3;
        }
        
        /* Additional memory access with different offset */
        int idx2 = i - offset;
        if (idx2 >= 0) {
            /* High latency operation mixed with memory access */
            array_b[i] = array_a[idx2] / (sum + 1.0);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %f\n", sum);
    
    /* Also use array_c to prevent optimization */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_c[i];
    }
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
