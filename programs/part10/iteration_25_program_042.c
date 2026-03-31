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
    
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Both pointers use post-decrement */
    while (src_end >= src) {
        *dest_end-- = *src_end--;  /* Dual post-decrement pattern */
    }
}

/* Function 3: Mixed types with different access patterns */
void process_structs(struct Point* points, int n) {
    struct Point* ptr = points;
    volatile struct Point* vptr = points;  /* volatile to prevent optimization */
    int i = 0;
    
    /* Index-based post-increment */
    for (i = 0; i < n; i++) {
        points[i].x = i;  /* Array index with implicit increment */
        points[i].y = i * 2;
    }
    
    /* Pointer-based with stride-like access */
    ptr = points;
    for (i = 0; i < n/2; i++) {
        ptr->label = 'A' + (i % 26);
        ptr += 2;  /* Stride of 2, not simple increment */
    }
    
    /* Volatile pointer traversal */
    while (n-- > 0) {
        (void)vptr->x;  /* Read volatile to force memory access */
        vptr++;  /* Post-increment on volatile pointer */
    }
}

/* Function 4: Character array with byte access */
int count_chars(const char* str) {
    const char* ptr = str;
    int count = 0;
    
    /* Pointer post-increment on char */
    while (*ptr != '\0') {
        if (*ptr++ == 'a') {  /* Post-increment in condition */
            count++;
        }
    }
    
    return count;
}

/* Function 5: Double array with store operations */
void init_doubles(double* arr, int n, double start) {
    double* ptr = arr;
    double value = start;
    int i;
    
    /* Store with post-increment */
    for (i = 0; i < n; i++) {
        *ptr++ = value;  /* Post-increment store */
        value += 1.5;
    }
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_multiply(const int* a, const int* b, int* result, int n) {
    int i, j, k;
    const int* a_ptr;
    const int* b_ptr;
    int* res_ptr;
    
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            result[i * n + j] = 0;
            a_ptr = &a[i * n];
            b_ptr = &b[j];
            
            /* Inner loop with pointer arithmetic */
            for (k = 0; k < n; k++) {
                result[i * n + j] += *a_ptr++ * *b_ptr;
                b_ptr += n;  /* Stride access */
            }
        }
    }
}

/* Function 7: Complex pattern with multiple increments */
void complex_access(int* arr, int n) {
    int* p1 = arr;
    int* p2 = arr + n/2;
    volatile int* vp = arr;  /* Prevent optimization */
    int i;
    
    /* Multiple pointers with post-increment in same loop */
    for (i = 0; i < n/2; i++) {
        *p2++ = *p1++ * 2;  /* Both pointers post-increment */
        
        /* Volatile access to prevent elimination */
        if (*vp > 100) {
            vp++;
        }
    }
}

int main(void) {
    int int_array[SIZE];
    int int_array2[SIZE];
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct Point points[SMALL_SIZE];
    int matrix_a[SMALL_SIZE * SMALL_SIZE];
    int matrix_b[SMALL_SIZE * SMALL_SIZE];
    int matrix_result[SMALL_SIZE * SMALL_SIZE];
    
    int i, sum;
    
    /* Initialize arrays with non-constant values to prevent constant folding */
    for (i = 0; i < SIZE; i++) {
        int_array[i] = (i * 3) % 97;  /* Non-trivial pattern */
        char_array[i] = 'a' + (i % 26);
    }
    
    for (i = 0; i < SMALL_SIZE; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    /* Test 1: Simple forward traversal with post-increment */
    sum = sum_array(int_array, SIZE);
    printf("Sum of array: %d\n", sum);
    
    /* Test 2: Reverse copy with post-decrement */
    reverse_copy(int_array2, int_array, SIZE);
    
    /* Test 3: Verify reverse copy */
    sum = sum_array(int_array2, SIZE);
    printf("Sum of reversed array: %d\n", sum);
    
    /* Test 4: Character array processing */
    int char_count = count_chars(char_array);
    printf("Character 'a' count: %d\n", char_count);
    
    /* Test 5: Struct processing with mixed patterns */
    process_structs(points, SMALL_SIZE);
    printf("First point: x=%d, y=%d, label=%c\n", 
           points[0].x, points[0].y, points[0].label);
    
    /* Test 6: Double array initialization */
    init_doubles(double_array, SMALL_SIZE, 1.0);
    printf("First double: %f\n", double_array[0]);
    
    /* Test 7: Initialize matrices */
    for (i = 0; i < SMALL_SIZE * SMALL_SIZE; i++) {
        matrix_a[i] = i % 10;
        matrix_b[i] = (i + 5) % 10;
    }
    
    /* Test 8: Matrix multiplication with nested loops */
    matrix_multiply(matrix_a, matrix_b, matrix_result, SMALL_SIZE);
    printf("Matrix result[0][0]: %d\n", matrix_result[0]);
    
    /* Test 9: Complex access pattern */
    complex_access(int_array, SIZE);
    
    /* Final verification sum */
    sum = 0;
    for (i = 0; i < SIZE; i++) {
        sum += int_array[i];
    }
    printf("Final sum: %d\n", sum);
    
    return 0;
}
