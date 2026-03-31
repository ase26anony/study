#include <stdio.h>
#include <stdlib.h>

/* Helper functions demonstrating different patterns */

/* Pattern 1: Direct array indexing at zero with post-increment */
int func_zero_index_postinc(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr++;          /* Post-increment in same basic block */
    }
    return sum;
}

/* Pattern 2: Pointer dereference with pre-decrement */
int func_ptr_deref_predec(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        --ptr;          /* Pre-decrement before access */
        sum += *ptr;    /* Dereference with zero offset */
    }
    return sum;
}

/* Pattern 3: Structure with first member access */
struct Data {
    int first;
    int second;
    char third;
};

int func_struct_first_member(struct Data *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr->first;  /* Access first member (offset 0) */
        ptr++;              /* Increment pointer */
    }
    return sum;
}

/* Pattern 4: Different data types - char */
int func_char_access(char *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* char access with zero offset */
        ptr--;          /* Decrement pointer */
    }
    return sum;
}

/* Pattern 5: Different data types - short */
int func_short_access(short *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* short access with zero offset */
        ptr++;          /* Increment pointer */
    }
    return sum;
}

/* Pattern 6: Different data types - long */
long func_long_access(long *ptr, int n) {
    long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* long access with zero offset */
        ptr--;          /* Decrement pointer */
    }
    return sum;
}

/* Pattern 7: Floating point types - float */
float func_float_access(float *ptr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* float access with zero offset */
        ptr++;          /* Increment pointer */
    }
    return sum;
}

/* Pattern 8: Floating point types - double */
double func_double_access(double *ptr, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* double access with zero offset */
        ptr++;          /* Increment pointer */
    }
    return sum;
}

/* Pattern 9: While loop with separated operations */
int func_separated_ops(int *ptr, int n) {
    int sum = 0;
    int i = 0;
    while (i < n) {
        int temp = ptr[0];  /* Memory access with zero offset */
        /* Small independent operation */
        int dummy = i * 2;
        (void)dummy;  /* Prevent unused variable warning */
        sum += temp;
        ptr++;        /* Increment after independent operation */
        i++;
    }
    return sum;
}

/* Pattern 10: Do-while loop */
int func_do_while(int *ptr, int n) {
    int sum = 0;
    int i = 0;
    if (n <= 0) return 0;
    
    do {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr--;          /* Decrement pointer */
        i++;
    } while (i < n);
    return sum;
}

/* Pattern 11: Nested loops */
int func_nested_loops(int *ptr, int rows, int cols) {
    int sum = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            sum += ptr[0];  /* Memory access with zero offset */
            ptr++;          /* Increment pointer */
        }
    }
    return sum;
}

/* Pattern 12: Conditional with always-taken branch */
int func_conditional_branch(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (ptr != NULL) {  /* Always true in our usage */
            sum += ptr[0];  /* Memory access with zero offset */
        }
        ptr++;              /* Increment pointer */
    }
    return sum;
}

/* Pattern 13: Mixed pre and post increments */
int func_mixed_increments(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* Memory access with zero offset */
        if (i % 2 == 0) {
            ptr++;      /* Post-increment */
        } else {
            ++ptr;      /* Pre-increment */
        }
    }
    return sum;
}

/* Pattern 14: Pointer arithmetic in loop increment expression */
int func_loop_increment_expr(int *ptr, int n) {
    int sum = 0;
    for (int i = 0; i < n; ptr++, i++) {
        sum += ptr[0];  /* Memory access with zero offset */
    }
    return sum;
}

/* Main function to execute all patterns */
int main() {
    /* Initialize arrays of different types */
    int int_arr[100];
    char char_arr[100];
    short short_arr[100];
    long long_arr[100];
    float float_arr[100];
    double double_arr[100];
    struct Data struct_arr[100];
    
    /* Initialize arrays with values */
    for (int i = 0; i < 100; i++) {
        int_arr[i] = i;
        char_arr[i] = (char)(i % 128);
        short_arr[i] = (short)i;
        long_arr[i] = i * 10L;
        float_arr[i] = i * 1.5f;
        double_arr[i] = i * 2.5;
        struct_arr[i].first = i;
        struct_arr[i].second = i * 2;
        struct_arr[i].third = (char)(i % 128);
    }
    
    volatile int total_sum = 0;  /* Prevent dead code elimination */
    
    /* Call each function multiple times with different arguments */
    for (int iter = 0; iter < 10; iter++) {
        total_sum += func_zero_index_postinc(int_arr + 50, 10);
        total_sum += func_ptr_deref_predec(int_arr + 60, 10);
        total_sum += func_struct_first_member(struct_arr, 10);
        total_sum += func_char_access(char_arr + 50, 10);
        total_sum += func_short_access(short_arr + 50, 10);
        total_sum += func_long_access(long_arr + 50, 10);
        total_sum += (int)func_float_access(float_arr + 50, 10);
        total_sum += (int)func_double_access(double_arr + 50, 10);
        total_sum += func_separated_ops(int_arr + 30, 10);
        total_sum += func_do_while(int_arr + 40, 10);
        total_sum += func_nested_loops(int_arr, 5, 5);
        total_sum += func_conditional_branch(int_arr + 20, 10);
        total_sum += func_mixed_increments(int_arr + 10, 10);
        total_sum += func_loop_increment_expr(int_arr + 70, 10);
    }
    
    /* Print result to prevent optimization */
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
