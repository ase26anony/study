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
    
    /* Classic while loop with *ptr++ pattern */
    while (ptr < end) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(char* dest, const char* src, int n) {
    const char* src_end = src + n - 1;
    char* dest_end = dest + n - 1;
    
    /* Using *dest-- = *src-- pattern */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function 3: Mixed operations with different data types */
void process_arrays(int* int_arr, double* dbl_arr, struct Point* pt_arr, int n) {
    int* int_ptr = int_arr;
    double* dbl_ptr = dbl_arr;
    struct Point* pt_ptr = pt_arr;
    
    /* Multiple post-increment operations in same loop */
    for (int i = 0; i < n; i++) {
        /* Store with post-increment */
        *int_ptr++ = i * 2;
        
        /* Load with post-increment */
        double val = *dbl_ptr++;
        
        /* Struct access with post-increment */
        pt_ptr->x = i;
        pt_ptr->y = i * 2;
        pt_ptr->label = 'A' + (i % 26);
        pt_ptr++;
    }
}

/* Function 4: Nested loops with auto-increment */
void matrix_multiply(int* result, const int* a, const int* b, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        const int* a_row = a + i * cols;
        int* res_row = result + i * cols;
        
        for (int j = 0; j < cols; j++) {
            const int* b_col = b + j;
            int sum = 0;
            
            /* Inner loop with pointer arithmetic */
            for (int k = 0; k < cols; k++) {
                sum += a_row[k] * b_col[k * cols];
            }
            
            /* Store with index post-increment */
            res_row[j] = sum;
        }
    }
}

/* Function 5: Volatile pointer traversal */
int volatile_sum(volatile int* arr, int n) {
    volatile int* ptr = arr;
    int sum = 0;
    
    /* Using volatile to prevent certain optimizations */
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;  /* Separate increment to test pattern matching */
    }
    
    return sum;
}

/* Function 6: Strided access with post-increment */
void strided_copy(short* dest, const short* src, int n, int stride) {
    const short* s = src;
    short* d = dest;
    
    /* Strided access - may trigger different patterns */
    for (int i = 0; i < n; i++) {
        *d = *s;
        s += stride;
        d += stride;
    }
}

/* Function 7: Array initialization with index post-increment */
void init_buffer(char* buffer, int size) {
    int i = 0;
    
    /* Using index with post-increment in loop body */
    while (i < size) {
        buffer[i] = (i % 26) + 'A';
        i++;  /* Post-increment separate from array access */
    }
}

/* Function 8: Mixed pre and post operations for comparison */
void mixed_operations(int* arr, int n) {
    int* ptr = arr;
    
    /* Mix of pre and post operations */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            *ptr++ = i;      /* Post-increment */
        } else {
            *++ptr = i * 2;  /* Pre-increment - different pattern */
        }
    }
}

int main() {
    /* Declare and initialize arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point point_array[SMALL_SIZE];
    short short_array[SIZE];
    
    volatile int volatile_array[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i;
        char_array[i] = 'a' + (i % 26);
        double_array[i] = i * 1.5;
        short_array[i] = i * 3;
        volatile_array[i] = i * 2;
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        point_array[i].x = i;
        point_array[i].y = i * 2;
        point_array[i].label = 'A' + i;
    }
    
    /* Test various patterns */
    
    /* 1. Forward traversal with post-increment */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* 2. Reverse copy with post-decrement */
    char reversed[SIZE];
    reverse_copy(reversed, char_array, SIZE);
    printf("First char of reversed: %c\n", reversed[0]);
    
    /* 3. Mixed operations */
    process_arrays(int_array, double_array, point_array, SMALL_SIZE);
    printf("Processed arrays, first int: %d\n", int_array[0]);
    
    /* 4. Matrix-like operations */
    int matrix_a[SMALL_SIZE * SMALL_SIZE];
    int matrix_b[SMALL_SIZE * SMALL_SIZE];
    int matrix_result[SMALL_SIZE * SMALL_SIZE];
    
    for (int i = 0; i < SMALL_SIZE * SMALL_SIZE; i++) {
        matrix_a[i] = i;
        matrix_b[i] = i * 2;
    }
    
    matrix_multiply(matrix_result, matrix_a, matrix_b, SMALL_SIZE, SMALL_SIZE);
    printf("Matrix result[0]: %d\n", matrix_result[0]);
    
    /* 5. Volatile access */
    int volatile_sum_result = volatile_sum(volatile_array, SIZE);
    printf("Volatile sum: %d\n", volatile_sum_result);
    
    /* 6. Strided access */
    short dest_array[SIZE];
    strided_copy(dest_array, short_array, SIZE / 2, 2);
    printf("Strided copy first element: %d\n", dest_array[0]);
    
    /* 7. Index-based initialization */
    char buffer[SIZE];
    init_buffer(buffer, SIZE);
    printf("Buffer[0]: %c\n", buffer[0]);
    
    /* 8. Mixed operations */
    mixed_operations(int_array, SIZE);
    printf("After mixed operations, int_array[1]: %d\n", int_array[1]);
    
    /* Additional test: Pointer arithmetic in loop condition */
    {
        int* p = int_array;
        int* end = int_array + SIZE;
        int count = 0;
        
        while (p != end) {
            if (*p > 50) {
                count++;
            }
            p++;  /* Post-increment in loop update */
        }
        printf("Count of values > 50: %d\n", count);
    }
    
    /* Test with function parameters to force stack operations */
    {
        int local_array[SIZE];
        for (int i = 0; i < SIZE; i++) {
            local_array[i] = i * 3;
        }
        
        int local_sum = sum_array(local_array, SIZE);
        printf("Local array sum: %d\n", local_sum);
    }
    
    return 0;
}
