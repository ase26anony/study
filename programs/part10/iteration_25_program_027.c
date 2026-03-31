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
    const char* src_ptr = src + n - 1;
    char* dest_ptr = dest + n - 1;
    
    /* Using *dest-- = *src-- pattern */
    for (int i = 0; i < n; i++) {
        *dest_ptr-- = *src_ptr--;
    }
}

/* Function 3: Mixed load/store with post-increment */
void scale_array(int* arr, int n, int factor) {
    int* ptr = arr;
    int* end = arr + n;
    
    /* Both load and store with post-increment */
    while (ptr < end) {
        int val = *ptr;
        *ptr++ = val * factor;
    }
}

/* Function 4: Struct traversal with pointer arithmetic */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    int total = 0;
    
    /* Access struct members with pointer increment */
    for (int i = 0; i < n; i++) {
        total += ptr->x + ptr->y;
        ptr++;  /* Post-increment after access */
    }
    return total;
}

/* Function 5: Double array with stride */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    double sum = 0.0;
    int count = 0;
    
    /* Pointer increment by 2 (stride) */
    while (count < n) {
        sum += *ptr;
        ptr += 2;  /* Stride of 2 */
        count += 2;
    }
    return sum;
}

/* Function 6: Volatile pointer traversal */
int volatile_sum(const volatile int* arr, int n) {
    volatile const int* ptr = arr;
    int sum = 0;
    
    /* Volatile prevents some optimizations, keeping pattern intact */
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

/* Function 7: Nested loops with auto-increment */
void matrix_sum_rows(const int matrix[][SMALL_SIZE], int rows, int* results) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        int sum = 0;
        
        /* Inner loop with pointer traversal */
        for (int j = 0; j < SMALL_SIZE; j++) {
            sum += *row_ptr++;
        }
        results[i] = sum;
    }
}

/* Function 8: Index-based post-increment */
void initialize_with_index(int* arr, int n) {
    /* Classic for loop with array[index++] */
    for (int i = 0; i < n; ) {
        arr[i++] = i * 2;  /* Post-increment in array access */
    }
}

/* Function 9: Mixed pre and post operations for comparison */
void mixed_operations(char* dest, const char* src, int n) {
    char* d = dest;
    const char* s = src;
    
    /* Mix of patterns */
    for (int i = 0; i < n; i++) {
        *d++ = *s++;      /* Post-increment both */
        
        if (i % 2 == 0) {
            *d = *(s - 1);  /* No increment here */
        }
    }
}

int main() {
    /* Declare and initialize arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SMALL_SIZE];
    volatile int volatile_array[SIZE];
    int matrix[SMALL_SIZE][SMALL_SIZE];
    int results[SMALL_SIZE];
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 1.5;
        if (i < SIZE/2) {
            volatile_array[i] = i * 3;
        }
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + i;
        
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrix[i][j] = i * SMALL_SIZE + j;
        }
    }
    
    /* Call functions to trigger various patterns */
    
    /* 1. Forward pointer traversal */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    /* 2. Reverse copy with post-decrement */
    char reversed[SIZE];
    reverse_copy(reversed, char_array, SIZE);
    printf("First char of reversed: %c\n", reversed[0]);
    
    /* 3. Load/store with post-increment */
    scale_array(int_array, SIZE, 2);
    sum1 = sum_array(int_array, SIZE/2);  /* Check partial sum */
    printf("Scaled array partial sum: %d\n", sum1);
    
    /* 4. Struct traversal */
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* 5. Stride access */
    double stride_sum = sum_every_other(double_array, SIZE);
    printf("Stride sum of doubles: %.2f\n", stride_sum);
    
    /* 6. Volatile access */
    int volatile_sum_result = volatile_sum(volatile_array, SIZE/2);
    printf("Volatile array sum: %d\n", volatile_sum_result);
    
    /* 7. Nested loops */
    matrix_sum_rows(matrix, SMALL_SIZE, results);
    printf("Matrix row sums calculated\n");
    
    /* 8. Index-based post-increment */
    int new_array[SIZE];
    initialize_with_index(new_array, SIZE);
    printf("New array initialized with index pattern\n");
    
    /* 9. Mixed operations */
    char dest[SIZE];
    mixed_operations(dest, char_array, SIZE);
    printf("Mixed operations completed\n");
    
    /* Additional pattern: while loop with pointer comparison */
    const int* ptr = int_array;
    const int* end = int_array + SIZE;
    int final_check = 0;
    
    while (ptr != end) {
        final_check ^= *ptr++;  /* XOR with post-increment */
    }
    printf("Final XOR result: %d\n", final_check);
    
    return 0;
}
