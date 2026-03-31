/* test_tree_nodes.c - Comprehensive test to trigger tree_kind coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables and operations */
int global_var_1 = 10;
int global_var_2 = 20;
float global_var_3 = 3.14;
char global_var_4 = 'A';

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Pattern 3: TREE_BINFO - C++ class hierarchy */
#ifdef __cplusplus
class BaseClass {
public:
    virtual int get_value() { return 42; }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual int get_value() override { return 84; }
};
#endif

/* Pattern 4: SSA_NAME - Functions with loops for SSA */
__attribute__((noinline)) 
int ssa_pattern_1(int n) {
    int x = 0;
    int y = 1;
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    return x + y;
}

__attribute__((noinline))
int ssa_pattern_2(int n) {
    int a = n;
    int b = 1;
    while (a > 0) {
        b = b * a;
        a = a - 1;
    }
    return b;
}

/* Pattern 5: BLOCK - Nested blocks and statement expressions */
__attribute__((noinline))
int block_pattern() {
    int result = 0;
    
    /* Level 1 block */
    {
        int a = 10;
        
        /* Level 2 block */
        {
            int b = 20;
            
            /* Level 3 block - GCC statement expression */
            result = ({
                int c = a + b;
                c * 2;
            });
        }
    }
    
    /* Another block with label address */
    {
        void* label_ptr = &&my_label;
        goto *label_ptr;
        
        my_label:
        result += 100;
    }
    
    return result;
}

/* Pattern 6: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
int constructor_pattern() {
    /* Struct with designated initializer */
    struct ComplexStruct {
        int id;
        float values[3];
        char name[20];
    };
    
    struct ComplexStruct cs = {
        .id = 1,
        .values = {1.1, 2.2, 3.3},
        .name = "test"
    };
    
    /* Array initializer */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Compound literal */
    int* ptr = (int[3]){100, 200, 300};
    
    /* Nested initializer */
    struct Nested {
        struct {
            int x;
            int y;
        } point;
        int data[2];
    } nested = { .point = {.x = 5, .y = 10}, .data = {1, 2} };
    
    return cs.id + arr[0] + ptr[0] + nested.point.x;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int* arr = (int*)__builtin_alloca(size * sizeof(int));
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) schedule(static)
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP directive */
    #pragma omp parallel
    {
        #pragma omp single
        {
            sum += 1;
        }
    }
    
    return sum;
}

/* Vector operations function */
__attribute__((noinline))
int vector_pattern() {
#ifdef __GNUC__
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    v4sf f1 = {1.0, 2.0, 3.0, 4.0};
    v4sf f2 = {0.5, 0.5, 0.5, 0.5};
    v4sf f3 = f1 * f2;
    
    /* Use vectors in expressions */
    int result = c[0] + c[1] + c[2] + c[3];
    result += (int)(f3[0] + f3[1] + f3[2] + f3[3]);
    return result;
#else
    return 0;
#endif
}

/* C++ polymorphism function */
#ifdef __cplusplus
__attribute__((noinline))
int cpp_binfo_pattern() {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* This should involve TREE_BINFO nodes */
    int value = base_ptr->get_value();
    
    /* Try dynamic_cast */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        value += derived_ptr->get_value();
    }
    
    return value;
}
#endif

/* Main function that combines all patterns */
int main() {
    volatile int final_result = 0;
    
    /* Use identifiers in various ways */
    final_result += global_var_1;
    final_result += global_var_2;
    final_result += (int)global_var_3;
    final_result += global_var_4;
    
    /* Take addresses of identifiers */
    int* ptr1 = &global_var_1;
    float* ptr2 = &global_var_3;
    
    /* sizeof expressions with identifiers */
    final_result += sizeof(global_var_1);
    final_result += sizeof(global_var_4);
    
    /* Call pattern functions */
    final_result += ssa_pattern_1(10);
    final_result += ssa_pattern_2(5);
    final_result += block_pattern();
    final_result += constructor_pattern();
    final_result += vector_pattern();
    
    /* OpenMP pattern */
    final_result += omp_pattern(100);
    
#ifdef __cplusplus
    /* C++ patterns */
    final_result += cpp_binfo_pattern();
#endif
    
    /* Prevent dead code elimination */
    volatile int output __attribute__((unused)) = final_result;
    
#ifdef __cplusplus
    std::cout << "Result: " << final_result << std::endl;
#else
    printf("Result: %d\n", final_result);
#endif
    
    return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
