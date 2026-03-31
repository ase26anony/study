/* Compile with: gcc -O2 -fopenmp -fdump-tree-all -fdump-rtl-all tree_test.c -o tree_test */
/* For C++ version (includes BINFO): g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External function to prevent optimization */
extern void opaque_external_function(int *ptr, const char *name);

/* Global identifiers (IDENTIFIER_NODE) */
int global_counter = 0;
static int static_hidden = 42;
extern int external_reference;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int values[4];
    double ratio;
    char tag;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int id;
};

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_constructor(int depth, int base) {
    struct ComplexStruct result;
    
    /* Initialize with aggregate initializer (CONSTRUCTOR node) */
    result = (struct ComplexStruct){
        .values = {base, base * 2, base * 3, base * 4},
        .ratio = depth > 0 ? 1.0 / depth : 1.0,
        .tag = 'A' + (depth % 26)
    };
    
    if (depth > 0) {
        struct ComplexStruct deeper = recursive_constructor(depth - 1, base + 1);
        /* Combine results */
        for (int i = 0; i < 4; i++) {
            result.values[i] += deeper.values[i];
        }
    }
    
    return result;
}

/* Function with complex control flow and SSA_NAME generation */
int ssa_generator(int n, int *arr) {
    int x = 0;
    int y = 0;
    
    /* Loop with conditional assignments (creates phi nodes -> SSA_NAME) */
    for (int i = 0; i < n; i++) {
        if (arr[i] > 0) {
            x = x + arr[i];
            y = 1;
        } else if (arr[i] < 0) {
            x = x - arr[i];
            y = -1;
        } else {
            x = x * 2;
            y = 0;
        }
        
        /* Complex expression to prevent optimization */
        volatile int prevent_opt = y;
        (void)prevent_opt;
    }
    
    /* Another SSA opportunity */
    int z;
    if (x > 100) {
        z = x / 2;
    } else {
        z = x * 2;
    }
    
    return z + y;
}

/* Function with BLOCK nodes and goto */
int block_and_goto(int seed) {
    int result = seed;
    
    /* Outer block */
    {
        int local_a = seed * 2;
        
        /* Inner block with goto */
        {
            int local_b = local_a + 5;
            if (local_b > 100) {
                goto skip_part;
            }
            
            int local_c = local_b * 3;
            result += local_c;
            
            skip_part:
            /* Jump here from above */
            result += local_b;
        }
        
        /* Another block with label */
        another_label:
        {
            int hidden = 77;
            result ^= hidden;
        }
        
        /* Prevent label removal */
        if (result < 0) {
            goto another_label;
        }
    }
    
    return result;
}

/* Main function with OpenMP and all constructs */
int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time optimization */
    int iterations = argc > 1 ? atoi(argv[1]) : 100;
    int use_openmp = argc > 2 ? atoi(argv[2]) : 1;
    
    /* CONSTRUCTOR: Array with designators (TREE_VEC in C) */
    int sparse_array[10] = {[0] = 1, [3] = 4, [7] = 8, [9] = 10};
    
    /* CONSTRUCTOR: Struct initialization */
    struct NestedStruct nested = {
        .inner = {
            .values = {[0] = 1, [2] = 3, [3] = 4},
            .ratio = 3.14159,
            .tag = 'X'
        },
        .id = 1001
    };
    
    /* Call recursive constructor function */
    struct ComplexStruct constructed = recursive_constructor(3, 5);
    
    /* Call function with SSA generation */
    int ssa_result = ssa_generator(iterations, sparse_array);
    
    /* Call function with blocks and goto */
    int block_result = block_and_goto(ssa_result);
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE nodes) */
    int openmp_sum = 0;
    int openmp_product = 1;
    
    if (use_openmp) {
        #pragma omp parallel num_threads(4) \
                default(none) \
                shared(sparse_array, iterations, openmp_sum, openmp_product) \
                private(constructed) \
                firstprivate(block_result) \
                reduction(+:openmp_sum) \
                reduction(*:openmp_product)
        {
            int local_sum = 0;
            int local_product = 1;
            
            #pragma omp for schedule(dynamic, 2) collapse(1) nowait
            for (int i = 0; i < iterations; i++) {
                /* Complex computation using all variables */
                int val = sparse_array[i % 10] + block_result + i;
                local_sum += val;
                local_product *= (val % 10 + 1);
                
                /* Nested OpenMP directive */
                #pragma omp atomic
                openmp_sum += local_sum;
            }
            
            #pragma omp critical
            {
                openmp_product *= local_product;
            }
        }
    }
    
    /* Call external function with various identifiers */
    opaque_external_function(&global_counter, "test_identifier");
    
    /* Use static and external identifiers */
    static_hidden += openmp_sum;
    external_reference = openmp_product;
    
    /* Final computation using all results */
    int final_result = 
        constructed.values[0] +
        nested.inner.values[2] +
        ssa_result +
        block_result +
        openmp_sum +
        (openmp_product % 1000);
    
    printf("Final result: %d\n", final_result);
    
    return final_result > 0 ? 0 : 1;
}

/* Dummy definition to satisfy linker (normally in another file) */
int external_reference = 0;

void opaque_external_function(int *ptr, const char *name) {
    /* Prevent optimization */
    volatile int dummy = *ptr + (int)name[0];
    (void)dummy;
    *ptr += 1;
}
