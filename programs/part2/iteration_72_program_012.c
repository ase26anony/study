#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper functions demonstrating different patterns */

/* Pattern 1: Direct array indexing at zero with post-increment */
int func_zero_index_postinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // Memory access with zero offset
        arr++;          // Pointer arithmetic in same basic block
    }
    return sum;
}

/* Pattern 2: Pointer dereference with pre-decrement */
double func_ptr_deref_predec(double *dptr, int n) {
    double sum = 0.0;
    
    for (int i = 0; i < n; i++) {
        --dptr;         // Pre-decrement
        sum += *dptr;   // Dereference with implicit zero offset
    }
    return sum;
}

/* Pattern 3: Structure with first member access */
struct Data {
    int value;
    char tag;
    float extra;
};

int func_struct_first_member(struct Data *data, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += data[0].value;  // Access first member (zero offset)
        data++;                // Increment pointer
    }
    return sum;
}

/* Pattern 4: Different data types and sizes */
short func_mixed_types(short *sptr, char *cptr, int n) {
    short sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += sptr[0];  // HImode access
        sptr++;          // Post-increment
        
        sum += cptr[0];  // QImode access  
        cptr--;          // Post-decrement
    }
    return sum;
}

/* Pattern 5: Loop with pointer arithmetic in increment expression */
float func_loop_inc(float *farr, int n) {
    float sum = 0.0f;
    float *ptr = farr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  // Zero offset access
        // Pointer increment in loop increment expression
    }
    return sum;
}

/* Pattern 6: Memory access and arithmetic separated by trivial code */
long func_separated_ops(long *larr, int n) {
    long sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += larr[0];  // Memory access with zero offset
        
        // Small independent operation
        int temp = i * 2;
        (void)temp;  // Use to prevent optimization
        
        larr++;      // Pointer arithmetic after separation
    }
    return sum;
}

/* Pattern 7: Do-while loop ensuring execution */
unsigned char func_do_while(unsigned char *bytes, int n) {
    unsigned char sum = 0;
    unsigned char *ptr = bytes;
    int count = n;
    
    if (count > 0) {
        do {
            sum += ptr[0];  // Zero offset access
            ptr--;          // Decrement in loop body
            count--;
        } while (count > 0);
    }
    return sum;
}

/* Pattern 8: While loop with pre-increment */
int func_while_preinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    while (n-- > 0) {
        ++ptr;          // Pre-increment
        sum += ptr[0];  // Access with zero offset from new pointer
    }
    return sum;
}

/* Pattern 9: Nested pointer access */
int func_nested_access(int **ptr_arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += (*ptr_arr)[0];  // Dereference then zero index
        (*ptr_arr)++;          // Increment the pointer
    }
    return sum;
}

/* Pattern 10: Volatile to prevent optimization across calls */
volatile int global_counter = 0;

int func_volatile_mix(int *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += arr[0];  // Zero offset
        arr++;          // Post-increment
        
        // Mix with volatile access
        global_counter++;
    }
    return sum;
}

/* Main function that exercises all patterns */
int main() {
    const int SIZE = 100;
    int result = 0;
    
    /* Initialize test arrays */
    int int_arr[SIZE];
    double double_arr[SIZE];
    short short_arr[SIZE];
    char char_arr[SIZE];
    float float_arr[SIZE];
    long long_arr[SIZE];
    unsigned char byte_arr[SIZE];
    struct Data data_arr[SIZE];
    int *ptr_arr[SIZE];
    
    /* Fill arrays with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i + 1;
        double_arr[i] = (i + 1) * 1.5;
        short_arr[i] = (short)(i + 1);
        char_arr[i] = (char)((i % 26) + 'A');
        float_arr[i] = (float)(i + 1) * 0.7f;
        long_arr[i] = (long)(i + 1) * 1000L;
        byte_arr[i] = (unsigned char)(i % 256);
        data_arr[i].value = i * 2;
        data_arr[i].tag = 'X';
        data_arr[i].extra = i * 0.5f;
        ptr_arr[i] = &int_arr[i];
    }
    
    /* Call each function multiple times with different arguments */
    for (int iter = 0; iter < 10; iter++) {
        result += func_zero_index_postinc(int_arr, SIZE / 10);
        result += (int)func_ptr_deref_predec(double_arr + SIZE/2, SIZE / 20);
        result += func_struct_first_member(data_arr, SIZE / 10);
        result += func_mixed_types(short_arr, char_arr, SIZE / 10);
        result += (int)func_loop_inc(float_arr, SIZE / 10);
        result += (int)func_separated_ops(long_arr, SIZE / 10);
        result += func_do_while(byte_arr + SIZE/2, SIZE / 20);
        result += func_while_preinc(int_arr + SIZE/4, SIZE / 20);
        result += func_nested_access(ptr_arr, SIZE / 10);
        result += func_volatile_mix(int_arr + SIZE/3, SIZE / 20);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
