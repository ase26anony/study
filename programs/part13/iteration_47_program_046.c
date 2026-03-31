/* Test program to trigger tree_kind dispatch for uncovered TREE_CODE cases */
#ifdef __cplusplus
#include <vector>
#include <iostream>
using namespace std;

/* For TREE_BINFO - C++ class hierarchy */
class Base {
public:
    virtual int method() { return 1; }
    virtual ~Base() {}
};

class Derived : public Base {
public:
    virtual int method() override { return 2; }
    int extra;
};

template<typename T>
class TemplateClass {
    T data;
public:
    TemplateClass(T d) : data(d) {}
    T get() { return data; }
};
#endif

/* External function to prevent optimization */
extern void opaque_external_function(int*);

/* Global identifiers (IDENTIFIER_NODE) */
int global_counter = 0;
static int static_accumulator = 0;
volatile int volatile_indicator = 0;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    int b;
    int c[3];
};

struct ComplexStruct recursive_function(int depth, int base) {
    /* BLOCK node with local variable */
    {
        int local_in_block = depth * 2;
        if (local_in_block > 10) {
            /* Another nested block */
            int nested = local_in_block / 2;
            opaque_external_function(&nested);
        }
    }
    
    /* CONSTRUCTOR node - aggregate initializer */
    struct ComplexStruct result = {
        .a = base + depth,
        .b = base - depth,
        .c = {[0] = depth, [1] = base, [2] = depth + base}
    };
    
    if (depth > 0) {
        struct ComplexStruct inner = recursive_function(depth - 1, base);
        result.a += inner.a;
        result.b += inner.b;
        result.c[0] += inner.c[0];
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    /* Use argc to prevent optimization */
    int iterations = (argc > 1) ? 10 : 5;
    int sum = 0;
    
    /* CONSTRUCTOR nodes - various initializations */
    struct ComplexStruct cs = {1, 2, {3, 4, 5}};
    int array_with_designator[5] = {[0] = 1, [2] = argc, [4] = 3};
    
    /* BLOCK nodes with goto */
    int block_var = 0;
    goto skip_init;
    
    {
        int hidden_init = 42;
        block_var = hidden_init;
    }
    
skip_init:
    /* Force use of block_var to keep it live */
    opaque_external_function(&block_var);
    
    /* SSA_NAME generation - complex conditional assignments */
    int ssa_var = 0;
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            ssa_var = i * 2;
        } else {
            ssa_var = i * 3 + 1;
        }
        
        /* Use in expression to create phi nodes */
        int temp = ssa_var + argc;
        sum += temp;
        
        /* Another SSA opportunity */
        int another;
        if (ssa_var > 10) {
            another = ssa_var / 2;
        } else {
            another = ssa_var * 2;
        }
        sum += another;
    }
    
    /* Recursive function call for CONSTRUCTOR nodes */
    struct ComplexStruct rec_result = recursive_function(3, argc);
    sum += rec_result.a + rec_result.b + rec_result.c[0];
    
    /* OpenMP region for OMP_CLAUSE nodes */
    #pragma omp parallel reduction(+:sum) private(ssa_var) shared(global_counter) if(iterations > 1)
    {
        #pragma omp for collapse(2) schedule(dynamic)
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                int local_sum = i * j + argc;
                #pragma omp atomic
                sum += local_sum;
            }
        }
        
        /* Nested OpenMP with more clauses */
        #pragma omp sections firstprivate(iterations) lastprivate(block_var)
        {
            #pragma omp section
            {
                block_var = 1;
                opaque_external_function(&block_var);
            }
            #pragma omp section
            {
                block_var = 2;
                opaque_external_function(&block_var);
            }
        }
    }
    
    /* More complex OpenMP with multiple clauses */
    int reduction_var = 0;
    #pragma omp parallel for private(ssa_var) reduction(+:reduction_var) \
            linear(iterations:1) ordered
    for (int i = 0; i < 10; i++) {
        #pragma omp ordered
        {
            reduction_var += i + argc;
        }
    }
    sum += reduction_var;
    
#ifdef __cplusplus
    /* TREE_VEC through template instantiation */
    vector<int> vec_int;
    vec_int.push_back(argc);
    vec_int.push_back(sum);
    
    /* More template usage */
    TemplateClass<int> tc(sum);
    sum = tc.get();
    
    /* TREE_BINFO through polymorphism */
    Base* base_ptr;
    if (argc % 2 == 0) {
        base_ptr = new Derived();
    } else {
        base_ptr = new Base();
    }
    
    sum += base_ptr->method();
    
    /* dynamic_cast for BINFO usage */
    Derived* derived_ptr = dynamic_cast<Derived*>(base_ptr);
    if (derived_ptr) {
        sum += 100;
    }
    
    delete base_ptr;
#endif
    
    /* Use various identifiers */
    static_accumulator += sum;
    global_counter++;
    volatile_indicator = sum;
    
    /* Final opaque call with mix of identifiers */
    opaque_external_function(&sum);
    opaque_external_function(&static_accumulator);
    opaque_external_function(&global_counter);
    
    /* Print to ensure code isn't eliminated */
    printf("Result: %d\n", sum);
    
    return sum > 100 ? 0 : 1;
}
