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
    
    /* Classic post-increment pattern */
    while (ptr < end) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Post-decrement pattern */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function 3: Mixed operations with volatile */
void process_volatile_chars(volatile char* data, int n) {
    volatile char* ptr = data;
    volatile char* end = data + n;
    
    /* Volatile pointer with post-increment */
    while (ptr < end) {
        char val = *ptr++;
        /* Use the value to prevent optimization */
        (void)val;
    }
}

/* Function 4: Array initialization with index post-increment */
void init_with_post_inc(int* arr, int n) {
    int i = 0;
    
    /* Index-based post-increment */
    while (i < n) {
        arr[i++] = i;  /* Post-increment of index */
    }
}

/* Function 5: Double stride with pointer arithmetic */
double sum_doubles_with_stride(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    
    /* Post-increment with stride */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;  /* Different stride, might trigger other patterns */
    }
    
    return sum;
}

/* Function 6: Struct traversal with pointer */
int sum_point_coordinates(const struct Point* points, int n) {
    const struct Point* ptr = points;
    const struct Point* end = points + n;
    int total = 0;
    
    /* Post-increment with struct */
    while (ptr < end) {
        total += ptr->x + ptr->y;
        ptr++;
    }
    
    return total;
}

/* Function 7: Nested loops with auto-increment */
void matrix_multiply(int a[SIZE][SIZE], int b[SIZE][SIZE], int result[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            int sum = 0;
            int* a_ptr = &a[i][0];
            int* b_ptr = &b[0][j];
            
            /* Inner loop with pointer arithmetic */
            for (int k = 0; k < SIZE; k++) {
                sum += *a_ptr++ * *b_ptr;
                b_ptr += SIZE;  /* Move to next row */
            }
            
            result[i][j] = sum;
        }
    }
}

/* Function 8: Store operations with post-increment */
void fill_array_with_pattern(int* arr, int n, int pattern) {
    int* ptr = arr;
    int* end = arr + n;
    
    /* Store with post-increment */
    while (ptr < end) {
        *ptr++ = pattern;
        pattern = (pattern * 13 + 7) & 0xFF;  /* Simple pseudo-random */
    }
}

int main(void) {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SMALL_SIZE];
    int dest_array[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i;
        char_array[i] = (char)(i & 0xFF);
        double_array[i] = i * 1.5;
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    /* Use volatile to prevent optimization */
    volatile int use_volatile = 1;
    
    /* Test 1: Forward traversal with post-increment */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* Test 2: Reverse copy with post-decrement */
    reverse_copy(dest_array, int_array, SIZE);
    printf("First element after reverse copy: %d\n", dest_array[0]);
    
    /* Test 3: Volatile char processing */
    process_volatile_chars((volatile char*)char_array, SIZE);
    printf("Processed volatile chars\n");
    
    /* Test 4: Index-based post-increment */
    int temp_array[SMALL_SIZE];
    init_with_post_inc(temp_array, SMALL_SIZE);
    printf("Last element after init: %d\n", temp_array[SMALL_SIZE - 1]);
    
    /* Test 5: Double array with stride */
    double dsum = sum_doubles_with_stride(double_array, SIZE);
    printf("Sum of doubles (stride 2): %f\n", dsum);
    
    /* Test 6: Struct traversal */
    int coord_sum = sum_point_coordinates(points, SMALL_SIZE);
    printf("Sum of point coordinates: %d\n", coord_sum);
    
    /* Test 7: Store operations */
    fill_array_with_pattern(temp_array, SMALL_SIZE, 42);
    printf("Pattern-filled array first element: %d\n", temp_array[0]);
    
    /* Test 8: Complex nested loop (smaller for speed) */
    int small_matrix[10][10] = {0};
    int small_matrix2[10][10] = {0};
    int result_matrix[10][10] = {0};
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            small_matrix[i][j] = i + j;
            small_matrix2[i][j] = i * j;
        }
    }
    
    /* Call with smaller matrices */
    int (*a)[10] = small_matrix;
    int (*b)[10] = small_matrix2;
    int (*r)[10] = result_matrix;
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int sum = 0;
            int* a_ptr = &a[i][0];
            int* b_ptr = &b[0][j];
            
            for (int k = 0; k < 10; k++) {
                sum += *a_ptr++ * *b_ptr;
                b_ptr += 10;
            }
            
            r[i][j] = sum;
        }
    }
    
    printf("Matrix result[0][0]: %d\n", result_matrix[0][0]);
    
    /* Ensure all results are used */
    if (use_volatile) {
        printf("All tests completed successfully\n");
    }
    
    return 0;
}
