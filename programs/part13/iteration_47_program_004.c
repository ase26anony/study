/* tree_coverage_test.c - Comprehensive test for GCC tree node coverage */
/* Compile with: gcc -O2 -fopenmp -fdump-tree-all -std=c99 tree_coverage_test.c -o tree_test */
/* For C++ version: g++ -O2 -fopenmp -fdump-tree-all -std=c++11 tree_coverage_test.cc -o tree_test */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization - creates IDENTIFIER_NODE references */
extern void opaque_external_function(int, char**, double*);

/* Global variables with various linkages for IDENTIFIER_NODE generation */
static int static_global_counter = 0;
extern int external_global_reference;
int public_global_variable = 42;

/* Complex struct for CONSTRUCTOR nodes */
struct ComplexData {
    int id;
    double values[4];
    struct ComplexData* next;
};

/* Nested struct for more complex CONSTRUCTOR */
struct OuterStruct {
    struct {
        int inner_a;
        char inner_b;
    } nested;
    float outer_array[3];
};

/* Recursive function returning struct (CONSTRUCTOR + control flow) */
struct ComplexData recursive_struct_builder(int depth, int base_id) {
    /* Local block with goto for BLOCK node */
    {
        volatile int block_local = depth * 10;
        if (depth % 2 == 0) {
            goto skip_initialization;
        }
        block_local += 5;
    skip_initialization:
        /* Use the variable to prevent removal */
        opaque_external_function(block_local, NULL, NULL);
    }
    
    struct ComplexData result;
    result.id = base_id + depth;
    
    /* Array initializer with designator (TREE_VEC-like representation) */
    double temp_values[4] = {[0] = 1.0, [2] = depth * 2.0, [3] = 3.14};
    memcpy(result.values, temp_values, sizeof(temp_values));
    
    if (depth > 0) {
        struct ComplexData next = recursive_struct_builder(depth - 1, base_id);
        result.next = &next;
        /* Force SSA_NAME creation through conditional assignment */
        int ssa_var;
        if (depth % 3 == 0) {
            ssa_var = next.id * 2;
        } else if (depth % 3 == 1) {
            ssa_var = next.id + 100;
        } else {
            ssa_var = next.id / 2;
        }
        result.values[1] = ssa_var; /* Use the SSA variable */
    } else {
        result.next = NULL;
    }
    
    return result;
}

/* Function with complex control flow for SSA_NAME generation */
int ssa_intensive_function(int n, int* arr) {
    int result = 0;
    volatile int condition = n % 2; /* volatile prevents constant folding */
    
    /* Loop with multiple assignments to same variable */
    for (int i = 0; i < n; i++) {
        int temp;
        if (condition) {
            temp = arr[i] * 2;
        } else {
            temp = arr[i] + i;
        }
        
        /* Another conditional assignment */
        if (i % 4 == 0) {
            temp += 10;
        } else if (i % 4 == 1) {
            temp -= 5;
        } else if (i % 4 == 2) {
            temp *= 3;
        } else {
            temp /= 2;
        }
        
        result += temp;
    }
    
    /* Additional SSA complexity with nested loops */
    int x = 0, y = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            if (arr[i] > arr[j]) {
                x = x + 1;
                y = y - 1;
            } else {
                x = x - 1;
                y = y + 1;
            }
        }
    }
    
    return result + x + y;
}

/* OpenMP function with multiple clauses for OMP_CLAUSE nodes */
void openmp_comprehensive_test(int size, double* data) {
    int i, j;
    double total = 0.0;
    int chunk_size = 4;
    
    /* Complex OpenMP region with multiple clauses */
    #pragma omp parallel private(i, j) firstprivate(size, chunk_size) \
                shared(data) reduction(+:total) num_threads(2) \
                if(size > 1000)
    {
        #pragma omp for schedule(dynamic, chunk_size) collapse(2) \
                    nowait
        for (i = 0; i < size; i++) {
            for (j = 0; j < size; j++) {
                int index = i * size + j;
                double val = data[index];
                
                /* Conditional inside parallel region for more SSA */
                if (val > 0.5) {
                    val *= 2.0;
                } else {
                    val /= 2.0;
                }
                
                total += val;
                
                /* Nested OpenMP directive */
                #pragma omp atomic
                data[index] = val;
            }
        }
        
        /* Another OpenMP section with different clauses */
        #pragma omp sections private(i) lastprivate(j)
        {
            #pragma omp section
            {
                for (i = 0; i < size/2; i++) {
                    j = i * 2;
                }
            }
            #pragma omp section
            {
                for (i = size/2; i < size; i++) {
                    j = i * 3;
                }
            }
        }
    }
    
    /* Single directive with copyprivate */
    double master_value;
    #pragma omp parallel
    {
        #pragma omp single copyprivate(master_value)
        {
            master_value = total / (size * size);
        }
        
        #pragma omp barrier
        
        #pragma omp for
        for (i = 0; i < size; i++) {
            data[i] += master_value;
        }
    }
}

