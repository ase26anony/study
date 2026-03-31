#include <stdio.h>
#include <stddef.h>

#define SIZE 100
#define SMALL_SIZE 10

struct Point {
    int x;
    int y;
    char label;
};

/* Function using pointer post-increment for forward traversal */
int sum_array(const int* arr, size_t n) {
    const int* ptr = arr;
    const int* end = arr + n;
    int sum = 0;
    
    /* Classic post-increment pattern: *ptr++ */
    while (ptr < end) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Function using pointer post-decrement for backward traversal */
void reverse_copy(int* dest, const int* src, size_t n) {
    const int* src_ptr = src + n - 1;
    int* dest_ptr = dest + n - 1;
    
    /* Post-decrement pattern: *dest_ptr-- = *src_ptr-- */
    while (src_ptr >= src) {
        *dest_ptr-- = *src_ptr--;
    }
}

/* Function with stride (pointer += 2) */
int sum_every_other(const int* arr, size_t n) {
    const int* ptr = arr;
    const int* end = arr + n;
    int sum = 0;
    
    /* Stride pattern: ptr += 2 */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;
    }
    
    return sum;
}

/* Function with volatile pointer */
void volatile_traverse(volatile char* arr, size_t n) {
    volatile char* ptr = arr;
    volatile char* end = arr + n;
    
    /* Volatile post-increment - prevents some optimizations */
    while (ptr < end) {
        volatile char c = *ptr++;
        (void)c;  /* Use the value to prevent dead code elimination */
    }
}

/* Function with struct traversal */
int sum_points(const struct Point* points, size_t n) {
    const struct Point* ptr = points;
    const struct Point* end = points + n;
    int total = 0;
    
    /* Post-increment with struct pointer */
    while (ptr < end) {
        total += ptr->x + ptr->y;
        ptr++;
    }
    
    return total;
}

/* Function with index post-increment */
void initialize_array(int* arr, size_t n) {
    /* Index post-increment pattern: arr[i++] */
    for (size_t i = 0; i < n; i++) {
        arr[i] = (int)i * 2;
    }
}

/* Nested loops with inner auto-increment */
void matrix_traverse(int matrix[][SMALL_SIZE], size_t rows) {
    for (size_t i = 0; i < rows; i++) {
        int* ptr = matrix[i];
        int* end = ptr + SMALL_SIZE;
        
        /* Inner loop with post-increment */
        while (ptr < end) {
            *ptr++ += i;  /* Both load and store with post-increment */
        }
    }
}

/* Function with mixed operations */
void mixed_operations(double* arr, size_t n) {
    double* ptr = arr;
    double* end = arr + n;
    
    /* Store with post-increment */
    for (size_t i = 0; i < n; i++) {
        *ptr++ = (double)i * 1.5;
    }
    
    /* Reset pointer and read back */
    ptr = arr;
    double sum = 0.0;
    
    /* Load with post-increment */
    while (ptr < end) {
        sum += *ptr++;
    }
    
    /* Use sum to prevent optimization */
    printf("Double sum: %f\n", sum);
}

int main(void) {
    /* Declare arrays of different types */
    int int_array[SIZE];
    int dest_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SIZE];
    int matrix[SMALL_SIZE][SMALL_SIZE];
    
    /* Initialize arrays with volatile to prevent pre-computation */
    volatile int seed = 42;
    
    /* Initialize int array with index post-increment pattern */
    initialize_array(int_array, SIZE);
    
    /* Sum using pointer post-increment */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* Reverse copy using post-decrement */
    reverse_copy(dest_array, int_array, SIZE);
    
    /* Sum every other element with stride */
    int stride_sum = sum_every_other(int_array, SIZE);
    printf("Sum of every other element: %d\n", stride_sum);
    
    /* Traverse char array with volatile pointer */
    for (int i = 0; i < SIZE; i++) {
        char_array[i] = (char)(i + seed);
    }
    volatile_traverse(char_array, SIZE);
    
    /* Initialize and traverse struct array */
    for (int i = 0; i < SIZE; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    int point_sum = sum_points(points, SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* Initialize matrix */
    for (int i = 0; i < SMALL_SIZE; i++) {
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrix[i][j] = i * SMALL_SIZE + j;
        }
    }
    
    /* Matrix traversal with nested loops */
    matrix_traverse(matrix, SMALL_SIZE);
    
    /* Mixed operations on double array */
    mixed_operations(double_array, SIZE);
    
    /* Additional pattern: pointer arithmetic in loop condition */
    int* ptr = int_array;
    int* end = int_array + SIZE;
    int check_sum = 0;
    
    /* Another post-increment pattern */
    while (ptr != end) {
        check_sum ^= *ptr++;  /* XOR with post-increment */
    }
    printf("Check sum: %d\n", check_sum);
    
    /* Pattern with post-increment in function argument */
    ptr = int_array;
    for (int i = 0; i < 5; i++) {
        printf("Value %d: %d\n", i, *ptr++);
    }
    
    return 0;
}
