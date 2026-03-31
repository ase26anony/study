#include <stdio.h>
#include <stdlib.h>

/* Helper function 1: Direct array indexing at zero with post-increment */
int func_zero_index_postinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr++;          /* Post-increment on pointer */
    }
    return sum;
}

/* Helper function 2: Pointer dereference with pre-decrement */
int func_ptr_deref_predec(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n - 1;
    
    for (int i = 0; i < n; i++) {
        --ptr;          /* Pre-decrement on pointer */
        sum += *ptr;    /* Memory dereference (implicit offset 0) */
    }
    return sum;
}

/* Helper function 3: Structure with first member access */
struct Data {
    int value;
    char padding[16];
    long extra;
};

int func_struct_first_member(struct Data *data, int n) {
    int sum = 0;
    struct Data *ptr = data;
    
    for (int i = 0; i < n; i++) {
        sum += ptr->value;  /* Access first member (offset 0) */
        ptr++;              /* Increment pointer */
    }
    return sum;
}

/* Helper function 4: Different data types - char */
int func_char_access(char *arr, int n) {
    int sum = 0;
    char *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* char access with zero offset */
        ptr++;          /* Post-increment */
    }
    return sum;
}

/* Helper function 5: Different data types - short */
int func_short_access(short *arr, int n) {
    int sum = 0;
    short *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* short access with zero offset */
        ptr++;          /* Post-increment */
    }
    return sum;
}

/* Helper function 6: Different data types - long */
long func_long_access(long *arr, int n) {
    long sum = 0;
    long *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* long access with zero offset */
        ptr++;          /* Post-increment */
    }
    return sum;
}

/* Helper function 7: Floating point types - float */
float func_float_access(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* float access with zero offset */
        ptr++;          /* Post-increment */
    }
    return sum;
}

/* Helper function 8: Floating point types - double */
double func_double_access(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* double access with zero offset */
        ptr++;          /* Post-increment */
    }
    return sum;
}

/* Helper function 9: While loop with pointer arithmetic */
int func_while_loop(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int count = n;
    
    while (count-- > 0) {
        sum += *ptr;    /* Dereference pointer (offset 0) */
        ptr++;          /* Increment in loop body */
    }
    return sum;
}

/* Helper function 10: Do-while loop */
int func_do_while(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int count = n;
    
    if (count <= 0) return 0;
    
    do {
        sum += ptr[0];  /* Array access with zero offset */
        ptr++;          /* Increment pointer */
    } while (--count > 0);
    
    return sum;
}

/* Helper function 11: Memory access and arithmetic separated by independent code */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        int temp = ptr[0];  /* Memory access with zero offset */
        
        /* Independent operation to separate memory access from increment */
        int dummy = i * 2;
        (void)dummy;  /* Prevent unused variable warning */
        
        ptr++;          /* Pointer increment separated from access */
        sum += temp;
    }
    return sum;
}

/* Helper function 12: Conditional that's always taken */
int func_always_taken_branch(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (ptr != NULL) {  /* Always true condition */
            sum += ptr[0];  /* Memory access with zero offset */
        }
        ptr++;              /* Pointer increment */
    }
    return sum;
}

/* Helper function 13: Mixed pre and post increments */
int func_mixed_increments(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr;    /* Dereference */
        ++ptr;          /* Pre-increment */
    }
    
    ptr = arr;  /* Reset */
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Zero offset access */
        ptr--;          /* Pre-decrement in next iteration */
    }
    
    return sum;
}

/* Helper function 14: Nested zero offset accesses */
int func_nested_access(int **arr, int n) {
    int sum = 0;
    int **ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (ptr[0] != NULL) {      /* First dereference with zero offset */
            sum += ptr[0][0];      /* Second dereference with zero offset */
        }
        ptr++;                     /* Increment pointer to pointer */
    }
    return sum;
}

/* Main function that exercises all patterns */
int main() {
    const int SIZE = 100;
    volatile int result = 0;  /* volatile to prevent dead code elimination */
    
    /* Initialize arrays of different types */
    int int_arr[SIZE];
    char char_arr[SIZE];
    short short_arr[SIZE];
    long long_arr[SIZE];
    float float_arr[SIZE];
    double double_arr[SIZE];
    struct Data struct_arr[SIZE];
    int *ptr_arr[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        char_arr[i] = (char)(i % 128);
        short_arr[i] = (short)(i % 32768);
        long_arr[i] = i * 2L;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
        struct_arr[i].value = i;
        ptr_arr[i] = &int_arr[i];
    }
    
    /* Call each function multiple times with different arguments */
    for (int iter = 0; iter < 10; iter++) {
        int offset = iter % 50;
        
        result += func_zero_index_postinc(int_arr + offset, SIZE - offset);
        result += func_ptr_deref_predec(int_arr + SIZE - 1, SIZE - offset);
        result += func_struct_first_member(struct_arr + offset, SIZE - offset);
        result += func_char_access(char_arr + offset, SIZE - offset);
        result += func_short_access(short_arr + offset, SIZE - offset);
        result += func_long_access(long_arr + offset, SIZE - offset);
        result += func_float_access(float_arr + offset, SIZE - offset);
        result += func_double_access(double_arr + offset, SIZE - offset);
        result += func_while_loop(int_arr + offset, SIZE - offset);
        result += func_do_while(int_arr + offset, SIZE - offset);
        result += func_separated_ops(int_arr + offset, SIZE - offset);
        result += func_always_taken_branch(int_arr + offset, SIZE - offset);
        result += func_mixed_increments(int_arr + offset, SIZE - offset);
        result += func_nested_access(ptr_arr + offset, SIZE - offset);
    }
    
    /* Print checksum to ensure all code executes */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
