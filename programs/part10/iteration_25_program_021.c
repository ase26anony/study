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
void process_data(char* data, int count) {
    volatile char* vptr = data;  /* volatile to prevent optimization */
    char* ptr = data;
    int i = 0;
    
    /* Mix of volatile and non-volatile accesses */
    while (i < count) {
        char val = *vptr++;      /* volatile read with post-increment */
        *ptr++ = val + 1;        /* non-volatile write with post-increment */
        i++;
    }
}

/* Function 4: Strided access with post-increment */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    int step = 2;
    
    /* Strided access - may trigger different patterns */
    while (ptr < end) {
        sum += *ptr;
        ptr += step;  /* Not simple ++, but still post-modify */
    }
    
    return sum;
}

/* Function 5: Struct array traversal */
void init_points(struct Point* points, int n) {
    struct Point* ptr = points;
    int i = 0;
    
    /* Struct access with post-increment */
    while (i < n) {
        ptr->x = i;
        ptr->y = i * 2;
        ptr->label = 'A' + (i % 26);
        ptr++;  /* Post-increment on struct pointer */
        i++;
    }
}

/* Function 6: Nested loops with auto-increment */
void matrix_sum(const int matrix[][10], int rows, int* result) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        int row_sum = 0;
        int j = 0;
        
        /* Inner loop with pointer traversal */
        while (j < 10) {
            row_sum += *row_ptr++;
            j++;
        }
        
        result[i] = row_sum;
    }
}

/* Function 7: Index-based post-increment (common pattern) */
void fill_sequence(int* arr, int n) {
    /* Classic for loop with array[index++] */
    for (int i = 0; i < n; ) {
        arr[i++] = i;  /* Post-increment of index */
    }
}

/* Function 8: Backward traversal with char array */
int find_last_match(const char* str, int len, char target) {
    const char* ptr = str + len - 1;
    int position = len - 1;
    
    /* Backward search with post-decrement */
    while (position >= 0) {
        if (*ptr-- == target) {  /* Post-decrement in condition */
            return position;
        }
        position--;
    }
    
    return -1;
}

int main(void) {
    /* Declare and initialize arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[5][10];
    int results[5];
    
    /* Initialize arrays with some data */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i % 50;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 1.5;
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Test 1: Forward pointer traversal with post-increment */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* Test 2: Reverse copy with post-decrement */
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    printf("First element of reversed: %d\n", reversed[0]);
    
    /* Test 3: Mixed volatile/non-volatile operations */
    process_data(char_array, SIZE);
    printf("Processed char at index 10: %c\n", char_array[10]);
    
    /* Test 4: Strided access */
    double dsum = sum_every_other(double_array, SIZE);
    printf("Sum of every other double: %.2f\n", dsum);
    
    /* Test 5: Struct traversal */
    init_points(points, SMALL_SIZE);
    printf("First point: (%d, %d, %c)\n", 
           points[0].x, points[0].y, points[0].label);
    
    /* Test 6: Nested loops */
    matrix_sum(matrix, 5, results);
    printf("Matrix row sums: %d, %d, %d, %d, %d\n",
           results[0], results[1], results[2], results[3], results[4]);
    
    /* Test 7: Index-based post-increment */
    int seq_array[SMALL_SIZE];
    fill_sequence(seq_array, SMALL_SIZE);
    printf("Sequence array[5] = %d\n", seq_array[5]);
    
    /* Test 8: Backward char search */
    const char* test_str = "Hello, World!";
    int pos = find_last_match(test_str, 13, 'o');
    printf("Last 'o' at position: %d\n", pos);
    
    /* Additional test: Multiple operations in same loop */
    int src[SIZE], dst[SIZE];
    int* s = src;
    int* d = dst;
    volatile int* vs = src;  /* volatile pointer */
    
    for (int i = 0; i < SIZE; i++) {
        src[i] = i * 2;
    }
    
    /* Complex pattern: read from volatile, write to non-volatile */
    for (int i = 0; i < SIZE; i++) {
        int val = *vs++;    /* volatile read with post-inc */
        *d++ = val + *s++;  /* two non-volatile accesses with post-inc */
    }
    
    printf("Final test result: %d\n", dst[SIZE-1]);
    
    return 0;
}
