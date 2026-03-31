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
        ptr++;              /* Increment pointer */
    }
    return sum;
}

/* Pattern 4: Different data types - char */
int func_char_access(char *arr, int n) {
    int sum = 0;
    char *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* char access with zero offset */
        ptr++;          /* Pointer increment */
    }
    return sum;
}

/* Pattern 5: Different data types - short */
int func_short_access(short *arr, int n) {
    int sum = 0;
    short *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* short access with zero offset */
        ptr++;          /* Pointer increment */
    }
    return sum;
}

/* Pattern 6: Different data types - long */
int func_long_access(long *arr, int n) {
    int sum = 0;
    long *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += (int)ptr[0];  /* long access with zero offset */
        ptr++;               /* Pointer increment */
    }
    return sum;
}

/* Pattern 7: Floating point types - float */
float func_float_access(float *arr, int n) {
    float sum = 0.0f;
    float *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* float access with zero offset */
        ptr++;          /* Pointer increment */
    }
    return sum;
}

/* Pattern 8: Floating point types - double */
double func_double_access(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* double access with zero offset */
        ptr++;          /* Pointer increment */
    }
    return sum;
}

/* Pattern 9: While loop with pointer arithmetic in body */
int func_while_loop(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    while (n-- > 0) {
        sum += ptr[0];  /* Zero offset access */
        ptr++;          /* Increment in loop body */
    }
    return sum;
}

/* Pattern 10: Do-while loop ensuring at least one execution */
int func_do_while(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    if (n > 0) {
        do {
            sum += ptr[0];  /* Zero offset access */
            ptr++;          /* Increment in loop body */
        } while (--n > 0);
    }
    return sum;
}

/* Pattern 11: Memory access and arithmetic separated by independent code */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        int temp = ptr[0];  /* Memory access with zero offset */
        
        /* Independent operation to separate memory access and increment */
        int dummy = i * 2;
        (void)dummy;  /* Prevent unused variable warning */
        
        ptr++;        /* Pointer increment separated from access */
        sum += temp;
    }
    return sum;
}

/* Pattern 12: Conditional that's always taken */
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

/* Pattern 13: Post-decrement variant */
int func_post_decrement(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n;
    
    for (int i = 0; i < n; i++) {
        ptr--;          /* Post-decrement */
        sum += ptr[0];  /* Zero offset access */
    }
    return sum;
}

/* Pattern 14: Pre-increment variant */
int func_pre_increment(int *arr, int n) {
    int sum = 0;
    int *ptr = arr - 1;
    
    for (int i = 0; i < n; i++) {
        sum += *(++ptr);  /* Pre-increment and dereference */
    }
    return sum;
}

/* Pattern 15: Mixed operations in loop increment expression */
int func_mixed_increment(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int i = 0;
    
    while (i < n) {
        sum += ptr[0];  /* Zero offset access */
        i++, ptr++;     /* Mixed increment in while condition update */
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
        short_arr[i] = (short)i;
        long_arr[i] = (long)i * 10;
        float_arr[i] = (float)i * 1.5f;
        double_arr[i] = (double)i * 2.5;
        struct_arr[i].value = i * 3;
    }
    
    /* Execute each pattern multiple times */
    for (int iter = 0; iter < 10; iter++) {
        result += func_zero_index_postinc(int_arr, SIZE);
        result += func_ptr_deref_predec(int_arr, SIZE);
        result += func_struct_first_member(struct_arr, SIZE);
        result += func_char_access(char_arr, SIZE);
        result += func_short_access(short_arr, SIZE);
        result += func_long_access(long_arr, SIZE);
        result += (int)func_float_access(float_arr, SIZE);
        result += (int)func_double_access(double_arr, SIZE);
        result += func_while_loop(int_arr, SIZE);
        result += func_do_while(int_arr, SIZE);
        result += func_separated_ops(int_arr, SIZE);
        result += func_always_taken(int_arr, SIZE);
        result += func_post_decrement(int_arr, SIZE);
        result += func_pre_increment(int_arr, SIZE);
        result += func_mixed_increment(int_arr, SIZE);
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", result);
    
    return 0;
}
