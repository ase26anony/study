#include <stdio.h>
#include <string.h>

#define SIZE 100
#define SMALL_SIZE 20

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
    
    /* Classic pattern: *ptr++ in while loop */
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(char* dest, const char* src, int n) {
    const char* src_end = src + n - 1;
    char* dest_end = dest + n - 1;
    
    /* Pattern: *dest-- = *src-- */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function 3: Mixed operations with volatile */
void volatile_traversal(volatile int* arr, int n) {
    volatile int* ptr = arr;
    volatile int* end = arr + n;
    
    /* Volatile prevents some optimizations, may keep pattern intact */
    while (ptr < end) {
        volatile int val = *ptr++;
        (void)val;  /* Use the value to prevent elimination */
    }
}

/* Function 4: Struct traversal with pointer arithmetic */
void init_points(struct Point* points, int n) {
    struct Point* ptr = points;
    struct Point* end = points + n;
    
    /* Access struct members with pointer increment */
    while (ptr < end) {
        ptr->x = (int)(ptr - points);
        ptr->y = (int)(ptr - points) * 2;
        strncpy(ptr->label, "point", 7);
        ptr++;  /* Post-increment after struct access */
    }
}

/* Function 5: Double array with stride */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    int count = 0;
    
    /* Pattern with stride - might trigger different optimization */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;  /* Stride of 2 */
        count++;
        if (count >= n/2) break;
    }
    return sum;
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_sum(const int matrix[][10], int rows, int* result) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        const int* row_end = row_ptr + 10;
        int row_sum = 0;
        
        /* Inner loop with pointer increment */
        while (row_ptr < row_end) {
            row_sum += *row_ptr++;
        }
        result[i] = row_sum;
    }
}

int main() {
    /* Declare various arrays */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SMALL_SIZE];
    volatile int volatile_array[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i * 2;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 1.5;
        if (i < SIZE) volatile_array[i] = i * 3;
    }
    
    /* Pattern 1: Index-based post-increment initialization */
    int buffer[SIZE];
    for (int i = 0; i < SIZE; i++) {
        buffer[i] = i;  /* May generate different pattern */
    }
    
    /* Call functions to trigger optimizations */
    
    /* 1. Forward pointer traversal */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* 2. Reverse pointer traversal */
    char reversed[SIZE];
    reverse_copy(reversed, char_array, SIZE);
    printf("First char after reverse: %c\n", reversed[0]);
    
    /* 3. Volatile access pattern */
    volatile_traversal(volatile_array, SIZE);
    printf("Volatile traversal completed\n");
    
    /* 4. Struct traversal */
    init_points(points, SMALL_SIZE);
    printf("First point: (%d, %d, %s)\n", points[0].x, points[0].y, points[0].label);
    
    /* 5. Stride pattern */
    double dsum = sum_every_other(double_array, SIZE);
    printf("Sum of every other double: %f\n", dsum);
    
    /* 6. Nested loop pattern */
    int matrix[5][10];
    int row_sums[5];
    
    /* Initialize matrix */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    matrix_sum(matrix, 5, row_sums);
    printf("Matrix row sums: %d, %d, %d, %d, %d\n", 
           row_sums[0], row_sums[1], row_sums[2], row_sums[3], row_sums[4]);
    
    /* Additional pattern: char pointer with post-increment */
    const char* str = "Hello, World!";
    int length = 0;
    const char* p = str;
    
    /* Count string length with *p++ */
    while (*p != '\0') {
        length++;
        p++;  /* Post-increment */
    }
    printf("String length: %d\n", length);
    
    /* Pattern: Pointer decrement in loop */
    int reverse_sum = 0;
    const int* rev_ptr = int_array + SIZE - 1;
    
    for (int i = 0; i < SIZE; i++) {
        reverse_sum += *rev_ptr--;
    }
    printf("Reverse sum: %d\n", reverse_sum);
    
    return 0;
}
