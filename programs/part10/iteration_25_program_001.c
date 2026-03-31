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
    
    /* Classic pattern: *ptr++ in while loop */
    while (ptr < end) {
        sum += *ptr++;
    }
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_end = src + n - 1;
    int* dest_end = dest + n - 1;
    
    /* Pattern: *dest-- = *src-- */
    while (src_end >= src) {
        *dest_end-- = *src_end--;
    }
}

/* Function 3: Mixed operations with different types */
void process_data(void) {
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct Point point_array[SMALL_SIZE];
    
    /* Pattern 1: Index with post-increment for char array */
    volatile char* vptr = char_array;
    for (int i = 0; i < SIZE; i++) {
        char_array[i] = (char)(i % 256);
    }
    
    /* Pattern 2: Pointer post-increment with volatile */
    volatile char* volatile_ptr = char_array;
    for (int i = 0; i < SIZE; i++) {
        char val = *volatile_ptr++;
        (void)val; /* Use value to prevent elimination */
    }
    
    /* Pattern 3: Double array with stride (ptr += 2) */
    double* dptr = double_array;
    for (int i = 0; i < SMALL_SIZE; i += 2) {
        *dptr = i * 1.5;
        dptr += 2;  /* Stride of 2 */
    }
    
    /* Pattern 4: Struct array with pointer post-increment */
    struct Point* ppt = point_array;
    for (int i = 0; i < SMALL_SIZE; i++) {
        ppt->x = i;
        ppt->y = i * 2;
        ppt->label = 'A' + (i % 26);
        ppt++;  /* Post-increment after use */
    }
}

/* Function 4: Nested loops with inner auto-increment */
void matrix_operation(int rows, int cols, int matrix[rows][cols]) {
    for (int i = 0; i < rows; i++) {
        int* row_ptr = matrix[i];
        /* Inner loop with pointer post-increment */
        for (int j = 0; j < cols; j++) {
            *row_ptr++ = i * cols + j;
        }
    }
}

/* Function 5: Backward traversal with post-decrement */
int find_last_nonzero(const int* arr, int n) {
    const int* ptr = arr + n - 1;
    int count = 0;
    
    /* Pattern: while loop with *ptr-- */
    while (ptr >= arr && count < 5) {  /* Limit iterations to avoid infinite */
        if (*ptr != 0) {
            return (int)(ptr - arr);
        }
        ptr--;
        count++;
    }
    return -1;
}

/* Function 6: Store operations with post-increment */
void initialize_buffer(char* buffer, int size, char value) {
    char* ptr = buffer;
    char* end = buffer + size;
    
    /* Pattern: *ptr++ = value (store with post-increment) */
    while (ptr < end) {
        *ptr++ = value;
    }
}

int main(void) {
    int array1[SIZE];
    int array2[SIZE];
    int matrix[10][10];
    char buffer[SIZE];
    
    /* Initialize arrays with index post-increment pattern */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i * 2;
    }
    
    /* Test forward traversal */
    int sum = sum_array(array1, SIZE);
    printf("Sum: %d\n", sum);
    
    /* Test reverse copy */
    reverse_copy(array2, array1, SIZE);
    
    /* Test nested loops */
    matrix_operation(10, 10, matrix);
    
    /* Test store operations */
    initialize_buffer(buffer, SIZE, 'X');
    
    /* Test backward traversal */
    int last_pos = find_last_nonzero(array1, SIZE);
    printf("Last non-zero position: %d\n", last_pos);
    
    /* Test mixed operations */
    process_data();
    
    /* Use results to prevent dead code elimination */
    volatile int prevent_opt = sum + last_pos + buffer[0] + matrix[5][5];
    
    return 0;
}
