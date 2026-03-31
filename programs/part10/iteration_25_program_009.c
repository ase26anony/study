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
    
    /* Classic pattern: *ptr++ in loop */
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_ptr = src + n - 1;
    int* dest_ptr = dest + n - 1;
    
    /* Pattern: *dest-- = *src-- */
    for (int i = 0; i < n; i++) {
        *dest_ptr-- = *src_ptr--;
    }
}

/* Function 3: Mixed operations with different data types */
void process_structs(struct Point* points, int n) {
    struct Point* ptr = points;
    
    /* Post-increment on struct pointer */
    for (int i = 0; i < n; i++) {
        ptr->x = i;
        ptr->y = i * 2;
        ptr->label = 'A' + (i % 26);
        ptr++;  /* Post-increment separated from access */
    }
}

/* Function 4: Volatile pointer traversal */
int volatile_sum(volatile int* arr, int n) {
    volatile int* ptr = arr;
    int sum = 0;
    
    /* Volatile prevents some optimizations, may preserve pattern */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;  /* Separate increment */
    }
    return sum;
}

/* Function 5: Array initialization with index post-increment */
void init_array(double* arr, int n) {
    /* Pattern: arr[i++] = value */
    for (int i = 0; i < n; ) {
        arr[i] = i * 1.5;
        i++;  /* Post-increment in separate statement */
    }
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_sum(const int* matrix, int rows, int cols, int* result) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix + i * cols;
        int row_sum = 0;
        
        /* Inner loop with pointer traversal */
        for (int j = 0; j < cols; j++) {
            row_sum += *row_ptr++;
        }
        result[i] = row_sum;
    }
}

/* Function 7: Char array with stride (ptr += 2) */
int sum_every_other(const char* arr, int n) {
    const char* ptr = arr;
    int sum = 0;
    
    /* Stride of 2 - may trigger different pattern */
    for (int i = 0; i < n; i += 2) {
        sum += *ptr;
        ptr += 2;
    }
    return sum;
}

/* Function 8: Backward traversal with post-decrement in access */
void clear_backwards(int* arr, int n) {
    int* ptr = arr + n - 1;
    
    /* Pattern: *ptr-- = 0 */
    for (int i = 0; i < n; i++) {
        *ptr-- = 0;
    }
}

int main() {
    /* Local arrays (stack-based) */
    int int_array[SIZE];
    int dest_array[SIZE];
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[5][10];
    int row_sums[5];
    
    volatile int volatile_array[SIZE];
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        char_array[i] = (i % 26) + 'A';
        volatile_array[i] = i * 2;
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        double_array[i] = 0.0;
        points[i].x = 0;
        points[i].y = 0;
        points[i].label = '\0';
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Test different patterns */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    reverse_copy(dest_array, int_array, SIZE);
    printf("First element of reversed copy: %d\n", dest_array[0]);
    
    process_structs(points, SMALL_SIZE);
    printf("First struct point: (%d, %d, %c)\n", 
           points[0].x, points[0].y, points[0].label);
    
    int sum2 = volatile_sum(volatile_array, SIZE);
    printf("Volatile array sum: %d\n", sum2);
    
    init_array(double_array, SMALL_SIZE);
    printf("First double: %.2f\n", double_array[0]);
    
    matrix_sum(&matrix[0][0], 5, 10, row_sums);
    printf("First row sum: %d\n", row_sums[0]);
    
    int sum3 = sum_every_other(char_array, SIZE);
    printf("Sum of every other char: %d\n", sum3);
    
    clear_backwards(dest_array, SIZE);
    printf("Last element after clear: %d\n", dest_array[SIZE-1]);
    
    /* Additional pattern: index with post-increment in access */
    int buffer[SMALL_SIZE];
    for (int i = 0; i < SMALL_SIZE; ) {
        buffer[i] = i * 3;
        i++;  /* Post-increment */
    }
    printf("Buffer[5] = %d\n", buffer[5]);
    
    /* Mixed types in single loop */
    {
        int* int_ptr = int_array;
        char* char_ptr = char_array;
        for (int i = 0; i < 20; i++) {
            *int_ptr++ += *char_ptr++;
        }
        printf("Mixed access result: %d\n", int_array[0]);
    }
    
    return 0;
}
