#include <stdio.h>
#include <string.h>

#define SIZE 100
#define SMALL_SIZE 10

struct Point {
    int x;
    int y;
    char label[8];
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
    const char* src_end = src + n - 1;
    char* dest_end = dest + n - 1;
    
    /* Both pointers use post-decrement */
    while (src_end >= src) {
        *dest_end-- = *src_end--;  /* Dual post-decrement pattern */
    }
}

/* Function 3: Mixed load/store with different strides */
void process_doubles(double* arr, int n) {
    double* ptr = arr;
    volatile double* vptr = (volatile double*)arr;  /* Volatile to prevent optimization */
    int i;
    
    /* Forward with stride of 2 using pointer arithmetic */
    for (i = 0; i < n - 1; i += 2) {
        double temp = *ptr;
        ptr += 2;  /* Stride of 2 - may trigger different pattern */
        *vptr = temp * 2.0;  /* Volatile store */
        vptr += 2;
    }
}

/* Function 4: Struct traversal with post-increment */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    int total = 0;
    int i = 0;
    
    /* Use volatile to prevent loop unrolling */
    volatile int limit = n;
    
    while (i < limit) {
        total += ptr->x + ptr->y;
        ptr++;  /* Post-increment of struct pointer */
        i++;
    }
    
    return total;
}

/* Function 5: Index-based post-increment in loop */
void initialize_buffer(int* buffer, int n) {
    int i;
    
    /* Classic index with post-increment access pattern */
    for (i = 0; i < n; i++) {
        buffer[i] = i * 2;  /* May generate base+offset addressing */
    }
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_sum(const int matrix[][SMALL_SIZE], int rows, int cols, int* result) {
    int i, j;
    
    for (i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        int row_sum = 0;
        
        /* Inner loop with pointer traversal */
        for (j = 0; j < cols; j++) {
            row_sum += *row_ptr++;  /* Inner loop post-increment */
        }
        
        result[i] = row_sum;
    }
}

/* Function 7: Char array with volatile pointer */
int count_chars(const char* str, int len) {
    volatile const char* vptr = str;  /* Volatile pointer */
    int count = 0;
    int i = 0;
    
    /* Loop with volatile read */
    while (i < len) {
        if (*vptr != 0) {
            count++;
        }
        vptr++;  /* Post-increment of volatile pointer */
        i++;
    }
    
    return count;
}

/* Function 8: Backward traversal with array index post-decrement */
void reverse_fill(int* arr, int n, int value) {
    int idx = n - 1;
    
    while (idx >= 0) {
        arr[idx--] = value;  /* Post-decrement of index */
        value--;  /* Change value to prevent optimization */
    }
}

int main() {
    /* Declare and initialize various arrays */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[SMALL_SIZE][SMALL_SIZE];
    int results[SMALL_SIZE];
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 1.5;
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        points[i].x = i * 10;
        points[i].y = i * 20;
        snprintf(points[i].label, sizeof(points[i].label), "P%d", i);
        
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Call functions to trigger various patterns */
    
    /* 1. Forward pointer traversal with post-increment */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* 2. Reverse copy with post-decrement */
    char char_copy[SIZE];
    reverse_copy(char_copy, char_array, SIZE);
    printf("First char in copy: %c\n", char_copy[0]);
    
    /* 3. Mixed stride pattern */
    process_doubles(double_array, SIZE);
    printf("First double: %f\n", double_array[0]);
    
    /* 4. Struct traversal */
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* 5. Index-based initialization */
    int buffer[SIZE];
    initialize_buffer(buffer, SIZE);
    printf("Buffer[10] = %d\n", buffer[10]);
    
    /* 6. Nested loops with inner auto-increment */
    matrix_sum(matrix, SMALL_SIZE, SMALL_SIZE, results);
    printf("Matrix row 0 sum: %d\n", results[0]);
    
    /* 7. Volatile pointer traversal */
    int char_count = count_chars(char_array, SIZE);
    printf("Non-zero chars: %d\n", char_count);
    
    /* 8. Backward traversal with post-decrement */
    reverse_fill(int_array, 50, 1000);
    printf("int_array[0] after reverse_fill: %d\n", int_array[0]);
    
    /* Additional pattern: Pointer arithmetic in expression */
    {
        int* ptr = buffer;
        int* end = buffer + SIZE;
        int product = 1;
        
        /* Complex expression with post-increment */
        while (ptr < end) {
            product *= (*ptr++) + 1;  /* Post-increment in expression */
        }
        printf("Product: %d\n", product);
    }
    
    return 0;
}
