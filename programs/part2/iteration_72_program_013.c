#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variable to prevent dead code elimination */
volatile int global_sum = 0;

/* Structure with first member access */
struct Data {
    int first;
    double second;
    char third;
};

/* Function 1: Array access with zero index and post-increment */
int func_zero_index_postinc(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with zero offset: arr[0] */
        sum += ptr[0];  /* Line should generate mem_loc with offset 0 */
        /* Post-increment on pointer */
        ptr++;  /* Adjacent arithmetic operation */
    }
    return sum;
}

/* Function 2: Pointer dereference with pre-decrement */
int func_ptr_deref_predec(int *arr, int n) {
    int sum = 0;
    int *ptr = arr + n - 1;  /* Start from end */
    
    for (int i = 0; i < n; i++) {
        /* Pre-decrement then dereference */
        --ptr;  /* Arithmetic operation before memory access */
        /* Memory access with zero offset: *ptr */
        sum += *ptr;  /* Should trigger mem_insn setup */
    }
    return sum;
}

/* Function 3: Loop with pointer increment in loop expression */
int func_loop_inc(char *data, int n) {
    int sum = 0;
    char *ptr = data;
    
    /* Loop with increment in loop expression */
    for (int i = 0; i < n; ptr++, i++) {  /* ptr++ in loop increment */
        /* Access with zero offset */
        sum += ptr[0];  /* Should generate mem_loc with offset 0 */
    }
    return sum;
}

/* Function 4: While loop with post-decrement */
int func_while_postdec(short *arr, int n) {
    int sum = 0;
    short *ptr = arr + n - 1;
    
    /* While loop ensures basic block structure */
    while (n-- > 0) {
        /* Memory access with zero offset */
        sum += *ptr;  /* ptr[0] equivalent */
        /* Post-decrement */
        ptr--;  /* Adjacent arithmetic */
    }
    return sum;
}

/* Function 5: Do-while loop with structure first member access */
int func_struct_first_member(struct Data *data, int n) {
    int sum = 0;
    struct Data *ptr = data;
    int i = 0;
    
    /* Do-while ensures at least one execution */
    do {
        /* Access first member of structure (offset 0) */
        sum += ptr->first;  /* Should generate mem_loc with offset 0 */
        /* Pointer increment */
        ptr++;  /* Adjacent arithmetic */
        i++;
    } while (i < n);
    
    return sum;
}

/* Function 6: Memory access and arithmetic separated by trivial code */
int func_separated_ops(double *arr, int n) {
    double sum = 0.0;
    double *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with zero offset */
        sum += ptr[0];  /* Line 1352-1358 should be triggered here */
        
        /* Small amount of independent code */
        int temp = i * 2;  /* Trivial independent calculation */
        (void)temp;  /* Use to prevent optimization */
        
        /* Pointer increment separated by code */
        ptr++;  /* find_inc should search through instructions */
    }
    return (int)sum;
}

/* Function 7: Mixed types and access sizes */
int func_mixed_types(void) {
    int sum = 0;
    
    /* Different types for different memory modes */
    char c_arr[100];
    short s_arr[100];
    int i_arr[100];
    long l_arr[100];
    float f_arr[100];
    double d_arr[100];
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        c_arr[i] = i % 128;
        s_arr[i] = i * 2;
        i_arr[i] = i * 3;
        l_arr[i] = i * 4;
        f_arr[i] = i * 1.5f;
        d_arr[i] = i * 2.5;
    }
    
    /* Access each with zero offset patterns */
    char *c_ptr = c_arr;
    for (int i = 0; i < 10; i++) {
        sum += c_ptr[0];  /* QImode access */
        c_ptr++;
    }
    
    short *s_ptr = s_arr;
    for (int i = 0; i < 10; i++) {
        sum += s_ptr[0];  /* HImode access */
        s_ptr++;
    }
    
    int *i_ptr = i_arr;
    for (int i = 0; i < 10; i++) {
        sum += i_ptr[0];  /* SImode access */
        i_ptr++;
    }
    
    long *l_ptr = l_arr;
    for (int i = 0; i < 10; i++) {
        sum += l_ptr[0];  /* DImode access */
        l_ptr++;
    }
    
    return sum;
}

/* Function 8: Nested loops with pointer arithmetic */
int func_nested_loops(int *matrix, int rows, int cols) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = matrix + i * cols;
        
        for (int j = 0; j < cols; j++) {
            /* Access with zero offset */
            sum += row_ptr[0];  /* Should trigger mem_insn setup */
            /* Increment pointer */
            row_ptr++;  /* Adjacent in inner loop */
        }
    }
    
    return sum;
}

/* Function 9: Conditional with always-taken branch */
int func_conditional_branch(int *arr, int n) {
    int sum = 0;
    int *ptr = arr;
    
    for (int i = 0; i < n; i++) {
        /* Memory access with zero offset */
        sum += ptr[0];
        
        /* Always-taken conditional */
        if (ptr != NULL) {  /* Always true */
            /* Pointer increment inside branch */
            ptr++;
        }
    }
    
    return sum;
}

/* Main function to execute all patterns */
int main(void) {
    const int SIZE = 100;
    
    /* Initialize test arrays */
    int int_arr[SIZE];
    char char_arr[SIZE];
    short short_arr[SIZE];
    double double_arr[SIZE];
    struct Data struct_arr[SIZE];
    int matrix[10][10];
    
    /* Fill arrays with data */
    for (int i = 0; i < SIZE; i++) {
        int_arr[i] = i;
        char_arr[i] = i % 128;
        short_arr[i] = i * 2;
        double_arr[i] = i * 1.5;
        struct_arr[i].first = i * 3;
        struct_arr[i].second = i * 2.5;
        struct_arr[i].third = i % 128;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    /* Execute all functions multiple times */
    int total_sum = 0;
    
    for (int iteration = 0; iteration < 3; iteration++) {
        total_sum += func_zero_index_postinc(int_arr, SIZE);
        total_sum += func_ptr_deref_predec(int_arr, SIZE);
        total_sum += func_loop_inc(char_arr, SIZE);
        total_sum += func_while_postdec(short_arr, SIZE);
        total_sum += func_struct_first_member(struct_arr, SIZE);
        total_sum += func_separated_ops(double_arr, SIZE);
        total_sum += func_mixed_types();
        total_sum += func_nested_loops(&matrix[0][0], 10, 10);
        total_sum += func_conditional_branch(int_arr, SIZE);
    }
    
    /* Store in volatile to prevent optimization */
    global_sum = total_sum;
    
    /* Print result to ensure code isn't eliminated */
    printf("Total sum: %d\n", total_sum);
    
    return 0;
}
