#include <stdio.h>
#include <string.h>

#define SIZE 100
#define SMALL_SIZE 10

struct Point {
    int x;
    int y;
    char label[8];
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
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Post-decrement pattern: *dest-- = *src-- */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function with mixed operations and volatile to prevent optimization */
void process_chars(volatile char* data, int n) {
    volatile char* ptr = data;
    volatile char* end = data + n;
    
    /* Volatile pointer with post-increment */
    while (ptr < end) {
        volatile char value = *ptr++;
        (void)value;  /* Use value to avoid dead code elimination */
    }
}

/* Function with struct traversal using pointer arithmetic */
void init_points(struct Point* points, int n) {
    struct Point* ptr = points;
    struct Point* end = points + n;
    int i = 0;
    
    /* Pointer post-increment with struct access */
    while (ptr < end) {
        ptr->x = i;
        ptr->y = i * 2;
        snprintf(ptr->label, sizeof(ptr->label), "P%d", i);
        ptr++;  /* Post-increment after struct access */
        i++;
    }
}

/* Function with array index post-increment */
void fill_with_index(int* arr, int n) {
    int i = 0;
    
    /* Array index with post-increment: arr[i++] */
    while (i < n) {
        arr[i++] = i * 3;  /* Post-increment of index */
    }
}

/* Function with stride (ptr += 2) pattern */
int sum_every_other(const int* arr, int n) {
    const int* ptr = arr;
    const int* end = arr + n;
    int sum = 0;
    
    /* Stride of 2 - may trigger different patterns */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;  /* Not post-increment, but tests other paths */
    }
    return sum;
}

/* Nested loop with inner auto-increment */
void matrix_sum(const int matrix[][SMALL_SIZE], int rows, int cols, int* result) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        const int* row_end = matrix[i] + cols;
        int row_sum = 0;
        
        /* Inner loop with pointer post-increment */
        while (row_ptr < row_end) {
            row_sum += *row_ptr++;
        }
        result[i] = row_sum;
    }
}

/* Function with double array traversal */
double sum_doubles(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    
    /* Double pointer with post-increment */
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

/* Store operations with post-increment */
void store_sequence(int* dest, int start, int n) {
    int* ptr = dest;
    int* end = dest + n;
    int value = start;
    
    /* Store with post-increment: *ptr++ = value */
    while (ptr < end) {
        *ptr++ = value++;
    }
}

int main(void) {
    /* Declare and initialize arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[SMALL_SIZE][SMALL_SIZE];
    int results[SMALL_SIZE];
    
    /* Initialize arrays with some data */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = (double)i / 2.0;
    }
    
    /* Initialize matrix */
    for (int i = 0; i < SMALL_SIZE; i++) {
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrix[i][j] = i * SMALL_SIZE + j;
        }
    }
    
    /* Test various patterns that should trigger auto-inc-dec optimization */
    
    /* 1. Pointer post-increment for forward traversal */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* 2. Pointer post-decrement for backward traversal */
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    printf("First element of reversed: %d\n", reversed[0]);
    
    /* 3. Volatile pointer access */
    process_chars(char_array, SIZE);
    
    /* 4. Struct traversal with pointer */
    init_points(points, SMALL_SIZE);
    printf("First point: (%d, %d, %s)\n", points[0].x, points[0].y, points[0].label);
    
    /* 5. Array index post-increment */
    int filled[SIZE];
    fill_with_index(filled, SIZE);
    printf("Last filled value: %d\n", filled[SIZE-1]);
    
    /* 6. Stride pattern */
    int stride_sum = sum_every_other(int_array, SIZE);
    printf("Sum of every other element: %d\n", stride_sum);
    
    /* 7. Nested loops with inner auto-increment */
    matrix_sum(matrix, SMALL_SIZE, SMALL_SIZE, results);
    printf("Matrix row sums: ");
    for (int i = 0; i < SMALL_SIZE; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    /* 8. Double array traversal */
    double dsum = sum_doubles(double_array, SIZE);
    printf("Sum of doubles: %.2f\n", dsum);
    
    /* 9. Store operations with post-increment */
    int stored[SIZE];
    store_sequence(stored, 100, SIZE);
    printf("First stored: %d, Last stored: %d\n", stored[0], stored[SIZE-1]);
    
    /* Use volatile variable in loop condition to prevent optimization */
    volatile int limit = SIZE;
    int* volatile volatile_ptr = int_array;
    int volatile_sum = 0;
    
    for (volatile int i = 0; i < limit; i++) {
        volatile_sum += *volatile_ptr++;
    }
    printf("Volatile pointer sum: %d\n", volatile_sum);
    
    return 0;
}
