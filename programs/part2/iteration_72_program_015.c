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
        sum += *ptr;    /* Memory access with zero offset (implicit) */
    }
    return sum;
}

/* Helper function 3: Structure with first member access */
struct Data {
    int first;
    int second;
    int third;
};

int func_struct_first_member(struct Data *data, int n) {
    int sum = 0;
    struct Data *ptr = data;
    
    for (int i = 0; i < n; i++) {
        sum += ptr->first;  /* Access first member (zero offset) */
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

/* Helper function 9: While loop with pointer arithmetic in body */
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

/* Helper function 10: Do-while loop */
int func_do_while(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int i = 0;
    
    if (n > 0) {
        do {
            sum += ptr[0];  /* Zero offset access */
            ptr++;          /* Increment in loop body */
            i++;
        } while (i < n);
    }
    return sum;
}

/* Helper function 11: Memory access and pointer arithmetic separated by code */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        int val = ptr[0];  /* Memory access with zero offset */
        
        /* Small amount of independent code */
        int temp = val * 2;
        (void)temp;  /* Use temp to prevent optimization */
        
        ptr++;        /* Pointer increment separated from access */
        sum += val;
    }
    return sum;
}

/* Helper function 12: Conditional that's always taken */
int func_always_taken(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (ptr != NULL) {  /* Always true condition */
            sum += ptr[0];  /* Zero offset access */
        }
        ptr++;              /* Pointer increment */
    }
    return sum;
}

/* Helper function 13: Multiple increments in same basic block */
int func_multi_increment(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i += 2) {
        sum += ptr[0];  /* First access */
        ptr++;          /* First increment */
        
        if (i + 1 < n) {
            sum += ptr[0];  /* Second access */
            ptr++;          /* Second increment */
        }
    }
    return sum;
}

/* Helper function 14: Using index variable with array[0] */
int func_index_var(int *arr, int n) {
    int sum = 0;
    int idx = 0;
    
    for (int i = 0; i < n; i++) {
        sum += arr[idx];  /* Access via index variable */
        idx++;            /* Increment index */
    }
    return sum;
}

/* Helper function 15: Mixed pre and post increments */
int func_mixed_increments(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += *ptr;    /* Dereference */
        ++ptr;          /* Pre-increment */
        
        if (i + 1 < n) {
            sum += ptr[0];  /* Zero offset */
            ptr++;          /* Post-increment */
        }
    }
    return sum;
}

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
        struct_arr[i].first = i;
        struct_arr[i].second = i * 2;
        struct_arr[i].third = i * 3;
    }
    
    /* Call all functions multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
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
        result += func_always_taken(int_arr, SIZE);
        result += func_multi_increment(int_arr, SIZE);
        result += func_index_var(int_arr, SIZE);
        result += func_mixed_increments(int_arr, SIZE);
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
