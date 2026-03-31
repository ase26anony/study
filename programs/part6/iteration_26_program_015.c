/* test_early_remat.c - Test program for GCC early rematerialization pass */
/* Compile with: gcc -O2 -c test_early_remat.c -fdump-rtl-early_remat -da */
/* Or for coverage: gcc -O2 -c test_early_remat.c -fprofile-arcs -ftest-coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Pure function attribute to encourage rematerialization */
static int __attribute__((const)) pure_compute(int x, int y) {
    return (x * 3) + (y / 2);
}

/* Another pure function with different computation */
static double __attribute__((const)) pure_double(double a, double b) {
    return (a * 3.14159) + (b / 2.71828);
}

/* Struct with mixed types to create varied register usage */
struct mixed_data {
    int id;
    double value;
    float factor;
    char tag;
    int32_t offset;
};

/* Hot function 1: Creates high integer register pressure */
__attribute__((noinline))
static int hot_function1(int *array, int size) {
    int sum = 0;
    int i, j;
    
    /* Outer loop creates values that need to be rematerialized in inner loop */
    for (i = 0; i < size; i++) {
        /* Create many live values with cheap computations */
        int base = array[i];
        int val1 = pure_compute(base, i);      /* Candidate for remat */
        int val2 = base * 7;                   /* Another candidate */
        int val3 = (base & 0xFF) + 128;        /* Yet another */
        int val4 = (base >> 4) * 3;            /* And another */
        
        /* Inner loop with high register pressure */
        for (j = 0; j < 8; j++) {
            /* Use all values multiple times, preventing them from staying in registers */
            int t1 = val1 + j;      /* Use val1 - might need remat */
            int t2 = val2 - j;      /* Use val2 - might need remat */
            int t3 = val3 * j;      /* Use val3 - might need remat */
            int t4 = val4 / (j + 1); /* Use val4 - might need remat */
            
            /* Complex expression using all temporaries */
            sum += t1 * t2 + t3 - t4;
            
            /* More uses to increase pressure */
            sum += pure_compute(t1, t2);  /* Pure function call - good remat candidate */
            
            /* Use inline asm to create artificial register references */
            asm volatile ("# Force register use %0" : : "r" (t3));
        }
        
        /* Conditional jump to create non-trivial CFG */
        if (val1 > 1000) {
            goto special_case;
        }
        
        /* Normal path continues */
        sum += val2;
        continue;
        
    special_case:
        /* Alternate path with different computations */
        sum -= val3;
        /* Use goto to create back edge in CFG */
        if (val4 < 50) {
            goto inner_done;
        }
        sum += 1000;
    inner_done:
        ;
    }
    
    return sum;
}

/* Hot function 2: Mixed integer and floating point pressure */
__attribute__((noinline))
static double hot_function2(struct mixed_data *data, int count) {
    double total = 0.0;
    int i;
    
    /* Complex loop with mixed types */
    for (i = 0; i < count; i++) {
        /* Multiple floating point computations */
        double base_val = data[i].value;
        float factor = data[i].factor;
        
        /* Candidates for rematerialization */
        double d1 = pure_double(base_val, factor);  /* Pure function */
        double d2 = base_val * 2.5;                 /* Cheap computation */
        double d3 = (double)factor * 1.618;         /* Type conversion */
        
        /* Integer computations mixed in */
        int offset = data[i].offset;
        int id = data[i].id;
        
        /* More candidates */
        int i1 = pure_compute(id, offset);          /* Pure function */
        int i2 = offset * 3;                        /* Cheap computation */
        int i3 = (id & 0xF) << 4;                   /* Bit operations */
        
        /* Use all values in complex expressions */
        total += d1 * d2 - d3;
        total += (double)i1 * d1;
        total += (double)i2 / d2;
        total += d3 * (double)i3;
        
        /* Array indexing with computed offset */
        int idx = (i1 + i2) & 0x7F;
        asm volatile ("# Array index %0" : : "r" (idx));
        
        /* Switch statement for complex control flow */
        switch (data[i].tag) {
            case 'A':
                total += d1 * 10.0;
                break;
            case 'B':
                total += d2 * 20.0;
                /* Fall through */
            case 'C':
                total += d3 * 30.0;
                break;
            default:
                total += (double)i3;
                break;
        }
    }
    
    return total;
}

/* Hot function 3: Pointer arithmetic and addressing modes */
__attribute__((noinline))
static int hot_function3(int *matrix, int rows, int cols) {
    int result = 0;
    int i, j;
    
    /* Nested loops with pointer arithmetic */
    for (i = 0; i < rows; i++) {
        int *row_start = matrix + i * cols;
        
        /* Compute values that might be rematerialized */
        int row_sum = 0;
        int row_base = i * 17;
        
        for (j = 0; j < cols; j++) {
            /* Address computation - might use addressing mode registers */
            int *elem_ptr = row_start + j;
            
            /* Multiple computations with same base */
            int val1 = *elem_ptr + row_base;      /* Load + computation */
            int val2 = val1 * 3;                  /* Dependent computation */
            int val3 = pure_compute(val1, val2);  /* Pure function */
            
            /* Use all values */
            row_sum += val1 + val2 + val3;
            
            /* More complex expression with multiple uses */
            result += (val1 * val2) / (val3 + 1);
            
            /* Conditional with goto to inhibit optimizations */
            if (val1 > 1000) {
                goto skip_addition;
            }
            result += val2;
        skip_addition:
            
            /* Use register variables to hint at register pressure */
            register int r1 asm ("r12") = val1;
            register int r2 asm ("r13") = val2;
            asm volatile ("# Using registers %0, %1" : : "r" (r1), "r" (r2));
        }
        
        result += row_sum;
    }
    
    return result;
}

/* Main function that drives everything */
int main(void) {
    const int ARRAY_SIZE = 1000;
    const int STRUCT_COUNT = 500;
    const int MATRIX_ROWS = 50;
    const int MATRIX_COLS = 20;
    
    /* Allocate and initialize test data */
    int *array = malloc(ARRAY_SIZE * sizeof(int));
    struct mixed_data *structs = malloc(STRUCT_COUNT * sizeof(struct mixed_data));
    int *matrix = malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(int));
    
    if (!array || !structs || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (i * 37) & 0xFFF;
    }
    
    for (int i = 0; i < STRUCT_COUNT; i++) {
        structs[i].id = i;
        structs[i].value = (double)(i * 2) / 3.0;
        structs[i].factor = (float)(i % 100) / 10.0f;
        structs[i].tag = 'A' + (i % 3);
        structs[i].offset = (i * 7) & 0x3FF;
    }
    
    for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        matrix[i] = (i * 13) & 0x7FF;
    }
    
    /* Call hot functions to trigger rematerialization */
    #pragma GCC optimize ("O2")
    {
        int result1 = hot_function1(array, ARRAY_SIZE);
        double result2 = hot_function2(structs, STRUCT_COUNT);
        int result3 = hot_function3(matrix, MATRIX_ROWS, MATRIX_COLS);
        
        /* Combine results in a non-trivial way */
        int final_result = result1 + (int)result2 + result3;
        
        /* Print result to prevent dead code elimination */
        printf("Final result: %d\n", final_result);
    }
    
    /* Clean up */
    free(array);
    free(structs);
    free(matrix);
    
    return 0;
}
