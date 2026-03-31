#include <stdio.h>
#include <stddef.h>

#define SIZE 100
#define SMALL_SIZE 20

/* Simple struct to test different data sizes */
struct point {
    int x;
    int y;
    char label[4];
};

/* Function using pointer post-increment for forward traversal */
int sum_array(const int* arr, int n) {
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
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Post-decrement pattern: *dest-- = *src-- */
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
        volatile char value = *ptr++;
        /* Use value to prevent elimination */
        (void)value;
    }
}

/* Function with struct traversal */
int sum_points(const struct point* points, int n) {
    const struct point* ptr = points;
    const struct point* end = points + n;
    int total = 0;
    
    /* Struct pointer post-increment */
    while (ptr < end) {
        total += ptr->x + ptr->y;
        ptr++;
    }
    return total;
}

/* Function with index-based post-increment */
void initialize_array(int* arr, int n) {
    int i = 0;
    
    /* Index with post-increment: arr[i++] */
    while (i < n) {
        arr[i++] = i * 2;
    }
}

/* Function with stride (ptr += 2) */
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
void matrix_sum(const int matrix[3][3], int result[3]) {
    for (int i = 0; i < 3; i++) {
        const int* row_ptr = matrix[i];
        const int* row_end = matrix[i] + 3;
        int row_sum = 0;
        
        /* Inner loop with pointer post-increment */
        while (row_ptr < row_end) {
            row_sum += *row_ptr++;
        }
        result[i] = row_sum;
    }
}

/* Store operations with post-increment */
void fill_array(int* dest, int value, int n) {
    int* ptr = dest;
    int* end = dest + n;
    
    /* Store with post-increment: *ptr++ = value */
    while (ptr < end) {
        *ptr++ = value++;
    }
}

int main(void) {
    /* Local arrays (not static/global) to encourage stack addressing */
    int int_array[SIZE];
    int dest_array[SIZE];
    char char_array[SMALL_SIZE];
    double double_array[SMALL_SIZE];
    struct point points[SMALL_SIZE];
    int matrix[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
    int matrix_result[3];
    
    /* Initialize arrays with some data */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 1.5;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label[0] = 'P';
        points[i].label[1] = '0' + (i % 10);
        points[i].label[2] = '\0';
    }
    
    /* Use volatile variable in loop condition to prevent over-optimization */
    volatile int loop_limit = SIZE;
    
    /* Test 1: Pointer post-increment (load) */
    int sum = sum_array(int_array, loop_limit);
    printf("Sum of array: %d\n", sum);
    
    /* Test 2: Pointer post-decrement (store) */
    reverse_copy(dest_array, int_array, loop_limit);
    printf("First element of reversed copy: %d\n", dest_array[0]);
    
    /* Test 3: Volatile pointer access */
    process_chars(char_array, SMALL_SIZE);
    printf("Processed char array\n");
    
    /* Test 4: Struct traversal */
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* Test 5: Index-based post-increment */
    int new_array[SMALL_SIZE];
    initialize_array(new_array, SMALL_SIZE);
    printf("Initialized array[5] = %d\n", new_array[5]);
    
    /* Test 6: Stride access */
    int stride_sum = sum_every_other(int_array, loop_limit);
    printf("Sum of every other element: %d\n", stride_sum);
    
    /* Test 7: Nested loops */
    matrix_sum(matrix, matrix_result);
    printf("Matrix row sums: %d, %d, %d\n", 
           matrix_result[0], matrix_result[1], matrix_result[2]);
    
    /* Test 8: Store with post-increment */
    int filled_array[SMALL_SIZE];
    fill_array(filled_array, 10, SMALL_SIZE);
    printf("Filled array[3] = %d\n", filled_array[3]);
    
    /* Test 9: Double array traversal with pointer */
    double* dptr = double_array;
    double dsum = 0.0;
    for (int i = 0; i < SMALL_SIZE; i++) {
        dsum += *dptr++;
    }
    printf("Sum of doubles: %.2f\n", dsum);
    
    /* Test 10: Mixed operations in single loop */
    int* src = int_array;
    int* dst = dest_array;
    for (int i = 0; i < 10; i++) {
        *dst++ = *src++ + 5;  /* Both load and store with post-increment */
    }
    printf("Mixed ops result[0]: %d\n", dest_array[0]);
    
    return 0;
}
