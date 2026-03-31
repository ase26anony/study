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
    int sum = 0;
    int i = 0;
    
    /* Use volatile to prevent loop unrolling */
    volatile int limit = n;
    
    while (i < limit) {
        sum += *ptr++;  /* Post-increment access - target for optimization */
        i++;
    }
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(char* dest, const char* src, int n) {
    const char* src_ptr = src + n - 1;
    char* dest_ptr = dest + n - 1;
    
    /* Use volatile to keep the pattern */
    volatile int count = n;
    
    while (count-- > 0) {
        *dest_ptr-- = *src_ptr--;  /* Post-decrement access */
    }
}

/* Function 3: Mixed operations with different strides */
void process_doubles(double* arr, int n) {
    double* ptr = arr;
    volatile int iterations = n;
    
    /* Forward with stride of 2 */
    for (int i = 0; i < iterations; i += 2) {
        double temp = *ptr;
        ptr += 2;  /* Stride of 2 - may trigger different pattern */
        /* Use the value to prevent elimination */
        arr[i/2] = temp * 2.0;
    }
}

/* Function 4: Struct traversal with pointer arithmetic */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    int total = 0;
    
    /* Prevent optimization with volatile */
    volatile int limit = n;
    int i = 0;
    
    while (i < limit) {
        total += ptr->x + ptr->y;  /* Access struct members */
        ptr++;  /* Post-increment of struct pointer */
        i++;
    }
    return total;
}

/* Function 5: Index-based post-increment */
void initialize_buffer(int* buffer, int n) {
    int index = 0;
    
    /* Classic index-based post-increment */
    for (int i = 0; i < n; i++) {
        buffer[index++] = i * 2;  /* Post-increment of index */
    }
}

/* Function 6: Volatile pointer traversal */
unsigned char checksum(const unsigned char* data, int length) {
    volatile const unsigned char* ptr = data;  /* Volatile pointer */
    unsigned char sum = 0;
    int i = 0;
    
    /* Use while loop to encourage pointer arithmetic */
    while (i < length) {
        sum ^= *ptr++;  /* Post-increment with volatile pointer */
        i++;
    }
    return sum;
}

/* Function 7: Nested loops with inner auto-increment */
void matrix_multiply(const int* a, const int* b, int* result, int n) {
    for (int i = 0; i < n; i++) {
        const int* row_ptr = a + i * n;
        for (int j = 0; j < n; j++) {
            const int* col_ptr = b + j;
            int sum = 0;
            
            /* Inner loop with pointer auto-increment */
            for (int k = 0; k < n; k++) {
                sum += *row_ptr++ * *col_ptr;
                col_ptr += n;  /* Move to next element in column */
            }
            
            /* Reset row pointer for next column */
            row_ptr = a + i * n;
            result[i * n + j] = sum;
        }
    }
}

int main(void) {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    unsigned char byte_array[SIZE];
    struct Point points[SMALL_SIZE];
    int buffer[SIZE];
    int matrix_a[SMALL_SIZE * SMALL_SIZE];
    int matrix_b[SMALL_SIZE * SMALL_SIZE];
    int matrix_result[SMALL_SIZE * SMALL_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 1.5;
        byte_array[i] = i & 0xFF;
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        snprintf(points[i].label, sizeof(points[i].label), "P%d", i);
    }
    
    /* Initialize matrices */
    for (int i = 0; i < SMALL_SIZE * SMALL_SIZE; i++) {
        matrix_a[i] = i;
        matrix_b[i] = i * 2;
    }
    
    /* Call functions to trigger optimizations */
    
    /* 1. Forward pointer traversal with post-increment */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* 2. Reverse copy with post-decrement */
    char reversed[SIZE];
    reverse_copy(reversed, char_array, SIZE);
    printf("First char after reverse: %c\n", reversed[0]);
    
    /* 3. Double array with stride */
    process_doubles(double_array, SIZE);
    printf("Double array[0]: %f\n", double_array[0]);
    
    /* 4. Struct traversal */
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* 5. Index-based post-increment */
    initialize_buffer(buffer, SIZE);
    printf("Buffer[0]: %d, Buffer[%d]: %d\n", buffer[0], SIZE-1, buffer[SIZE-1]);
    
    /* 6. Volatile pointer traversal */
    unsigned char csum = checksum(byte_array, SIZE);
    printf("Checksum: 0x%02X\n", csum);
    
    /* 7. Nested loops with inner auto-increment */
    matrix_multiply(matrix_a, matrix_b, matrix_result, SMALL_SIZE);
    printf("Matrix result[0][0]: %d\n", matrix_result[0]);
    
    /* Additional test: Mixed operations in main */
    int* ptr = int_array;
    int local_sum = 0;
    
    /* Another pointer traversal loop */
    for (int i = 0; i < SIZE; i++) {
        local_sum += *ptr++;
    }
    printf("Local sum: %d\n", local_sum);
    
    /* Backward traversal */
    int* rev_ptr = int_array + SIZE - 1;
    int rev_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        rev_sum += *rev_ptr--;
    }
    printf("Reverse sum: %d\n", rev_sum);
    
    return 0;
}
