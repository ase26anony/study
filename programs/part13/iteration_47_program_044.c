/* Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */

#include <cstdio>
#include <cstdlib>

/* External function to prevent optimization */
extern "C" void opaque_external_function(int*);

/* Global identifiers for IDENTIFIER_NODE coverage */
int global_identifier_1;
static int static_identifier_2;
extern int extern_identifier_3;

/* Complex struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    double b;
    char c;
    int* d;
};

/* Array with designated initializer (TREE_VEC) */
int designated_array[10] = {[0] = 1, [5] = 2, [9] = 3};

/* Recursive function returning struct (CONSTRUCTOR) */
ComplexStruct recursive_struct_builder(int depth, int value) {
    ComplexStruct result;
    result.a = value;
    result.b = value * 1.5;
    result.c = 'A' + (value % 26);
    result.d = &global_identifier_1;
    
    if (depth > 0) {
        ComplexStruct nested = recursive_struct_builder(depth - 1, value + 1);
        result.a += nested.a;
        result.b += nested.b;
    }
    
    return result;  /* CONSTRUCTOR node for return value */
}

/* C++ classes for TREE_BINFO coverage */
class BaseClass {
public:
    virtual int virtual_method(int x) {
        return x * 2;
    }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method(int x) override {
        return x * 3;
    }
    
    /* Template instantiation for TREE_VEC */
    template<typename T>
    T template_method(T value) {
        return value * 2;
    }
};

/* Function with complex control flow for SSA_NAME and BLOCK nodes */
int complex_control_flow(int argc, char** argv) {
    volatile int seed = argc;  /* Prevent optimization */
    int result = 0;
    
    /* Outer block */
    {
        int block_local_1 = seed * 2;
        
        /* Inner block with goto */
        {
            int block_local_2 = block_local_1 + 1;
            if (seed % 2 == 0) {
                goto skip_section;
            }
            
            block_local_2 *= 3;
            
        skip_section:
            result = block_local_2;
        }
        
        /* Loop with phi node creation for SSA_NAME */
        int phi_variable = 0;
        for (int i = 0; i < seed; i++) {
            if (i % 3 == 0) {
                phi_variable = i * 2;      /* SSA assignment 1 */
            } else if (i % 3 == 1) {
                phi_variable = i + 5;      /* SSA assignment 2 */
            } else {
                phi_variable = i / 2;      /* SSA assignment 3 */
            }
            result += phi_variable;        /* Use forces SSA phi node */
        }
    }
    
    return result;
}

/* OpenMP function with multiple clauses for OMP_CLAUSE nodes */
int openmp_reduction_test(int size) {
    int sum = 0;
    int product = 1;
    
    /* Complex OpenMP region with multiple clauses */
    #pragma omp parallel for reduction(+:sum) reduction(*:product) \
            private(size) firstprivate(product) shared(sum) \
            collapse(2) schedule(dynamic, 4)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int local_temp = i * 100 + j;
            sum += local_temp;
            if (local_temp != 0) {
                product *= (local_temp % 10) + 1;
            }
        }
    }
    
    /* Nested OpenMP region */
    #pragma omp parallel
    {
        #pragma omp sections private(sum)
        {
            #pragma omp section
            { sum += 1; }
            
            #pragma omp section  
            { sum += 2; }
        }
    }
    
    return sum + product;
}

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    volatile int dynamic_value = argc;
    
    /* 1. CONSTRUCTOR nodes - aggregate initialization */
    ComplexStruct cs = {dynamic_value, dynamic_value * 3.14, 'X', &dynamic_value};
    ComplexStruct cs2 = recursive_struct_builder(3, dynamic_value);
    
    /* 2. BLOCK nodes - nested blocks with gotos */
    int block_result = 0;
    {
        int inner_var = 10;
        goto middle;
        
        {
            int skipped_var = 20;
        middle:
            block_result = inner_var + dynamic_value;
        }
        
        /* Another block with local declaration */
        {
            int another_local = 30;
            block_result += another_local;
        }
    }
    
    /* 3. SSA_NAME nodes - complex control flow */
    int ssa_result = complex_control_flow(argc, argv);
    
    /* 4. OpenMP with OMP_CLAUSE nodes */
    int omp_result = openmp_reduction_test(dynamic_value);
    
    /* 5. C++ classes for TREE_BINFO */
    BaseClass* base_ptr;
    if (dynamic_value % 2 == 0) {
        base_ptr = new BaseClass();
    } else {
        base_ptr = new DerivedClass();
    }
    
    int virtual_result = base_ptr->virtual_method(dynamic_value);
    
    /* Template instantiation for TREE_VEC */
    DerivedClass derived;
    int template_result = derived.template_method<int>(dynamic_value);
    
    /* 6. Use various identifiers (IDENTIFIER_NODE) */
    global_identifier_1 = dynamic_value;
    static_identifier_2 = dynamic_value * 2;
    opaque_external_function(&extern_identifier_3);
    
    /* 7. Array with complex access pattern */
    int array_sum = 0;
    for (int i = 0; i < 10; i++) {
        array_sum += designated_array[i];
    }
    
    /* 8. Complex expression using all results */
    int final_result = cs.a + cs2.a + block_result + ssa_result + 
                      omp_result + virtual_result + template_result + 
                      array_sum + global_identifier_1 + static_identifier_2;
    
    printf("Final result: %d\n", final_result);
    
    delete base_ptr;
    return (final_result > 1000) ? 0 : 1;
}

/* External function definition to satisfy linker */
extern "C" void opaque_external_function(int* x) {
    *x = (*x) * 7 + 13;
}
