#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent inlining */
extern int external_helper(int x);

int external_helper(int x) {
    return (x * 1103515245 + 12345) & 0x7fffffff;
}

/* Complex loop with mixed operations and dependencies */
void process_loop(int n, int* arr1, int* arr2, float* farr1, float* farr2, float* result) {
    int acc = 0;
    float f_acc = 0.0f;
    
    /* Loop-carried dependencies and complex indexing */
    for (int i = 1; i < n; ++i) {
        /* Multiple memory accesses with non-trivial indexing */
        int idx1 = i;
        int idx2 = i - 1;  /* Creates dependency on previous iteration */
        int idx3 = (2 * i) % n;
        
        /* Integer operations with loop-carried dependency */
        int temp = arr1[idx1] + arr2[idx2];
        acc = acc + temp;  /* Loop-carried dependency */
        
        /* Floating-point operations */
        float f_temp = farr1[idx1] * 1.5f + farr2[idx2];
        f_acc = f_acc + f_temp;  /* Another loop-carried dependency */
        
        /* Conditional control flow inside loop */
        if (farr1[i] > 0.5f && (arr1[i] & 0x1)) {
            /* Complex floating-point operation */
            farr2[i] = sqrtf(fabsf(farr1[i])) + 0.1f;
            
            /* Mixed integer/float operations */
            arr1[i] = (arr1[i] & 0xFF) + (int)(farr2[i] * 10.0f);
        } else {
            /* Alternative computation path */
            farr2[i] = powf(farr1[i], 1.5f);
            arr1[i] = (arr1[i] >> 2) | (arr2[i] << 3);
        }
        
        /* More complex computation with multiple dependencies */
        result[i] = (float)acc * 0.01f + f_acc * 0.5f + 
                   (float)(arr1[idx3] % 256) * 0.001f;
        
        /* Bitwise operations mixed with arithmetic */
        arr2[i] = (arr2[i] ^ (arr1[i-1] & 0xFF)) + (i * 17);
        
        /* Additional floating-point operation with dependency */
        if (i % 3 == 0) {
            farr1[i] = sinf(result[i-1] * 0.01f) + cosf(farr2[i-1]);
        }
    }
}

/* Another complex loop with different pattern */
void second_loop(int n, int* arr1, int* arr2, float* farr1, float* farr2) {
    volatile int limit = n;  /* Prevent constant propagation */
    int sum = 0;
    
    for (int i = 0; i < limit; ++i) {
        /* Complex addressing modes */
        int* ptr1 = &arr1[i];
        int* ptr2 = &arr2[(i * 3) % n];
        
        /* Pointer arithmetic and dereferencing */
        int val1 = *ptr1;
        int val2 = *ptr2;
        
        /* Data-dependent computation */
        int product = val1 * val2;
        sum += product;
        
        /* Floating-point with type conversion */
        float f_val = (float)product / 256.0f;
        
        /* Conditional with external function call */
        if (external_helper(i) % 7 == 0) {
            farr1[i] = f_val * 2.0f;
            arr1[i] = (arr1[i] + sum) & 0xFFFF;
        } else {
            farr2[i] = f_val * 0.5f;
            arr2[i] = (arr2[i] ^ sum) | 0xFF;
        }
        
        /* Additional computation to create more ILP opportunities */
        if (i > 0) {
            farr1[i] += farr2[i-1] * 0.3f;
            arr1[i] += arr2[i-1] % 128;
        }
    }
}

int main(int argc, char** argv) {
    /* Use argc to make loop bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Initialize arrays */
    int arr1[SIZE], arr2[SIZE];
    float farr1[SIZE], farr2[SIZE], result[SIZE];
    
    srand(time(NULL));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = (float)rand() / RAND_MAX;
        result[i] = 0.0f;
    }
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; ++outer) {
        /* Call first complex loop */
        process_loop(n, arr1, arr2, farr1, farr2, result);
        
        /* Call second complex loop */
        second_loop(n, arr1, arr2, farr1, farr2);
        
        /* Modify data slightly between outer iterations */
        for (int i = 0; i < n; i += 10) {
            arr1[i] += outer;
            farr1[i] += outer * 0.1f;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long int_sum = 0;
    float float_sum = 0.0f;
    
    for (int i = 0; i < n; ++i) {
        int_sum += arr1[i] + arr2[i];
        float_sum += farr1[i] + farr2[i] + result[i];
    }
    
    printf("Checksum: int_sum = %lld, float_sum = %f\n", int_sum, float_sum);
    
    return 0;
}
