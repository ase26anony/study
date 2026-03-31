/* Test program to exercise tree node dispatch in GCC */
#ifdef __cplusplus
#include <vector>
#include <iostream>
extern "C" {
#endif

/* External function to prevent optimization */
extern void opaque_external_function(void*);

/* Global identifiers (IDENTIFIER_NODE) */
int global_counter = 0;
static int static_hidden = 42;
volatile int volatile_guard = 0;

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
    virtual int virtual_method(int x) override { 
        return x * 3 + base_data; 
    }
    int derived_data;
};

template<typename T>
class TemplateClass {
public:
    T template_data;
    std::vector<T> vec_member;  /* TREE_VEC generation */
};
#endif

/* Complex struct with constructor */
struct Aggregate {
    int a;
    int b;
    int c;
    double d;
};

/* Recursive function returning struct (CONSTRUCTOR) */
struct Aggregate recursive_constructor(int depth, int value) {
    struct Aggregate result;
    if (depth <= 0) {
        /* Aggregate initializer (CONSTRUCTOR node) */
        result = (struct Aggregate){value, value + 1, value + 2, value * 1.5};
        return result;
    }
    
    /* Nested block with local variable (BLOCK node) */
    {
        int local_in_block = depth * 10;
        if (local_in_block > 50) {
            /* Jump to label */
            goto skip_part;
        }
        value += local_in_block;
    skip_part:
        /* Use after label */
        opaque_external_function(&local_in_block);
    }
    
    /* Recursive call */
    struct Aggregate temp = recursive_constructor(depth - 1, value);
    result.a = temp.a + 1;
    result.b = temp.b + 2;
    result.c = temp.c + 3;
    result.d = temp.d * 1.1;
    return result;
}

/* Function with SSA_NAME generation */
int ssa_generator(int iterations, int condition) {
    int x = 0, y = 0, z = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Conditional assignment creating phi nodes */
        if (condition & 1) {
            x = i * 2;
            y = x + 1;
        } else {
            x = i * 3;
            y = x - 1;
        }
        
        /* Complex expression with multiple uses */
        z += x * y;
        
        /* Nested condition for more SSA */
        if (z > 1000) {
            x = z % 100;
        } else {
            x = z % 50;
        }
        
        /* Use volatile to prevent optimization */
        if (volatile_guard) {
            y = volatile_guard;
        }
    }
    
    /* Multiple assignments to same variable */
    int result = z;
    result = result * 2;
    result = result + x;
    result = result - y;
    
    return result;
}

int main(int argc, char* argv[]) {
    int i, j, sum = 0;
    
    /* Use argc to prevent compile-time optimization */
    int iterations = (argc > 1) ? 100 : 200;
    int use_openmp = (argc > 2);
    
    /* Array with designated initializer (CONSTRUCTOR) */
    int designated_array[10] = {[0] = 1, [5] = argc, [9] = 99};
    
    /* Struct with complex initializer (CONSTRUCTOR) */
    struct Aggregate agg = {.a = 1, .b = 2, .c = argc, .d = 3.14159};
    
    /* Nested blocks with labels and gotos (BLOCK nodes) */
    {
        int block_var1 = 10;
        goto label2;
        
        {
            int block_var2 = 20;
        label1:
            block_var1 += block_var2;
            goto label3;
        }
        
    label2:
        {
            int block_var3 = 30;
            goto label1;
        }
        
    label3:
        sum += block_var1;
    }
    
    /* Call recursive function */
    struct Aggregate rec_result = recursive_constructor(3, argc);
    sum += rec_result.a + rec_result.b;
    
    /* Generate SSA_NAME nodes */
    sum += ssa_generator(iterations, argc);
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE nodes) */
    if (use_openmp) {
        int private_var = 0;
        int firstprivate_var = argc;
        int shared_sum = 0;
        int reduction_sum = 0;
        
        #pragma omp parallel for private(i, private_var) \
            firstprivate(firstprivate_var) shared(shared_sum) \
            reduction(+:reduction_sum) collapse(2) \
            schedule(dynamic)
        for (i = 0; i < 10; i++) {
            for (j = 0; j < 10; j++) {
                private_var = i * j;
                firstprivate_var += private_var;
                reduction_sum += private_var;
                
                /* Nested OpenMP directive */
                #pragma omp atomic
                shared_sum += private_var;
            }
        }
        
        sum += shared_sum + reduction_sum + firstprivate_var;
    }
    
    /* Complex array initialization (TREE_VEC in C mode) */
    int complex_init[3][2] = {{1, 2}, {3, argc}, {5, 6}};
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 2; j++) {
            sum += complex_init[i][j];
        }
    }
    
    /* Call external function with various identifiers */
    opaque_external_function(&sum);
    opaque_external_function(&global_counter);
    opaque_external_function(&static_hidden);
    opaque_external_function(&agg);
    opaque_external_function(designated_array);
    
#ifdef __cplusplus
    /* C++ specific code for TREE_BINFO and template TREE_VEC */
    DerivedClass derived_obj;
    derived_obj.base_data = argc;
    derived_obj.derived_data = iterations;
    
    BaseClass* base_ptr = &derived_obj;
    sum += base_ptr->virtual_method(argc);
    
    /* Template instantiation (TREE_VEC) */
    TemplateClass<int> template_obj;
    template_obj.template_data = sum;
    template_obj.vec_member.push_back(argc);
    template_obj.vec_member.push_back(iterations);
    
    sum += template_obj.vec_member.size() * 10;
#endif
    
    /* Final output to ensure all code is live */
    printf("Result: %d\n", sum);
    
    return sum > 1000 ? 0 : 1;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
