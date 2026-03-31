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
        *dest_end-- = *src_end--;  /* Both pointers use post-decrement */
    }
}

/* Function 3: Mixed operations with different types */
void process_structs(struct Point* points, int n) {
    struct Point* ptr = points;
    volatile struct Point* vptr = points;  /* Volatile to prevent optimization */
    
    /* Process with non-volatile pointer */
    for (int i = 0; i < n; i++) {
        ptr->x = i;
        ptr->y = i * 2;
        ptr->label = 'A' + (i % 26);
        ptr++;  /* Explicit post-increment */
    }
    
    /* Read back with volatile pointer */
    for (int i = 0; i < n; i++) {
        volatile char c = vptr->label;  /* Force memory read */
        (void)c;  /* Use the value */
        vptr++;  /* Volatile pointer increment */
    }
}

/* Function 4: Array initialization with index post-increment */
void init_buffer(char* buffer, int n) {
    /* Index-based post-increment */
    for (int i = 0; i < n; ) {
        buffer[i] = (char)i;
        i++;  /* Post-increment in separate statement */
    }
}

/* Function 5: Strided access pattern */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    int count = 0;
    
    /* Strided access - may not trigger the specific uncovered code
       but tests other parts of the optimization */
    while (ptr < end) {
        sum += *ptr;
        ptr += 2;  /* Stride of 2 */
        count++;
        if (count > n/2) break;  /* Safety check */
    }
    return sum;
}

/* Function 6: Nested loops with inner auto-increment */
void matrix_traversal(int matrix[][SMALL_SIZE], int rows) {
    for (int i = 0; i < rows; i++) {
        int* row_ptr = matrix[i];
        volatile int* v_row_ptr = matrix[i];
        
        /* Inner loop with pointer traversal */
        for (int j = 0; j < SMALL_SIZE; j++) {
            row_ptr[j] = i * SMALL_SIZE + j;
        }
        
        /* Another inner loop with explicit pointer */
        for (int j = 0; j < SMALL_SIZE; j++) {
            *v_row_ptr = *v_row_ptr * 2;
            v_row_ptr++;  /* Post-increment on volatile pointer */
        }
    }
}

/* Function 7: Complex pattern with multiple increments */
void complex_pattern(int* data, int n) {
    int* p1 = data;
    int* p2 = data + n/2;
    volatile int* vp = data;
    
    /* Multiple pointers with post-increments */
    for (int i = 0; i < n/2; i++) {
        *p2++ = *p1++ * 2;  /* Both pointers post-increment */
        *vp = *vp + 1;      /* Volatile access */
        vp++;               /* Volatile pointer increment */
    }
}

int main(void) {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SIZE];
    struct Point points[SIZE];
    int matrix[SMALL_SIZE][SMALL_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i;
        char_array[i] = 'A' + (i % 26);
        double_array[i] = i * 1.5;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    /* Initialize matrix */
    for (int i = 0; i < SMALL_SIZE; i++) {
        for (int j = 0; j < SMALL_SIZE; j++) {
            matrix[i][j] = i * SMALL_SIZE + j;
        }
    }
    
    /* Call functions to trigger optimizations */
    
    /* 1. Simple forward traversal with post-increment */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* 2. Reverse copy with post-decrement */
    int reversed[SIZE];
    reverse_copy(reversed, int_array, SIZE);
    printf("First element of reversed: %d\n", reversed[0]);
    
    /* 3. Struct processing with mixed volatile/non-volatile */
    process_structs(points, SMALL_SIZE);
    printf("Processed %d structs\n", SMALL_SIZE);
    
    /* 4. Index-based initialization */
    char buffer[SIZE];
    init_buffer(buffer, SIZE);
    printf("Buffer[50] = %d\n", (int)buffer[50]);
    
    /* 5. Strided access */
    double strided_sum = sum_every_other(double_array, SIZE);
    printf("Strided sum: %f\n", strided_sum);
    
    /* 6. Nested loops */
    matrix_traversal(matrix, SMALL_SIZE);
    printf("Matrix[0][0] = %d\n", matrix[0][0]);
    
    /* 7. Complex pattern */
    complex_pattern(int_array, SIZE);
    printf("int_array[0] = %d\n", int_array[0]);
    
    /* Additional volatile pointer loop */
    volatile char* vc_ptr = char_array;
    volatile char vc_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        vc_sum += *vc_ptr++;  /* Volatile pointer with post-increment */
    }
    printf("Volatile char sum: %d\n", (int)vc_sum);
    
    /* Use results to prevent dead code elimination */
    int total = sum + reversed[0] + (int)buffer[50] + (int)strided_sum + 
                matrix[0][0] + int_array[0] + (int)vc_sum;
    printf("Total checksum: %d\n", total);
    
    return 0;
}
