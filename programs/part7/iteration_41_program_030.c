#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent inlining */
extern int get_value(void);

/* Function with complex loop to engage selective scheduler */
void process_arrays(int n, int threshold, float fthreshold) {
    int arr1[SIZE], arr2[SIZE];
    float farr1[SIZE], farr2[SIZE];
    volatile int limit = n;  /* Prevent constant propagation */
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = 0.0f;
    }
    
    int acc = 0;  /* Loop-carried dependency */
    float facc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int repeat = 0; repeat < 3; repeat++) {
        /* Complex inner loop with mixed operations and control flow */
        for (int i = 1; i < limit; i++) {
            /* Loop-carried integer dependency */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer/float operations */
            int temp = (arr1[i] & 0xFF) + (arr2[i] >> 2);
            float ftemp = (float)temp * 0.5f;
            
            /* Data-dependent conditional with floating point */
            if (farr1[i] > fthreshold) {
                farr2[i] = sqrtf(farr1[i] * ftemp);
                facc += farr2[i];
            } else {
                farr2[i] = farr1[i] * ftemp;
            }
            
            /* More complex integer arithmetic with bitwise ops */
            arr1[i] = (arr1[i] ^ arr2[i-1]) + (int)(farr2[i] * 100.0f);
            
            /* Another conditional with integer comparison */
            if (acc > threshold) {
                arr2[i] = arr2[i] / 2 + get_value();
                acc = acc % 1000;
            } else {
                arr2[i] = arr2[i] * 3 - 1;
            }
            
            /* Additional floating point operation */
            farr1[i] = farr1[i] + sinf((float)i * 0.01f);
        }
        
        /* Modify limit slightly each outer iteration */
        limit = (limit > 10) ? limit - 1 : n;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr1[i] + arr2[i] + (long long)farr2[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    printf("Accumulator: %d, Float Accumulator: %f\n", acc, facc);
}

/* External function implementation */
int get_value(void) {
    static int counter = 0;
    return counter++ % 100;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    /* Use command line argument for variability */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n < 10) n = 10;
    if (n > SIZE) n = SIZE;
    
    /* Thresholds from command line or random */
    int threshold = (argc > 2) ? atoi(argv[2]) : 5000;
    float fthreshold = (argc > 3) ? atof(argv[3]) : 0.5f;
    
    process_arrays(n, threshold, fthreshold);
    
    return 0;
}
