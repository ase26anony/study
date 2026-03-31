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
    int sum = 0;
    int i = 0;
    
    /* Use while loop to encourage pointer arithmetic */
    while (i < n) {
        /* Pattern: *ptr++ - should generate post-increment RTL */
        sum += *ptr++;
        i++;
    }
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_ptr = src + n - 1;
    int* dest_ptr = dest + n - 1;
    
    /* Use volatile to prevent loop unrolling */
    volatile int count = n;
    
    while (count-- > 0) {
        /* Pattern: *dest_ptr-- = *src_ptr-- */
        *dest_ptr-- = *src_ptr--;
    }
}

/* Function 3: Mixed types with different access patterns */
void process_mixed_types() {
    char char_arr[SIZE];
    double double_arr[SMALL_SIZE];
    struct Point points[SMALL_SIZE];
    
    /* Initialize with index post-increment */
    for (int i = 0; i < SIZE; i++) {
        char_arr[i] = (char)(i % 256);
    }
    
    /* Pointer traversal with stride (ptr += 2) */
    double* dptr = double_arr;
    for (int i = 0; i < SMALL_SIZE; i += 2) {
        *dptr = i * 1.5;
        dptr += 2;  /* Stride of 2 */
    }
    
    /* Struct traversal with pointer increment */
    struct Point* ppt = points;
    for (int i = 0; i < SMALL_SIZE; i++) {
        ppt->x = i;
        ppt->y = i * 2;
        ppt->label = 'A' + (i % 26);
        ppt++;  /* Post-increment of struct pointer */
    }
}

/* Function 4: Volatile pointer access */
volatile int volatile_sum = 0;

void volatile_traversal(volatile int* arr, int n) {
    volatile int* vptr = arr;
    int local_sum = 0;
    
    /* Volatile pointer with post-increment */
    for (int i = 0; i < n; i++) {
        local_sum += *vptr++;
    }
    
    volatile_sum = local_sum;
}

/* Function 5: Nested loops with inner auto-increment */
void matrix_process(int rows, int cols, int mat[rows][cols]) {
    for (int i = 0; i < rows; i++) {
        int* row_ptr = mat[i];
        
        /* Inner loop with pointer auto-increment */
        for (int j = 0; j < cols; j++) {
            *row_ptr++ = i * cols + j;
        }
    }
}

/* Function 6: Backward initialization with post-decrement */
void init_backwards(char* buf, int size) {
    char* ptr = buf + size - 1;
    
    while (size-- > 0) {
        *ptr-- = '0' + (size % 10);
    }
}

/* Function 7: Store operations with auto-increment */
void store_sequence(int* dest, int start, int count) {
    int* ptr = dest;
    int value = start;
    
    while (count-- > 0) {
        *ptr++ = value++;
    }
}

int main() {
    int arr1[SIZE];
    int arr2[SIZE];
    int arr3[SIZE];
    volatile int varr[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i + 1;
        varr[i] = i * 2;
    }
    
    /* Test 1: Forward sum with pointer auto-increment */
    int sum1 = sum_array(arr1, SIZE);
    printf("Sum1: %d\n", sum1);
    
    /* Test 2: Reverse copy with pointer auto-decrement */
    reverse_copy(arr2, arr1, SIZE);
    
    /* Test 3: Volatile access pattern */
    volatile_traversal(varr, SIZE);
    printf("Volatile sum: %d\n", volatile_sum);
    
    /* Test 4: Store operations with auto-increment */
    store_sequence(arr3, 100, SIZE);
    
    /* Test 5: Backward initialization */
    char buffer[SIZE];
    init_backwards(buffer, SIZE);
    printf("First char: %c\n", buffer[0]);
    
    /* Test 6: Mixed type processing */
    process_mixed_types();
    
    /* Test 7: Matrix processing with nested loops */
    int matrix[5][5];
    matrix_process(5, 5, matrix);
    
    /* Use results to prevent dead code elimination */
    int final_check = sum1 + arr2[0] + arr3[SIZE-1] + buffer[0];
    printf("Final check: %d\n", final_check);
    
    return 0;
}
