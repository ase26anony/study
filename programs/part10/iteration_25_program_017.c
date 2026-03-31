#include <stdio.h>
#include <stdint.h>

#define SIZE 100
#define SMALL_SIZE 20

/* Simple struct to test different data types */
struct point {
    int x;
    int y;
    char label;
};

/* Function using pointer post-increment for forward traversal */
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

/* Function using pointer post-decrement for backward traversal */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Post-decrement pattern: *dest-- = *src-- */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function with mixed load/store and post-increment */
void scale_array(int* arr, int n, int factor) {
    int* ptr = arr;
    int* end = arr + n;
    
    /* Both load and store with post-increment */
    while (ptr < end) {
        *ptr = *ptr * factor;
        ptr++;  /* Separate increment statement */
    }
}

/* Function with volatile pointer to prevent optimization */
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

/* Function with different data types */
double sum_doubles(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    
    /* Double precision post-increment */
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

/* Function with struct traversal */
int sum_point_x(const struct point* points, int n) {
    const struct point* ptr = points;
    const struct point* end = points + n;
    int sum = 0;
    
    /* Struct access with post-increment */
    while (ptr < end) {
        sum += ptr->x;
        ptr++;
    }
    return sum;
}

/* Function with char array and stride */
int count_chars(const char* str, char target) {
    const char* ptr = str;
    int count = 0;
    
    /* Char access with post-increment */
    while (*ptr != '\0') {
        if (*ptr == target) {
            count++;
        }
        ptr++;
    }
    return count;
}

/* Function with nested loops */
void matrix_sum_rows(const int matrix[][10], int rows, int* sums) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        const int* row_end = row_ptr + 10;
        int sum = 0;
        
        /* Inner loop with post-increment */
        while (row_ptr < row_end) {
            sum += *row_ptr++;
        }
        sums[i] = sum;
    }
}

/* Function with index-based post-increment */
void fill_with_index(int* arr, int n) {
    /* Index-based post-increment: arr[i++] */
    for (int i = 0; i < n; ) {
        arr[i] = i;
        i++;  /* Post-increment separate */
    }
}

/* Function with pointer arithmetic stride */
void stride_copy(int* dest, const int* src, int n, int stride) {
    const int* src_ptr = src;
    int* dest_ptr = dest;
    const int* src_end = src + n * stride;
    
    /* Stride access - may trigger different patterns */
    while (src_ptr < src_end) {
        *dest_ptr++ = *src_ptr;
        src_ptr += stride;
    }
}

int main() {
    /* Declare and initialize arrays of different types */
    int int_array[SIZE];
    int int_array2[SIZE];
    volatile int volatile_array[SIZE];
    double double_array[SIZE];
    char char_array[SIZE];
    struct point points[SMALL_SIZE];
    int matrix[5][10];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i % 50;
        volatile_array[i] = i % 30;
        double_array[i] = i * 0.5;
        char_array[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        points[i].x = i * 2;
        points[i].y = i * 3;
        points[i].label = 'A' + i;
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Test various patterns to trigger auto-inc-dec optimization */
    
    /* 1. Pointer post-increment (forward traversal) */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int_array: %d\n", sum1);
    
    /* 2. Pointer post-decrement (backward traversal) */
    reverse_copy(int_array2, int_array, SIZE);
    printf("First element of reversed copy: %d\n", int_array2[0]);
    
    /* 3. Mixed load/store with post-increment */
    scale_array(int_array, SIZE, 2);
    printf("Scaled element at index 10: %d\n", int_array[10]);
    
    /* 4. Volatile access with post-increment */
    int sum2 = volatile_sum(volatile_array, SIZE);
    printf("Volatile sum: %d\n", sum2);
    
    /* 5. Different data type (double) */
    double sum3 = sum_doubles(double_array, SIZE);
    printf("Sum of doubles: %.2f\n", sum3);
    
    /* 6. Struct traversal */
    int sum4 = sum_point_x(points, SMALL_SIZE);
    printf("Sum of point.x: %d\n", sum4);
    
    /* 7. Char array traversal */
    int count = count_chars(char_array, 'C');
    printf("Count of 'C' in char_array: %d\n", count);
    
    /* 8. Nested loops */
    int row_sums[5];
    matrix_sum_rows(matrix, 5, row_sums);
    printf("Sum of first row: %d\n", row_sums[0]);
    
    /* 9. Index-based post-increment */
    int filled_array[SMALL_SIZE];
    fill_with_index(filled_array, SMALL_SIZE);
    printf("Filled array element 5: %d\n", filled_array[5]);
    
    /* 10. Stride access */
    int stride_dest[50];
    stride_copy(stride_dest, int_array, 50, 2);
    printf("Stride copy first element: %d\n", stride_dest[0]);
    
    /* Additional test: direct loop with post-increment in condition */
    int direct_sum = 0;
    int* ptr = int_array;
    for (int i = 0; i < 10; i++) {
        direct_sum += *ptr++;
    }
    printf("Direct sum: %d\n", direct_sum);
    
    /* Test with different loop structures */
    int another_sum = 0;
    int* p = int_array;
    int* end_p = int_array + 20;
    do {
        another_sum += *p++;
    } while (p < end_p);
    printf("Do-while sum: %d\n", another_sum);
    
    return 0;
}
