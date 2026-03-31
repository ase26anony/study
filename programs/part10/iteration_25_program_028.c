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

/* Function 3: Mixed operations with volatile */
void process_chars(volatile char* data, int n) {
    volatile char* ptr = data;
    volatile char* end = data + n;
    
    /* Volatile pointer with post-increment */
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

/* Function 6: Stride pattern (ptr += 2) */
int sum_every_other(const int* arr, int n) {
    const int* ptr = arr;
    const int* end = arr + n;
    int sum = 0;
    
    /* Stride of 2 - may trigger different pattern */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;
    }
    return sum;
}

/* Function 7: Nested loops with inner auto-increment */
void matrix_sum(const int* matrix, int rows, int cols, int* result) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix + i * cols;
        const int* row_end = row_ptr + cols;
        int row_sum = 0;
        
        /* Inner loop with pointer increment */
        while (row_ptr < row_end) {
            row_sum += *row_ptr++;
        }
        result[i] = row_sum;
    }
}

/* Function 8: Store operations with post-increment */
void fill_pattern(int* arr, int n, int start) {
    int* ptr = arr;
    int* end = arr + n;
    int value = start;
    
    /* Store with post-increment */
    while (ptr < end) {
        *ptr++ = value++;
    }
}

int main(void) {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[5][10];
    int result_array[5];
    
    /* Initialize arrays with some values */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i % 50;
        char_array[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    /* Test 1: Forward traversal with post-increment */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test 2: Reverse copy with post-decrement */
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    
    /* Test 3: Volatile char processing */
    process_chars(char_array, SIZE);
    
    /* Test 4: Double array initialization */
    init_array(double_array, SMALL_SIZE);
    
    /* Test 5: Struct traversal */
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* Test 6: Stride pattern */
    int stride_sum = sum_every_other(int_array, SIZE);
    printf("Sum of every other: %d\n", stride_sum);
    
    /* Test 7: Nested loops (matrix) */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    matrix_sum(&matrix[0][0], 5, 10, result_array);
    
    /* Test 8: Store operations */
    int pattern_array[SIZE];
    fill_pattern(pattern_array, SIZE, 100);
    
    /* Use volatile to prevent optimization of results */
    volatile int check_sum = sum1 + point_sum + stride_sum;
    
    /* Print some results to ensure they're used */
    printf("Check sum: %d\n", check_sum);
    
    return 0;
}
