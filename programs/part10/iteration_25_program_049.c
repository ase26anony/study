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

/* Function 3: Mixed operations with different data types */
void process_data(char* data, int length) {
    char* ptr = data;
    volatile char* vptr = data;  /* volatile to prevent optimization */
    int i = 0;
    
    /* Pattern with volatile pointer */
    while (i < length) {
        char val = *vptr++;
        *ptr++ = val + 1;
        i++;
    }
}

/* Function 4: Struct traversal with pointer arithmetic */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    int total = 0;
    int i = 0;
    
    /* Access struct members with pointer increment */
    while (i < n) {
        total += ptr->x + ptr->y;
        ptr++;  /* Post-increment of struct pointer */
        i++;
    }
    return total;
}

/* Function 5: Double array with stride */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    double sum = 0.0;
    int count = 0;
    
    /* Pattern with stride (ptr += 2) */
    while (count < n) {
        sum += *ptr;
        ptr += 2;  /* Stride of 2 */
        count += 2;
    }
    return sum;
}

/* Function 6: Nested loops with auto-increment */
void matrix_multiply(const int* a, const int* b, int* result, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int sum = 0;
            const int* a_ptr = a + i * size;
            const int* b_ptr = b + j;
            
            /* Inner loop with pointer increment */
            for (int k = 0; k < size; k++) {
                sum += *a_ptr++ * *b_ptr;
                b_ptr += size;  /* Move to next row in column */
            }
            result[i * size + j] = sum;
        }
    }
}

int main(void) {
    /* Declare and initialize arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SMALL_SIZE];
    
    volatile int loop_counter = SIZE;  /* volatile to prevent loop unrolling */
    
    /* Pattern 1: Index-based initialization with post-increment */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i * 2;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 0.5;
    }
    
    /* Initialize struct array */
    for (int i = 0; i < SMALL_SIZE; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    /* Call functions to trigger various patterns */
    int sum = sum_array(int_array, loop_counter);
    printf("Sum of int array: %d\n", sum);
    
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    
    process_data(char_array, SIZE);
    printf("First char after processing: %c\n", char_array[0]);
    
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    double stride_sum = sum_every_other(double_array, SIZE);
    printf("Sum of every other double: %f\n", stride_sum);
    
    /* Small matrix multiplication */
    int matrix_a[4] = {1, 2, 3, 4};
    int matrix_b[4] = {5, 6, 7, 8};
    int matrix_result[4];
    
    matrix_multiply(matrix_a, matrix_b, matrix_result, 2);
    printf("Matrix result[0]: %d\n", matrix_result[0]);
    
    /* Additional pattern: Pointer traversal with different increments */
    int* ptr = int_array;
    int* end_ptr = int_array + SIZE;
    int check_sum = 0;
    
    /* Mixed increment patterns */
    while (ptr < end_ptr) {
        check_sum += *ptr;
        ptr++;  /* Post-increment */
        
        if (ptr < end_ptr) {
            check_sum += *ptr;
            ptr += 1;  /* Explicit increment */
        }
    }
    printf("Check sum: %d\n", check_sum);
    
    /* Backward traversal with char */
    char* char_ptr = char_array + SIZE - 1;
    char reverse_chars[SIZE];
    char* rev_ptr = reverse_chars;
    
    for (int i = 0; i < SIZE; i++) {
        *rev_ptr++ = *char_ptr--;
    }
    
    return 0;
}
