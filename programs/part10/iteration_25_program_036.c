#include <stdio.h>
#include <stdint.h>

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
    
    /* Classic pointer traversal with post-increment */
    while (ptr < end) {
        sum += *ptr++;  /* This should generate post-increment RTL */
    }
    
    /* Use volatile to prevent loop unrolling */
    volatile int dummy = sum;
    return dummy;
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

/* Function 3: Mixed operations with different data types */
void process_structs(struct Point* points, int n) {
    struct Point* ptr = points;
    struct Point* end = ptr + n;
    
    /* Struct pointer with post-increment */
    while (ptr < end) {
        ptr->x = ptr->y * 2;
        ptr->label = 'A' + (ptr->x % 26);
        ptr++;  /* Post-increment in separate statement */
    }
}

/* Function 4: Char array with volatile pointer */
int count_chars(const char* str) {
    volatile const char* ptr = str;  /* Volatile to prevent optimization */
    int count = 0;
    
    /* Volatile pointer with post-increment */
    while (*ptr != '\0') {
        if (*ptr == 'a') count++;
        ptr++;  /* Post-increment */
    }
    
    return count;
}

/* Function 5: Double array with stride */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    
    /* Pointer with stride (ptr += 2) */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;  /* Stride of 2 */
    }
    
    return sum;
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_multiply(const int* a, const int* b, int* result, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int sum = 0;
            const int* a_ptr = a + i * n;
            const int* b_ptr = b + j;
            
            /* Inner loop with pointer auto-increment */
            for (int k = 0; k < n; k++) {
                sum += *a_ptr++ * *b_ptr;
                b_ptr += n;  /* Move to next column */
            }
            
            result[i * n + j] = sum;
        }
    }
}

/* Function 7: Array initialization with index post-increment */
void init_array(int* arr, int n) {
    /* Index-based post-increment */
    for (int i = 0; i < n; ) {
        arr[i] = i++;
        /* The i++ happens in the array index calculation */
    }
}

/* Function 8: Store operations with auto-increment */
void fill_pattern(char* buffer, int size, char pattern) {
    char* ptr = buffer;
    char* end = buffer + size;
    
    /* Store with post-increment */
    while (ptr < end) {
        *ptr++ = pattern++;  /* Both store and pattern use post-increment */
    }
}

int main() {
    /* Declare arrays of different types */
    int int_array[SIZE];
    int int_array2[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SMALL_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i * 2;
        char_array[i] = 'a' + (i % 26);
        double_array[i] = i * 1.5;
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + i;
    }
    
    /* Test 1: Forward traversal with post-increment */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int_array: %d\n", sum);
    
    /* Test 2: Reverse copy with post-decrement */
    reverse_copy(int_array2, int_array, SIZE);
    printf("First element after reverse copy: %d\n", int_array2[0]);
    
    /* Test 3: Struct processing */
    process_structs(points, SMALL_SIZE);
    printf("Processed %d structs\n", SMALL_SIZE);
    
    /* Test 4: Char counting with volatile */
    int char_count = count_chars(char_array);
    printf("Found 'a' %d times\n", char_count);
    
    /* Test 5: Stride access */
    double dsum = sum_every_other(double_array, SIZE);
    printf("Sum of every other double: %f\n", dsum);
    
    /* Test 6: Nested loops */
    int small_size = 5;
    int mat_a[25], mat_b[25], mat_result[25];
    for (int i = 0; i < 25; i++) {
        mat_a[i] = i;
        mat_b[i] = i * 2;
    }
    matrix_multiply(mat_a, mat_b, mat_result, small_size);
    printf("Matrix multiply result[0]: %d\n", mat_result[0]);
    
    /* Test 7: Index-based post-increment initialization */
    int init_array_test[10];
    init_array(init_array_test, 10);
    printf("Init array first element: %d\n", init_array_test[0]);
    
    /* Test 8: Store operations */
    char buffer[20];
    fill_pattern(buffer, 20, 'A');
    printf("Buffer first char: %c\n", buffer[0]);
    
    /* Use results to prevent dead code elimination */
    volatile int final_check = sum + char_count + (int)dsum + mat_result[0];
    
    return final_check > 0 ? 0 : 1;
}
