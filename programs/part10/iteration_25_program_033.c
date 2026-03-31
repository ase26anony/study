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

/* Function 3: Mixed operations with different strides */
void process_doubles(double* arr, int n) {
    double* ptr = arr;
    double* end = arr + n;
    
    /* Simple forward traversal */
    while (ptr < end) {
        *ptr = *ptr * 2.0;
        ptr++;  /* Separate increment - may still be optimized */
    }
    
    /* Backward traversal with different pattern */
    ptr = end - 1;
    while (ptr >= arr) {
        *ptr = *ptr / 2.0;
        ptr--;  /* Separate decrement */
    }
}

/* Function 4: Struct traversal with pointer arithmetic */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    int total_x = 0;
    
    for (int i = 0; i < n; i++) {
        total_x += ptr->x;
        ptr++;  /* Pointer increment by struct size */
    }
    
    return total_x;
}

/* Function 5: Volatile pointer access - prevents some optimizations */
int volatile_sum(const int* arr, int n) {
    const int* ptr = arr;
    volatile int* volatile_ptr = (volatile int*)arr;
    int sum = 0;
    
    /* Mix volatile and non-volatile accesses */
    for (int i = 0; i < n; i++) {
        sum += *ptr++;          /* Non-volatile with post-increment */
        (void)*volatile_ptr++;  /* Volatile access - pattern preserved */
    }
    
    return sum;
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_multiply(int a[SIZE][SIZE], int b[SIZE][SIZE], int result[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            int sum = 0;
            int* a_ptr = &a[i][0];
            int* b_ptr = &b[0][j];
            
            /* Inner loop with pointer increments */
            for (int k = 0; k < SIZE; k++) {
                sum += *a_ptr++ * *b_ptr;
                b_ptr += SIZE;  /* Move to next row */
            }
            
            result[i][j] = sum;
        }
    }
}

/* Function 7: Array initialization with index post-increment */
void init_array(int* arr, int n) {
    /* Index-based post-increment */
    for (int i = 0; i < n; i++) {
        arr[i] = i * 2;  /* May generate different pattern */
    }
    
    /* Alternative with explicit post-increment */
    int idx = 0;
    while (idx < n) {
        arr[idx] = idx * 3;
        idx++;  /* Separate increment */
    }
}

/* Function 8: Char array processing with mixed patterns */
void process_chars(char* str, int len) {
    char* front = str;
    char* back = str + len - 1;
    
    /* Process from both ends */
    while (front < back) {
        *front = toupper(*front);
        front++;  /* Forward increment */
        
        *back = tolower(*back);
        back--;   /* Backward decrement */
    }
}

int main() {
    /* Local arrays (stack-based) */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct Point points[SMALL_SIZE];
    
    /* Initialize arrays */
    init_array(int_array, SIZE);
    
    for (int i = 0; i < SIZE; i++) {
        char_array[i] = 'a' + (i % 26);
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        double_array[i] = i * 1.5;
        points[i].x = i;
        points[i].y = i * 2;
        snprintf(points[i].label, 8, "P%d", i);
    }
    
    /* Test different patterns */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    char reversed[SIZE];
    reverse_copy(reversed, char_array, SIZE);
    printf("First 10 reversed chars: %.10s\n", reversed);
    
    process_doubles(double_array, SMALL_SIZE);
    printf("Processed double[0]: %f\n", double_array[0]);
    
    int sum_points_x = sum_points(points, SMALL_SIZE);
    printf("Sum of points x: %d\n", sum_points_x);
    
    int volatile_sum_result = volatile_sum(int_array, SIZE);
    printf("Volatile sum: %d\n", volatile_sum_result);
    
    /* Small matrix test */
    int a[10][10], b[10][10], result[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            a[i][j] = i + j;
            b[i][j] = i - j;
        }
    }
    matrix_multiply(a, b, result);
    printf("Matrix result[0][0]: %d\n", result[0][0]);
    
    /* Char processing */
    char test_str[] = "HelloWorld";
    process_chars(test_str, strlen(test_str));
    printf("Processed string: %s\n", test_str);
    
    /* Additional test with pointer in loop condition */
    int* ptr = int_array;
    int* end = int_array + SIZE;
    int final_check = 0;
    
    /* While loop with pointer comparison */
    while (ptr != end) {
        final_check ^= *ptr++;  /* XOR with post-increment */
    }
    printf("Final XOR result: %d\n", final_check);
    
    return 0;
}
