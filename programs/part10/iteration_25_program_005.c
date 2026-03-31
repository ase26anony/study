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
    
    /* Classic pointer traversal with post-increment */
    while (ptr < end) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Both pointers use post-decrement */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function 3: Mixed load/store with different strides */
void process_chars(char* data, int n) {
    char* ptr = data;
    char* end = data + n;
    
    /* Simple post-increment store */
    while (ptr < end) {
        *ptr++ = 'A' + ((ptr - data) % 26);
    }
    
    /* Now read with post-increment and modify */
    ptr = data;
    while (ptr < end) {
        char c = *ptr++;
        if (c >= 'A' && c <= 'Z') {
            *(ptr - 1) = c + ('a' - 'A');
        }
    }
}

/* Function 4: Double array with index post-increment */
double average_doubles(const double* arr, int n) {
    double sum = 0.0;
    int i = 0;
    
    /* Index-based post-increment */
    while (i < n) {
        sum += arr[i++];
    }
    
    return (n > 0) ? sum / n : 0.0;
}

/* Function 5: Struct array traversal with pointer arithmetic */
void offset_points(struct Point* points, int n, int dx, int dy) {
    struct Point* ptr = points;
    struct Point* end = points + n;
    
    /* Post-increment on struct pointer */
    while (ptr < end) {
        ptr->x += dx;
        ptr->y += dy;
        ptr++;
    }
}

/* Function 6: Volatile pointer access - prevents some optimizations */
int volatile_sum(const int* arr, int n) {
    const int* ptr = arr;
    const int* end = arr + n;
    volatile int sum = 0;  /* volatile to prevent optimization */
    
    /* Volatile forces memory access pattern to remain */
    while (ptr < end) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Function 7: Nested loops with inner auto-increment */
void matrix_process(int matrix[][10], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        int* row_ptr = matrix[i];
        int* row_end = row_ptr + cols;
        
        /* Inner loop with pointer post-increment */
        while (row_ptr < row_end) {
            *row_ptr++ *= 2;
        }
    }
}

/* Function 8: Multiple operations in loop */
void complex_operation(int* a, int* b, int* c, int n) {
    int* a_ptr = a;
    int* b_ptr = b;
    int* c_ptr = c;
    int* end = a + n;
    
    /* Multiple post-increments in same loop */
    while (a_ptr < end) {
        *c_ptr++ = *a_ptr++ + *b_ptr++;
    }
}

int main(void) {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[5][10];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i * 2;
        char_array[i] = 'a' + (i % 26);
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        double_array[i] = i * 1.5;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Test different patterns */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    printf("First element of reversed: %d\n", reversed[0]);
    
    process_chars(char_array, SIZE);
    printf("First char after processing: %c\n", char_array[0]);
    
    double avg = average_doubles(double_array, SMALL_SIZE);
    printf("Average of doubles: %.2f\n", avg);
    
    offset_points(points, SMALL_SIZE, 10, -5);
    printf("First point after offset: (%d, %d)\n", points[0].x, points[0].y);
    
    int sum2 = volatile_sum(int_array, SIZE);
    printf("Volatile sum: %d\n", sum2);
    
    matrix_process(matrix, 5, 10);
    printf("Matrix[0][0] after processing: %d\n", matrix[0][0]);
    
    int a[SIZE], b[SIZE], c[SIZE];
    for (int i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = i * 3;
    }
    
    complex_operation(a, b, c, SIZE);
    printf("Complex operation result[0]: %d\n", c[0]);
    
    /* Additional test: pointer with stride */
    int* ptr = int_array;
    int stride_sum = 0;
    for (int i = 0; i < SIZE/2; i++) {
        stride_sum += *ptr;
        ptr += 2;  /* Stride of 2 */
    }
    printf("Stride sum: %d\n", stride_sum);
    
    /* Backward traversal with index */
    int back_sum = 0;
    for (int i = SIZE - 1; i >= 0; i--) {
        back_sum += int_array[i];
    }
    printf("Backward sum: %d\n", back_sum);
    
    return 0;
}
