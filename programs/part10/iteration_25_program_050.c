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

/* Function 3: Mixed operations with volatile pointer */
void process_chars(volatile char* data, int n) {
    volatile char* ptr = data;
    volatile char* end = data + n;
    
    /* Volatile prevents some optimizations, leaving pattern intact */
    while (ptr < end) {
        char val = *ptr++;  /* Load with post-increment */
        *ptr = val + 1;     /* Store at next location */
    }
}

/* Function 4: Array initialization with index post-increment */
void init_array(double* arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = i * 1.5;  /* May generate different pattern */
    }
}

/* Function 5: Struct traversal with pointer arithmetic */
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

/* Function 6: Strided access pattern */
void strided_access(int* arr, int n, int stride) {
    int* ptr = arr;
    int* end = arr + n;
    
    /* Stride may trigger different optimization path */
    while (ptr < end) {
        *ptr = *ptr * 2;
        ptr += stride;  /* Not simple +/-1, but included for completeness */
    }
}

/* Function 7: Nested loop with inner auto-increment */
void matrix_sum(const int matrix[][SMALL_SIZE], int rows, int cols, int* result) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        const int* row_end = matrix[i] + cols;
        int row_sum = 0;
        
        /* Inner loop with pointer traversal */
        while (row_ptr < row_end) {
            row_sum += *row_ptr++;
        }
        
        result[i] = row_sum;
    }
}

int main() {
    /* Local arrays (not static/global) to encourage stack addressing */
    int arr1[SIZE];
    int arr2[SIZE];
    char char_arr[SIZE];
    double double_arr[SIZE];
    struct Point points[SIZE];
    int matrix[SMALL_SIZE][SMALL_SIZE];
    int row_sums[SMALL_SIZE];
    
    /* Initialize arrays with non-constant values to prevent pre-computation */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i * 3;
        char_arr[i] = 'A' + (i % 26);
        double_arr[i] = i * 0.5;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    /* Initialize matrix */
    for (int i = 0; i < SMALL_SIZE; i++) {
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrix[i][j] = i * SMALL_SIZE + j;
        }
    }
    
    /* Use volatile variable in loop condition to prevent optimization */
    volatile int limit = SIZE;
    
    /* Test 1: Forward traversal with post-increment */
    int sum = sum_array(arr1, limit);
    printf("Sum of array: %d\n", sum);
    
    /* Test 2: Reverse copy with post-decrement */
    reverse_copy(arr2, arr1, limit);
    printf("First element of reversed copy: %d\n", arr2[0]);
    
    /* Test 3: Volatile char processing */
    process_chars(char_arr, limit);
    printf("First char after processing: %c\n", (char)char_arr[0]);
    
    /* Test 4: Array initialization */
    init_array(double_arr, limit);
    printf("First double value: %f\n", double_arr[0]);
    
    /* Test 5: Struct traversal */
    int point_sum = sum_points(points, limit);
    printf("Sum of points: %d\n", point_sum);
    
    /* Test 6: Strided access (though not simple +/-1) */
    strided_access(arr1, limit, 2);
    printf("Element at index 2 after strided access: %d\n", arr1[2]);
    
    /* Test 7: Nested loop pattern */
    matrix_sum(matrix, SMALL_SIZE, SMALL_SIZE, row_sums);
    printf("Sum of first matrix row: %d\n", row_sums[0]);
    
    /* Additional pattern: Direct loop with post-increment index */
    int buffer[SIZE];
    for (int i = 0; i < limit; i++) {
        buffer[i] = i * 2;  /* Common array[i++] pattern */
    }
    printf("Buffer element at index 5: %d\n", buffer[5]);
    
    /* Mixed pattern in main to ensure coverage */
    int* ptr = arr1;
    int* end = arr1 + limit;
    int checksum = 0;
    
    /* Combined load and store with post-increment */
    while (ptr < end) {
        int val = *ptr;      /* Load */
        *ptr++ = val + 1;    /* Store with post-increment */
        checksum += val;
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
