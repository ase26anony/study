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
    const int* end = arr + n;
    int sum = 0;
    
    /* Classic post-increment pattern: *ptr++ */
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

/* Function 2: Pointer-based backward traversal with post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_ptr = src + n - 1;
    int* dest_ptr = dest + n - 1;
    
    /* Post-decrement pattern: *dest-- = *src-- */
    while (dest_ptr >= dest) {
        *dest_ptr-- = *src_ptr--;
    }
}

/* Function 3: Mixed load/store with post-increment */
void scale_array(int* arr, int n, int factor) {
    int* ptr = arr;
    int* end = arr + n;
    
    /* Both load and store with post-increment */
    while (ptr < end) {
        int val = *ptr;      /* Load */
        *ptr++ = val * factor; /* Store with post-increment */
    }
}

/* Function 4: Volatile pointer traversal */
int volatile_sum(volatile int* arr, int n) {
    volatile int* ptr = arr;
    volatile int* end = arr + n;
    int sum = 0;
    
    /* Volatile access with post-increment */
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

/* Function 5: Different data types - char array */
int count_chars(const char* str) {
    const char* ptr = str;
    int count = 0;
    
    /* Char pointer with post-increment */
    while (*ptr != '\0') {
        if (*ptr++ == 'a') {
            count++;
        }
    }
    return count;
}

/* Function 6: Double array with stride */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    
    /* Double pointer with post-increment by 2 */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;  /* Stride of 2 */
    }
    return sum;
}

/* Function 7: Struct array traversal */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    const struct Point* end = points + n;
    int total_x = 0;
    
    /* Struct pointer with post-increment */
    while (ptr < end) {
        total_x += ptr->x;
        ptr++;  /* Post-increment of struct pointer */
    }
    return total_x;
}

/* Function 8: Nested loops with inner auto-increment */
void matrix_sum_rows(const int matrix[][10], int rows, int* sums) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        const int* row_end = row_ptr + 10;
        int sum = 0;
        
        /* Inner loop with pointer post-increment */
        while (row_ptr < row_end) {
            sum += *row_ptr++;
        }
        sums[i] = sum;
    }
}

/* Function 9: Index-based with post-increment */
void initialize_array(int* arr, int n) {
    /* Index-based post-increment: arr[i++] */
    for (int i = 0; i < n; ) {
        arr[i++] = i;  /* Post-increment of index */
    }
}

/* Function 10: Mixed patterns in same function */
void complex_traversal(int* arr1, int* arr2, int n) {
    int* src = arr1;
    int* dst = arr2;
    int* end = arr1 + n;
    
    /* Multiple post-increments in same loop */
    while (src < end) {
        int val1 = *src++;
        int val2 = *src++;
        *dst++ = val1 + val2;
    }
}

int main() {
    /* Declare and initialize arrays of different types */
    int int_array[SIZE];
    int int_array2[SIZE];
    volatile int volatile_array[SIZE];
    char char_array[] = "test string with multiple a characters";
    double double_array[SIZE];
    struct Point point_array[SMALL_SIZE];
    int matrix[5][10];
    int row_sums[5];
    
    /* Prevent over-optimization by using function calls in conditions */
    volatile int limit = SIZE;
    
    /* 1. Initialize int array with index post-increment pattern */
    initialize_array(int_array, limit);
    
    /* 2. Sum using pointer post-increment */
    int sum1 = sum_array(int_array, limit);
    printf("Sum of int array: %d\n", sum1);
    
    /* 3. Reverse copy using post-decrement */
    reverse_copy(int_array2, int_array, limit);
    
    /* 4. Scale array with mixed load/store post-increment */
    scale_array(int_array, limit, 2);
    
    /* 5. Volatile array access */
    for (int i = 0; i < limit; i++) {
        volatile_array[i] = i;
    }
    int sum2 = volatile_sum(volatile_array, limit);
    printf("Sum of volatile array: %d\n", sum2);
    
    /* 6. Char array traversal */
    int char_count = count_chars(char_array);
    printf("Count of 'a' characters: %d\n", char_count);
    
    /* 7. Initialize and traverse double array */
    for (int i = 0; i < limit; i++) {
        double_array[i] = i * 1.5;
    }
    double sum3 = sum_every_other(double_array, limit);
    printf("Sum of every other double: %f\n", sum3);
    
    /* 8. Struct array traversal */
    for (int i = 0; i < SMALL_SIZE; i++) {
        point_array[i].x = i;
        point_array[i].y = i * 2;
        point_array[i].label = 'A' + (i % 26);
    }
    int point_sum = sum_points(point_array, SMALL_SIZE);
    printf("Sum of point x values: %d\n", point_sum);
    
    /* 9. Matrix traversal with nested loops */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    matrix_sum_rows(matrix, 5, row_sums);
    printf("Matrix row sums: %d, %d, %d, %d, %d\n", 
           row_sums[0], row_sums[1], row_sums[2], row_sums[3], row_sums[4]);
    
    /* 10. Complex mixed pattern */
    complex_traversal(int_array, int_array2, limit / 2);
    
    /* Use results to prevent dead code elimination */
    int final_check = int_array2[0] + int_array[0] + sum1 + sum2 + char_count;
    printf("Final check value: %d\n", final_check);
    
    return 0;
}
