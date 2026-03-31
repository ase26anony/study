#include <stdio.h>
#include <stdint.h>

#define SIZE 100
#define SMALL_SIZE 20

/* Simple struct to test different memory access patterns */
struct point {
    int x;
    int y;
    char label;
};

/* Function using pointer post-increment for forward traversal */
int sum_array(const int *arr, int n) {
    const int *ptr = arr;
    const int *end = arr + n;
    int sum = 0;
    
    /* Classic pointer post-increment pattern */
    while (ptr < end) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Function using pointer post-decrement for backward traversal */
void reverse_copy(int *dest, const int *src, int n) {
    const int *src_end = src + n - 1;
    int *dest_end = dest + n - 1;
    
    /* Pointer post-decrement in both source and destination */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function with mixed operations and volatile */
void process_chars(volatile char *data, int n) {
    volatile char *ptr = data;
    int i = 0;
    
    /* Mix of index and pointer operations */
    for (i = 0; i < n; i++) {
        char val = data[i];  /* Index access */
        *ptr++ = val + 1;    /* Pointer post-increment store */
    }
}

/* Function with struct traversal */
int sum_points(const struct point *points, int n) {
    const struct point *ptr = points;
    int total = 0;
    int i = 0;
    
    /* Loop with struct pointer increment */
    for (i = 0; i < n; i++) {
        total += ptr->x + ptr->y;
        ptr++;  /* Post-increment of struct pointer */
    }
    
    return total;
}

/* Function with double precision floating point */
double sum_doubles(const double *arr, int n) {
    const double *ptr = arr;
    const double *end = arr + n;
    double sum = 0.0;
    
    /* Double precision with pointer increment */
    while (ptr < end) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Function with stride (ptr += 2) pattern */
int sum_every_other(const int *arr, int n) {
    const int *ptr = arr;
    const int *end = arr + n;
    int sum = 0;
    
    /* Stride of 2 - may trigger different patterns */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;
    }
    
    return sum;
}

/* Nested loops with inner auto-increment */
void matrix_sum_rows(const int matrix[][10], int rows, int *result) {
    int i, j;
    
    for (i = 0; i < rows; i++) {
        const int *row_ptr = matrix[i];
        int sum = 0;
        
        /* Inner loop with pointer traversal */
        for (j = 0; j < 10; j++) {
            sum += *row_ptr++;
        }
        
        result[i] = sum;
    }
}

/* Function that prevents optimization with volatile in loop condition */
int sum_with_volatile_bound(volatile int *bound, const int *arr) {
    const int *ptr = arr;
    int sum = 0;
    volatile int *limit = bound;
    
    /* Volatile bound prevents loop unrolling */
    while (ptr < (const int *)limit) {
        sum += *ptr++;
    }
    
    return sum;
}

int main(void) {
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct point points[SMALL_SIZE];
    int matrix[5][10];
    int results[5];
    volatile int bound = SIZE;
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i;              /* Index post-increment in initialization */
        char_array[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        double_array[i] = i * 1.5;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + i;
    }
    
    /* Initialize matrix */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Test various patterns */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    printf("First element of reversed: %d\n", reversed[0]);
    
    process_chars(char_array, SIZE);
    printf("First char after processing: %c\n", (char)char_array[0]);
    
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    double double_sum = sum_doubles(double_array, SMALL_SIZE);
    printf("Sum of doubles: %.2f\n", double_sum);
    
    int stride_sum = sum_every_other(int_array, SIZE);
    printf("Sum of every other: %d\n", stride_sum);
    
    matrix_sum_rows(matrix, 5, results);
    printf("Matrix row sums: %d, %d, %d, %d, %d\n", 
           results[0], results[1], results[2], results[3], results[4]);
    
    int volatile_sum = sum_with_volatile_bound(&bound, int_array);
    printf("Sum with volatile bound: %d\n", volatile_sum);
    
    /* Additional test: mixed load/store with post-increment */
    int temp[SIZE];
    int *src = int_array;
    int *dst = temp;
    int *end = int_array + SIZE;
    
    /* Mixed load/store with post-increment */
    while (src < end) {
        *dst++ = *src++ + 1;
    }
    
    printf("First element of transformed: %d\n", temp[0]);
    
    return 0;
}
