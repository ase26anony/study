#include <stdio.h>
#include <string.h>

#define SIZE 100
#define SMALL_SIZE 10

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
    
    /* Classic pointer traversal with post-increment */
    while (ptr < end) {
        sum += *ptr++;  /* This should generate post-increment RTL */
    }
    
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(char* dest, const char* src, int n) {
    const char* src_end = src + n - 1;
    char* dest_end = dest + n - 1;
    
    /* Both pointers use post-decrement */
    while (src_end >= src) {
        *dest_end-- = *src_end--;  /* Should generate post-decrement RTL */
    }
}

/* Function 3: Mixed operations with volatile */
void process_volatile(volatile int* data, int n) {
    volatile int* ptr = data;
    int i;
    
    /* Volatile pointer with post-increment */
    for (i = 0; i < n; i++) {
        int val = *ptr++;  /* Volatile read with post-increment */
        /* Do something to prevent optimization */
        (void)val;
    }
}

/* Function 4: Struct traversal with pointer arithmetic */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    int total_x = 0;
    int i;
    
    /* Access struct members with pointer post-increment */
    for (i = 0; i < n; i++) {
        total_x += ptr->x;
        ptr++;  /* Post-increment after access */
    }
    
    return total_x;
}

/* Function 5: Double array with stride */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    int count = 0;
    
    /* Pointer with stride (ptr += 2) - tests different patterns */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;
        count++;
        if (count > n/2) break; /* Safety check */
    }
    
    return sum;
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_sum_rows(const int matrix[][SMALL_SIZE], int rows, int* results) {
    int i, j;
    
    for (i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        int sum = 0;
        
        /* Inner loop with pointer post-increment */
        for (j = 0; j < SMALL_SIZE; j++) {
            sum += *row_ptr++;
        }
        
        results[i] = sum;
    }
}

/* Function 7: Index-based post-increment */
void initialize_array(int* arr, int n) {
    int i;
    
    /* Classic index-based post-increment pattern */
    for (i = 0; i < n; i++) {
        arr[i] = i * 2;  /* May generate different addressing patterns */
    }
}

/* Function 8: Char array with post-increment in while condition */
int count_chars(const char* str) {
    const char* ptr = str;
    int count = 0;
    
    /* Post-increment in loop condition */
    while (*ptr++ != '\0') {
        count++;
    }
    
    return count;
}

int main(void) {
    /* Declare and initialize arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[SMALL_SIZE][SMALL_SIZE];
    int row_sums[SMALL_SIZE];
    
    volatile int volatile_array[SIZE];
    
    int i, j;
    
    /* Initialize arrays with some data */
    for (i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 1.5;
        if (i < SIZE/2) {
            volatile_array[i] = i * 3;
        }
    }
    
    for (i = 0; i < SMALL_SIZE; i++) {
        points[i].x = i * 10;
        points[i].y = i * 20;
        snprintf(points[i].label, sizeof(points[i].label), "P%d", i);
        
        for (j = 0; j < SMALL_SIZE; j++) {
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Test 1: Pointer post-increment for sum */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* Test 2: Pointer post-decrement for reverse copy */
    char char_copy[SIZE];
    reverse_copy(char_copy, char_array, SIZE);
    printf("First char of reversed copy: %c\n", char_copy[0]);
    
    /* Test 3: Volatile access with post-increment */
    process_volatile(volatile_array, SIZE/2);
    printf("Processed volatile array\n");
    
    /* Test 4: Struct traversal */
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of point x values: %d\n", point_sum);
    
    /* Test 5: Double array with stride */
    double partial_sum = sum_every_other(double_array, SIZE);
    printf("Sum of every other double: %.2f\n", partial_sum);
    
    /* Test 6: Nested loops with inner auto-increment */
    matrix_sum_rows(matrix, SMALL_SIZE, row_sums);
    printf("Matrix row sums: %d, %d\n", row_sums[0], row_sums[SMALL_SIZE-1]);
    
    /* Test 7: Index-based initialization */
    int new_array[SIZE];
    initialize_array(new_array, SIZE);
    printf("New array initialized, first value: %d\n", new_array[0]);
    
    /* Test 8: Char counting with post-increment */
    const char* test_string = "Hello, World!";
    int char_count = count_chars(test_string);
    printf("String length: %d\n", char_count);
    
    /* Additional test: Mixed pattern in main */
    int local_buffer[SIZE];
    int* ptr = local_buffer;
    
    /* Write with post-increment */
    for (i = 0; i < SIZE; i++) {
        *ptr++ = i * i;  /* Store with post-increment */
    }
    
    /* Read with post-increment */
    ptr = local_buffer;
    int check_sum = 0;
    for (i = 0; i < SIZE; i++) {
        check_sum += *ptr++;  /* Load with post-increment */
    }
    printf("Check sum of local buffer: %d\n", check_sum);
    
    /* Test with different data sizes */
    short short_array[SIZE];
    short* short_ptr = short_array;
    
    for (i = 0; i < SIZE; i++) {
        *short_ptr++ = (short)(i % 1000);  /* Different element size */
    }
    
    return 0;
}
