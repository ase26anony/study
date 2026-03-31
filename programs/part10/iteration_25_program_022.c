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
    
    /* Classic pattern: *ptr++ in while loop */
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(char* dest, const char* src, int n) {
    dest += n - 1;
    src += n - 1;
    
    /* Pattern: *dest-- = *src-- */
    for (int i = 0; i < n; i++) {
        *dest-- = *src--;
    }
}

/* Function 3: Mixed operations with different data types */
void process_arrays(int* int_arr, double* dbl_arr, struct Point* point_arr, int n) {
    int* int_ptr = int_arr;
    double* dbl_ptr = dbl_arr;
    struct Point* point_ptr = point_arr;
    
    /* Multiple post-increment patterns in same function */
    for (int i = 0; i < n; i++) {
        /* Store operation with post-increment */
        *int_ptr++ = i * 2;
        
        /* Load and store with different types */
        *dbl_ptr++ = (double)(*int_arr++) / 10.0;
        
        /* Struct access with pointer increment */
        point_ptr->x = i;
        point_ptr->y = i * 2;
        point_ptr++;
    }
}

/* Function 4: Volatile pointer traversal */
int volatile_sum(volatile int* arr, int n) {
    volatile int* ptr = arr;
    int sum = 0;
    
    /* Volatile prevents some optimizations, keeping pattern intact */
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    return sum;
}

/* Function 5: Nested loops with inner auto-increment */
void matrix_multiply(const int* a, const int* b, int* result, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int sum = 0;
            const int* a_ptr = a + i * size;
            const int* b_ptr = b + j;
            
            /* Inner loop with pointer increment by stride */
            for (int k = 0; k < size; k++) {
                sum += *a_ptr++ * *b_ptr;
                b_ptr += size;  /* Stride access, not simple increment */
            }
            *result++ = sum;
        }
    }
}

/* Function 6: Index-based post-increment */
void init_with_index(int* arr, int n) {
    /* Classic for loop with array[index++] pattern */
    for (int i = 0; i < n; i++) {
        arr[i] = i * 3;  /* Compiler may recognize this as post-increment pattern */
    }
}

/* Function 7: Pointer arithmetic with different strides */
void stride_access(char* data, int n) {
    char* ptr = data;
    
    /* Access every other element */
    for (int i = 0; i < n / 2; i++) {
        char temp = *ptr;
        ptr += 2;  /* Stride of 2 */
        
        /* Force compiler to keep the increment */
        asm volatile("" : "+r"(ptr) : : "memory");
    }
}

int main() {
    /* Local arrays (not static/global) to encourage stack addressing */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point point_array[SMALL_SIZE];
    int result_array[SMALL_SIZE * SMALL_SIZE];
    
    /* Initialize arrays with volatile to prevent pre-optimization */
    volatile int init_val = 0;
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = init_val + i;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 1.5;
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        point_array[i].x = i;
        point_array[i].y = i * 2;
        snprintf(point_array[i].label, 8, "P%d", i);
    }
    
    /* Test 1: Forward pointer traversal */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum1);
    
    /* Test 2: Reverse copy with post-decrement */
    char char_copy[SIZE];
    reverse_copy(char_copy, char_array, SIZE);
    printf("First char in copy: %c\n", char_copy[0]);
    
    /* Test 3: Mixed operations */
    process_arrays(int_array, double_array, point_array, SMALL_SIZE);
    printf("Processed point[5].x = %d\n", point_array[5].x);
    
    /* Test 4: Volatile access */
    volatile int* volatile_ptr = int_array;
    int sum2 = volatile_sum(volatile_ptr, SIZE / 2);
    printf("Volatile sum: %d\n", sum2);
    
    /* Test 5: Nested loops */
    int matrix_a[SMALL_SIZE * SMALL_SIZE];
    int matrix_b[SMALL_SIZE * SMALL_SIZE];
    
    for (int i = 0; i < SMALL_SIZE * SMALL_SIZE; i++) {
        matrix_a[i] = i;
        matrix_b[i] = i * 2;
    }
    
    matrix_multiply(matrix_a, matrix_b, result_array, SMALL_SIZE);
    printf("Matrix result[0] = %d\n", result_array[0]);
    
    /* Test 6: Index-based initialization */
    int new_array[SIZE];
    init_with_index(new_array, SIZE);
    printf("new_array[10] = %d\n", new_array[10]);
    
    /* Test 7: Stride access */
    stride_access(char_array, SIZE);
    
    /* Use results to prevent dead code elimination */
    int total = sum1 + sum2 + new_array[10] + result_array[0];
    printf("Total checksum: %d\n", total);
    
    return total > 0 ? 0 : 1;
}
