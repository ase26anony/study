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
    dest += n - 1;
    src += n - 1;
    
    /* Pattern: *dest-- = *src-- */
    for (int i = 0; i < n; i++) {
        *dest-- = *src--;
    }
}

/* Function 3: Mixed operations with different data types */
void process_data() {
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct Point points[SMALL_SIZE];
    
    /* Initialize with index post-increment */
    for (int i = 0; i < SIZE; i++) {
        char_array[i] = (char)(i % 256);
    }
    
    /* Pointer traversal with stride (ptr += 2) */
    double* dptr = double_array;
    for (int i = 0; i < SMALL_SIZE; i += 2) {
        *dptr = i * 1.5;
        dptr += 2;  /* Non-unity stride */
    }
    
    /* Struct traversal with pointer */
    struct Point* pptr = points;
    for (int i = 0; i < SMALL_SIZE; i++) {
        pptr->x = i;
        pptr->y = i * 2;
        pptr->label = 'A' + (i % 26);
        pptr++;  /* Post-increment on struct pointer */
    }
}

/* Function 4: Store operations with post-increment */
void initialize_buffer(int* buffer, int n, int value) {
    int* ptr = buffer;
    int* end = buffer + n;
    
    /* Pattern: *ptr++ = value */
    while (ptr < end) {
        *ptr++ = value++;
    }
}

/* Function 5: Volatile pointer access */
int volatile_sum(volatile int* arr, int n) {
    volatile int* vptr = arr;
    int sum = 0;
    
    /* Volatile prevents some optimizations */
    for (int i = 0; i < n; i++) {
        sum += *vptr;
        vptr++;  /* Post-increment on volatile pointer */
    }
    return sum;
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_operation(int rows, int cols, int matrix[rows][cols]) {
    for (int i = 0; i < rows; i++) {
        int* row_ptr = matrix[i];
        
        /* Inner loop with pointer traversal */
        for (int j = 0; j < cols; j++) {
            row_ptr[j] = i * cols + j;
        }
    }
}

/* Function 7: Backward traversal with post-decrement */
int find_last_nonzero(const int* arr, int n) {
    const int* ptr = arr + n - 1;
    
    /* Pattern: while loop with *ptr-- */
    for (int i = n - 1; i >= 0; i--) {
        if (*ptr != 0) {
            return i;
        }
        ptr--;
    }
    return -1;
}

/* Function 8: Mixed increment/decrement in same function */
void bidirectional_process(int* forward, int* backward, int n) {
    int* fptr = forward;
    int* bptr = backward + n - 1;
    
    for (int i = 0; i < n; i++) {
        *fptr++ = i;        /* Post-increment */
        *bptr-- = n - i;    /* Post-decrement */
    }
}

int main() {
    int array1[SIZE];
    int array2[SIZE];
    int result[SIZE];
    volatile int volatile_array[SMALL_SIZE];
    
    /* Prevent over-optimization with volatile in loop conditions */
    volatile int limit = SIZE;
    
    /* 1. Initialize arrays */
    initialize_buffer(array1, SIZE, 1);
    
    /* 2. Sum with forward pointer traversal */
    int total = sum_array(array1, limit);
    printf("Sum: %d\n", total);
    
    /* 3. Reverse copy */
    reverse_copy(array2, array1, SIZE);
    
    /* 4. Process different data types */
    process_data();
    
    /* 5. Volatile access */
    for (int i = 0; i < SMALL_SIZE; i++) {
        volatile_array[i] = i * 2;
    }
    int volatile_total = volatile_sum(volatile_array, SMALL_SIZE);
    printf("Volatile sum: %d\n", volatile_total);
    
    /* 6. Matrix operation */
    int matrix[5][5];
    matrix_operation(5, 5, matrix);
    
    /* 7. Backward search */
    int last_nonzero = find_last_nonzero(array1, SIZE);
    printf("Last nonzero at: %d\n", last_nonzero);
    
    /* 8. Bidirectional processing */
    bidirectional_process(array1, result, SIZE);
    
    /* 9. Additional pattern: char pointer with post-increment */
    char message[] = "Test string for char pointer traversal";
    char* cptr = message;
    int char_count = 0;
    
    while (*cptr != '\0') {
        if (*cptr == ' ') {
            char_count++;
        }
        cptr++;  /* Char pointer post-increment */
    }
    printf("Space count: %d\n", char_count);
    
    /* 10. Loop with post-increment in array access */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += array1[i];  /* Array index access */
    }
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}
