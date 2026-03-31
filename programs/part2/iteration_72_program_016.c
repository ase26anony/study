#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper functions demonstrating different patterns */

/* Pattern 1: Direct array indexing at 0 with post-increment */
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
        sum += *ptr;    /* Memory access via dereference (zero offset) */
    }
    return sum;
}

/* Pattern 3: Structure with first member access */
struct Data {
    int value;
    char padding[12];
    double extra;
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
short func_short_type(short *arr, int n) {
    short sum = 0;
    short *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* QImode/HImode access with zero offset */
        ptr++;          /* Pointer arithmetic */
    }
    return sum;
}

/* Pattern 5: Floating point types */
double func_double_type(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* DImode/DFmode access with zero offset */
        ptr++;          /* Pointer increment */
    }
    return sum;
}

/* Pattern 6: While loop with post-decrement */
int func_while_postdec(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n - 1;
    
    while (n-- > 0) {
        sum += ptr[0];  /* Memory access with zero offset */
        ptr--;          /* Post-decrement */
    }
    return sum;
}

/* Pattern 7: Do-while loop ensuring execution */
int func_dowhile_mixed(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    if (n <= 0) return 0;
    
    do {
        sum += *ptr;    /* Dereference (zero offset) */
        ptr++;          /* Post-increment */
    } while (--n > 0);
    
    return sum;
}

/* Pattern 8: Memory access and arithmetic separated by independent code */
int func_separated_ops(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        int temp = ptr[0];  /* Memory access with zero offset */
        
        /* Independent operation that might inhibit fusion */
        int dummy = i * 2;
        (void)dummy;
        
        sum += temp;
        ptr++;              /* Increment separated from access */
    }
    return sum;
}

/* Pattern 9: Nested array access */
int func_nested_array(int arr[][10], int rows) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = arr[i];
        for (int j = 0; j < 10; j++) {
            sum += row_ptr[0];  /* Zero offset access */
            row_ptr++;          /* Pointer increment */
        }
    }
    return sum;
}

/* Pattern 10: Char array with byte access */
int func_char_array(char *arr, int n) {
    int sum = 0;
    char *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        sum += ptr[0];  /* QImode access with zero offset */
        ptr++;          /* Byte pointer increment */
    }
    return sum;
}

/* Pattern 11: Conditional that's always taken */
int func_conditional_always(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        if (ptr != NULL) {  /* Always true in this context */
            sum += ptr[0];  /* Memory access with zero offset */
        }
        ptr++;              /* Pointer increment */
    }
    return sum;
}

/* Pattern 12: Mixed increment/decrement in same function */
int func_mixed_inc_dec(int *arr, int n) {
    int sum = 0;
    int *ptr1 = arr;
    int *ptr2 = arr + n - 1;
    
    for (int i = 0; i < n/2; i++) {
        sum += ptr1[0];  /* Zero offset access */
        ptr1++;          /* Increment one pointer */
        
        sum += ptr2[0];  /* Zero offset access */
        ptr2--;          /* Decrement other pointer */
    }
    return sum;
}

/* Main function to execute all patterns */
int main() {
    const int SIZE = 100;
    volatile int result = 0;  /* Prevent dead code elimination */
    
    /* Initialize arrays of different types */
    int int_arr[SIZE];
    short short_arr[SIZE];
    double double_arr[SIZE];
    char char_arr[SIZE];
    struct Data struct_arr[SIZE];
    int nested_arr[5][10];
    
    /* Fill arrays with data */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        short_arr[i] = i % 100;
        double_arr[i] = i * 1.5;
        char_arr[i] = 'A' + (i % 26);
        struct_arr[i].value = i * 2;
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            nested_arr[i][j] = i * 10 + j;
        }
    }
    
    /* Execute all patterns multiple times */
    for (int iter = 0; iter < 3; iter++) {
        result += func_zero_index_postinc(int_arr, SIZE);
        result += func_ptr_deref_predec(int_arr, SIZE);
        result += func_struct_first_member(struct_arr, SIZE);
        result += func_short_type(short_arr, SIZE);
        result += (int)func_double_type(double_arr, SIZE);
        result += func_while_postdec(int_arr, SIZE);
        result += func_dowhile_mixed(int_arr, SIZE);
        result += func_separated_ops(int_arr, SIZE);
        result += func_nested_array(nested_arr, 5);
        result += func_char_array(char_arr, SIZE);
        result += func_conditional_always(int_arr, SIZE);
        result += func_mixed_inc_dec(int_arr, SIZE);
    }
    
    /* Print checksum to ensure all code executes */
    printf("Result checksum: %d\n", result);
    
    return 0;
}
