/* Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */

#include <cstdio>
#include <cstdlib>

/* External function to prevent optimization */
extern "C" void opaque_external_function(int*);

/* Global identifiers for IDENTIFIER_NODE coverage */
int global_identifier_1;
static int static_identifier_2;
extern int extern_identifier_3;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    double b;
    char c;
    int* d;
};

/* Array with complex initializer (TREE_VEC) */
int global_array[] = {[0] = 1, [2] = 3, [4] = 5, [6] = 7, [8] = 9};

/* Recursive function returning struct (CONSTRUCTOR) */
ComplexStruct recursive_struct_builder(int depth, int base) {
    ComplexStruct result;
    result.a = base;
    result.b = base * 1.5;
    result.c = 'A' + (base % 26);
    result.d = &global_array[base % 5];
    
    if (depth > 0) {
        ComplexStruct nested = recursive_struct_builder(depth - 1, base + 1);
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
    int virtual_method(int x) override {
        return x * 3;
    }
    
    void specific_method() {
        /* Empty but creates BINFO */
    }
};

template<typename T>
class TemplateClass {
public:
    T value;
    TemplateClass(T v) : value(v) {}
    
    T get_value() { return value; }
};

/* Function with complex control flow for SSA_NAME and BLOCK nodes */
int complex_control_flow(int argc, char** argv) {
    int result = 0;
    volatile int volatile_var = argc;  /* Prevent optimization */
    
    /* BLOCK node with local variable */
    {
        int block_local = 10;
        result += block_local;
        
        /* Another nested block */
        {
            int nested_block_local = 20;
            result += nested_block_local;
            goto skip_part;  /* Jump to stress BLOCK handling */
            
            int unused = 30;  /* This won't be executed due to goto */
        }
        
        skip_part:
        /* Continue after goto */
        result += 5;
    }
    
    /* SSA_NAME generation through phi nodes */
    int ssa_var;
    if (volatile_var > 5) {
        ssa_var = 100;
    } else {
        ssa_var = 200;
    }
    
    /* Use ssa_var in loop to ensure SSA formation */
    for (int i = 0; i < ssa_var % 10; i++) {
        result += i;
        
        /* More SSA complexity */
        int inner_ssa;
        if (i % 2 == 0) {
            inner_ssa = i * 2;
        } else {
            inner_ssa = i * 3;
        }
        result += inner_ssa;
    }
    
    return result;
}

/* OpenMP function with multiple clauses for OMP_CLAUSE coverage */
int openmp_reduction_test(int size) {
    int sum = 0;
    int product = 1;
    
    #pragma omp parallel for reduction(+:sum) reduction(*:product) \
            private(size) firstprivate(product) shared(global_array) \
            schedule(dynamic, 4) collapse(2) if(size > 1000)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int local_sum = i * 100 + j;
            sum += local_sum;
            product *= (local_sum % 7) + 1;
        }
    }
    
    /* Nested OpenMP region */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task private(sum)
            {
                int task_local = sum % 100;
                opaque_external_function(&task_local);
            }
        }
    }
    
    return sum + product;
}

int main(int argc, char** argv) {
    int final_result = 0;
    
    /* Use command line to prevent constant folding */
    int dynamic_value = argc > 1 ? atoi(argv[1]) : 10;
    
    /* 1. CONSTRUCTOR nodes - struct initialization */
    ComplexStruct cs = {.a = 1, .b = 2.5, .c = 'X', .d = &dynamic_value};
    ComplexStruct cs2 = recursive_struct_builder(3, dynamic_value);
    final_result += cs.a + (int)cs.b + cs.c;
    
    /* 2. TREE_VEC nodes - template instantiation */
    TemplateClass<int> tc_int(42);
    TemplateClass<double> tc_double(3.14159);
    TemplateClass<ComplexStruct> tc_struct(cs2);
    
    final_result += tc_int.get_value();
    final_result += (int)tc_double.get_value();
    final_result += tc_struct.get_value().a;
    
    /* 3. TREE_BINFO nodes - C++ polymorphism */
    BaseClass* base_ptr;
    if (dynamic_value % 2 == 0) {
        base_ptr = new DerivedClass();
    } else {
        base_ptr = new BaseClass();
    }
    
    final_result += base_ptr->virtual_method(dynamic_value);
    
    DerivedClass derived_obj;
    BaseClass& base_ref = derived_obj;
    final_result += base_ref.virtual_method(dynamic_value + 1);
    
    /* 4. SSA_NAME and BLOCK nodes */
    final_result += complex_control_flow(argc, argv);
    
    /* 5. OpenMP with OMP_CLAUSE nodes */
    final_result += openmp_reduction_test(dynamic_value);
    
    /* 6. IDENTIFIER_NODE usage - various identifiers */
    global_identifier_1 = final_result;
    static_identifier_2 = final_result * 2;
    
    /* Call external function with identifiers */
    opaque_external_function(&global_identifier_1);
    opaque_external_function(&static_identifier_2);
    
    /* 7. Array with designators (more TREE_VEC) */
    int local_array[10] = {[0] = 1, [2] = dynamic_value, [5] = final_result % 100};
    for (int i = 0; i < 10; i++) {
        final_result += local_array[i];
    }
    
    /* 8. Complex goto pattern for BLOCK stress */
    {
        int block_var1 = 100;
        goto middle_of_block;
        
        int block_var2 = 200;  /* Skipped by goto */
        
        middle_of_block:
        int block_var3 = 300;
        final_result += block_var1 + block_var3;
        
        goto end_of_block;
        
        {
            int inner_block_var = 400;
            final_result += inner_block_var;
        }
        
        end_of_block: ;
    }
    
    /* Cleanup */
    delete base_ptr;
    
    printf("Final result: %d\n", final_result);
    return final_result % 256;
}

/* Dummy implementation to satisfy external reference */
extern "C" void opaque_external_function(int* x) {
    *x = (*x * 1103515245 + 12345) % 2147483647;
}
