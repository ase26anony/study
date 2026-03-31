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
    float f;
    char c;
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

/* Pattern 4: Different data types and sizes */
long func_mixed_types(char *carr, short *sarr, int *iarr, long *larr, int n) {
    long sum = 0;
    
    for (int i = 0; i < n; i++) {
        sum += carr[0];  /* QImode access */
        sum += sarr[0];  /* HImode access */
        sum += iarr[0];  /* SImode access */
        sum += larr[0];  /* DImode access */
        
        carr++;
        sarr++;
        iarr++;
        larr++;
    }
    return sum;
}

/* Pattern 5: Floating point types */
double func_float_types(float *farr, double *darr, int n) {
    double sum = 0.0;
    float *fptr = farr;
    double *dptr = darr;
    
    for (int i = 0; i < n; i++) {
        sum += fptr[0];  /* SFmode access */
        sum += dptr[0];  /* DFmode access */
        
        fptr++;
        dptr++;
    }
    return sum;
}

/* Pattern 6: While loop with pointer arithmetic in body */
int func_while_loop(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int count = n;
    
    while (count-- > 0) {
        sum += ptr[0];  /* Zero offset access */
        ptr++;          /* Increment in loop body */
    }
    return sum;
}

/* Pattern 7: Do-while loop ensuring at least one execution */
int func_do_while(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int count = n;
    
    if (count > 0) {
        do {
            sum += ptr[0];  /* Zero offset access */
            ptr++;          /* Increment pointer */
        } while (--count > 0);
    }
    return sum;
}

/* Pattern 8: Memory access and arithmetic separated by independent code */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        int temp = ptr[0];  /* Memory access with zero offset */
        
        /* Independent operation that might inhibit fusion */
        int dummy = i * 2 + 1;
        (void)dummy;
        
        ptr++;              /* Pointer increment separated from access */
        sum += temp;
    }
    return sum;
}

/* Pattern 9: Nested pointer arithmetic */
int func_nested_ptr(int **ptr_arr, int n) {
    int sum = 0;
    int **ptr = ptr_arr;
    
    for (int i = 0; i < n; i++) {
        sum += (*ptr)[0];  /* Double dereference with zero offset */
        ptr++;             /* Increment pointer to pointer */
    }
    return sum;
}

/* Pattern 10: Conditional increment/decrement */
int func_conditional_inc(int *arr, int n, int flag) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Zero offset access */
        
        if (flag) {
            ptr++;      /* Increment in conditional branch */
        } else {
            ptr--;      /* Decrement in alternative branch */
        }
    }
    return sum;
}

/* Main function that exercises all patterns */
int main() {
    const int SIZE = 100;
    volatile int result = 0;  /* Prevent dead code elimination */
    
    /* Initialize arrays with different types */
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
        char_arr[i] = i % 128;
        short_arr[i] = i * 2;
        long_arr[i] = i * 100L;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
        struct_arr[i].value = i * 3;
        ptr_arr[i] = &int_arr[i];
    }
    
    /* Call each function multiple times with different arguments */
    for (int iter = 0; iter < 10; iter++) {
        int n = 10 + (iter % 20);
        
        result += func_zero_index_postinc(int_arr, n);
        result += func_ptr_deref_predec(int_arr, n);
        result += func_struct_first_member(struct_arr, n);
        result += func_mixed_types(char_arr, short_arr, int_arr, long_arr, n);
        result += func_float_types(float_arr, double_arr, n);
        result += func_while_loop(int_arr, n);
        result += func_do_while(int_arr, n);
        result += func_separated_ops(int_arr, n);
        result += func_nested_ptr(ptr_arr, n);
        result += func_conditional_inc(int_arr, n, iter % 2);
    }
    
    /* Print checksum to ensure all code executes */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
