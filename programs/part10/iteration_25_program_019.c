#include <stdio.h>
#include <string.h>

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
void reverse_copy(char* dest, const char* src, int n) {
    const char* src_end = src + n - 1;
    char* dest_end = dest + n - 1;
    
    /* Copy in reverse using *dest-- = *src-- */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function 3: Mixed operations with different strides */
void process_doubles(double* arr, int n) {
    double* ptr = arr;
    double* end = arr + n;
    
    /* Forward traversal with stride of 1 */
    while (ptr < end) {
        *ptr = (*ptr) * 2.0;
        ptr++;  /* Post-increment in separate statement */
    }
    
    /* Backward traversal */
    ptr = end - 1;
    while (ptr >= arr) {
        *ptr = (*ptr) / 2.0;
        ptr--;  /* Post-decrement in separate statement */
    }
}

/* Function 4: Struct traversal with pointer arithmetic */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    int total = 0;
    int i = 0;
    
    /* Using array index with post-increment */
    while (i < n) {
        total += points[i].x + points[i].y;
        i++;  /* Post-increment of index */
    }
    
    /* Reset and use pointer directly */
    ptr = points;
    for (i = 0; i < n; i++) {
        total += ptr->x - ptr->y;
        ptr++;  /* Post-increment of struct pointer */
    }
    
    return total;
}

/* Function 5: Volatile access pattern */
void volatile_access(volatile char* buffer, int n) {
    volatile char* ptr = buffer;
    int i;
    
    /* Volatile read with post-increment */
    for (i = 0; i < n; i++) {
        char val = *ptr;
        ptr++;  /* Keep increment separate to match pattern */
        (void)val;  /* Use value to prevent elimination */
    }
    
    /* Volatile write with post-increment */
    ptr = buffer;
    for (i = 0; i < n; i++) {
        *ptr = (char)(i & 0xFF);
        ptr++;
    }
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_multiply(const int* a, const int* b, int* result, int n) {
    int i, j, k;
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            int sum = 0;
            const int* a_ptr = a + i * n;
            const int* b_ptr = b + j;
            
            /* Inner loop with pointer increment */
            for (k = 0; k < n; k++) {
                sum += *a_ptr * *b_ptr;
                a_ptr++;  /* Post-increment */
                b_ptr += n;  /* Stride through column */
            }
            
            /* Store result with index post-increment */
            result[i * n + j] = sum;
        }
    }
}

/* Function 7: Initialize array with index post-increment */
void init_buffer(int* buffer, int n) {
    int i;
    
    /* Classic for loop with array[i++] */
    for (i = 0; i < n; ) {
        buffer[i] = i * 2;
        i++;  /* Post-increment in loop body */
    }
    
    /* Alternative: post-increment in array index */
    for (i = 0; i < n; i++) {
        buffer[i] = buffer[i] + 1;  /* Read and write with same index */
    }
}

int main() {
    /* Declare and initialize arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct Point points[SMALL_SIZE];
    volatile char volatile_buffer[SIZE];
    
    int i;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        char_array[i] = 'A' + (i % 26);
        if (i < SIZE) volatile_buffer[i] = 0;
    }
    
    for (i = 0; i < SMALL_SIZE; i++) {
        double_array[i] = (double)i / 2.0;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + i;
    }
    
    /* Test different patterns */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    char reversed[SIZE];
    reverse_copy(reversed, char_array, SIZE);
    printf("First char reversed: %c\n", reversed[0]);
    
    process_doubles(double_array, SMALL_SIZE);
    printf("Processed double[0]: %f\n", double_array[0]);
    
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    volatile_access(volatile_buffer, SIZE);
    printf("Volatile buffer[0]: %d\n", (int)volatile_buffer[0]);
    
    /* Small matrix multiplication */
    const int n = 3;
    int a[9] = {1,2,3,4,5,6,7,8,9};
    int b[9] = {9,8,7,6,5,4,3,2,1};
    int result[9];
    matrix_multiply(a, b, result, n);
    printf("Matrix result[0]: %d\n", result[0]);
    
    /* Initialize buffer with post-increment patterns */
    int buffer[SIZE];
    init_buffer(buffer, SIZE);
    printf("Buffer[0]: %d\n", buffer[0]);
    
    /* Additional test: pointer with stride */
    int* ptr = int_array;
    int stride_sum = 0;
    for (i = 0; i < SIZE / 2; i++) {
        stride_sum += *ptr;
        ptr += 2;  /* Stride of 2 */
    }
    printf("Stride sum: %d\n", stride_sum);
    
    return 0;
}
