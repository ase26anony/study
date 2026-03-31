/* Test program to trigger tree_kind dispatch for uncovered TREE_CODE cases */
#ifdef __cplusplus
#include <vector>
#include <iostream>
extern "C" {
#endif

/* External function to prevent optimization */
extern void opaque_external_function(int*);

/* Global identifiers (IDENTIFIER_NODE) */
int global_counter = 0;
static int static_hidden = 42;
volatile int volatile_global = 100;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    int b;
    float c;
    double d;
};

/* Array with complex initializer (TREE_VEC in C mode) */
int complex_array[5] = {[0] = 10, [2] = 20, [4] = 30};

#ifdef __cplusplus
/* C++ class hierarchy for TREE_BINFO */
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

template<typename T>
class TemplateClass {
    T data;
public:
    TemplateClass(T d) : data(d) {}
    T get() { return data; }
};
#endif

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_struct_builder(int depth, int value) {
    struct ComplexStruct result;
    if (depth <= 0) {
        result.a = value;
        result.b = value * 2;
        result.c = value * 3.0f;
        result.d = value * 4.0;
        return result;
    }
    
    /* Create BLOCK with local variable */
    {
        int local_in_block = depth * 10;
        if (local_in_block > 50) {
            goto skip_part;
        }
        value += local_in_block;
    skip_part:
        value += 5;
    }
    
    struct ComplexStruct inner = recursive_struct_builder(depth - 1, value);
    result.a = inner.a + 1;
    result.b = inner.b + 2;
    result.c = inner.c + 3.0f;
    result.d = inner.d + 4.0;
    return result;
}

/* Function with complex control flow for SSA_NAME */
int ssa_generator(int n, int* arr) {
    int result = 0;
    int i;
    
    /* This creates phi nodes during SSA formation */
    for (i = 0; i < n; i++) {
        int temp;
        if (i % 2 == 0) {
            temp = arr[i] * 2;
        } else {
            temp = arr[i] + 3;
        }
        
        /* Multiple assignments to same variable in loop */
        if (temp > 100) {
            result = result + temp;
        } else {
            result = result - temp;
        }
        
        /* Another SSA opportunity */
        int x;
        if (i % 3 == 0) {
            x = temp * 2;
        } else if (i % 3 == 1) {
            x = temp / 2;
        } else {
            x = temp + 100;
        }
        result += x;
    }
    
    /* Jump between blocks */
    {
        int block_var1 = 10;
        goto middle;
        
        int unused = 20; /* This will be skipped */
        
    middle:
        {
            int block_var2 = 30;
            result += block_var1 + block_var2;
            goto end_block;
            
            int also_unused = 40;
        }
        
    end_block:
        result += 5;
    }
    
    return result;
}

int main(int argc, char** argv) {
    /* Use argc to prevent optimization */
    int iterations = (argc > 1) ? 100 : 200;
    int use_openmp = (argc > 2);
    
    /* CONSTRUCTOR: Struct initialization */
    struct ComplexStruct cs = {.a = 1, .b = 2, .c = 3.14f, .d = 2.71828};
    
    /* CONSTRUCTOR: Array with designated initializer */
    int matrix[3][3] = {{[0] = 1, [2] = 3}, {[1] = 4}, {[0] = 7, [1] = 8, [2] = 9}};
    
    /* BLOCK: Nested blocks with local variables */
    {
        int block_local = 42;
        {
            int inner_block_local = block_local * 2;
            global_counter += inner_block_local;
        }
        
        /* goto jumping into another block */
        if (block_local > 0) {
            goto target_label;
        }
        
        int skipped_var = 99; /* May be optimized out */
        
    target_label:
        {
            int target_var = 77;
            global_counter += target_var;
        }
    }
    
    /* Call recursive function */
    struct ComplexStruct recursive_result = recursive_struct_builder(3, 10);
    
    /* Prepare array for SSA testing */
    int data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = i * (i % 7);
    }
    
    /* SSA_NAME generation */
    int ssa_result = ssa_generator(iterations, data);
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE) */
    int openmp_sum = 0;
    if (use_openmp) {
        #pragma omp parallel for private(i) firstprivate(iterations) \
                shared(data) reduction(+:openmp_sum) collapse(2) \
                schedule(dynamic, 4)
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                int idx = i * 10 + j;
                if (idx < 100) {
                    openmp_sum += data[idx] * (i + 1);
                }
            }
        }
        
        /* Nested OpenMP with different clauses */
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp task private(ssa_result) firstprivate(openmp_sum)
                {
                    int task_local = ssa_result + openmp_sum;
                    opaque_external_function(&task_local);
                }
            }
        }
    }
    
#ifdef __cplusplus
    /* TREE_BINFO: C++ class hierarchy usage */
    BaseClass* base_ptr;
    DerivedClass derived_obj;
    derived_obj.base_data = 100;
    derived_obj.derived_data = 200;
    
    base_ptr = &derived_obj;
    int virtual_result = base_ptr->virtual_method(iterations);
    
    /* TREE_VEC: Template instantiation */
    std::vector<int> int_vector;
    int_vector.push_back(1);
    int_vector.push_back(2);
    int_vector.push_back(3);
    
    TemplateClass<double> template_instance(3.14159);
    double template_value = template_instance.get();
    
    /* Use all results to prevent optimization */
    std::cout << "Results: " << global_counter << " " << ssa_result 
              << " " << openmp_sum << " " << virtual_result 
              << " " << template_value << std::endl;
#else
    /* Call external function with various identifiers */
    opaque_external_function(&global_counter);
    opaque_external_function(&ssa_result);
    opaque_external_function(&openmp_sum);
    
    /* Use volatile to prevent dead code elimination */
    volatile int final_result = global_counter + ssa_result + openmp_sum 
                                + recursive_result.a + cs.b;
    
    /* Print to ensure code is live */
    printf("Final: %d\n", final_result);
#endif
    
    return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
