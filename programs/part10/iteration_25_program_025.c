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
void process_data(char* data, int count) {
    volatile char* vptr = data;  /* volatile to prevent over-optimization */
    char* ptr = data;
    int i = 0;
    
    /* Mix of volatile and non-volatile accesses */
    while (i < count) {
        char val = *vptr++;      /* volatile read with post-increment */
        *ptr++ = val + 1;        /* non-volatile write with post-increment */
        i++;
    }
}

/* Function 4: Struct array traversal */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    int total = 0;
    int i = 0;
    
    /* Prevent loop unrolling with volatile counter */
    volatile int limit = n;
    
    while (i < limit) {
        total += ptr->x + ptr->y;
        ptr++;  /* Pointer increment after access */
        i++;
    }
    
    return total;
}

/* Function 5: Double array with stride */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    int step = 2;  /* Non-1 stride */
    
    /* Loop with explicit stride */
    while (ptr < end) {
        sum += *ptr;
        ptr += step;  /* This might trigger different pattern */
    }
    
    return sum;
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_sum(const int matrix[][SMALL_SIZE], int rows, int cols, int* result) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        int row_sum = 0;
        
        /* Inner loop with pointer traversal */
        for (int j = 0; j < cols; j++) {
            row_sum += *row_ptr++;
        }
        
        result[i] = row_sum;
    }
}

/* Function 7: Index-based post-increment */
void fill_with_indices(int* arr, int n) {
    /* Classic for loop with array[index++] pattern */
    for (int i = 0; i < n; i++) {
        arr[i] = i;  /* May generate index-based addressing */
    }
}

/* Function 8: Backward fill with post-decrement */
void fill_backwards(char* arr, int n) {
    char* ptr = arr + n - 1;
    char value = 'z';
    
    while (ptr >= arr) {
        *ptr-- = value--;
    }
}

int main() {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SIZE];
    int matrix[SMALL_SIZE][SMALL_SIZE];
    int results[SMALL_SIZE];
    
    /* Initialize arrays */
    fill_with_indices(int_array, SIZE);
    
    for (int i = 0; i < SIZE; i++) {
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
    
    /* Call functions to trigger various patterns */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    printf("First element of reversed: %d\n", reversed[0]);
    
    process_data(char_array, SIZE);
    printf("Processed char: %c\n", char_array[0]);
    
    int point_sum = sum_points(points, SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    double alt_sum = sum_every_other(double_array, SIZE);
    printf("Sum of every other double: %.2f\n", alt_sum);
    
    matrix_sum(matrix, SMALL_SIZE, SMALL_SIZE, results);
    printf("Matrix row sums: ");
    for (int i = 0; i < SMALL_SIZE; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    fill_backwards(char_array, SIZE);
    printf("Last char after backward fill: %c\n", char_array[SIZE-1]);
    
    /* Additional volatile pointer loop */
    volatile int* volatile_ptr = int_array;
    int volatile_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        volatile_sum += *volatile_ptr++;
    }
    printf("Volatile sum: %d\n", volatile_sum);
    
    return 0;
}
