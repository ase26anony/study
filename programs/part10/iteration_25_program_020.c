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
    
    /* Classic pointer traversal with post-increment */
    while (ptr < end) {
        sum += *ptr++;  /* This should generate post-increment RTL */
    }
    
    /* Use volatile to prevent loop unrolling */
    volatile int dummy = sum;
    return dummy;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_ptr = src + n - 1;
    int* dest_ptr = dest + n - 1;
    
    /* Both pointers use post-decrement */
    while (dest_ptr >= dest) {
        *dest_ptr-- = *src_ptr--;  /* Should generate post-decrement RTL */
    }
    
    /* Prevent optimization of empty loop */
    volatile int* vptr = dest;
    (void)vptr;
}

/* Function 3: Mixed operations with different types */
void process_structs(struct Point* points, int n) {
    struct Point* ptr = points;
    struct Point* end = points + n;
    
    /* Process struct array with pointer increment */
    while (ptr < end) {
        ptr->x = ptr->x * 2;
        ptr->y = ptr->y + 1;
        ptr++;  /* Separate increment - might still be optimized */
    }
}

/* Function 4: Char array with volatile pointer */
int count_chars(const char* str, int len) {
    volatile const char* vptr = str;  /* Volatile to prevent optimizations */
    int count = 0;
    int i = 0;
    
    /* Using index with post-increment */
    while (i < len) {
        if (str[i++] != 0) {  /* Array access with post-increment index */
            count++;
        }
    }
    
    return count;
}

/* Function 5: Double array with stride */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    
    /* Pointer with stride - might trigger different pattern */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;  /* Stride of 2 */
    }
    
    return sum;
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_sum_rows(const int matrix[][10], int rows, int* result) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        int sum = 0;
        
        /* Inner loop with pointer traversal */
        for (int j = 0; j < 10; j++) {
            sum += *row_ptr++;  /* Inner loop auto-increment */
        }
        
        result[i] = sum;
    }
}

/* Function 7: Store operations with post-increment */
void initialize_array(int* arr, int n, int value) {
    int* ptr = arr;
    int* end = arr + n;
    
    /* Store with post-increment */
    while (ptr < end) {
        *ptr++ = value;  /* Store operation with auto-increment */
    }
}

/* Function 8: Backward initialization */
void initialize_backwards(char* arr, int n, char value) {
    char* ptr = arr + n - 1;
    
    /* Store with post-decrement */
    while (ptr >= arr) {
        *ptr-- = value;  /* Store with auto-decrement */
    }
}

int main(void) {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[5][10];
    int results[5];
    
    /* Initialize arrays with volatile to prevent compile-time optimization */
    volatile int seed = 42;
    
    /* Initialize int array using index with post-increment */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = (i * 3) % 7;  /* Non-trivial pattern */
    }
    
    /* Initialize char array */
    for (int i = 0; i < SIZE; i++) {
        char_array[i] = 'A' + (i % 26);
    }
    
    /* Initialize struct array */
    for (int i = 0; i < SMALL_SIZE; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    /* Initialize matrix */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Call functions that use different auto-increment patterns */
    
    /* 1. Forward traversal with load and post-increment */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* 2. Reverse copy with post-decrement */
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    printf("First element of reversed: %d\n", reversed[0]);
    
    /* 3. Process structs */
    process_structs(points, SMALL_SIZE);
    printf("First point: (%d, %d, %c)\n", points[0].x, points[0].y, points[0].label);
    
    /* 4. Char array with volatile */
    int char_count = count_chars(char_array, SIZE);
    printf("Non-zero chars: %d\n", char_count);
    
    /* 5. Double array with stride */
    for (int i = 0; i < SIZE; i++) {
        double_array[i] = i * 0.5;
    }
    double dsum = sum_every_other(double_array, SIZE);
    printf("Sum of every other double: %.2f\n", dsum);
    
    /* 6. Nested loops */
    matrix_sum_rows(matrix, 5, results);
    printf("Matrix row sums: %d, %d, %d, %d, %d\n", 
           results[0], results[1], results[2], results[3], results[4]);
    
    /* 7. Store with post-increment */
    int new_array[SIZE];
    initialize_array(new_array, SIZE, 99);
    printf("New array first element: %d\n", new_array[0]);
    
    /* 8. Backward store with post-decrement */
    char backward_array[SIZE];
    initialize_backwards(backward_array, SIZE, 'Z');
    printf("Backward array first element: %c\n", backward_array[0]);
    
    /* Use volatile result to prevent dead code elimination */
    volatile int final_result = sum + char_count + (int)dsum + results[0];
    
    return 0;
}
