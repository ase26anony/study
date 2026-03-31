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
    
    /* Classic pattern: *ptr++ in while loop */
    while (ptr < end) {
        sum += *ptr++;
    }
    
    return sum;
}

/* Function 2: Reverse copy with pointer post-decrement */
void reverse_copy(int* dest, const int* src, int n) {
    const int* src_ptr = src + n - 1;
    int* dest_ptr = dest + n - 1;
    
    /* Pattern: *dest-- = *src-- */
    for (int i = 0; i < n; i++) {
        *dest_ptr-- = *src_ptr--;
    }
}

/* Function 3: Mixed operations with different strides */
void process_chars(char* data, int n) {
    char* ptr = data;
    volatile char* vptr = (volatile char*)data;  /* volatile to prevent optimization */
    
    /* Simple post-increment with volatile */
    for (int i = 0; i < n; i++) {
        char val = *vptr++;
        *ptr++ = val + 1;
    }
}

/* Function 4: Struct traversal with pointer arithmetic */
int sum_points(const struct Point* points, int n) {
    const struct Point* ptr = points;
    int total = 0;
    
    /* Post-increment with struct pointer */
    for (int i = 0; i < n; i++) {
        total += ptr->x + ptr->y;
        ptr++;  /* This should generate post-increment addressing */
    }
    
    return total;
}

/* Function 5: Double array with index post-increment */
double average_doubles(const double* arr, int n) {
    double sum = 0.0;
    int index = 0;
    
    /* Array access with post-increment index */
    while (index < n) {
        sum += arr[index++];
    }
    
    return (n > 0) ? sum / n : 0.0;
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
                b_ptr += n;  /* Different stride */
            }
            
            result[i * n + j] = sum;
        }
    }
}

/* Function 7: Store operations with post-increment */
void initialize_array(int* arr, int n, int start) {
    int* ptr = arr;
    int value = start;
    
    /* Store with post-increment */
    for (int i = 0; i < n; i++) {
        *ptr++ = value++;
    }
}

/* Function 8: Backward initialization with post-decrement */
void initialize_backwards(char* arr, int n) {
    char* ptr = arr + n - 1;
    char value = 'z';
    
    /* Store with post-decrement */
    while (ptr >= arr) {
        *ptr-- = value--;
    }
}

int main(void) {
    /* Declare arrays of different types */
    int int_array[SIZE];
    char char_array[SIZE];
    double double_array[SMALL_SIZE];
    struct Point points[SMALL_SIZE];
    int result_array[SIZE];
    
    /* Initialize arrays with volatile to prevent compile-time optimization */
    volatile int init_val = 0;
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = init_val + i;
        char_array[i] = 'A' + (i % 26);
    }
    
    for (int i = 0; i < SMALL_SIZE; i++) {
        double_array[i] = i * 1.5;
        points[i].x = i;
        points[i].y = i * 2;
        points[i].label = 'A' + i;
    }
    
    /* Test 1: Forward traversal with post-increment */
    int sum = sum_array(int_array, SIZE);
    printf("Sum of int array: %d\n", sum);
    
    /* Test 2: Reverse copy with post-decrement */
    reverse_copy(result_array, int_array, SIZE);
    printf("First element after reverse copy: %d\n", result_array[0]);
    
    /* Test 3: Char processing with volatile */
    process_chars(char_array, SIZE);
    printf("First char after processing: %c\n", char_array[0]);
    
    /* Test 4: Struct traversal */
    int point_sum = sum_points(points, SMALL_SIZE);
    printf("Sum of points: %d\n", point_sum);
    
    /* Test 5: Double array with index post-increment */
    double avg = average_doubles(double_array, SMALL_SIZE);
    printf("Average of doubles: %.2f\n", avg);
    
    /* Test 6: Initialize with post-increment store */
    int new_array[20];
    initialize_array(new_array, 20, 100);
    printf("First element of new array: %d\n", new_array[0]);
    
    /* Test 7: Backward initialization with post-decrement */
    char backward_array[10];
    initialize_backwards(backward_array, 10);
    printf("First char of backward array: %c\n", backward_array[0]);
    
    /* Additional pattern: Array initialization with index post-increment */
    int buffer[SIZE];
    for (int i = 0; i < SIZE; i++) {
        buffer[i] = i * 2;  /* Common pattern that may generate auto-inc */
    }
    printf("Buffer[10] = %d\n", buffer[10]);
    
    /* Mixed access pattern to ensure various RTL patterns */
    int* volatile volatile_ptr = int_array;  /* volatile pointer */
    int volatile_sum = 0;
    for (int i = 0; i < 10; i++) {
        volatile_sum += *volatile_ptr++;
    }
    printf("Volatile pointer sum: %d\n", volatile_sum);
    
    return 0;
}
