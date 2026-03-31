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
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Post-decrement pattern: *dest-- = *src-- */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function with mixed operations and volatile */
void process_chars(volatile char* data, size_t n) {
    volatile char* ptr = data;
    volatile char* end = data + n;
    
    /* Volatile pointer with post-increment */
    while (ptr < end) {
        char val = *ptr++;
        (void)val;  /* Use the value to prevent elimination */
    }
}

/* Function with struct traversal using pointer arithmetic */
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

/* Function with stride (ptr += 2) pattern */
int sum_every_other(const int* arr, size_t n) {
    const int* ptr = arr;
    const int* end = arr + n;
    int sum = 0;
    
    /* Stride of 2 with post-increment-like pattern */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;
    }
    
    return sum;
}

/* Nested loops with inner auto-increment */
void matrix_sum(const int matrix[][SMALL_SIZE], size_t rows, int* result) {
    for (size_t i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        const int* row_end = row_ptr + SMALL_SIZE;
        int row_sum = 0;
        
        /* Inner loop with pointer post-increment */
        while (row_ptr < row_end) {
            row_sum += *row_ptr++;
        }
        
        result[i] = row_sum;
    }
}

/* Store operations with post-increment */
void initialize_array(int* arr, size_t n, int value) {
    int* ptr = arr;
    int* end = arr + n;
    
    /* Store with post-increment: *ptr++ = value */
    while (ptr < end) {
        *ptr++ = value++;
    }
}

/* Index-based traversal with post-increment */
double average_double(const double* arr, size_t n) {
    double sum = 0.0;
    size_t i = 0;
    
    /* Array index with post-increment: arr[i++] */
    while (i < n) {
        sum += arr[i++];
    }
    
    return n > 0 ? sum / n : 0.0;
}

int main(void) {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SIZE];
    int matrix[SMALL_SIZE][SMALL_SIZE];
    int results[SMALL_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 1.5;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrix[i][j] = i * SMALL_SIZE + j;
        }
    }
    
    /* Test different patterns */
    
    /* 1. Pointer post-increment (load) */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    /* 2. Pointer post-decrement (store) */
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    printf("First element of reversed: %d\n", reversed[0]);
    
    /* 3. Volatile access with post-increment */
    process_chars(char_array, SIZE);
    printf("Processed char array\n");
    
    /* 4. Struct traversal with post-increment */
    int point_sum = sum_points(points, SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* 5. Stride pattern */
    int stride_sum = sum_every_other(int_array, SIZE);
    printf("Sum of every other element: %d\n", stride_sum);
    
    /* 6. Nested loops with inner auto-increment */
    matrix_sum(matrix, SMALL_SIZE, results);
    printf("Matrix row sums calculated\n");
    
    /* 7. Store with post-increment */
    int new_array[SIZE];
    initialize_array(new_array, SIZE, 42);
    printf("Initialized array[0] = %d\n", new_array[0]);
    
    /* 8. Index-based post-increment */
    double avg = average_double(double_array, SIZE);
    printf("Average of double array: %.2f\n", avg);
    
    /* Additional mixed pattern in main */
    volatile int* volatile_ptr = int_array;
    for (int i = 0; i < 10; i++) {
        int val = *volatile_ptr++;
        printf("Volatile read %d: %d\n", i, val);
    }
    
    /* Use results to prevent dead code elimination */
    int total = sum1 + point_sum + stride_sum + results[0] + new_array[0] + (int)avg;
    printf("Final checksum: %d\n", total);
    
    return 0;
}
