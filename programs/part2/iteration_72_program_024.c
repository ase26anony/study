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
        sum += *ptr;    /* Dereference with implicit zero offset */
    }
    return sum;
}

/* Pattern 3: Structure with first member access */
struct Data {
    int value;
    char padding[16];
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

/* Pattern 9: While loop with post-decrement */
int func_while_postdec(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n - 1;
    
    while (n-- > 0) {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr--;          /* Post-decrement */
    }
    return sum;
}

/* Pattern 10: Do-while loop */
int func_do_while(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    if (n <= 0) return 0;
    
    do {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr++;          /* Pointer increment */
    } while (--n > 0);
    
    return sum;
}

/* Pattern 11: Memory access and arithmetic separated by independent code */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        int val = ptr[0];  /* Memory access with zero offset */
        
        /* Independent operation that might inhibit fusion */
        int temp = i * i;
        (void)temp;  /* Prevent unused variable warning */
        
        ptr++;       /* Pointer increment separated from access */
        sum += val;
    }
    return sum;
}

/* Pattern 12: Conditional that's always taken */
int func_always_taken(int *arr, int n) {
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

/* Pattern 13: Nested zero offset access */
int func_nested_access(int **arr, int n) {
    int sum = 0;
    int **ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (ptr[0] != NULL) {      /* First dereference with zero offset */
            sum += ptr[0][0];      /* Second dereference with zero offset */
        }
        ptr++;                     /* Pointer increment */
    }
    return sum;
}

/* Pattern 14: Mixed increment/decrement patterns */
int func_mixed_patterns(int *arr, int n) {
    int sum = 0;
    int *ptr1 = arr;
    int *ptr2 = arr + n - 1;
    
    for (int i = 0; i < n/2; i++) {
        sum += ptr1[0];  /* Access with zero offset */
        ptr1++;          /* Increment */
        
        sum += ptr2[0];  /* Access with zero offset */
        ptr2--;          /* Decrement */
    }
    return sum;
}

/* Main function that exercises all patterns */
int main() {
    const int SIZE = 100;
    volatile int result = 0;  /* Prevent dead code elimination */
    
    /* Initialize arrays */
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
        short_arr[i] = (short)i;
        long_arr[i] = i * 2L;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
        struct_arr[i].value = i;
        ptr_arr[i] = &int_arr[i];
    }
    
    /* Call each function multiple times */
    for (int iter = 0; iter < 10; iter++) {
        result += func_zero_index_postinc(int_arr, SIZE);
        result += func_ptr_deref_predec(int_arr, SIZE);
        result += func_struct_first_member(struct_arr, SIZE);
        result += func_char_access(char_arr, SIZE);
        result += func_short_access(short_arr, SIZE);
        result += func_long_access(long_arr, SIZE);
        
        /* Cast float/double results to int for accumulation */
        result += (int)func_float_access(float_arr, SIZE);
        result += (int)func_double_access(double_arr, SIZE);
        
        result += func_while_postdec(int_arr, SIZE);
        result += func_do_while(int_arr, SIZE);
        result += func_separated_ops(int_arr, SIZE);
        result += func_always_taken(int_arr, SIZE);
        result += func_nested_access(ptr_arr, SIZE/10);
        result += func_mixed_patterns(int_arr, SIZE);
    }
    
    /* Print checksum to prevent optimization */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
