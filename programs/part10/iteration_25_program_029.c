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
int count_chars(const char* str, char target) {
    const char* ptr = str;
    int count = 0;
    
    /* char pointer with post-increment */
    while (*ptr != '\0') {
        if (*ptr++ == target) {
            count++;
        }
    }
    
    return count;
}

/* Function 6: Struct array traversal */
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

/* Function 7: Double array with stride */
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

/* Function 8: Nested loops with auto-increment */
void matrix_sum_rows(const int matrix[][SMALL_SIZE], int rows, int* row_sums) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        const int* row_end = row_ptr + SMALL_SIZE;
        int sum = 0;
        
        /* Inner loop with pointer post-increment */
        while (row_ptr < row_end) {
            sum += *row_ptr++;
        }
        
        row_sums[i] = sum;
    }
}

/* Function 9: Index-based with post-increment */
void initialize_array(int* arr, int n) {
    /* Classic for-loop with array[index++] */
    for (int i = 0; i < n; i++) {
        arr[i] = i * 2;  /* May generate different pattern */
    }
}

/* Function 10: Mixed pre and post operations */
void mixed_increment(int* arr, int n) {
    int* ptr = arr;
    
    /* Mix of operations to test pattern recognition */
    for (int i = 0; i < n; i++) {
        /* Store with post-increment */
        *ptr++ = i;
        
        /* Additional store without increment */
        if (i % 3 == 0) {
            *ptr = -1;
        }
    }
}

int main() {
    /* Declare and initialize arrays of different types */
    int int_array[SIZE];
    int int_array2[SIZE];
    volatile int volatile_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point point_array[SMALL_SIZE];
    int matrix[SMALL_SIZE][SMALL_SIZE];
    int row_sums[SMALL_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        volatile_array[i] = i * 2;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 1.5;
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        point_array[i].x = i;
        point_array[i].y = i * 2;
        point_array[i].label = 'A' + i;
        
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Call functions to trigger various patterns */
    
    /* 1. Forward pointer traversal with post-increment */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int_array: %d\n", sum1);
    
    /* 2. Reverse copy with post-decrement */
    reverse_copy(int_array2, int_array, SIZE);
    printf("First element after reverse copy: %d\n", int_array2[0]);
    
    /* 3. Mixed load/store */
    scale_array(int_array, SIZE, 3);
    printf("First element after scaling: %d\n", int_array[0]);
    
    /* 4. Volatile access */
    int sum_volatile = volatile_sum(volatile_array, SIZE);
    printf("Sum of volatile array: %d\n", sum_volatile);
    
    /* 5. Char array traversal */
    int count = count_chars(char_array, 'C');
    printf("Count of 'C' in char_array: %d\n", count);
    
    /* 6. Struct array traversal */
    int point_sum = sum_points(point_array, SMALL_SIZE);
    printf("Sum of x coordinates: %d\n", point_sum);
    
    /* 7. Double array with stride */
    double sum_stride = sum_every_other(double_array, SIZE);
    printf("Sum of every other double: %.2f\n", sum_stride);
    
    /* 8. Nested loops */
    matrix_sum_rows(matrix, SMALL_SIZE, row_sums);
    printf("First row sum: %d\n", row_sums[0]);
    
    /* 9. Index-based initialization */
    initialize_array(int_array, SIZE);
    printf("Re-initialized first element: %d\n", int_array[0]);
    
    /* 10. Mixed operations */
    mixed_increment(int_array, SIZE);
    printf("After mixed operations, element at index 3: %d\n", int_array[3]);
    
    /* Prevent dead code elimination */
    volatile int dummy = sum1 + sum_volatile + count + point_sum + (int)sum_stride;
    
    return 0;
}
