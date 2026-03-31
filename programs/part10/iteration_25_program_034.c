#include <stdio.h>
#include <stdint.h>

#define SIZE 100
#define SMALL_SIZE 10

struct Point {
    int x;
    int y;
    char label;
};

/* Function 1: Forward traversal with pointer post-increment (load) */
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

/* Function 2: Reverse copy with pointer post-decrement (store) */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Pattern: *dest-- = *src-- */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function 3: Mixed operations with different data types */
void process_structs(struct Point* points, int n) {
    struct Point* ptr = points;
    struct Point* end = ptr + n;
    
    /* Process struct array with post-increment */
    while (ptr < end) {
        ptr->x += ptr->y;
        ptr->label = 'A' + (ptr->x % 26);
        ptr++;  /* Post-increment separated from access */
    }
}

/* Function 4: Volatile pointer traversal (prevents some optimizations) */
int volatile_sum(volatile int* arr, int n) {
    volatile int* ptr = arr;
    int sum = 0;
    int i;
    
    /* Use volatile pointer with post-increment */
    for (i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

/* Function 5: Strided access (ptr += 2) */
int sum_every_other(const int* arr, int n) {
    const int* ptr = arr;
    const int* end = arr + n;
    int sum = 0;
    
    /* Strided access - may trigger different patterns */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;  /* Not post-increment, but tests other paths */
    }
    return sum;
}

/* Function 6: Index-based post-increment */
void initialize_array(int* arr, int n) {
    int i = 0;
    
    /* Array access with index post-increment */
    while (i < n) {
        arr[i++] = i * 2;  /* Post-increment in array index */
    }
}

/* Function 7: Nested loops with inner auto-increment */
void matrix_sum_rows(const int matrix[][SMALL_SIZE], int rows, int* results) {
    int i, j;
    
    for (i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        int sum = 0;
        
        /* Inner loop with pointer auto-increment */
        for (j = 0; j < SMALL_SIZE; j++) {
            sum += *row_ptr++;
        }
        results[i] = sum;
    }
}

/* Function 8: Char array processing with post-decrement */
void reverse_string(char* str, int len) {
    char* start = str;
    char* end = str + len - 1;
    
    /* Post-decrement in both directions */
    while (start < end) {
        char temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}

/* Function 9: Double array with mixed patterns */
double sum_doubles(const double* arr, int n) {
    const double* ptr = arr;
    double sum = 0.0;
    int i = 0;
    
    /* Mix of pointer and index access */
    while (i < n) {
        sum += ptr[i];  /* Array index notation */
        i++;
        if (i < n) {
            sum += *ptr++;  /* Pointer dereference with post-increment */
        }
    }
    return sum;
}

int main() {
    int int_array[SIZE];
    int int_array2[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SIZE/10];
    int matrix[SMALL_SIZE][SMALL_SIZE];
    int row_sums[SMALL_SIZE];
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = (i + 1) * 1.5;
    }
    
    for (int i = 0; i < SIZE/10; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A';
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Test 1: Forward traversal with post-increment */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int_array: %d\n", sum1);
    
    /* Test 2: Reverse copy with post-decrement */
    reverse_copy(int_array2, int_array, SIZE);
    int sum2 = sum_array(int_array2, SIZE);
    printf("Sum of reversed copy: %d\n", sum2);
    
    /* Test 3: Struct processing */
    process_structs(points, SIZE/10);
    printf("First point label: %c\n", points[0].label);
    
    /* Test 4: Volatile access */
    volatile int* volatile_ptr = int_array;
    int sum3 = volatile_sum(volatile_ptr, SIZE/2);
    printf("Volatile sum (first half): %d\n", sum3);
    
    /* Test 5: Strided access */
    int sum4 = sum_every_other(int_array, SIZE);
    printf("Sum of every other element: %d\n", sum4);
    
    /* Test 6: Index-based post-increment */
    int new_array[SIZE];
    initialize_array(new_array, SIZE);
    printf("New array[50] = %d\n", new_array[50]);
    
    /* Test 7: Nested loops */
    matrix_sum_rows(matrix, SMALL_SIZE, row_sums);
    printf("Matrix row 0 sum: %d\n", row_sums[0]);
    
    /* Test 8: Char array reverse */
    char test_str[] = "HelloWorld";
    reverse_string(test_str, 10);
    printf("Reversed string: %s\n", test_str);
    
    /* Test 9: Double array mixed access */
    double sum5 = sum_doubles(double_array, SIZE);
    printf("Sum of doubles: %.2f\n", sum5);
    
    /* Additional: Simple for loop with post-increment (common pattern) */
    int buffer[SIZE];
    for (int i = 0; i < SIZE; i++) {
        buffer[i] = i * 3;  /* May generate different addressing patterns */
    }
    printf("Buffer[99] = %d\n", buffer[99]);
    
    return 0;
}
