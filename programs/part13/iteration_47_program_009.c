/* Test program to exercise specific tree node types in GCC's tree.cc */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void opaque_external_function(void*);

/* Global identifiers for IDENTIFIER_NODE coverage */
static int static_global_counter = 0;
extern int external_global_data;
volatile int volatile_global = 1;

/* Complex struct for CONSTRUCTOR nodes */
struct ComplexData {
    int values[4];
    struct {
        double x, y;
    } point;
    char* name;
};

/* Nested struct for more complex constructors */
struct OuterStruct {
    struct ComplexData data;
    int flags[3];
    struct OuterStruct* next;
};

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexData recursive_builder(int depth, int base) {
    struct ComplexData result = {
        .values = {base, base + 1, base + 2, base + 3},
        .point = {.x = depth * 1.5, .y = depth * 2.5},
        .name = depth > 0 ? "recursive" : "base"
    };
    
    if (depth > 0) {
        struct ComplexData inner = recursive_builder(depth - 1, base * 2);
        /* Combine results */
        for (int i = 0; i < 4; i++) {
            result.values[i] += inner.values[i];
        }
    }
    return result;
}

/* Function with complex control flow for SSA_NAME and BLOCK nodes */
int complex_control_flow(int n, int* results) {
    int x = 0;
    int y = 0;
    
    /* Outer block with local variable */
    {
        int block_local = n * 2;
        
        /* goto to create interesting control flow */
        if (n % 3 == 0) {
            goto special_case;
        }
        
        for (int i = 0; i < n; i++) {
            /* Conditional assignment creating phi nodes */
            if (i % 2 == 0) {
                x = i * 3;
            } else {
                x = i * 7;
            }
            
            /* Another variable with multiple assignments */
            if (i % 3 == 0) {
                y = x + 1;
            } else if (i % 3 == 1) {
                y = x - 1;
            } else {
                y = x * 2;
            }
            
            results[i] = x + y + block_local;
        }
        
        goto normal_exit;
        
    special_case:
        {
            /* Inner block with its own local */
            int special_local = 100;
            for (int i = 0; i < n; i++) {
                results[i] = i + special_local;
            }
        }
    }
    
normal_exit:
    return x + y;
}

#ifdef __cplusplus
/* C++ specific code for TREE_BINFO nodes */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int virtual_method(int x) {
        return x * 2;
    }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method(int x) override {
        return x * 3 + base_data;
    }
    int derived_data;
};

class SecondDerived : public DerivedClass {
public:
    virtual int virtual_method(int x) override {
        return x * 4 + base_data + derived_data;
    }
};

template<typename T>
class TemplateClass {
public:
    T data;
    TemplateClass(T val) : data(val) {}
    T process() { return data * 2; }
};

/* Function using templates (TREE_VEC) */
void template_usage() {
    TemplateClass<int> int_template(42);
    TemplateClass<double> double_template(3.14);
    
    int_template.process();
    double_template.process();
    
    /* Use standard library template */
    int arr[] = {1, 2, 3, 4, 5};
    /* This creates TREE_VEC nodes */
    for (auto val : arr) {
        volatile_global += val;
    }
}
#endif

int main(int argc, char** argv) {
    /* Use argc to prevent optimization */
    int iterations = argc > 1 ? atoi(argv[1]) : 10;
    int size = argc > 2 ? atoi(argv[2]) : 100;
    
    /* Array with complex initializer (CONSTRUCTOR) */
    int matrix[3][4] = {
        {1, 2, [3] = 10},
        {[0] = 5, 6, 7},
        {8, 9, 10, 11}
    };
    
    /* Struct with designated initializers (CONSTRUCTOR) */
    struct OuterStruct complex_struct = {
        .data = {
            .values = {100, 200, 300, 400},
            .point = {3.14, 2.718},
            .name = "test"
        },
        .flags = {1, 0, 1},
        .next = NULL
    };
    
    /* Call recursive function */
    struct ComplexData built = recursive_builder(3, 10);
    
    /* Array for results */
    int* results = (int*)malloc(size * sizeof(int));
    
    /* Complex control flow for SSA */
    int control_result = complex_control_flow(iterations, results);
    
    /* OpenMP region with multiple clauses */
    int sum = 0;
    #pragma omp parallel for reduction(+:sum) private(iterations) \
            firstprivate(size) shared(results, matrix) collapse(2) \
            schedule(dynamic)
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            int local_var = i * 100 + j;
            /* Conditional for SSA */
            if (local_var % 2 == 0) {
                local_var = matrix[i][j] * 2;
            } else {
                local_var = matrix[i][j] + results[j % iterations];
            }
            sum += local_var;
        }
    }
    
    /* Nested OpenMP with different clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task private(iterations) shared(sum)
            {
                sum += built.values[0];
            }
        }
    }
    
    /* Call external function with various identifiers */
    opaque_external_function(&built);
    opaque_external_function(results);
    opaque_external_function(&sum);
    
    #ifdef __cplusplus
    /* C++ specific: use classes for TREE_BINFO */
    BaseClass* base_ptr;
    DerivedClass derived;
    SecondDerived second_derived;
    
    /* Virtual calls through different pointer types */
    base_ptr = &derived;
    control_result += base_ptr->virtual_method(iterations);
    
    base_ptr = &second_derived;
    control_result += base_ptr->virtual_method(size);
    
    /* Template usage for TREE_VEC */
    template_usage();
    
    /* dynamic_cast for BINFO usage */
    BaseClass* casted = dynamic_cast<BaseClass*>(&second_derived);
    if (casted) {
        control_result += casted->virtual_method(5);
    }
    #endif
    
    /* Use all computed values to prevent elimination */
    printf("Result: sum=%d, control=%d, built=[%d,%d,%d,%d]\n",
           sum, control_result,
           built.values[0], built.values[1],
           built.values[2], built.values[3]);
    
    free(results);
    return 0;
}
