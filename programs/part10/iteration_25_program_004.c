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
    
    /* Volatile prevents some optimizations, keeping pattern intact */
    while (ptr < end) {
        char val = *ptr++;
        /* Use the value to prevent elimination */
        (void)val;
    }
}

/* Function 4: Struct traversal with pointer arithmetic */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        total += ptr->x + ptr->y;
        ptr++;  /* Post-increment of struct pointer */
    }
    
    return total;
}

/* Function 5: Double array with stride */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    
    /* Stride of 2 - may trigger different pattern */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;
    }
    
    return sum;
}

/* Function 6: Index-based post-increment */
void initialize_array(int* arr, int n) {
    /* Classic for loop with array[index++] */
    for (int i = 0; i < n; i++) {
        arr[i] = i * 2;
    }
}

/* Function 7: Nested loops with inner auto-increment */
void matrix_sum(const int* matrix, int rows, int cols, int* row_sums) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix + i * cols;
        int sum = 0;
        
        /* Inner loop with pointer traversal */
        for (int j = 0; j < cols; j++) {
            sum += *row_ptr++;
        }
        
        row_sums[i] = sum;
    }
}

/* Function 8: Store operations with post-increment */
void fill_sequence(int* arr, int n, int start) {
    int* ptr = arr;
    int* end = arr + n;
    int value = start;
    
    /* Store with post-increment */
    while (ptr < end) {
        *ptr++ = value++;
    }
}

int main() {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[5][10];
    int row_sums[5];
    
    /* Initialize arrays */
    initialize_array(int_array, SIZE);
    fill_sequence(int_array, SIZE, 0);
    
    for (int i = 0; i < SIZE; i++) {
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 0.5;
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
    
    /* Call functions to trigger optimizations */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    printf("First element of reversed: %d\n", reversed[0]);
    
    process_chars(char_array, SIZE);
    printf("Processed char array\n");
    
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    double sum2 = sum_every_other(double_array, SIZE);
    printf("Sum of every other double: %f\n", sum2);
    
    matrix_sum((const int*)matrix, 5, 10, row_sums);
    printf("Matrix row sums: %d, %d, %d, %d, %d\n", 
           row_sums[0], row_sums[1], row_sums[2], row_sums[3], row_sums[4]);
    
    /* Additional pattern: while loop with index post-increment */
    int buffer[SMALL_SIZE];
    int idx = 0;
    while (idx < SMALL_SIZE) {
        buffer[idx] = idx * 3;
        idx++;  /* Post-increment in separate statement */
    }
    
    /* Pattern: do-while with post-decrement */
    int temp = SMALL_SIZE;
    int acc = 0;
    do {
        acc += temp;
    } while (temp-- > 0);
    
    printf("Accumulated value: %d\n", acc);
    
    return 0;
}
