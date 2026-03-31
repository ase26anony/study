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
    int *ptr = &arr[n-1];
    
    for (int i = 0; i < n; i++) {
        --ptr;          /* Pre-decrement on pointer */
        sum += *ptr;    /* Memory access with zero offset (implicit) */
    }
    return sum;
}

/* Pattern 3: Structure with first member access */
struct Data {
    int value;
    char padding[60];
};

int func_struct_first_member(struct Data *data, int n) {
    int sum = 0;
    struct Data *ptr = data;
    
    for (int i = 0; i < n; i++) {
        sum += ptr->value;  /* Access first member (zero offset) */
        ptr++;              /* Pointer arithmetic */
    }
    return sum;
}

/* Pattern 4: Different data types - char */
int func_char_access(char *arr, int n) {
    int sum = 0;
    char *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* char access with zero offset */
        ptr++;          /* Post-increment */
    }
    return sum;
}

/* Pattern 5: Different data types - short */
int func_short_access(short *arr, int n) {
    int sum = 0;
    short *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* short access with zero offset */
        ptr++;          /* Post-increment */
    }
    return sum;
}

/* Pattern 6: Different data types - long */
long func_long_access(long *arr, int n) {
    long sum = 0;
    long *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* long access with zero offset */
        ptr++;          /* Post-increment */
    }
    return sum;
}

/* Pattern 7: Floating point types - float */
float func_float_access(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* float access with zero offset */
        ptr++;          /* Post-increment */
    }
    return sum;
}

/* Pattern 8: Floating point types - double */
double func_double_access(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* double access with zero offset */
        ptr++;          /* Post-increment */
    }
    return sum;
}

/* Pattern 9: While loop with pointer arithmetic in body */
int func_while_loop(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int i = 0;
    
    while (i < n) {
        sum += ptr[0];  /* Zero offset access */
        ptr++;          /* Increment in loop body */
        i++;
    }
    return sum;
}

/* Pattern 10: Do-while loop ensuring execution */
int func_do_while(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int i = 0;
    
    if (n <= 0) return 0;
    
    do {
        sum += ptr[0];  /* Zero offset access */
        ptr++;          /* Post-increment */
        i++;
    } while (i < n);
    
    return sum;
}

/* Pattern 11: Memory access and arithmetic separated by independent code */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        int temp = ptr[0];  /* Memory access with zero offset */
        
        /* Independent operation to separate memory access from increment */
        int dummy = i * 2;
        (void)dummy;  /* Prevent unused variable warning */
        
        ptr++;        /* Pointer increment separated from access */
        sum += temp;
    }
    return sum;
}

/* Pattern 12: Conditional that's always taken */
int func_always_taken_branch(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (ptr != NULL) {  /* Always true branch */
            sum += ptr[0];  /* Zero offset access */
        }
        ptr++;              /* Post-increment */
    }
    return sum;
}

/* Pattern 13: Multiple memory accesses with same base */
int func_multiple_accesses(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];      /* First access with zero offset */
        sum += ptr[0] * 2;  /* Second access with zero offset */
        ptr++;              /* Post-increment */
    }
    return sum;
}

/* Pattern 14: Pre-increment with array indexing */
int func_preinc_with_index(int *arr, int n) {
    int sum = 0;
    int idx = 0;
    
    for (int i = 0; i < n; i++) {
        sum += arr[idx];  /* Memory access with variable offset */
        ++idx;            /* Pre-increment on index */
    }
    return sum;
}

/* Pattern 15: Post-decrement with pointer dereference */
int func_postdec_deref(int *arr, int n) {
    int sum = 0;
    int *ptr = &arr[n-1];
    
    for (int i = 0; i < n; i++) {
        sum += *ptr;    /* Memory access with zero offset */
        ptr--;          /* Post-decrement */
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
    
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        char_arr[i] = (char)(i % 128);
        short_arr[i] = (short)(i % 32768);
        long_arr[i] = i * 10L;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
        struct_arr[i].value = i * 3;
    }
    
    /* Execute all patterns multiple times */
    for (int iter = 0; iter < 10; iter++) {
        result += func_zero_index_postinc(int_arr, SIZE);
        result += func_ptr_deref_predec(int_arr, SIZE);
        result += func_struct_first_member(struct_arr, SIZE);
        result += func_char_access(char_arr, SIZE);
        result += func_short_access(short_arr, SIZE);
        result += func_long_access(long_arr, SIZE);
        result += func_float_access(float_arr, SIZE);
        result += func_double_access(double_arr, SIZE);
        result += func_while_loop(int_arr, SIZE);
        result += func_do_while(int_arr, SIZE);
        result += func_separated_ops(int_arr, SIZE);
        result += func_always_taken_branch(int_arr, SIZE);
        result += func_multiple_accesses(int_arr, SIZE);
        result += func_preinc_with_index(int_arr, SIZE);
        result += func_postdec_deref(int_arr, SIZE);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
