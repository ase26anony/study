#include <stdio.h>
#include <string.h>

#define SIZE 100
#define SMALL_SIZE 10

struct Point {
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
void reverse_copy(char* dest, const char* src, int n) {
    const char* src_ptr = src + n - 1;
    char* dest_ptr = dest + n - 1;
    
    /* Post-decrement pattern: *dest-- = *src-- */
    while (n-- > 0) {
        *dest_ptr-- = *src_ptr--;
    }
}

/* Function with mixed operations and volatile */
void process_with_volatile(volatile int* data, int n) {
    volatile int* ptr = data;
    int i = n;
    
    /* Volatile pointer with post-decrement */
    while (i-- > 0) {
        int val = *ptr++;
        /* Use the value to prevent optimization */
        *ptr = val + 1;
    }
}

/* Function with array index post-increment */
void initialize_array(double* arr, int n) {
    int i = 0;
    
    /* Array index with post-increment: arr[i++] */
    while (i < n) {
        arr[i++] = i * 1.5;
    }
}

/* Function with struct traversal */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    int total = 0;
    int i = 0;
    
    /* Pointer arithmetic with struct size */
    while (i++ < n) {
        total += ptr->x + ptr->y;
        ptr++;  /* Post-increment of struct pointer */
    }
    
    return total;
}

/* Function with stride (ptr += 2) pattern */
int sum_every_other(const int* arr, int n) {
    const int* ptr = arr;
    const int* end = arr + n;
    int sum = 0;
    
    /* Stride pattern - may trigger different optimization */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;
    }
    
    return sum;
}

/* Nested loop with inner auto-increment */
void matrix_sum(const int matrix[][SMALL_SIZE], int rows, int cols, int* result) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        int row_sum = 0;
        
        /* Inner loop with pointer post-increment */
        for (int j = 0; j < cols; j++) {
            row_sum += *row_ptr++;
        }
        
        result[i] = row_sum;
    }
}

/* Store operations with post-increment */
void fill_array(int* dest, int value, int n) {
    int* ptr = dest;
    int* end = dest + n;
    
    /* Store with post-increment: *ptr++ = value */
    while (ptr < end) {
        *ptr++ = value++;
    }
}

int main() {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SIZE];
    int matrix[SMALL_SIZE][SMALL_SIZE];
    int result_array[SMALL_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 0.5;
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
    
    /* Use volatile to prevent optimization */
    volatile int volatile_data[SIZE];
    for (int i = 0; i < SIZE; i++) {
        volatile_data[i] = i * 3;
    }
    
    /* Call functions with different patterns */
    
    /* 1. Pointer post-increment (load) */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* 2. Pointer post-decrement (store) */
    char reversed[SIZE];
    reverse_copy(reversed, char_array, SIZE);
    printf("First char of reversed: %c\n", reversed[0]);
    
    /* 3. Volatile access pattern */
    process_with_volatile((int*)volatile_data, SIZE/2);
    printf("Volatile data[0]: %d\n", volatile_data[0]);
    
    /* 4. Array index post-increment */
    initialize_array(double_array, SIZE);
    printf("Double array[0]: %.2f\n", double_array[0]);
    
    /* 5. Struct pointer post-increment */
    int point_sum = sum_points(points, SIZE/10);
    printf("Sum of points: %d\n", point_sum);
    
    /* 6. Stride pattern */
    int stride_sum = sum_every_other(int_array, SIZE);
    printf("Sum of every other: %d\n", stride_sum);
    
    /* 7. Nested loop with inner auto-increment */
    matrix_sum(matrix, SMALL_SIZE, SMALL_SIZE, result_array);
    printf("Matrix row sums: ");
    for (int i = 0; i < SMALL_SIZE; i++) {
        printf("%d ", result_array[i]);
    }
    printf("\n");
    
    /* 8. Store with post-increment */
    int filled[SIZE];
    fill_array(filled, 100, SIZE);
    printf("Filled array[0]: %d, [SIZE-1]: %d\n", filled[0], filled[SIZE-1]);
    
    /* Additional pattern: mixed increment/decrement in same function */
    {
        int data[20];
        int* p1 = data;
        int* p2 = data + 19;
        
        /* Mixed patterns */
        for (int i = 0; i < 10; i++) {
            *p1++ = i;      /* Post-increment store */
            *p2-- = i * 2;  /* Post-decrement store */
        }
        printf("Mixed pattern result: %d, %d\n", data[0], data[19]);
    }
    
    /* Prevent dead code elimination */
    volatile int dummy = sum + point_sum + stride_sum;
    
    return 0;
}
