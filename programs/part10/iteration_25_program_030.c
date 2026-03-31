#include <stdio.h>
#include <stdint.h>

#define SIZE 100
#define SMALL_SIZE 10

struct Point {
    int x;
    int y;
    char label;
};

/* Function 1: Forward traversal with pointer post-increment */
int sum_array(const int* arr, int n) {
    const int* ptr = arr;
    const int* end = arr + n;
    int sum = 0;
    
    /* Classic while loop with *ptr++ pattern */
    while (ptr < end) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Copy in reverse using post-decrement */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function 3: Mixed operations with different data types */
void process_arrays(char* chars, double* doubles, struct Point* points, int n) {
    char* cptr = chars;
    double* dptr = doubles;
    struct Point* pptr = points;
    
    /* Process arrays in parallel with post-increment */
    for (int i = 0; i < n; i++) {
        /* Multiple post-increment operations */
        *cptr++ = (char)(i % 256);
        *dptr++ = (double)i * 1.5;
        pptr->x = i;
        pptr->y = i * 2;
        pptr->label = 'A' + (i % 26);
        pptr++;  /* Post-increment on struct pointer */
    }
}

/* Function 4: Strided access with post-increment by 2 */
void strided_copy(int* dest, const int* src, int n) {
    const int* s = src;
    int* d = dest;
    
    /* Copy every other element */
    for (int i = 0; i < n / 2; i++) {
        *d = *s;
        d += 2;  /* Post-increment with stride */
        s += 2;  /* Post-increment with stride */
    }
}

/* Function 5: Volatile pointer traversal */
int volatile_sum(volatile int* arr, int n) {
    volatile int* vptr = arr;
    int sum = 0;
    
    /* Using volatile to prevent certain optimizations */
    for (int i = 0; i < n; i++) {
        sum += *vptr++;
    }
    
    return sum;
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_multiply(int A[][SMALL_SIZE], int B[][SMALL_SIZE], int C[][SMALL_SIZE]) {
    for (int i = 0; i < SMALL_SIZE; i++) {
        for (int j = 0; j < SMALL_SIZE; j++) {
            C[i][j] = 0;
            for (int k = 0; k < SMALL_SIZE; k++) {
                /* Inner loop with array indexing that may use post-increment */
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

/* Function 7: Index-based post-increment */
void fill_with_index(int* arr, int n) {
    /* Traditional for loop with array[i++] pattern */
    for (int i = 0; i < n; i++) {
        arr[i] = i;  /* May generate post-increment addressing */
    }
}

/* Function 8: Mixed pre and post operations for comparison */
void mixed_increments(int* arr, int n) {
    int* ptr1 = arr;
    int* ptr2 = arr + n - 1;
    
    /* Process from both ends */
    for (int i = 0; i < n / 2; i++) {
        /* Post-increment on one pointer, pre-decrement on another */
        *ptr1++ = i;
        *--ptr2 = n - i - 1;
    }
}

int main() {
    /* Declare arrays of different types */
    int int_array[SIZE];
    int int_array2[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point point_array[SIZE];
    volatile int volatile_array[SIZE];
    
    int matrixA[SMALL_SIZE][SMALL_SIZE];
    int matrixB[SMALL_SIZE][SMALL_SIZE];
    int matrixC[SMALL_SIZE][SMALL_SIZE];
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i * 2 + 1;
        volatile_array[i] = i * 3;
    }
    
    /* Test 1: Forward traversal with post-increment */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum1: %d\n", sum1);
    
    /* Test 2: Reverse copy with post-decrement */
    reverse_copy(int_array2, int_array, SIZE);
    
    /* Test 3: Mixed data types with post-increment */
    process_arrays(char_array, double_array, point_array, SIZE);
    
    /* Test 4: Strided access */
    strided_copy(int_array, int_array2, SIZE);
    
    /* Test 5: Volatile access */
    int sum2 = volatile_sum(volatile_array, SIZE);
    printf("Sum2: %d\n", sum2);
    
    /* Test 6: Nested loops (matrix operations) */
    for (int i = 0; i < SMALL_SIZE; i++) {
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrixA[i][j] = i + j;
            matrixB[i][j] = i - j;
        }
    }
    matrix_multiply(matrixA, matrixB, matrixC);
    
    /* Test 7: Index-based post-increment */
    fill_with_index(int_array, SIZE);
    
    /* Test 8: Mixed increments */
    mixed_increments(int_array2, SIZE);
    
    /* Verify some results to prevent dead code elimination */
    int verify_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        verify_sum += int_array[i] + int_array2[i] + char_array[i];
    }
    printf("Verification sum: %d\n", verify_sum);
    
    /* Print a sample from matrix multiplication */
    printf("Matrix sample: %d\n", matrixC[0][0]);
    
    return 0;
}
