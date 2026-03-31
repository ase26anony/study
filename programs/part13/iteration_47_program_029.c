/* Test program to trigger tree_kind dispatch for various TREE_CODE cases */
#ifdef __cplusplus
#include <vector>
#include <iostream>
extern "C" {
#endif

/* External function to prevent optimization */
extern void opaque_external_function(int*);

/* Global identifiers (IDENTIFIER_NODE) */
static int static_global_counter = 0;
extern int external_global_data;
int global_accumulator = 0;

/* Struct for CONSTRUCTOR nodes */
struct ComplexData {
    int values[4];
    struct Inner {
        double x;
        double y;
    } point;
};

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexData build_data(int depth, int base) {
    struct ComplexData result;
    
    /* Array initializer with designator (TREE_VEC) */
    int temp[4] = {[0] = base, [2] = depth, [3] = base * depth};
    
    for (int i = 0; i < 4; i++) {
        result.values[i] = temp[i];
    }
    
    result.point.x = (double)base;
    result.point.y = (double)depth;
    
    if (depth > 0) {
        struct ComplexData nested = build_data(depth - 1, base + 1);
        for (int i = 0; i < 4; i++) {
            result.values[i] += nested.values[i];
        }
    }
    
    return result;
}

#ifdef __cplusplus
/* C++ classes for TREE_BINFO generation */
class BaseClass {
public:
    virtual int virtual_method(int x) {
        return x * 2;
    }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
private:
    int extra_data;
public:
    DerivedClass(int val) : extra_data(val) {}
    virtual int virtual_method(int x) override {
        return x * 3 + extra_data;
    }
    
    /* Template instantiation (TREE_VEC) */
    std::vector<int> create_vector() {
        std::vector<int> vec;
        vec.push_back(extra_data);
        vec.push_back(extra_data * 2);
        return vec;
    }
};
#endif

int main(int argc, char* argv[]) {
    /* Use argc to prevent optimization */
    int iterations = (argc > 1) ? 10 : 5;
    int use_openmp = (argc > 2);
    
    /* CONSTRUCTOR nodes - aggregate initialization */
    struct ComplexData data = {{1, 2, 3, 4}, {5.0, 6.0}};
    struct ComplexData data2 = {.values = {7, 8}, .point = {9.0, 10.0}};
    
    /* BLOCK nodes with goto */
    {
        int block_local = 100;
        goto skip_declaration;
        
        {
            int hidden_variable = 999; /* This won't be executed */
        }
        
    skip_declaration:
        /* Use block_local to keep it alive */
        data.values[0] += block_local;
    }
    
    /* Nested blocks */
    {
        int outer = 50;
        {
            int inner = outer + 10;
            data.values[1] += inner;
            
            /* Another block with label */
            another_label:
            data.values[2] += 5;
        }
    }
    
    /* Recursive call for CONSTRUCTOR */
    struct ComplexData recursive_result = build_data(3, 1);
    
    /* SSA_NAME generation - complex control flow */
    int ssa_var = 0;
    for (int i = 0; i < iterations; i++) {
        volatile int condition = (i % 3); /* volatile prevents optimization */
        
        if (condition == 0) {
            ssa_var = i * 2;
        } else if (condition == 1) {
            ssa_var = i * 3 + 1;
        } else {
            ssa_var = i * 4 + 2;
        }
        
        /* Use ssa_var in computation to create phi nodes */
        data.values[3] += ssa_var;
    }
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE) */
    int openmp_sum = 0;
    int openmp_array[100];
    
    for (int i = 0; i < 100; i++) {
        openmp_array[i] = i;
    }
    
    if (use_openmp) {
        #pragma omp parallel for private(iterations) firstprivate(data) \
                shared(openmp_array) reduction(+:openmp_sum) schedule(dynamic) \
                num_threads(4) if(iterations > 5)
        for (int i = 0; i < 100; i++) {
            openmp_sum += openmp_array[i];
            
            /* Nested OpenMP directive */
            #pragma omp atomic
            global_accumulator += 1;
        }
        
        /* Another OpenMP region with different clauses */
        #pragma omp parallel sections private(ssa_var) \
                reduction(*:openmp_sum) collapse(1) nowait
        {
            #pragma omp section
            {
                ssa_var = 1;
                openmp_sum *= 2;
            }
            #pragma omp section
            {
                ssa_var = 2;
                openmp_sum *= 3;
            }
        }
    }
    
#ifdef __cplusplus
    /* C++ specific code for TREE_BINFO */
    BaseClass* base_ptr;
    DerivedClass derived(42);
    base_ptr = &derived;
    
    /* Virtual call through base pointer */
    int virtual_result = base_ptr->virtual_method(10);
    
    /* Template instantiation (TREE_VEC) */
    std::vector<int> template_vec = derived.create_vector();
    std::vector<double> another_vec;
    another_vec.push_back(3.14);
    
    /* dynamic_cast for BINFO usage */
    DerivedClass* casted = dynamic_cast<DerivedClass*>(base_ptr);
    if (casted) {
        data.values[0] += casted->virtual_method(5);
    }
#endif
    
    /* Call external function with various identifiers */
    opaque_external_function(data.values);
    opaque_external_function(&ssa_var);
    opaque_external_function(&openmp_sum);
    
    /* Complex array initializer with designators (TREE_VEC) */
    int complex_array[10] = {[0] = 1, [2] = 2, [4] = data.values[0], 
                             [6] = ssa_var, [8] = openmp_sum % 100};
    
    /* Compute final checksum */
    int final_result = 0;
    for (int i = 0; i < 4; i++) {
        final_result += data.values[i];
        final_result += recursive_result.values[i];
    }
    
    final_result += ssa_var;
    final_result += openmp_sum;
    
#ifdef __cplusplus
    final_result += virtual_result;
    if (!template_vec.empty()) {
        final_result += template_vec[0];
    }
#endif
    
    for (int i = 0; i < 10; i++) {
        final_result += complex_array[i];
    }
    
    /* Print result to ensure code isn't eliminated */
    printf("Final checksum: %d\n", final_result);
    
    return final_result > 0 ? 0 : 1;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
