#include <stdio.h>
#include <stdlib.h>

/* Helper functions demonstrating different patterns */

/* Pattern 1: Direct array indexing at zero with post-increment */
int func_zero_index_postinc(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr++;          /* Post-increment on pointer */
    }
    return sum;
}

/* Pattern 2: Pointer dereference with pre-decrement */
int func_ptr_deref_predec(int *ptr, int n) {
    int sum = 0;
    ptr += n;  /* Start from end */
    for (int i = 0; i < n; i++) {
        --ptr;          /* Pre-decrement */
        sum += *ptr;    /* Dereference with implicit zero offset */
    }
    return sum;
}

/* Pattern 3: Structure with first member access */
struct Data {
    int value;
    char padding[60];
};

int func_struct_first_member(struct Data *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0].value;  /* Access first member at zero offset */
        ptr++;                /* Pointer arithmetic */
    }
    return sum;
}

/* Pattern 4: Different data types - char */
int func_char_access(char *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* char access with zero offset */
        ptr++;          /* Increment char pointer */
    }
    return sum;
}

/* Pattern 5: Different data types - short */
int func_short_access(short *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* short access with zero offset */
        ptr++;          /* Increment short pointer */
    }
    return sum;
}

/* Pattern 6: Different data types - long */
int func_long_access(long *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += (int)ptr[0];  /* long access with zero offset */
        ptr++;               /* Increment long pointer */
    }
    return sum;
}

/* Pattern 7: Floating point types - float */
float func_float_access(float *ptr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* float access with zero offset */
        ptr++;          /* Increment float pointer */
    }
    return sum;
}

/* Pattern 8: Floating point types - double */
double func_double_access(double *ptr, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* double access with zero offset */
        ptr++;          /* Increment double pointer */
    }
    return sum;
}

/* Pattern 9: While loop with post-decrement */
int func_while_postdec(int *ptr, int n) {
    int sum = 0;
    int *end = ptr + n;
    while (ptr < end) {
        sum += ptr[0];  /* Zero offset access */
        ptr++;          /* Post-increment */
    }
    return sum;
}

/* Pattern 10: Do-while loop */
int func_dowhile(int *ptr, int n) {
    int sum = 0;
    int i = 0;
    if (n <= 0) return 0;
    
    do {
        sum += ptr[0];  /* Zero offset access */
        ptr++;          /* Pointer increment */
        i++;
    } while (i < n);
    
    return sum;
}

/* Pattern 11: Memory access and arithmetic separated by independent code */
int func_separated_ops(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        int val = ptr[0];  /* Memory access with zero offset */
        
        /* Independent operation that might inhibit fusion */
        int temp = i * 2;
        if (temp > 10) {
            temp = 10;
        }
        
        sum += val + temp;
        ptr++;  /* Pointer increment separated from memory access */
    }
    return sum;
}

/* Pattern 12: Multiple memory accesses with same base */
int func_multiple_accesses(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* First access with zero offset */
        sum += ptr[0];  /* Second access with zero offset */
        ptr++;          /* Single increment for both accesses */
    }
    return sum;
}

/* Pattern 13: Conditional increment */
int func_conditional_inc(int *ptr, int n, int threshold) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Memory access with zero offset */
        
        /* Conditional that's always true to create basic block boundary */
        if (sum < threshold * 1000) {  /* Always true for reasonable inputs */
            ptr++;  /* Increment in conditional block */
        }
    }
    return sum;
}

/* Pattern 14: Nested loops */
int func_nested_loops(int *ptr, int rows, int cols) {
    int sum = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            sum += ptr[0];  /* Memory access with zero offset */
            ptr++;          /* Pointer increment */
        }
    }
    return sum;
}

/* Pattern 15: Mixed pre and post increments */
int func_mixed_increments(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            sum += ptr[0];  /* Access with zero offset */
            ptr++;          /* Post-increment */
        } else {
            ++ptr;          /* Pre-increment */
            sum += ptr[0];  /* Access with zero offset */
        }
    }
    return sum;
}

int main() {
    const int SIZE = 100;
    
    /* Initialize arrays of different types */
    int int_arr[SIZE];
    char char_arr[SIZE];
    short short_arr[SIZE];
    long long_arr[SIZE];
    float float_arr[SIZE];
    double double_arr[SIZE];
    struct Data struct_arr[SIZE];
    
    /* Initialize arrays with values */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        char_arr[i] = (char)(i % 128);
        short_arr[i] = (short)(i % 32768);
        long_arr[i] = i * 10L;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
        struct_arr[i].value = i * 3;
    }
    
    volatile int total = 0;  /* volatile to prevent optimization */
    
    /* Call all functions multiple times with different patterns */
    for (int iter = 0; iter < 10; iter++) {
        total += func_zero_index_postinc(int_arr, SIZE);
        total += func_ptr_deref_predec(int_arr, SIZE);
        total += func_struct_first_member(struct_arr, SIZE);
        total += func_char_access(char_arr, SIZE);
        total += func_short_access(short_arr, SIZE);
        total += func_long_access(long_arr, SIZE);
        
        /* Cast results to int for accumulation */
        total += (int)func_float_access(float_arr, SIZE);
        total += (int)func_double_access(double_arr, SIZE);
        
        total += func_while_postdec(int_arr, SIZE);
        total += func_dowhile(int_arr, SIZE);
        total += func_separated_ops(int_arr, SIZE);
        total += func_multiple_accesses(int_arr, SIZE);
        total += func_conditional_inc(int_arr, SIZE, 1000);
        total += func_nested_loops(int_arr, 10, 10);
        total += func_mixed_increments(int_arr, SIZE);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %d\n", total);
    
    return 0;
}
