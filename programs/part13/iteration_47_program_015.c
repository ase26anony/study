/* Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* External function to prevent optimization */
extern void opaque_external_function(int*);

/* Global identifiers (IDENTIFIER_NODE) */
int global_counter = 0;
static int static_accumulator = 0;
volatile int volatile_indicator = 1;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int values[4];
    double factor;
    char tag;
};

/* Array initializer with designators (TREE_VEC) */
int designator_array[10] = {[0] = 1, [3] = 4, [7] = 8, [9] = 10};

/* C++ classes for TREE_BINFO */
#ifdef __cplusplus
class BaseClass {
public:
    virtual int virtual_method(int x) { return x * 2; }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
private:
    int extra_data;
public:
    DerivedClass() : extra_data(42) {}
    virtual int virtual_method(int x) override { 
        return x * 3 + extra_data; 
    }
    int get_extra() const { return extra_data; }
};

template<typename T>
class TemplateClass {
    T data[10];
public:
    T process(int idx) { return data[idx % 10]; }
};
#endif

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_struct_builder(int depth, int seed) {
    struct ComplexStruct result;
    if (depth <= 0) {
        /* Aggregate initializer (CONSTRUCTOR node) */
        result = {{seed, seed+1, seed+2, seed+3}, 1.5, 'A'};
        return result;
    }
    
    /* Nested block with local variable (BLOCK node) */
    {
        int local_temp = seed * 2;
        goto skip_hidden;
        
        int hidden_var = 100;  /* This won't be executed due to goto */
        
        skip_hidden:
        result = recursive_struct_builder(depth - 1, local_temp);
        result.factor *= 1.1;
    }
    
    return result;
}

/* Function with complex control flow for SSA_NAME generation */
int ssa_generator(int iterations, int threshold) {
    int result = 0;
    int i;
    
    /* Loop with conditional assignments (creates phi nodes) */
    for (i = 0; i < iterations; i++) {
        int temp;
        
        /* Conditional assignment creating SSA_NAME */
        if (i % 3 == 0) {
            temp = i * 2;
        } else if (i % 3 == 1) {
            temp = i + threshold;
        } else {
            temp = threshold - i;
        }
        
        /* Use temp to prevent elimination */
        result += temp;
        
        /* Nested block with goto (BLOCK node) */
        if (i == threshold / 2) {
            goto special_case;
        }
        
        continue;
        
        special_case:
        {
            int block_local = 999;
            result += block_local;
        }
    }
    
    /* Another SSA example with switch */
    int switch_var;
    switch (iterations % 4) {
        case 0: switch_var = result * 2; break;
        case 1: switch_var = result + 100; break;
        case 2: switch_var = result - 50; break;
        default: switch_var = result / 2; break;
    }
    
    return switch_var;
}

int main(int argc, char** argv) {
    int i, j, k;
    int sum = 0;
    
    /* Use argc to prevent compile-time optimization */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    
    /* CONSTRUCTOR: Struct initialization */
    struct ComplexStruct cs = {{1, 2, 3, 4}, 2.5, 'X'};
    
    /* CONSTRUCTOR: Array initialization with nested initializer */
    struct ComplexStruct struct_array[3] = {
        {{1,2,3,4}, 1.0, 'A'},
        {{5,6,7,8}, 2.0, 'B'},
        {{9,10,11,12}, 3.0, 'C'}
    };
    
    /* Call recursive function */
    struct ComplexStruct recursive_result = recursive_struct_builder(3, 5);
    sum += recursive_result.values[0];
    
    /* SSA_NAME generation */
    int ssa_result = ssa_generator(iterations, 50);
    sum += ssa_result;
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE nodes) */
    #pragma omp parallel private(i, j) shared(sum, iterations) \
                         reduction(+:global_counter) if(iterations > 50)
    {
        int local_sum = 0;
        
        /* Nested OpenMP directive */
        #pragma omp for collapse(2) schedule(dynamic) \
                     firstprivate(iterations) nowait
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 10; j++) {
                /* Complex expression with conditional */
                int val = (i * j) % 7;
                if (val > 3) {
                    local_sum += val * 2;
                } else {
                    local_sum += val + 1;
                }
            }
        }
        
        #pragma omp atomic
        sum += local_sum;
        
        /* Another OpenMP construct with different clauses */
        #pragma omp single copyprivate(static_accumulator)
        {
            static_accumulator = omp_get_num_threads();
        }
    }
    
    /* Additional OpenMP constructs for more clause coverage */
    #pragma omp parallel sections private(k) \
                         lastprivate(volatile_indicator)
    {
        #pragma omp section
        {
            volatile_indicator = 1;
            k = 100;
        }
        #pragma omp section
        {
            volatile_indicator = 2;
            k = 200;
        }
    }
    
    /* C++ specific code for TREE_BINFO */
    #ifdef __cplusplus
    {
        BaseClass* base_ptr;
        DerivedClass derived_obj;
        TemplateClass<double> template_obj;
        
        /* Virtual call through base pointer */
        base_ptr = &derived_obj;
        int virtual_result = base_ptr->virtual_method(10);
        sum += virtual_result;
        
        /* dynamic_cast for RTTI */
        DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
        if (derived_ptr) {
            sum += derived_ptr->get_extra();
        }
        
        /* Template instantiation (TREE_VEC) */
        double template_result = template_obj.process(5);
        sum += (int)template_result;
    }
    #endif
    
    /* Use external function with various identifiers */
    opaque_external_function(&sum);
    
    /* Use designator array (TREE_VEC) */
    for (i = 0; i < 10; i++) {
        sum += designator_array[i];
    }
    
    /* Complex block structure with labels (BLOCK nodes) */
    {
        int block_var1 = 10;
        goto middle_of_block;
        
        int block_var2 = 20;  /* Unreachable declaration */
        
        middle_of_block:
        {
            int inner_block_var = 30;
            sum += block_var1 + inner_block_var;
            goto block_end;
            
            int hidden_again = 40;  /* Another unreachable declaration */
        }
        
        int after_inner = 50;  /* This won't be executed */
        
        block_end:
        sum += 1;
    }
    
    printf("Final checksum: %d\n", sum);
    return sum % 256;
}

/* Dummy definition to satisfy linker (in real test would be in separate file) */
void opaque_external_function(int* ptr) {
    *ptr += 42;
}
