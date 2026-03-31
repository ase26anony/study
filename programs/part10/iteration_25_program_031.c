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

/* Function 3: Mixed operations with volatile pointer */
void process_chars(volatile char* data, int n) {
    volatile char* ptr = data;
    volatile char* end = data + n;
    
    /* Volatile prevents some optimizations, leaving pattern intact */
    while (ptr < end) {
        char val = *ptr++;
        /* Use the value to prevent elimination */
        (void)val;
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
        ptr++;
    }
    
    return total;
}

/* Function 5: Double array with stride (ptr += 2) */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    int count = 0;
    
    /* Stride of 2 - may trigger different pattern */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;
        count++;
        if (count >= n/2) break;
    }
    
    return sum;
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_sum_rows(const int matrix[][SMALL_SIZE], int rows, int* results) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        const int* row_end = matrix[i] + SMALL_SIZE;
        int row_sum = 0;
        
        /* Inner loop with pointer increment */
        while (row_ptr < row_end) {
            row_sum += *row_ptr++;
        }
        
        results[i] = row_sum;
    }
}

/* Function 7: Index-based post-increment */
void initialize_with_index(int* arr, int n) {
    /* Classic for loop with array[index++] */
    for (int i = 0; i < n; i++) {
        arr[i] = i * 2;
    }
}

/* Function 8: Store operations with auto-increment */
void fill_array(int* dest, int value, int n) {
    int* ptr = dest;
    int* end = dest + n;
    
    /* Store pattern: *ptr++ = value */
    while (ptr < end) {
        *ptr++ = value;
    }
}

/* Function 9: Mixed load/store with post-modify */
void copy_and_transform(int* dest, const int* src, int n) {
    const int* src_ptr = src;
    int* dest_ptr = dest;
    const int* end = src + n;
    
    /* Both load and store with post-increment */
    while (src_ptr < end) {
        *dest_ptr++ = *src_ptr++ + 1;
    }
}

int main() {
    /* Local arrays (stack-based) */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[SMALL_SIZE][SMALL_SIZE];
    int results[SMALL_SIZE];
    int dest_array[SIZE];
    int src_array[SIZE];
    
    /* Initialize arrays with volatile to prevent pre-optimization */
    volatile int init_val = 0;
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = init_val + i;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 1.5;
        if (i < SMALL_SIZE) {
            points[i].x = i;
            points[i].y = i * 2;
            points[i].label = 'A' + i;
        }
    }
    
    /* Initialize matrix */
    for (int i = 0; i < SMALL_SIZE; i++) {
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrix[i][j] = i * SMALL_SIZE + j;
        }
    }
    
    /* Call functions to trigger various patterns */
    
    /* 1. Forward traversal with post-increment */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* 2. Reverse copy with post-decrement */
    reverse_copy(dest_array, int_array, SIZE);
    printf("First element after reverse: %d\n", dest_array[0]);
    
    /* 3. Volatile char processing */
    process_chars((volatile char*)char_array, SIZE);
    printf("Processed char array\n");
    
    /* 4. Struct traversal */
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* 5. Stride access */
    double stride_sum = sum_every_other(double_array, SIZE);
    printf("Sum of every other double: %f\n", stride_sum);
    
    /* 6. Nested loops */
    matrix_sum_rows(matrix, SMALL_SIZE, results);
    printf("Matrix row sums calculated\n");
    
    /* 7. Index-based initialization */
    initialize_with_index(src_array, SIZE);
    printf("Source array initialized\n");
    
    /* 8. Store operations */
    fill_array(dest_array, 42, SIZE);
    printf("Destination array filled\n");
    
    /* 9. Mixed load/store */
    copy_and_transform(dest_array, src_array, SIZE);
    printf("Copy and transform completed\n");
    
    /* Final check to use all results */
    int final_check = dest_array[SIZE/2] + results[0];
    printf("Final check value: %d\n", final_check);
    
    return 0;
}
