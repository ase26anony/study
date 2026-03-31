/* Compile with: g++ -O2 -fopenmp -fdump-tree-all -std=c++11 tree_coverage.cc */

#include <cstdio>
#include <cstdlib>

/* External function to prevent optimization */
extern "C" void opaque_external_function(int*);

/* Global identifiers for IDENTIFIER_NODE coverage */
int global_var_1;
static int static_var_2;
extern int extern_var_3;

/* Complex struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    double b;
    char c;
    int* d;
};

/* Nested struct for deeper CONSTRUCTOR coverage */
struct NestedStruct {
    ComplexStruct inner;
    float extra;
};

/* Class hierarchy for TREE_BINFO coverage */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int virtual_method(int x) { return x * 2; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method(int x) override { return x * 3; }
    int derived_data;
};

/* Template for TREE_VEC coverage */
template<typename T, int N>
class FixedArray {
    T data[N];
public:
    T& operator[](int idx) { return data[idx]; }
};

/* Recursive function returning struct (CONSTRUCTOR + control flow) */
ComplexStruct recursive_struct_builder(int depth, int seed) {
    ComplexStruct result;
    result.a = seed;
    result.b = seed * 1.5;
    result.c = 'A' + (seed % 26);
    result.d = &global_var_1;
    
    if (depth > 0) {
        ComplexStruct nested = recursive_struct_builder(depth - 1, seed + 1);
        result.a += nested.a;
        result.b += nested.b;
    }
    
    return result;  /* CONSTRUCTOR node for return value */
}

/* Function with complex control flow for SSA_NAME and BLOCK coverage */
int complex_control_flow(int argc, char** argv) {
    volatile int x = argc;  /* Prevent optimization */
    int y, z;
    
    /* BLOCK node with local variable */
    {
        int local_in_block = x * 2;
        
        /* Conditional with phi node potential */
        if (x > 10) {
            y = local_in_block + 5;
            goto skip_part;  /* Goto for BLOCK stress */
        } else {
            y = local_in_block - 5;
        }
        
        /* Unreachable code that might still generate tree nodes */
        {
            int hidden_var = 100;
            skip_part:
            z = y + hidden_var;  /* Use of variable from different block */
        }
    }
    
    /* Another BLOCK with switch */
    {
        int switch_var = x % 4;
        int result;
        
        switch (switch_var) {
            case 0: result = z * 2; break;
            case 1: result = z + 10; break;
            case 2: result = z - 10; break;
            default: result = z / 2; break;
        }
        
        /* Loop with SSA_NAME generation */
        for (int i = 0; i < result; i++) {
            /* Multiple assignments to same variable */
            int ssa_candidate = i;
            if (i % 2 == 0) {
                ssa_candidate *= 2;
            } else {
                ssa_candidate += 3;
            }
            z += ssa_candidate;  /* Use to prevent elimination */
        }
    }
    
    return z;
}

/* OpenMP function for OMP_CLAUSE coverage */
int openmp_reduction_example(int size) {
    int sum = 0;
    int product = 1;
    
    /* Multi-dimensional array with designator (TREE_VEC potential) */
    int md_array[3][3] = {{[0] = 1, [2] = 3}, {[1] = 5}, {[0] = 7, [1] = 8, [2] = 9}};
    
    #pragma omp parallel for reduction(+:sum) reduction(*:product) \
            private(md_array) firstprivate(size) shared(global_var_1) \
            collapse(2) schedule(dynamic, 4)
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int temp = md_array[i][j];
            sum += temp;
            product *= (temp > 0 ? temp : 1);
            
            /* Nested OpenMP for additional clause coverage */
            #pragma omp atomic
            global_var_1++;
        }
    }
    
    /* OpenMP sections with different clauses */
    #pragma omp parallel sections private(static_var_2) \
            lastprivate(product) copyin(global_var_1)
    {
        #pragma omp section
        {
            static_var_2 = sum % 100;
        }
        #pragma omp section
        {
            product = (product + sum) % 1000;
        }
    }
    
    return sum + product;
}

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations < 1) iterations = 1;
    
    /* CONSTRUCTOR: Aggregate initialization */
    ComplexStruct cs = {.a = 10, .b = 3.14, .c = 'X', .d = &global_var_1};
    
    /* CONSTRUCTOR: Array with designators */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3, [2] = 4};
    
    /* CONSTRUCTOR: Nested struct initialization */
    NestedStruct ns = {{20, 6.28, 'Y', &static_var_2}, 99.5f};
    
    /* TREE_VEC: Template instantiation */
    FixedArray<double, 5> template_array;
    for (int i = 0; i < 5; i++) {
        template_array[i] = i * 1.1;
    }
    
    /* BLOCK: Nested blocks with gotos */
    int block_result = 0;
    {
        int level1 = 100;
        goto middle;
        
        {
            int hidden1 = 999;  /* Never executed but parsed */
            middle:
            {
                int level2 = 200;
                block_result = level1 + level2;
                goto end_block;
            }
            int hidden2 = 888;  /* Never executed */
        }
        end_block:;
    }
    
    /* TREE_BINFO: C++ class hierarchy usage */
    BaseClass* base_ptr;
    if (iterations % 2 == 0) {
        base_ptr = new BaseClass();
    } else {
        base_ptr = new DerivedClass();  /* Dynamic type */
    }
    
    int virtual_result = base_ptr->virtual_method(iterations);
    base_ptr->base_data = virtual_result;
    
    /* Dynamic cast for additional BINFO usage */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        derived_ptr->derived_data = virtual_result * 2;
    }
    
    /* Call functions that generate various tree nodes */
    int control_flow_result = complex_control_flow(argc, argv);
    ComplexStruct recursive_result = recursive_struct_builder(3, iterations);
    int omp_result = openmp_reduction_example(iterations);
    
    /* Use external function with various identifiers */
    int external_call_arg = control_flow_result + omp_result + recursive_result.a;
    opaque_external_function(&external_call_arg);
    
    /* Final computation using all results */
    int final_result = 
        block_result +
        virtual_result +
        control_flow_result +
        omp_result +
        recursive_result.a +
        cs.a +
        ns.inner.a +
        (int)template_array[0];
    
    /* Use all variables to prevent dead code elimination */
    printf("Result: %d (args: %d, arr[2]=%d, ns.extra=%.1f)\n",
           final_result, argc, arr[2], ns.extra);
    
    delete base_ptr;
    return (final_result > 1000) ? 0 : 1;
}

/* Dummy definition to satisfy linker if needed */
extern "C" void opaque_external_function(int* ptr) {
    *ptr += 42;
}
