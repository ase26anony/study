/* tree_coverage_test.c - Comprehensive test for GCC tree node coverage */
/* Compile with: gcc -O2 -fopenmp -fdump-tree-all -o test tree_coverage_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function declarations to prevent optimization */
extern void opaque_external_function(int *x);
extern int unpredictable_external_check(void);

/* Global identifiers (IDENTIFIER_NODE) */
int global_counter = 0;
static int static_hidden = 42;
volatile int volatile_global = 100;
extern int external_reference;

/* Complex struct for CONSTRUCTOR nodes */
struct ComplexData {
    int values[4];
    struct {
        double x;
        double y;
    } point;
    char *name;
};

/* Another struct for nested constructors */
struct Nested {
    struct ComplexData data;
    int flags[3];
};

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexData recursive_builder(int depth, int base) {
    struct ComplexData result;
    
    /* Complex initializer with designators */
    if (depth <= 0) {
        struct ComplexData leaf = {
            .values = {base, base+1, base+2, base+3},
            .point = {.x = 1.0, .y = 2.0},
            .name = "leaf"
        };
        return leaf;
    }
    
    /* Recursive construction */
    struct ComplexData left = recursive_builder(depth - 1, base * 2);
    struct ComplexData right = recursive_builder(depth - 1, base * 2 + 1);
    
    /* Combine results */
    for (int i = 0; i < 4; i++) {
        result.values[i] = left.values[i] + right.values[i];
    }
    result.point.x = left.point.x + right.point.x;
    result.point.y = left.point.y + right.point.y;
    result.name = "combined";
    
    return result;
}

/* Function with complex control flow for SSA_NAME generation */
int ssa_generator(int n, int *arr) {
    int x = 0;
    int y = 0;
    int z = 0;
    
    /* Loop with conditional assignments - creates phi nodes */
    for (int i = 0; i < n; i++) {
        /* Multiple assignments to same variable in different paths */
        if (arr[i] > 0) {
            x = x + arr[i];
            y = y * 2;
        } else if (arr[i] < 0) {
            x = x - arr[i];
            y = y / 2;
        } else {
            x = x * 2;
            y = y + 1;
        }
        
        /* Use both branches to force SSA */
        z = x + y;
        
        /* Volatile to prevent optimization */
        volatile_global = z;
    }
    
    /* Another SSA opportunity */
    int result;
    if (z > 1000) {
        result = z / 2;
    } else {
        result = z * 2;
    }
    
    return result;
}

/* Function with blocks and labels (BLOCK nodes) */
int block_test(int param) {
    int a = param;
    
    /* Nested block with local variable */
    {
        int hidden = a * 2;
        a += hidden;
        
    mid_block:
        {
            /* Another nested block */
            int deeply_hidden = hidden + 10;
            a += deeply_hidden;
        }
    }
    
    /* Jump to label */
    if (a > 100) {
        goto skip_part;
    }
    
    {
        /* Block that might be skipped */
        int skipped_var = 999;
        a += skipped_var;
    }
    
skip_part:
    /* Target of goto */
    {
        int after_goto = 50;
        a -= after_goto;
    }
    
    return a;
}

/* OpenMP function with multiple clauses (OMP_CLAUSE nodes) */
void openmp_test(int size, double *data) {
    int i, j;
    double sum = 0.0;
    double local_sum;
    
    /* Complex OpenMP region with multiple clauses */
    #pragma omp parallel private(i, j, local_sum) shared(data, size, sum) \
                         firstprivate(global_counter) reduction(+:sum)
    {
        local_sum = 0.0;
        
        /* Nested loops with collapse clause */
        #pragma omp for collapse(2) schedule(dynamic, 4) nowait
        for (i = 0; i < size; i++) {
            for (j = 0; j < size; j++) {
                int idx = i * size + j;
                if (idx % 2 == 0) {
                    local_sum += data[idx] * 2.0;
                } else {
                    local_sum += data[idx] * 0.5;
                }
                
                /* Thread-specific operation */
                data[idx] += (i + j) * 0.01;
            }
        }
        
        /* Reduction */
        sum += local_sum;
        
        /* Barrier with nowait removed */
        #pragma omp barrier
        
        /* Single region */
        #pragma omp single
        {
            global_counter++;
        }
        
        /* Master region */
        #pragma omp master
        {
            volatile_global = (int)sum;
        }
    }
    
    /* Another OpenMP region with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) \
                            if(size > 1000) num_threads(4)
    for (i = 0; i < size * size; i++) {
        if ((int)data[i] > max_val) {
            max_val = (int)data[i];
        }
    }
}

/* Main function with diverse tree node creation */
int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time optimization */
    int size = (argc > 1) ? atoi(argv[1]) : 10;
    if (size < 2) size = 2;
    
    /* Array initialization with designator (TREE_VEC) */
    int init_array[10] = {[0] = 1, [2] = 3, [5] = 6, [9] = 10};
    
    /* Complex struct initialization (CONSTRUCTOR) */
    struct Nested nested = {
        .data = {
            .values = {1, 2, 3, 4},
            .point = {.x = 3.14, .y = 2.71},
            .name = "test"
        },
        .flags = {[0] = 1, [2] = 1}
    };
    
    /* Call recursive constructor builder */
    struct ComplexData built = recursive_builder(3, 1);
    
    /* Dynamic array for SSA test */
    int *dynamic_array = malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        dynamic_array[i] = i * (i % 2 ? -1 : 1);
    }
    
    /* Test SSA generation */
    int ssa_result = ssa_generator(size, dynamic_array);
    
    /* Test block and goto */
    int block_result = block_test(ssa_result);
    
    /* Prepare data for OpenMP test */
    double *omp_data = malloc(size * size * sizeof(double));
    for (int i = 0; i < size * size; i++) {
        omp_data[i] = (i % 100) * 0.1;
    }
    
    /* OpenMP test with multiple clauses */
    openmp_test(size, omp_data);
    
    /* Call external function (uses external identifier) */
    opaque_external_function(&global_counter);
    
    /* Complex expression with multiple identifiers */
    int final_result = 
        global_counter + 
        static_hidden + 
        block_result + 
        (int)built.point.x +
        nested.data.values[0] +
        init_array[5];
    
    /* Conditional based on external check */
    if (unpredictable_external_check()) {
        final_result *= 2;
    } else {
        final_result /= 2;
    }
    
    /* Use volatile to ensure all computations are kept */
    volatile int output = final_result;
    
    printf("Result: %d\n", output);
    
    /* Cleanup */
    free(dynamic_array);
    free(omp_data);
    
    return 0;
}

/* Dummy external functions to satisfy references */
void opaque_external_function(int *x) {
    *x += 1;
}

int unpredictable_external_check(void) {
    return rand() % 2;
}
