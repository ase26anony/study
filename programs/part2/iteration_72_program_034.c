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
    char padding[16];
    double extra;
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

/* Pattern 4: Different data types and sizes */
short func_short_type(short *arr, int n) {
    short sum = 0;
    short *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* QImode/HImode access */
        ptr++;          /* Pointer arithmetic */
    }
    return sum;
}

/* Pattern 5: Floating point types */
double func_double_type(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* DImode/DFmode access */
        ptr++;          /* Pointer increment */
    }
    return sum;
}

/* Pattern 6: While loop with post-decrement */
int func_while_postdec(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n - 1;
    
    while (n-- > 0) {
        sum += ptr[0];  /* Zero offset access */
        ptr--;          /* Post-decrement */
    }
    return sum;
}

/* Pattern 7: Do-while loop */
int func_do_while(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    if (n <= 0) return 0;
    
    do {
        sum += ptr[0];  /* Memory access */
        ptr++;          /* Increment */
    } while (--n > 0);
    
    return sum;
}

/* Pattern 8: Separated operations with trivial code between */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    int temp;
    
    for (int i = 0; i < n; i++) {
        temp = ptr[0];      /* Memory access with zero offset */
        /* Small independent operation */
        sum += temp * 2;    /* Use the value */
        ptr++;              /* Increment separated by one operation */
    }
    return sum;
}

/* Pattern 9: Multiple arrays with different types */
void func_mixed_types(int *int_arr, double *dbl_arr, char *char_arr, int n) {
    int *int_ptr = int_arr;
    double *dbl_ptr = dbl_arr;
    char *char_ptr = char_arr;
    
    for (int i = 0; i < n; i++) {
        /* Access all with zero offset */
        int_ptr[0] = i;
        dbl_ptr[0] = i * 1.5;
        char_ptr[0] = i & 0xFF;
        
        /* Increment all pointers */
        int_ptr++;
        dbl_ptr++;
        char_ptr++;
    }
}

/* Pattern 10: Nested loops with pointer arithmetic */
int func_nested_loops(int **matrix, int rows, int cols) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = matrix[i];
        
        for (int j = 0; j < cols; j++) {
            sum += row_ptr[0];  /* Zero offset access */
            row_ptr++;          /* Increment pointer */
        }
    }
    return sum;
}

/* Pattern 11: Conditional with always-taken branch */
int func_conditional_branch(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (ptr != NULL) {  /* Always true in this context */
            sum += ptr[0];  /* Memory access */
        }
        ptr++;              /* Increment after branch */
    }
    return sum;
}

/* Pattern 12: Long type for different mode */
long func_long_type(long *arr, int n) {
    long sum = 0;
    long *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* SImode/DImode access */
        ptr++;          /* Pointer increment */
    }
    return sum;
}

/* Main function to exercise all patterns */
int main() {
    const int SIZE = 100;
    volatile int result = 0;  /* Volatile to prevent optimization */
    
    /* Initialize arrays with different types */
    int int_arr[SIZE];
    short short_arr[SIZE];
    long long_arr[SIZE];
    double dbl_arr[SIZE];
    char char_arr[SIZE];
    struct Data struct_arr[SIZE];
    
    int *int_matrix[10];
    for (int i = 0; i < 10; i++) {
        int_matrix[i] = malloc(SIZE * sizeof(int));
        for (int j = 0; j < SIZE; j++) {
            int_matrix[i][j] = i * j;
        }
    }
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        short_arr[i] = i & 0x7FFF;
        long_arr[i] = i * 100L;
        dbl_arr[i] = i * 1.5;
        char_arr[i] = i & 0xFF;
        struct_arr[i].value = i * 2;
    }
    
    /* Call each function multiple times */
    for (int iter = 0; iter < 10; iter++) {
        result += func_zero_index_postinc(int_arr, SIZE);
        result += func_ptr_deref_predec(int_arr, SIZE);
        result += func_struct_first_member(struct_arr, SIZE / 2);
        result += func_short_type(short_arr, SIZE);
        result += (int)func_double_type(dbl_arr, SIZE);
        result += func_while_postdec(int_arr, SIZE);
        result += func_do_while(int_arr, SIZE);
        result += func_separated_ops(int_arr, SIZE);
        
        func_mixed_types(int_arr, dbl_arr, char_arr, SIZE);
        result += func_nested_loops(int_matrix, 10, SIZE);
        result += func_conditional_branch(int_arr, SIZE);
        result += (int)func_long_type(long_arr, SIZE);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    /* Cleanup */
    for (int i = 0; i < 10; i++) {
        free(int_matrix[i]);
    }
    
    return 0;
}
