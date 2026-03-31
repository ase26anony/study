#include <stdio.h>
#include <stdint.h>

#define SIZE 100
#define SMALL_SIZE 10

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
    
    /* Classic pointer traversal with post-increment */
    while (ptr < end) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Function using pointer post-decrement for backward traversal */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Copy in reverse using post-decrement */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function with mixed operations and volatile */
void process_chars(volatile char* data, int n) {
    volatile char* ptr = data;
    int i = 0;
    
    /* Mix of pointer and index operations */
    for (i = 0; i < n; i++) {
        char val = *ptr++;  /* Post-increment load */
        *ptr = val + 1;     /* Store without increment */
    }
}

/* Function with struct traversal */
void init_points(struct Point* points, int n) {
    struct Point* ptr = points;
    int i;
    
    /* Initialize struct array with post-increment */
    for (i = 0; i < n; i++) {
        ptr->x = i;
        ptr->y = i * 2;
        ptr->label = 'A' + (i % 26);
        ptr++;  /* Explicit post-increment */
    }
}

/* Function with double array and stride-like pattern */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    int count = 0;
    
    /* Access with pointer increment in loop body */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;  /* Stride of 2 */
        count++;
        if (count >= n/2) break;
    }
    
    return sum;
}

/* Nested loops with inner auto-increment */
void matrix_sum(const int* matrix, int rows, int cols, int* row_sums) {
    int i, j;
    
    for (i = 0; i < rows; i++) {
        const int* row_ptr = matrix + i * cols;
        int sum = 0;
        
        /* Inner loop with pointer traversal */
        for (j = 0; j < cols; j++) {
            sum += *row_ptr++;
        }
        
        row_sums[i] = sum;
    }
}

/* Function to prevent optimization */
volatile int dummy_volatile = 0;

int main() {
    int int_array[SIZE];
    int dest_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[5][5];
    int row_sums[5];
    
    int i, j;
    
    /* Initialize arrays with index post-increment */
    for (i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        char_array[i] = 'a' + (i % 26);
        double_array[i] = i * 1.5;
    }
    
    /* Initialize matrix */
    int counter = 0;
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
            matrix[i][j] = counter++;
        }
    }
    
    /* Use volatile to prevent premature optimization */
    dummy_volatile = 1;
    
    /* Test different patterns */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    reverse_copy(dest_array, int_array, SIZE);
    printf("First element of reversed copy: %d\n", dest_array[0]);
    
    process_chars(char_array, SIZE);
    printf("First char after processing: %c\n", (char)char_array[1]);
    
    init_points(points, SMALL_SIZE);
    printf("First point: (%d, %d, %c)\n", points[0].x, points[0].y, points[0].label);
    
    double dsum = sum_every_other(double_array, SIZE);
    printf("Sum of every other double: %f\n", dsum);
    
    matrix_sum((const int*)matrix, 5, 5, row_sums);
    printf("Matrix row sums: %d, %d, %d, %d, %d\n", 
           row_sums[0], row_sums[1], row_sums[2], row_sums[3], row_sums[4]);
    
    /* Additional test with pointer arithmetic in loop condition */
    int* ptr = int_array;
    int* end = int_array + SIZE;
    int local_sum = 0;
    
    while (ptr != end) {
        local_sum += *ptr;
        ptr++;  /* Post-increment in loop body */
    }
    printf("Local sum: %d\n", local_sum);
    
    /* Backward traversal with char */
    char* cptr = char_array + SIZE - 1;
    char* cstart = char_array;
    int char_count = 0;
    
    while (cptr >= cstart) {
        if (*cptr-- == 'a') {  /* Post-decrement in condition */
            char_count++;
        }
    }
    printf("Found 'a' %d times in reverse traversal\n", char_count);
    
    return 0;
}
