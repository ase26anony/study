#include <stdio.h>
#include <stdint.h>

#define SIZE 100
#define SMALL_SIZE 20

struct Point {
    int x;
    int y;
    char label;
};

/* Function 1: Pointer-based forward traversal with post-increment */
int sum_array(const int* arr, int n) {
    const int* ptr = arr;
    const int* end = arr + n;
    int sum = 0;
    
    /* Classic post-increment pattern: *ptr++ */
    while (ptr < end) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Function 2: Pointer-based backward traversal with post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_ptr = src + n - 1;
    int* dest_ptr = dest + n - 1;
    
    /* Post-decrement pattern: *dest-- = *src-- */
    while (src_ptr >= src) {
        *dest_ptr-- = *src_ptr--;
    }
}

/* Function 3: Mixed load/store with post-increment */
void scale_array(int* arr, int n, int factor) {
    int* ptr = arr;
    int* end = arr + n;
    
    /* Both load and store with post-increment */
    while (ptr < end) {
        int val = *ptr;      /* Load */
        *ptr++ = val * factor; /* Store with post-increment */
    }
}

/* Function 4: Volatile pointer traversal */
volatile int volatile_sum(const int* arr, int n) {
    volatile int sum = 0;
    volatile const int* volatile_ptr = (volatile const int*)arr;
    volatile const int* volatile_end = (volatile const int*)(arr + n);
    
    /* Volatile access with post-increment */
    while (volatile_ptr < volatile_end) {
        sum += *volatile_ptr++;
    }
    
    return sum;
}

/* Function 5: Struct array traversal with pointer arithmetic */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    const struct Point* end = points + n;
    int total = 0;
    
    /* Struct access with post-increment */
    while (ptr < end) {
        total += ptr->x + ptr->y;
        ptr++;
    }
    
    return total;
}

/* Function 6: Index-based traversal with post-increment */
void initialize_array(int* arr, int n) {
    int i = 0;
    
    /* Index-based post-increment: arr[i++] */
    while (i < n) {
        arr[i++] = i * 2;  /* Post-increment of index */
    }
}

/* Function 7: Double array with stride (ptr += 2) */
double sum_every_other(const double* arr, int n) {
    const double* ptr = arr;
    const double* end = arr + n;
    double sum = 0.0;
    int count = 0;
    
    /* Stride of 2 with post-increment */
    while (ptr < end && count < n/2) {
        sum += *ptr;
        ptr += 2;  /* Stride, not simple increment */
        count++;
    }
    
    return sum;
}

/* Function 8: Nested loops with inner auto-increment */
void matrix_sum_rows(const int matrix[][10], int rows, int* row_sums) {
    for (int i = 0; i < rows; i++) {
        const int* row_ptr = matrix[i];
        const int* row_end = row_ptr + 10;
        int sum = 0;
        
        /* Inner loop with post-increment */
        while (row_ptr < row_end) {
            sum += *row_ptr++;
        }
        
        row_sums[i] = sum;
    }
}

/* Function 9: Char array with post-decrement */
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

/* Function 10: Mixed types and operations */
void process_buffer(char* buffer, int size) {
    char* read_ptr = buffer;
    char* write_ptr = buffer;
    char* end = buffer + size;
    
    /* Multiple pointers with post-increment */
    while (read_ptr < end) {
        char val = *read_ptr++;
        if (val != 0) {
            *write_ptr++ = val;
        }
    }
    
    /* Null-terminate */
    if (write_ptr < buffer + size) {
        *write_ptr = '\0';
    }
}

int main(void) {
    /* Declare and initialize arrays of different types */
    int int_array[SIZE];
    int int_array2[SIZE];
    double double_array[SIZE];
    char char_array[SIZE];
    struct Point points[SMALL_SIZE];
    
    /* Volatile array to prevent over-optimization */
    volatile int volatile_array[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        double_array[i] = (i + 1) * 1.5;
        char_array[i] = 'A' + (i % 26);
        if (i < SIZE) {
            volatile_array[i] = i * 3;
        }
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + (i % 26);
    }
    
    /* Test 1: Pointer forward traversal */
    int sum1 = sum_array(int_array, SIZE);
    printf("Sum of int_array: %d\n", sum1);
    
    /* Test 2: Reverse copy with post-decrement */
    reverse_copy(int_array2, int_array, SIZE);
    printf("First element of reversed copy: %d\n", int_array2[0]);
    
    /* Test 3: Scale array with mixed load/store */
    scale_array(int_array, SIZE, 2);
    printf("Scaled element at index 10: %d\n", int_array[10]);
    
    /* Test 4: Volatile access */
    volatile int vsum = volatile_sum(int_array, SIZE);
    printf("Volatile sum: %d\n", vsum);
    
    /* Test 5: Struct traversal */
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* Test 6: Index-based initialization */
    int init_array[SIZE];
    initialize_array(init_array, SIZE);
    printf("Initialized array element 5: %d\n", init_array[5]);
    
    /* Test 7: Stride access */
    double stride_sum = sum_every_other(double_array, SIZE);
    printf("Sum of every other double: %.2f\n", stride_sum);
    
    /* Test 8: Nested loops */
    int matrix[5][10];
    int row_sums[5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    matrix_sum_rows(matrix, 5, row_sums);
    printf("Matrix row 2 sum: %d\n", row_sums[2]);
    
    /* Test 9: String reversal */
    char test_string[] = "Hello, World!";
    int len = sizeof(test_string) - 1;
    reverse_string(test_string, len);
    printf("Reversed string: %s\n", test_string);
    
    /* Test 10: Mixed pointer operations */
    char buffer[SIZE];
    for (int i = 0; i < SIZE; i++) {
        buffer[i] = (i % 3 == 0) ? 0 : 'A' + (i % 26);
    }
    process_buffer(buffer, SIZE);
    printf("Processed buffer starts with: %c\n", buffer[0]);
    
    /* Additional loop to ensure patterns survive optimization */
    int final_sum = 0;
    int* ptr = int_array;
    for (int i = 0; i < SIZE; i++) {
        /* Force use of result to prevent dead code elimination */
        final_sum += *ptr++;
    }
    printf("Final sum: %d\n", final_sum);
    
    return 0;
}
