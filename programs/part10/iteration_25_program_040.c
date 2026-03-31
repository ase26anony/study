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
    
    /* Copy in reverse using post-decrement */
    while (src_end >= src) {
        *dest_end-- = *src_end--;  /* Both pointers use post-decrement */
    }
}

/* Function 3: Mixed operations with different data types */
void process_data(void) {
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    volatile char* vptr;  /* Volatile pointer to prevent optimization */
    
    /* Initialize char array with index post-increment */
    for (int i = 0; i < SIZE; i++) {
        char_array[i] = (char)(i % 256);
    }
    
    /* Process char array with volatile pointer */
    vptr = char_array;
    for (int i = 0; i < SIZE; i++) {
        char val = *vptr++;  /* Volatile read with post-increment */
        (void)val;  /* Use value to prevent elimination */
    }
    
    /* Initialize double array with pointer arithmetic */
    double* dptr = double_array;
    for (int i = 0; i < SMALL_SIZE; i++) {
        *dptr++ = i * 1.5;  /* Store with post-increment */
    }
}

/* Function 4: Struct array traversal with stride */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    int total = 0;
    int i = 0;
    
    /* Use while loop to encourage pointer post-increment */
    while (i < n) {
        total += ptr->x + ptr->y;
        ptr++;  /* Post-increment of struct pointer */
        i++;
    }
    
    return total;
}

/* Function 5: Nested loops with inner auto-increment */
void matrix_operation(int rows, int cols, int matrix[rows][cols]) {
    /* Initialize matrix with nested loops */
    for (int i = 0; i < rows; i++) {
        int* row_ptr = matrix[i];
        for (int j = 0; j < cols; j++) {
            *row_ptr++ = i * cols + j;  /* Inner loop uses pointer post-increment */
        }
    }
    
    /* Process matrix with backward traversal */
    for (int i = rows - 1; i >= 0; i--) {
        int* row_ptr = &matrix[i][cols - 1];
        for (int j = cols - 1; j >= 0; j--) {
            int val = *row_ptr--;  /* Post-decrement in inner loop */
            matrix[i][j] = val * 2;
        }
    }
}

/* Function 6: Pointer with stride (not simple +/-1) */
void stride_copy(int* dest, const int* src, int n, int stride) {
    const int* s = src;
    int* d = dest;
    
    /* Stride may not trigger the specific uncovered code,
       but tests other parts of the optimization */
    for (int i = 0; i < n; i++) {
        *d = *s;
        s += stride;  /* This might use different addressing */
        d += stride;
    }
}

int main(void) {
    int array1[SIZE];
    int array2[SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[10][10];
    
    volatile int counter = 0;  /* Volatile to prevent loop optimization */
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i + 1;
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    /* Test 1: Forward traversal with post-increment */
    int sum = sum_array(array1, SIZE);
    printf("Sum of array1: %d\n", sum);
    
    /* Test 2: Reverse copy with post-decrement */
    reverse_copy(array2, array1, SIZE);
    
    /* Verify reverse copy */
    int sum2 = sum_array(array2, SIZE);
    printf("Sum of array2 (reversed): %d\n", sum2);
    
    /* Test 3: Mixed data types with volatile */
    process_data();
    
    /* Test 4: Struct traversal */
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* Test 5: Nested loops with matrix */
    matrix_operation(10, 10, matrix);
    
    /* Test 6: Stride operation */
    int src[50];
    int dest[50];
    for (int i = 0; i < 50; i++) {
        src[i] = i * 3;
    }
    stride_copy(dest, src, 25, 2);  /* Copy every other element */
    
    /* Use results to prevent dead code elimination */
    counter = sum + sum2 + point_sum + matrix[5][5] + dest[10];
    printf("Final counter value: %d\n", counter);
    
    return 0;
}
