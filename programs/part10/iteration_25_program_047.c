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
    
    /* Classic post-increment pattern */
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Post-decrement pattern */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function 3: Mixed operations with different strides */
void process_chars(char* data, int n) {
    char* ptr = data;
    volatile char* vptr = data;  /* volatile to prevent optimization */
    int i = 0;
    
    /* Post-increment with stride of 2 */
    while (i < n) {
        char val = *ptr;
        ptr += 2;  /* Stride - may trigger different pattern */
        i += 2;
    }
    
    /* Simple post-increment with volatile */
    for (i = 0; i < n; i++) {
        char dummy = *vptr++;
        (void)dummy;  /* Use result to prevent elimination */
    }
}

/* Function 4: Struct traversal with post-increment */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    int total = 0;
    int i = 0;
    
    /* Post-increment on struct pointer */
    while (i++ < n) {
        total += ptr->x + ptr->y;
        ptr++;
    }
    return total;
}

/* Function 5: Double array with index post-increment */
double average_doubles(const double* arr, int n) {
    double sum = 0.0;
    int index = 0;
    
    /* Index-based post-increment */
    while (index < n) {
        sum += arr[index++];
    }
    return n > 0 ? sum / n : 0.0;
}

/* Function 6: Store operations with post-increment */
void initialize_buffer(int* buffer, int n, int value) {
    int* ptr = buffer;
    int* end = buffer + n;
    
    /* Store with post-increment */
    while (ptr < end) {
        *ptr++ = value++;
    }
}

/* Function 7: Nested loops with inner auto-increment */
void matrix_sum(const int matrix[][10], int rows, int* result) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        int sum = 0;
        int j = 0;
        
        /* Inner loop with pointer traversal */
        while (j < 10) {
            sum += *row_ptr++;
            j++;
        }
        result[i] = sum;
    }
}

/* Function 8: Complex pattern with multiple increments */
void complex_pattern(int* dest, const int* src1, const int* src2, int n) {
    int* d = dest;
    const int* s1 = src1;
    const int* s2 = src2;
    
    /* Multiple post-increments in same loop */
    for (int i = 0; i < n; i++) {
        *d++ = *s1++ + *s2++;
    }
}

int main(void) {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[5][10];
    int dest_array[SIZE];
    int src_array[SIZE];
    
    volatile int counter = 0;  /* Prevent loop unrolling */
    
    /* 1. Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i;
        char_array[i] = (char)(i % 256);
        src_array[i] = SIZE - i;
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
    
    /* 2. Call functions with different patterns */
    
    /* Pattern 1: Simple post-increment load */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* Pattern 2: Post-decrement store */
    reverse_copy(dest_array, src_array, SIZE);
    printf("First element after reverse copy: %d\n", dest_array[0]);
    
    /* Pattern 3: Volatile access with post-increment */
    process_chars(char_array, SIZE);
    
    /* Pattern 4: Struct access with post-increment */
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* Pattern 5: Index-based post-increment */
    double avg = average_doubles(double_array, SMALL_SIZE);
    printf("Average of doubles: %.2f\n", avg);
    
    /* Pattern 6: Store with post-increment */
    initialize_buffer(int_array, 50, counter++);
    
    /* Pattern 7: Nested loops */
    int row_sums[5];
    matrix_sum(matrix, 5, row_sums);
    printf("Matrix row sums: %d, %d, %d, %d, %d\n", 
           row_sums[0], row_sums[1], row_sums[2], row_sums[3], row_sums[4]);
    
    /* Pattern 8: Multiple post-increments */
    complex_pattern(dest_array, int_array, src_array, 50);
    
    /* 3. Additional loop patterns in main */
    
    /* Mixed index and pointer operations */
    int* ptr = int_array;
    for (int i = 0; i < SIZE; i++) {
        /* Access with index, modify pointer separately */
        int val = int_array[i];
        *ptr = val * 2;
        ptr++;  /* Post-increment separate from access */
    }
    
    /* Backward traversal with separate decrement */
    char* cptr = char_array + SIZE - 1;
    for (int i = SIZE - 1; i >= 0; i--) {
        char_array[i] = *cptr;
        cptr--;  /* Post-decrement separate from access */
    }
    
    /* Use results to prevent dead code elimination */
    volatile int check = dest_array[0] + char_array[0];
    (void)check;
    
    return 0;
}
