#include <stdio.h>
#include <stdint.h>

#define SIZE 100
#define SMALL_SIZE 20

struct Point {
    int x;
    int y;
    char label;
};

/* Function using pointer post-increment for forward traversal */
int sum_array(const int* arr, int n) {
    const int* ptr = arr;
    const int* end = arr + n;
    int sum = 0;
    
    /* Classic pointer post-increment pattern */
    while (ptr < end) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Function using pointer post-decrement for backward traversal */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Pointer post-decrement in both source and destination */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function with mixed operations and volatile */
void process_chars(volatile char* data, int n) {
    volatile char* ptr = data;
    volatile char* end = data + n;
    
    /* Volatile pointer with post-increment */
    while (ptr < end) {
        char val = *ptr++;
        /* Use the value to prevent optimization */
        (void)val;
    }
}

/* Function with array index post-increment */
void initialize_array(int* arr, int n) {
    int i = 0;
    
    /* Array index with post-increment */
    while (i < n) {
        arr[i++] = i * 2;  /* Post-increment of index */
    }
}

/* Function with struct traversal using pointer arithmetic */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    const struct Point* end = points + n;
    int total = 0;
    
    /* Pointer post-increment with struct type */
    while (ptr < end) {
        total += ptr->x + ptr->y;
        ptr++;
    }
    
    return total;
}

/* Function with stride (ptr += 2) pattern */
int sum_every_other(const int* arr, int n) {
    const int* ptr = arr;
    const int* end = arr + n;
    int sum = 0;
    
    /* Pointer with stride - may trigger different pattern */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;
    }
    
    return sum;
}

/* Nested loop with inner auto-increment */
void matrix_sum(const int matrix[][10], int rows, int* result) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        const int* row_end = row_ptr + 10;
        int row_sum = 0;
        
        /* Inner loop with pointer post-increment */
        while (row_ptr < row_end) {
            row_sum += *row_ptr++;
        }
        
        result[i] = row_sum;
    }
}

/* Function with store operations using post-increment */
void fill_sequence(double* arr, int n) {
    double* ptr = arr;
    double* end = arr + n;
    double value = 1.0;
    
    /* Store with pointer post-increment */
    while (ptr < end) {
        *ptr++ = value;
        value *= 1.1;
    }
}

int main() {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[5][10];
    int results[5];
    
    /* Initialize arrays with some data */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        char_array[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Call functions to trigger various patterns */
    
    /* 1. Pointer post-increment (load) */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* 2. Pointer post-decrement (store) */
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    printf("First element of reversed: %d\n", reversed[0]);
    
    /* 3. Volatile access with post-increment */
    process_chars(char_array, SIZE);
    printf("Processed char array\n");
    
    /* 4. Array index post-increment */
    int new_array[SMALL_SIZE];
    initialize_array(new_array, SMALL_SIZE);
    printf("Initialized array[0]: %d\n", new_array[0]);
    
    /* 5. Struct traversal with pointer */
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* 6. Stride pattern */
    int stride_sum = sum_every_other(int_array, SIZE);
    printf("Sum of every other element: %d\n", stride_sum);
    
    /* 7. Nested loops */
    matrix_sum(matrix, 5, results);
    printf("Matrix row sums: %d, %d, %d, %d, %d\n", 
           results[0], results[1], results[2], results[3], results[4]);
    
    /* 8. Store with post-increment */
    fill_sequence(double_array, SMALL_SIZE);
    printf("Double array[0]: %.2f\n", double_array[0]);
    
    /* Additional pattern: mixed in main loop */
    volatile int* volatile_ptr = int_array;
    for (int i = 0; i < 10; i++) {
        int val = *volatile_ptr++;
        printf("Volatile read %d: %d\n", i, val);
    }
    
    return 0;
}
