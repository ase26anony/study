#include <stdio.h>
#include <stdint.h>

#define SIZE 100
#define SMALL_SIZE 20

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
    
    /* Classic pattern: *ptr++ in while loop */
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Pattern: *dest-- = *src-- */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function 3: Mixed operations with volatile pointer */
void process_chars(volatile char* data, int n) {
    volatile char* ptr = data;
    volatile char* end = data + n;
    
    /* Volatile prevents some optimizations, leaving pattern intact */
    while (ptr < end) {
        char val = *ptr++;
        /* Use the value to prevent elimination */
        (void)val;
    }
}

/* Function 4: Array initialization with index post-increment */
void init_array(double* arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = i * 1.5;
    }
}

/* Function 5: Struct array traversal with pointer arithmetic */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    const struct Point* end = points + n;
    int total = 0;
    
    /* Access struct members with pointer increment */
    while (ptr < end) {
        total += ptr->x + ptr->y;
        ptr++;
    }
    return total;
}

/* Function 6: Strided access pattern */
void strided_copy(int* dest, const int* src, int n, int stride) {
    const int* s = src;
    int* d = dest;
    
    /* Pattern with stride - may trigger different optimization path */
    for (int i = 0; i < n; i++) {
        *d = *s;
        s += stride;
        d += stride;
    }
}

/* Function 7: Nested loops with inner auto-increment */
void matrix_sum(const int* matrix, int rows, int cols, int* row_sums) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix + i * cols;
        const int* row_end = row_ptr + cols;
        int sum = 0;
        
        /* Inner loop with pointer increment */
        while (row_ptr < row_end) {
            sum += *row_ptr++;
        }
        row_sums[i] = sum;
    }
}

/* Function 8: Multiple operations in same statement */
void complex_operation(int* a, int* b, int n) {
    int* a_ptr = a;
    int* b_ptr = b;
    int* end = a + n;
    
    /* Multiple auto-increment operations */
    while (a_ptr < end) {
        *a_ptr++ += *b_ptr++ * 2;
    }
}

int main(void) {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct Point points[SMALL_SIZE];
    int dest_array[SIZE];
    int matrix[10][10];
    int row_sums[10];
    
    /* Initialize arrays with non-constant values to prevent pre-computation */
    volatile int seed = 42;
    
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = (i * 3 + seed) % 100;
        char_array[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    /* Test 1: Forward traversal with post-increment */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* Test 2: Reverse copy with post-decrement */
    reverse_copy(dest_array, int_array, SIZE);
    
    /* Test 3: Volatile char processing */
    process_chars(char_array, SIZE);
    
    /* Test 4: Double array initialization */
    init_array(double_array, SMALL_SIZE);
    printf("Double array[0] = %f\n", double_array[0]);
    
    /* Test 5: Struct traversal */
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* Test 6: Strided access */
    int stride_array[SIZE];
    strided_copy(stride_array, int_array, SIZE/2, 2);
    
    /* Test 7: Nested loops */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    matrix_sum((const int*)matrix, 10, 10, row_sums);
    printf("Matrix row 0 sum: %d\n", row_sums[0]);
    
    /* Test 8: Complex operation */
    int src_array[SIZE];
    for (int i = 0; i < SIZE; i++) {
        src_array[i] = i;
    }
    complex_operation(int_array, src_array, SIZE);
    
    /* Final check to ensure all operations have effect */
    printf("Final int_array[0] = %d\n", int_array[0]);
    printf("Final dest_array[0] = %d\n", dest_array[0]);
    
    return 0;
}
