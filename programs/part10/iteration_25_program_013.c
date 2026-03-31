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
    
    /* Classic pointer traversal with post-increment */
    while (ptr < end) {
        sum += *ptr++;  /* This should generate post-increment RTL */
    }
    
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Reverse copy using post-decrement */
    while (src_end >= src) {
        *dest_end-- = *src_end--;  /* Both pointers use post-decrement */
    }
}

/* Function 3: Mixed operations with volatile */
void process_chars(volatile char* data, int n) {
    volatile char* ptr = data;
    volatile char* end = data + n;
    
    /* Volatile pointer with post-increment */
    while (ptr < end) {
        volatile char value = *ptr++;  /* Volatile access with increment */
        (void)value;  /* Use value to avoid dead code elimination */
    }
}

/* Function 4: Struct traversal with pointer arithmetic */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    const struct Point* end = points + n;
    int total = 0;
    
    /* Struct pointer with post-increment */
    while (ptr < end) {
        total += ptr->x + ptr->y;
        ptr++;  /* Separate increment - may still generate pattern */
    }
    
    return total;
}

/* Function 5: Double array with stride */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    int count = 0;
    
    /* Pointer with stride (ptr += 2) */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;  /* Stride of 2 */
        count++;
        if (count > n/2) break;  /* Safety check */
    }
    
    return sum;
}

/* Function 6: Nested loop with inner auto-increment */
void matrix_sum_rows(const int matrix[][SMALL_SIZE], int rows, int* row_sums) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        const int* row_end = row_ptr + SMALL_SIZE;
        int sum = 0;
        
        /* Inner loop with pointer traversal */
        while (row_ptr < row_end) {
            sum += *row_ptr++;  /* Inner loop auto-increment */
        }
        
        row_sums[i] = sum;
    }
}

/* Function 7: Index-based post-increment */
void initialize_array(int* arr, int n) {
    /* Traditional index-based loop with post-increment */
    for (int i = 0; i < n; i++) {
        arr[i] = i;  /* May generate different pattern */
    }
}

/* Function 8: Store operations with auto-increment */
void fill_sequence(int* arr, int n, int start) {
    int* ptr = arr;
    int* end = arr + n;
    int value = start;
    
    /* Store with post-increment */
    while (ptr < end) {
        *ptr++ = value++;  /* Both store and value use post-increment */
    }
}

int main() {
    /* Declare and initialize arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SIZE];
    int matrix[SMALL_SIZE][SMALL_SIZE];
    int row_sums[SMALL_SIZE];
    
    /* Initialize arrays */
    initialize_array(int_array, SIZE);
    fill_sequence(int_array, SIZE, 0);
    
    for (int i = 0; i < SIZE; i++) {
        char_array[i] = (char)('A' + (i % 26));
        double_array[i] = i * 1.5;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrix[i][j] = i * SMALL_SIZE + j;
        }
    }
    
    /* Test different patterns */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    printf("First element of reversed: %d\n", reversed[0]);
    
    process_chars(char_array, SIZE);
    printf("Processed char array\n");
    
    int point_sum = sum_points(points, SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    double stride_sum = sum_every_other(double_array, SIZE);
    printf("Sum of every other double: %.2f\n", stride_sum);
    
    matrix_sum_rows(matrix, SMALL_SIZE, row_sums);
    printf("Matrix row sums: ");
    for (int i = 0; i < SMALL_SIZE; i++) {
        printf("%d ", row_sums[i]);
    }
    printf("\n");
    
    /* Additional test with volatile pointer */
    volatile int* volatile_ptr = int_array;
    volatile int volatile_sum = 0;
    for (int i = 0; i < 10; i++) {
        volatile_sum += *volatile_ptr++;
    }
    printf("Volatile sum: %d\n", volatile_sum);
    
    /* Test with different data sizes */
    short short_array[SIZE];
    for (int i = 0; i < SIZE; i++) {
        short_array[i] = (short)(i % 1000);
    }
    
    short* short_ptr = short_array;
    short* short_end = short_array + SIZE;
    short short_sum = 0;
    while (short_ptr < short_end) {
        short_sum += *short_ptr++;  /* Different element size */
    }
    printf("Short array sum: %d\n", short_sum);
    
    return 0;
}
