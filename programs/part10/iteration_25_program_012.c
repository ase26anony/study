#include <stdio.h>
#include <stdint.h>

#define SIZE 100
#define SMALL_SIZE 20

struct Point {
    int x;
    int y;
    char label;
};

/* Function 1: Pointer-based forward traversal with post-increment */
int sum_array(const int* arr, int n) {
    const int* ptr = arr;
    int sum = 0;
    int i = 0;
    
    /* Use while loop to avoid loop unrolling */
    while (i < n) {
        sum += *ptr++;  /* Post-increment access pattern */
        i++;
    }
    return sum;
}

/* Function 2: Reverse copy using post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_ptr = src + n - 1;
    int* dest_ptr = dest + n - 1;
    
    /* Use volatile to prevent optimization of loop counter */
    volatile int count = n;
    while (count-- > 0) {
        *dest_ptr-- = *src_ptr--;  /* Post-decrement access pattern */
    }
}

/* Function 3: Mixed operations with different data types */
void process_data(char* data, int n) {
    char* ptr = data;
    volatile char* vptr = data + n/2;  /* Volatile pointer */
    
    /* Forward traversal with post-increment */
    for (int i = 0; i < n/2; i++) {
        *ptr = *ptr + 1;
        ptr++;  /* Separate increment to test pattern matching */
    }
    
    /* Backward traversal with volatile */
    for (int i = n/2 - 1; i >= 0; i--) {
        char val = *vptr;  /* Volatile read */
        vptr--;  /* Post-decrement */
        (void)val;  /* Use value to prevent elimination */
    }
}

/* Function 4: Struct array traversal with stride */
void init_points(struct Point* points, int n) {
    struct Point* ptr = points;
    
    /* Use index with post-increment */
    for (int i = 0; i < n; i++) {
        ptr->x = i;
        ptr->y = i * 2;
        ptr->label = 'A' + (i % 26);
        ptr++;  /* Post-increment of struct pointer */
    }
}

/* Function 5: Double array with mixed access patterns */
double average_doubles(const double* arr, int n) {
    const double* ptr = arr;
    double sum = 0.0;
    volatile int j = 0;  /* Volatile to prevent optimization */
    
    while (j < n) {
        sum += *ptr++;  /* Post-increment for doubles */
        j++;
    }
    return sum / n;
}

/* Function 6: Nested loop with inner auto-increment */
void matrix_operation(int matrix[][10], int rows) {
    for (int i = 0; i < rows; i++) {
        int* row_ptr = matrix[i];
        
        /* Inner loop with pointer traversal */
        for (int j = 0; j < 10; j++) {
            *row_ptr = *row_ptr * 2 + j;
            row_ptr++;  /* Post-increment in inner loop */
        }
    }
}

/* Function 7: Array initialization with index post-increment */
void init_with_index(int arr[], int n) {
    /* Classic pattern: arr[i++] */
    for (int i = 0; i < n; ) {
        arr[i] = i * 3;
        i++;  /* Separate increment - may still be optimized */
    }
}

/* Function 8: Pointer arithmetic with stride */
void stride_access(int* arr, int n, int stride) {
    int* ptr = arr;
    int* end = arr + n;
    
    while (ptr < end) {
        *ptr = *ptr * 2;
        ptr += stride;  /* Stride different from 1 */
    }
}

int main() {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[5][10];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i % 50;
        char_array[i] = 'a' + (i % 26);
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        double_array[i] = i * 1.5;
    }
    
    /* Test different patterns */
    int sum = sum_array(int_array, SIZE);
    printf("Sum: %d\n", sum);
    
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    printf("First element of reversed: %d\n", reversed[0]);
    
    process_data(char_array, SIZE);
    printf("First char: %c\n", char_array[0]);
    
    init_points(points, SMALL_SIZE);
    printf("First point: (%d, %d, %c)\n", 
           points[0].x, points[0].y, points[0].label);
    
    double avg = average_doubles(double_array, SMALL_SIZE);
    printf("Average: %.2f\n", avg);
    
    /* Initialize matrix */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    matrix_operation(matrix, 5);
    printf("Matrix[0][0]: %d\n", matrix[0][0]);
    
    int indexed_array[SMALL_SIZE];
    init_with_index(indexed_array, SMALL_SIZE);
    printf("Indexed array[5]: %d\n", indexed_array[5]);
    
    stride_access(int_array, SIZE, 2);
    printf("Stride access test: %d\n", int_array[10]);
    
    /* Additional test: volatile array access */
    volatile int volatile_array[SMALL_SIZE];
    volatile int* vptr = volatile_array;
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        *vptr = i * 10;
        vptr++;  /* Post-increment on volatile pointer */
    }
    
    /* Read back volatile array */
    vptr = volatile_array;
    volatile int volatile_sum = 0;
    for (int i = 0; i < SMALL_SIZE; i++) {
        volatile_sum += *vptr++;
    }
    
    printf("Volatile sum: %d\n", volatile_sum);
    
    return 0;
}
