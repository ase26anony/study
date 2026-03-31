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
double func_ptr_deref_predec(double *ptr, int n) {
    double sum = 0.0;
    ptr += n;  /* Start from end */
    for (int i = 0; i < n; i++) {
        --ptr;          /* Pre-decrement on pointer */
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
        ptr++;                /* Pointer increment */
    }
    return sum;
}

/* Pattern 4: Mixed types - char access */
char func_char_access(char *ptr, int n) {
    char result = 0;
    for (int i = 0; i < n; i++) {
        result ^= ptr[0];  /* char access at zero offset */
        ptr--;             /* Post-decrement */
    }
    return result;
}

/* Pattern 5: While loop with separated operations */
short func_separated_ops(short *ptr, int n) {
    short sum = 0;
    int i = 0;
    while (i < n) {
        sum += ptr[0];      /* Memory access with zero offset */
        /* Small independent operation */
        int temp = i * 2;
        (void)temp;         /* Use to prevent optimization */
        ptr++;              /* Pointer increment separated by code */
        i++;
    }
    return sum;
}

/* Pattern 6: Do-while loop with float */
float func_do_while_float(float *ptr, int n) {
    float sum = 0.0f;
    int i = 0;
    if (n <= 0) return sum;
    
    do {
        sum += ptr[0];      /* Float access at zero offset */
        ++ptr;              /* Pre-increment */
        i++;
    } while (i < n);
    return sum;
}

/* Pattern 7: Long type with complex control flow */
long func_complex_control(long *ptr, int n) {
    long sum = 0;
    for (int i = 0; i < n; i++) {
        if (i & 1) {
            sum -= ptr[0];  /* Access at zero offset */
        } else {
            sum += ptr[0];  /* Same access pattern */
        }
        ptr++;              /* Always increment */
    }
    return sum;
}

/* Pattern 8: Nested pointer arithmetic */
int func_nested_arithmetic(int **ptr_ptr, int n) {
    int sum = 0;
    int *ptr = *ptr_ptr;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];      /* Access through pointer */
        ptr = &ptr[1];      /* Equivalent to ptr++ */
    }
    *ptr_ptr = ptr;
    return sum;
}

/* Pattern 9: Multiple increments in same block */
int func_multi_increment(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i += 2) {
        sum += ptr[0];      /* First access */
        ptr++;              /* First increment */
        
        sum += ptr[0];      /* Second access */
        ptr++;              /* Second increment */
    }
    return sum;
}

/* Pattern 10: Volatile to prevent optimization */
int func_volatile_access(volatile int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];      /* Volatile access at zero offset */
        ptr++;              /* Pointer increment */
    }
    return sum;
}

/* Main function that exercises all patterns */
int main() {
    const int SIZE = 100;
    int result = 0;
    
    /* Initialize arrays of different types */
    int int_arr[SIZE];
    double double_arr[SIZE];
    struct Data struct_arr[SIZE];
    char char_arr[SIZE];
    short short_arr[SIZE];
    float float_arr[SIZE];
    long long_arr[SIZE];
    volatile int volatile_arr[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        double_arr[i] = i * 1.5;
        struct_arr[i].value = i * 2;
        char_arr[i] = (char)(i % 256);
        short_arr[i] = (short)(i * 3);
        float_arr[i] = i * 1.1f;
        long_arr[i] = i * 4L;
        volatile_arr[i] = i * 5;
    }
    
    /* Call each function multiple times */
    for (int iter = 0; iter < 10; iter++) {
        result += func_zero_index_postinc(int_arr, SIZE);
        result += (int)func_ptr_deref_predec(double_arr, SIZE);
        result += func_struct_first_member(struct_arr, SIZE);
        result += func_char_access(char_arr, SIZE);
        result += func_separated_ops(short_arr, SIZE);
        result += (int)func_do_while_float(float_arr, SIZE);
        result += (int)func_complex_control(long_arr, SIZE);
        
        int *ptr = int_arr;
        result += func_nested_arithmetic(&ptr, SIZE);
        result += func_multi_increment(int_arr, SIZE);
        result += func_volatile_access(volatile_arr, SIZE);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
