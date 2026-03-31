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
    
    /* Classic post-increment pattern */
    while (ptr < end) {
        sum += *ptr++;  /* This should generate post-increment RTL */
    }
    
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Post-decrement pattern */
    while (src_end >= src) {
        *dest_end-- = *src_end--;  /* Both src and dest use post-decrement */
    }
}

/* Function 3: Mixed operations with different data types */
void process_structs(struct Point* points, int n) {
    struct Point* ptr = points;
    volatile struct Point* vptr = points;  /* volatile to prevent optimization */
    int i = 0;
    
    /* Loop with struct pointer post-increment */
    while (i++ < n) {
        ptr->x = i;
        ptr->y = i * 2;
        ptr->label = 'A' + (i % 26);
        ptr++;  /* Post-increment after use */
    }
    
    /* Separate volatile access loop */
    for (i = 0; i < n; i++) {
        /* Force memory access pattern to survive optimization */
        char temp = vptr->label;
        (void)temp;  /* Use result to avoid dead code elimination */
        vptr++;  /* Post-increment of volatile pointer */
    }
}

/* Function 4: Array initialization with index post-increment */
void init_buffer(char* buffer, int n) {
    int i = 0;
    
    /* Index-based post-increment */
    while (i < n) {
        buffer[i] = (i % 26) + 'a';
        i++;  /* Post-increment */
    }
}

/* Function 5: Double precision with stride */
double sum_doubles(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    
    /* Simple post-increment with doubles */
    while (ptr < end) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_process(int matrix[][10], int rows) {
    for (int i = 0; i < rows; i++) {
        int* row_ptr = matrix[i];
        volatile int* v_row_ptr = matrix[i];
        
        /* Inner loop with pointer traversal */
        for (int j = 0; j < 10; j++) {
            *row_ptr++ = i * 10 + j;  /* Post-increment store */
        }
        
        /* Separate volatile read loop */
        for (int j = 0; j < 10; j++) {
            int val = *v_row_ptr++;  /* Post-increment load */
            (void)val;
        }
    }
}

/* Function 7: Backward traversal with char array */
int count_chars_backwards(const char* str, int len, char target) {
    const char* ptr = str + len - 1;
    int count = 0;
    volatile const char* vptr = str + len - 1;
    
    /* Backward traversal with post-decrement */
    while (ptr >= str) {
        if (*ptr-- == target) {  /* Post-decrement in condition */
            count++;
        }
    }
    
    /* Volatile version to ensure pattern survives */
    for (int i = 0; i < len; i++) {
        char c = *vptr--;  /* Post-decrement */
        (void)c;
    }
    
    return count;
}

int main() {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct Point points[SMALL_SIZE];
    int matrix[5][10];
    
    /* Initialize arrays with volatile to prevent compile-time optimization */
    volatile int init_val = 1;
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = init_val + i;
        char_array[i] = 'a' + (i % 26);
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        double_array[i] = (double)i * 1.5;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    /* Call functions to trigger various auto-inc/dec patterns */
    
    /* 1. Forward pointer post-increment */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* 2. Reverse copy with post-decrement */
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    printf("First element of reversed: %d\n", reversed[0]);
    
    /* 3. Struct processing with post-increment */
    process_structs(points, SMALL_SIZE);
    printf("First struct label: %c\n", points[0].label);
    
    /* 4. Index-based post-increment */
    init_buffer(char_array, SIZE);
    printf("First char: %c\n", char_array[0]);
    
    /* 5. Double precision post-increment */
    double dsum = sum_doubles(double_array, SMALL_SIZE);
    printf("Sum of doubles: %f\n", dsum);
    
    /* 6. Nested loops with inner auto-increment */
    matrix_process(matrix, 5);
    printf("Matrix[0][0]: %d\n", matrix[0][0]);
    
    /* 7. Backward traversal with post-decrement */
    const char* test_str = "Hello, World!";
    int len = 13;  /* Length of "Hello, World!" */
    int l_count = count_chars_backwards(test_str, len, 'l');
    printf("Count of 'l' (backwards): %d\n", l_count);
    
    /* Additional pattern: Pointer arithmetic with different strides */
    int* ptr = int_array;
    volatile int* vptr = int_array;
    
    /* Simple stride of 2 - may trigger different pattern */
    for (int i = 0; i < SIZE/2; i++) {
        int val = *ptr;
        ptr += 2;  /* Not post-increment, but tests other paths */
        (void)val;
    }
    
    /* Force use of all results to prevent dead code elimination */
    volatile int result = sum + reversed[0] + points[0].label + char_array[0] 
                         + (int)dsum + matrix[0][0] + l_count;
    
    return 0;
}
