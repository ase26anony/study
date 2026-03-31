/* Test program to trigger tree_kind dispatch for various TREE_CODE cases */
#ifdef __cplusplus
#include <vector>
#include <iostream>
#endif

/* External function to prevent optimization */
extern void opaque_external(int*);

/* Global identifiers (IDENTIFIER_NODE) */
int global_counter = 0;
static int static_hidden = 42;
extern int external_reference;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int values[4];
    struct Inner {
        double x, y;
    } inner;
    volatile int flags;
};

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_builder(int depth, int base) {
    struct ComplexStruct cs;
    
    /* Array initializer with designator (TREE_VEC) */
    cs.values[0] = base;
    cs.values[1] = base * 2;
    cs.values[2] = base * 3;
    cs.values[3] = base * 4;
    
    cs.inner.x = depth * 1.5;
    cs.inner.y = depth * 2.5;
    cs.flags = depth;
    
    if (depth > 0) {
        struct ComplexStruct nested = recursive_builder(depth - 1, base + 1);
        /* Combine values */
        for (int i = 0; i < 4; i++) {
            cs.values[i] += nested.values[i];
        }
    }
    
    return cs;
}

#ifdef __cplusplus
/* C++ classes for TREE_BINFO */
class BaseClass {
public:
    virtual int virtual_method(int x) {
        return x * 2;
    }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
private:
    std::vector<int> data;  /* TREE_VEC from template */
public:
    DerivedClass(int n) {
        for (int i = 0; i < n; i++) {
            data.push_back(i * i);
        }
    }
    
    int virtual_method(int x) override {
        int sum = 0;
        for (int val : data) {
            sum += val;
        }
        return x + sum;
    }
    
    void process_with_blocks() {
        /* BLOCK nodes with goto */
        {
            int local_in_block = 100;
            goto skip_init;
            
            int unused = 50;  /* Jumped over */
            
        skip_init:
            /* Use SSA_NAME generation */
            int ssa_var = local_in_block;
            for (int i = 0; i < 10; i++) {
                if (i % 2 == 0) {
                    ssa_var += i * 2;
                } else {
                    ssa_var -= i;
                }
            }
            opaque_external(&ssa_var);
        }
    }
};
#endif

int main(int argc, char* argv[]) {
    /* Use argc to prevent optimization */
    int iterations = (argc > 1) ? 10 : 20;
    int use_openmp = (argc > 2);
    
    /* CONSTRUCTOR nodes with complex initialization */
    struct ComplexStruct cs = {
        .values = {[0] = 1, [2] = 3, [1] = 2, [3] = 4},  /* Designated init */
        .inner = {3.14, 2.718},
        .flags = 0xABCD
    };
    
    /* Another CONSTRUCTOR with nested initializer */
    struct ComplexStruct cs2 = recursive_builder(3, 1);
    
    /* BLOCK nodes with labels and gotos */
    volatile int block_switch = 1;
    
    if (block_switch) {
        goto outer_block;
    }
    
    {
        int hidden_var = 999;
    outer_block:
        /* This creates BLOCK structure */
        int visible_var = hidden_var + 1;
        opaque_external(&visible_var);
    }
    
    /* SSA_NAME generation with complex control flow */
    int ssa_result = 0;
    for (int i = 0; i < iterations; i++) {
        int temp;
        if (i % 3 == 0) {
            temp = i * 2;
        } else if (i % 3 == 1) {
            temp = i + 5;
        } else {
            temp = i - 2;
        }
        
        /* Force phi node creation */
        ssa_result += temp;
        
        /* Additional SSA complexity */
        for (int j = 0; j < i % 5; j++) {
            ssa_result = (ssa_result % 2 == 0) ? ssa_result / 2 : ssa_result * 3 + 1;
        }
    }
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE) */
    int openmp_sum = 0;
    int openmp_array[100];
    
    #pragma omp parallel if(use_openmp) default(none) \
        private(iterations) shared(openmp_array, argc) reduction(+:openmp_sum)
    {
        #pragma omp for schedule(dynamic, 4) collapse(2) nowait
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < 10; j++) {
                int idx = i * 10 + j;
                openmp_array[idx] = i * j;
                openmp_sum += openmp_array[idx];
                
                /* Nested OpenMP directive */
                #pragma omp atomic
                global_counter++;
            }
        }
        
        /* Additional OpenMP construct */
        #pragma omp single
        {
            int single_var = 42;
            opaque_external(&single_var);
        }
    }
    
    #ifdef __cplusplus
    /* C++ specific: TREE_BINFO and TREE_VEC from templates */
    BaseClass* poly_obj = new DerivedClass(5);
    int poly_result = poly_obj->virtual_method(ssa_result);
    
    /* Use template (TREE_VEC) */
    std::vector<std::vector<int>> matrix(3, std::vector<int>(3));
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            matrix[i][j] = i * j + poly_result;
        }
    }
    
    /* Call method with blocks */
    static_cast<DerivedClass*>(poly_obj)->process_with_blocks();
    
    delete poly_obj;
    #endif
    
    /* Final computation using all results */
    int final_result = cs.values[0] + cs2.values[1] + ssa_result + openmp_sum;
    #ifdef __cplusplus
    final_result += poly_result;
    #endif
    
    opaque_external(&final_result);
    
    /* Prevent dead code elimination */
    if (argc > 3) {
        return final_result % 256;
    }
    
    return 0;
}
