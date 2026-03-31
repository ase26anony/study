#include <stdio.h>
#include <stdint.h>

#define SIZE 100
#define SMALL_SIZE 20

// Simple struct to test different memory access patterns
struct point {
    int x;
    int y;
    char label;
};

// Function using pointer post-increment for forward traversal
int sum_array(const int* arr, int n) {
    const int* ptr = arr;
    const int* end = arr + n;
    int sum = 0;
    
    // Classic pointer post-increment pattern
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

// Function using pointer post-decrement for backward traversal
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    // Pointer post-decrement in both source and destination
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

// Function with mixed load/store and post-increment
void scale_array(int* arr, int n, int factor) {
    int* ptr = arr;
    int* end = arr + n;
    
    // Store operation with post-increment
    while (ptr < end) {
        *ptr++ *= factor;
    }
}

// Function with volatile pointer to prevent optimization
int volatile_sum(volatile int* arr, int n) {
    volatile int* ptr = arr;
    volatile int* end = arr + n;
    int sum = 0;
    
    // Volatile access with post-increment
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

// Function with different data types
double sum_doubles(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    
    // Double precision with post-increment
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

// Function with struct traversal
int sum_points(const struct point* points, int n) {
    const struct point* ptr = points;
    const struct point* end = points + n;
    int total_x = 0;
    
    // Struct access with post-increment
    while (ptr < end) {
        total_x += ptr->x;
        ptr++;
    }
    return total_x;
}

// Function with index-based post-increment
void initialize_with_index(int* arr, int n) {
    // Index-based post-increment - may generate different patterns
    for (int i = 0; i < n; i++) {
        arr[i] = i * 2;
    }
}

// Function with stride (ptr += 2)
int sum_every_other(const int* arr, int n) {
    const int* ptr = arr;
    const int* end = arr + n;
    int sum = 0;
    
    // Stride of 2
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;
    }
    return sum;
}

// Nested loop with inner auto-increment
void matrix_sum_rows(const int matrix[][10], int rows, int* row_sums) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        const int* row_end = row_ptr + 10;
        int sum = 0;
        
        // Inner loop with pointer post-increment
        while (row_ptr < row_end) {
            sum += *row_ptr++;
        }
        row_sums[i] = sum;
    }
}

// Function with char array and post-increment
int count_chars(const char* str) {
    const char* ptr = str;
    int count = 0;
    
    // Char access with post-increment
    while (*ptr != '\0') {
        if (*ptr++ == 'a') {
            count++;
        }
    }
    return count;
}

int main() {
    // Declare and initialize arrays of different types
    int int_array[SIZE];
    int dest_array[SIZE];
    volatile int volatile_array[SMALL_SIZE];
    double double_array[SIZE];
    struct point points[SMALL_SIZE];
    char char_array[] = "test string with multiple a characters";
    int matrix[5][10];
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        double_array[i] = (i + 1) * 1.5;
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        volatile_array[i] = i * 3;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    // Initialize matrix
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    // Call functions to trigger different patterns
    
    // 1. Pointer post-increment (forward traversal)
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    // 2. Pointer post-decrement (backward traversal)
    reverse_copy(dest_array, int_array, SIZE);
    printf("First element of reversed copy: %d\n", dest_array[0]);
    
    // 3. Store with post-increment
    scale_array(int_array, SIZE, 2);
    printf("Scaled element at index 10: %d\n", int_array[10]);
    
    // 4. Volatile access with post-increment
    int sum2 = volatile_sum(volatile_array, SMALL_SIZE);
    printf("Volatile sum: %d\n", sum2);
    
    // 5. Different data type (double)
    double sum3 = sum_doubles(double_array, SIZE);
    printf("Sum of doubles: %.2f\n", sum3);
    
    // 6. Struct traversal
    int sum4 = sum_points(points, SMALL_SIZE);
    printf("Sum of point x values: %d\n", sum4);
    
    // 7. Index-based post-increment
    initialize_with_index(dest_array, SIZE);
    printf("Element at index 5 after init: %d\n", dest_array[5]);
    
    // 8. Stride pattern
    int sum5 = sum_every_other(int_array, SIZE);
    printf("Sum of every other element: %d\n", sum5);
    
    // 9. Nested loop with inner auto-increment
    int row_sums[5];
    matrix_sum_rows(matrix, 5, row_sums);
    printf("Sum of first matrix row: %d\n", row_sums[0]);
    
    // 10. Char array with post-increment
    int char_count = count_chars(char_array);
    printf("Count of 'a' characters: %d\n", char_count);
    
    // Additional pattern: Mixed increment/decrement in same loop
    int* fwd_ptr = int_array;
    int* rev_ptr = int_array + SIZE - 1;
    int mixed_sum = 0;
    
    // Mix forward and backward traversal
    for (int i = 0; i < SIZE/2; i++) {
        mixed_sum += *fwd_ptr++;
        mixed_sum += *rev_ptr--;
    }
    printf("Mixed forward/backward sum: %d\n", mixed_sum);
    
    return 0;
}
