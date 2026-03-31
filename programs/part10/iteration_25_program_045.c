#include <stdio.h>
#include <string.h>

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
    
    /* Classic pointer traversal with post-increment */
    while (ptr < end) {
        sum += *ptr++;  /* This should generate post-increment RTL */
    }
    
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(char* dest, const char* src, int n) {
    const char* src_ptr = src + n - 1;
    char* dest_ptr = dest + n - 1;
    
    /* Both pointers use post-decrement */
    while (n-- > 0) {
        *dest_ptr-- = *src_ptr--;  /* Should generate post-decrement RTL */
    }
}

/* Function 3: Mixed operations with volatile */
void process_volatile(volatile int* data, int n) {
    volatile int* ptr = data;
    volatile int* end = data + n;
    
    /* Volatile pointer with post-increment */
    while (ptr < end) {
        volatile int value = *ptr++;  /* Volatile access with increment */
        (void)value;  /* Use value to avoid warning */
    }
}

/* Function 4: Struct traversal with pointer arithmetic */
int sum_points(struct Point* points, int n) {
    struct Point* ptr = points;
    int total = 0;
    int i = 0;
    
    /* Using index with post-increment on struct array */
    while (i++ < n) {
        total += ptr->x + ptr->y;
        ptr++;  /* Explicit pointer increment - may be combined with access */
    }
    
    return total;
}

/* Function 5: Double array with stride */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    int count = 0;
    
    /* Loop with conditional increment - tests pattern recognition */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;  /* Stride of 2 - may trigger different pattern */
        count++;
        if (count > n/2) break;  /* Prevent infinite loop */
    }
    
    return sum;
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_sum_rows(const int matrix[][SMALL_SIZE], int rows, int* sums) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        int sum = 0;
        int j = 0;
        
        /* Inner loop with pointer traversal */
        while (j++ < SMALL_SIZE) {
            sum += *row_ptr++;  /* Inner loop auto-increment */
        }
        
        sums[i] = sum;
    }
}

int main() {
    /* Declare and initialize arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point point_array[SMALL_SIZE];
    volatile int volatile_array[SIZE];
    
    /* Initialize arrays with values */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 1.5;
        if (i < SIZE) volatile_array[i] = i * 2;
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        point_array[i].x = i;
        point_array[i].y = i * 2;
        point_array[i].label = 'A' + i;
    }
    
    /* Test 1: Simple forward traversal with post-increment */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test 2: Reverse copy with post-decrement */
    char char_copy[SIZE];
    reverse_copy(char_copy, char_array, SIZE);
    printf("First char in copy: %c\n", char_copy[0]);
    
    /* Test 3: Volatile access pattern */
    process_volatile(volatile_array, SIZE);
    printf("Processed volatile array\n");
    
    /* Test 4: Struct traversal */
    int point_sum = sum_points(point_array, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* Test 5: Stride pattern */
    double stride_sum = sum_every_other(double_array, SIZE);
    printf("Sum of every other double: %f\n", stride_sum);
    
    /* Test 6: Nested loops */
    int matrix[SMALL_SIZE][SMALL_SIZE];
    int row_sums[SMALL_SIZE];
    
    /* Initialize matrix */
    for (int i = 0; i < SMALL_SIZE; i++) {
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrix[i][j] = i * SMALL_SIZE + j;
        }
    }
    
    matrix_sum_rows(matrix, SMALL_SIZE, row_sums);
    printf("Matrix row sums calculated\n");
    
    /* Additional test: Index-based post-increment in loop */
    int buffer[SIZE];
    for (int i = 0; i < SIZE; ) {
        buffer[i] = i * 3;
        i++;  /* Post-increment separated from array access */
    }
    
    /* Use buffer to prevent elimination */
    int buffer_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        buffer_sum += buffer[i];
    }
    printf("Buffer sum: %d\n", buffer_sum);
    
    return 0;
}