/* Main function orchestrating all tests */
int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    int test_size = (argc > 1) ? atoi(argv[1]) : 50;
    if (test_size < 10) test_size = 10;
    if (test_size > 1000) test_size = 1000;
    
    /* CONSTRUCTOR: Complex aggregate initialization */
    struct OuterStruct complex_init = {
        .nested = { .inner_a = 42, .inner_b = 'X' },
        .outer_array = { [0] = 1.1f, [1] = 2.2f, [2] = 3.3f }
    };
    
    /* TREE_VEC: Array with complex initializer */
    int matrix[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    /* BLOCK: Nested blocks with goto */
    volatile int block_test = 0;
    
    {
        int inner_block_var = 100;
        if (argc > 2) {
            goto middle_of_block;
        }
        
        inner_block_var += 50;
        
        {
            int deeply_nested = inner_block_var * 2;
            middle_of_block:
            block_test = deeply_nested; /* Use potentially uninitialized to stress compiler */
        }
        
        /* Another block with switch for control flow */
        switch (test_size % 4) {
            case 0: block_test += 1; break;
            case 1: block_test += 2; break;
            case 2: block_test += 3; break;
            default: block_test += 4; break;
        }
    }
    
    /* SSA_NAME: Intensive SSA generation */
    int* dynamic_array = (int*)malloc(test_size * sizeof(int));
    for (int i = 0; i < test_size; i++) {
        dynamic_array[i] = i * (argc > 3 ? atoi(argv[3]) : 1);
    }
    
    int ssa_result = ssa_intensive_function(test_size, dynamic_array);
    
    /* CONSTRUCTOR: Recursive struct building */
    struct ComplexData recursive_data = recursive_struct_builder(5, 1000);
    
    /* OpenMP test with OMP_CLAUSE generation */
    double* omp_data = (double*)malloc(test_size * test_size * sizeof(double));
    for (int i = 0; i < test_size * test_size; i++) {
        omp_data[i] = (i % 100) / 100.0;
    }
    
    openmp_comprehensive_test(test_size, omp_data);
    
    /* IDENTIFIER_NODE: Many different identifiers */
    int local_variable_with_long_name = 42;
    static int static_local_variable = 24;
    volatile int volatile_local = ssa_result;
    register int register_variable asm("r12") = block_test;
    
    /* Call external function with many identifiers */
    opaque_external_function(local_variable_with_long_name, 
                            argv, 
                            &omp_data[0]);
    
    /* Complex expression with many temporaries */
    double final_result = 0.0;
    for (int i = 0; i < test_size; i++) {
        for (int j = 0; j < test_size; j++) {
            double val = omp_data[i * test_size + j];
            double transformed;
            
            /* Multiple conditional assignments for SSA */
            if (val < 0.3) {
                transformed = val * val;
            } else if (val < 0.6) {
                transformed = sqrt(val);
            } else {
                transformed = log(val + 1.0);
            }
            
            /* Use all the different variables */
            transformed += complex_init.outer_array[i % 3];
            transformed += matrix[i % 3][j % 3];
            transformed += recursive_data.values[j % 4];
            
            final_result += transformed;
        }
    }
    
    /* Print result to ensure code isn't eliminated */
    printf("Final result: %f\n", final_result);
    printf("SSA result: %d\n", ssa_result);
    printf("Block test: %d\n", block_test);
    printf("Recursive struct ID: %d\n", recursive_data.id);
    
    free(dynamic_array);
    free(omp_data);
    
    return (final_result > 1000.0) ? 0 : 1;
}

/* Dummy implementation of external function */
void opaque_external_function(int a, char** b, double* c) {
    /* Do nothing meaningful, just prevent optimization */
    static int call_count = 0;
    call_count++;
    if (c) *c += call_count;
}
