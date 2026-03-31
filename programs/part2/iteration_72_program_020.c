#include <stdio.h>
#include <stdlib.h>

/* Helper functions demonstrating different patterns */

/* Pattern 1: Direct array indexing at zero with post-increment */
int func_zero_index_postinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr++;          /* Post-increment on pointer */
    }
    return sum;
}

/* Pattern 2: Pointer dereference with pre-decrement */
int func_ptr_deref_predec(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n - 1;
    
    for (int i = 0; i < n; i++) {
        --ptr;          /* Pre-decrement on pointer */
        sum += *ptr;    /* Memory dereference (implicit offset 0) */
    }
    return sum;
}

/* Pattern 3: Structure with first member access */
struct Data {
    int value;
    char padding[12];
    float extra;
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

/* Pattern 4: Different data types - char */
int func_char_zero_offset(char *arr, int n) {
    int sum = 0;
    char *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* char access with zero offset */
        ptr++;          /* Pointer increment */
    }
    return sum;
}

/* Pattern 5: Different data types - short */
int func_short_zero_offset(short *arr, int n) {
    int sum = 0;
    short *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* short access with zero offset */
        ptr++;          /* Pointer increment */
    }
    return sum;
}

/* Pattern 6: Different data types - long */
int func_long_zero_offset(long *arr, int n) {
    int sum = 0;
    long *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += (int)ptr[0];  /* long access with zero offset */
        ptr++;               /* Pointer increment */
    }
    return sum;
}

/* Pattern 7: Floating point types */
float func_float_zero_offset(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* float access with zero offset */
        ptr++;          /* Pointer increment */
    }
    return sum;
}

/* Pattern 8: Double precision */
double func_double_zero_offset(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* double access with zero offset */
        ptr++;          /* Pointer increment */
    }
    return sum;
}

/* Pattern 9: While loop with separated operations */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int i = 0;
    
    while (i < n) {
        int temp = ptr[0];  /* Memory access with zero offset */
        
        /* Small independent calculation */
        int dummy = i * 2;
        (void)dummy;  /* Prevent unused variable warning */
        
        ptr++;        /* Pointer increment separated by code */
        sum += temp;
        i++;
    }
    return sum;
}

/* Pattern 10: Do-while loop */
int func_do_while(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int i = 0;
    
    if (n <= 0) return 0;
    
    do {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr++;          /* Pointer increment */
        i++;
    } while (i < n);
    
    return sum;
}

/* Pattern 11: Mixed pre and post increments */
int func_mixed_increments(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
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

/* Pattern 12: Nested array access */
int func_nested_zero_index(int arr[][10], int rows) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        sum += arr[i][0];  /* Second dimension zero index */
    }
    return sum;
}

/* Pattern 13: Pointer arithmetic in loop increment */
int func_loop_increment_expr(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; ptr++, i++) {
        sum += ptr[0];  /* Memory access with zero offset */
    }
    return sum;
}

/* Pattern 14: Decrement variations */
int func_decrement_variations(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n - 1;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr--;          /* Post-decrement */
    }
    
    ptr = arr + n - 1;
    for (int i = 0; i < n; i++) {
        --ptr;          /* Pre-decrement */
        sum += ptr[0];  /* Memory access with zero offset */
    }
    
    return sum;
}

/* Main function to execute all patterns */
int main() {
    const int SIZE = 100;
    volatile int result = 0;  /* volatile to prevent optimization */
    
    /* Initialize arrays of different types */
    int int_arr[SIZE];
    char char_arr[SIZE];
    short short_arr[SIZE];
    long long_arr[SIZE];
    float float_arr[SIZE];
    double double_arr[SIZE];
    struct Data struct_arr[SIZE];
    int nested_arr[5][10];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        char_arr[i] = (char)(i % 128);
        short_arr[i] = (short)(i * 2);
        long_arr[i] = i * 3L;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
        struct_arr[i].value = i * 4;
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            nested_arr[i][j] = i * 10 + j;
        }
    }
    
    /* Execute all patterns multiple times */
    for (int iteration = 0; iteration < 3; iteration++) {
        result += func_zero_index_postinc(int_arr, SIZE);
        result += func_ptr_deref_predec(int_arr, SIZE);
        result += func_struct_first_member(struct_arr, SIZE);
        result += func_char_zero_offset(char_arr, SIZE);
        result += func_short_zero_offset(short_arr, SIZE);
        result += func_long_zero_offset(long_arr, SIZE);
        result += (int)func_float_zero_offset(float_arr, SIZE);
        result += (int)func_double_zero_offset(double_arr, SIZE);
        result += func_separated_ops(int_arr, SIZE);
        result += func_do_while(int_arr, SIZE);
        result += func_mixed_increments(int_arr, SIZE);
        result += func_nested_zero_index(nested_arr, 5);
        result += func_loop_increment_expr(int_arr, SIZE);
        result += func_decrement_variations(int_arr, SIZE);
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", result);
    
    return 0;
}
