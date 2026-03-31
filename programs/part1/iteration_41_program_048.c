/* test_auto_inc_dec.c
 * Designed to trigger uncovered lines in GCC's auto-inc-dec.cc
 * Specifically targets the block handling simple register addressing
 */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing */
int global_arr[100] = {0};

/* Test 1: Simple parameter access with mixed addressing */
int test_simple_param(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - should trigger the uncovered block */
    sum += *base;                    /* Line: mem_insn.reg0 = XEXP (x, 0) */
    
    /* Register + constant offset */
    sum += base[5];
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access */
    int *simple_ptr = base + 10;
    sum += *simple_ptr;              /* Another candidate for simple register */
    
    return sum;
}

/* Test 2: Local pointer to global array */
int test_global_access(void) {
    int sum = 0;
    int *p = &global_arr[0];
    
    /* Multiple simple register accesses */
    sum += p[0];                     /* p[0] is *(p + 0) -> simple register */
    sum += *p;                       /* Direct dereference */
    
    /* Mixed with offset */
    sum += p[20];
    sum += p[30];
    
    /* Loop with simple access inside */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            /* Conditional simple access */
            sum += *p;
        }
        p++;
    }
    
    return sum;
}

/* Test 3: Struct access with simple pointer */
struct Data {
    int values[50];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    
    /* Access struct member via simple pointer */
    sum += data->count;              /* This becomes a memory access */
    
    /* Pointer to array within struct */
    int *arr_ptr = data->values;
    sum += arr_ptr[0];               /* Simple register access */
    
    /* Loop through struct array */
    for (int i = 0; i < data->count && i < 50; i++) {
        sum += *arr_ptr++;
    }
    
    return sum;
}

/* Test 4: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    
    /* Volatile simple access */
    sum += *base;
    
    /* Mix volatile and non-volatile */
    int *regular_ptr = (int *)base;
    sum += regular_ptr[0];           /* Should still be simple register */
    
    /* Loop with volatile */
    for (int i = 0; i < n; i++) {
        sum += *base;
        base++;
    }
    
    return sum;
}

/* Test 5: Nested loops with conditional simple access */
int test_nested_loops(int *arr, int rows, int cols) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        int *row_ptr = arr + i * cols;
        
        /* Simple access at start of each row */
        sum += row_ptr[0];           /* Simple register access */
        
        for (int j = 0; j < cols; j++) {
            /* Mix of addressing modes */
            if (j == 0) {
                sum += *row_ptr;     /* Simple register */
            } else {
                sum += row_ptr[j];   /* Register + offset */
            }
        }
    }
    
    return sum;
}

/* Test 6: Function pointer parameter with simple access */
static int load_simple(int *p) {
    return *p;                       /* Pure simple register access */
}

int test_function_pointer(int *arr, int n) {
    int sum = 0;
    
    /* Use function pointer to encourage different compilation path */
    int (*loader)(int *) = load_simple;
    
    for (int i = 0; i < n; i++) {
        sum += loader(&arr[i]);
        
        /* Also direct simple access */
        if (i % 3 == 0) {
            int *simple = &arr[i];
            sum += *simple;          /* Simple register */
        }
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize test data */
    int local_arr[200];
    for (int i = 0; i < 200; i++) {
        local_arr[i] = i;
        if (i < 100) {
            global_arr[i] = i * 2;
        }
    }
    
    struct Data data;
    data.count = 25;
    for (int i = 0; i < 50; i++) {
        data.values[i] = i * 3;
    }
    
    /* Run all tests */
    result += test_simple_param(local_arr, 20);
    result += test_global_access();
    result += test_struct_access(&data);
    result += test_volatile_access(local_arr, 15);
    
    int matrix[5][10];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    result += test_nested_loops(&matrix[0][0], 5, 10);
    result += test_function_pointer(local_arr, 30);
    
    /* Print result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
