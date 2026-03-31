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

/* Function 3: Mixed operations with different strides */
void process_chars(char* data, int n) {
    char* ptr = data;
    char* end = data + n;
    
    /* Simple post-increment with store */
    while (ptr < end) {
        *ptr++ = 'A' + (ptr - data) % 26;
    }
    
    /* Now read with post-increment */
    ptr = data;
    volatile char sink;  /* volatile to prevent optimization */
    while (ptr < end) {
        sink = *ptr++;   /* Load with post-increment */
    }
}

/* Function 4: Struct traversal with pointer arithmetic */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    const struct Point* end = points + n;
    int total = 0;
    
    /* Access struct members with pointer increment */
    while (ptr < end) {
        total += ptr->x + ptr->y;
        ptr++;  /* Post-increment of struct pointer */
    }
    return total;
}

/* Function 5: Double array with index post-increment */
double average_doubles(const double* arr, int n) {
    double sum = 0.0;
    int i = 0;
    
    /* Index-based post-increment */
    while (i < n) {
        sum += arr[i++];  /* Post-increment of index */
    }
    return n > 0 ? sum / n : 0.0;
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_multiply(int a[SIZE][SIZE], int b[SIZE][SIZE], int result[SIZE][SIZE]) {
    for (int i = 0; i < SMALL_SIZE; i++) {
        for (int j = 0; j < SMALL_SIZE; j++) {
            int* res_ptr = &result[i][j];
            *res_ptr = 0;
            
            const int* a_ptr = &a[i][0];
            const int* b_ptr = &b[0][j];
            
            /* Inner loop with pointer increments */
            for (int k = 0; k < SMALL_SIZE; k++) {
                *res_ptr += *a_ptr++ * *b_ptr;
                b_ptr += SIZE;  /* Different stride */
            }
        }
    }
}

/* Function 7: Volatile pointer traversal */
void volatile_traversal(volatile int* data, int n) {
    volatile int* ptr = data;
    volatile int* end = data + n;
    
    /* Volatile prevents other optimizations, leaving pattern intact */
    while (ptr < end) {
        *ptr++ = 0xDEADBEEF;  /* Store with post-increment */
    }
}

int main(void) {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct Point points[SMALL_SIZE];
    volatile int volatile_array[SMALL_SIZE];
    
    int matrix_a[SMALL_SIZE][SMALL_SIZE];
    int matrix_b[SMALL_SIZE][SMALL_SIZE];
    int matrix_result[SMALL_SIZE][SMALL_SIZE];
    
    /* Initialize arrays with index post-increment */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i * 2;
        char_array[i] = 'a' + (i % 26);
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        double_array[i] = i * 1.5;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
        volatile_array[i] = i * 3;
    }
    
    /* Call functions to trigger different patterns */
    
    /* 1. Forward pointer traversal with post-increment */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* 2. Reverse copy with post-decrement */
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    printf("First element of reversed: %d\n", reversed[0]);
    
    /* 3. Char processing with mixed operations */
    process_chars(char_array, SIZE);
    printf("First char: %c\n", char_array[0]);
    
    /* 4. Struct traversal */
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* 5. Double array with index post-increment */
    double avg = average_doubles(double_array, SMALL_SIZE);
    printf("Average of doubles: %.2f\n", avg);
    
    /* 6. Nested loops with inner auto-increment */
    for (int i = 0; i < SMALL_SIZE; i++) {
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrix_a[i][j] = i + j;
            matrix_b[i][j] = i - j;
        }
    }
    matrix_multiply(matrix_a, matrix_b, matrix_result);
    printf("Matrix result[0][0]: %d\n", matrix_result[0][0]);
    
    /* 7. Volatile access pattern */
    volatile_traversal(volatile_array, SMALL_SIZE);
    printf("Volatile array[0]: %d\n", volatile_array[0]);
    
    /* Additional pattern: Pointer with stride */
    int* ptr = int_array;
    int stride_sum = 0;
    for (int i = 0; i < SIZE/2; i++) {
        stride_sum += *ptr;
        ptr += 2;  /* Stride of 2 */
    }
    printf("Stride sum: %d\n", stride_sum);
    
    /* Pattern with post-decrement in while */
    char* char_ptr = char_array + SIZE - 1;
    int count = SIZE;
    while (count-- > 0) {
        *char_ptr-- = 'Z';  /* Post-decrement store */
    }
    printf("Last char: %c\n", char_array[SIZE-1]);
    
    return 0;
}
