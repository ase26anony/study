/* test_tree_kind.c - Comprehensive test for GCC tree node kinds */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int some_unique_identifier_123;
void identifier_function() {
    int local_identifier = 42;
    some_unique_identifier_123 = local_identifier;
}

/* ==================== TREE_VEC ==================== */
/* GCC statement expressions can create TREE_VEC nodes */
#ifdef __GNUC__
int tree_vec_example() {
    /* Using statement expression with multiple elements */
    int result = ({ 
        int a = 5; 
        int b = 10; 
        int c = a + b; 
        c; 
    });
    return result;
}
#endif

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic with loops creates SSA_NAME nodes */
int ssa_name_example(int n) {
    int a = 0;
    int b = 1;
    
    /* Loop with arithmetic to force SSA form */
    for (int i = 0; i < n; ++i) {
        a = a + b * i;      /* Creates phi nodes in SSA */
        b = b + a;
        if (i % 2 == 0) {
            a = a - 1;
        } else {
            b = b + 2;
        }
    }
    
    /* More complex control flow */
    int c = 0;
    while (c < 100) {
        c = c + a + b;
        if (c > 50) {
            a = a * 2;
        }
    }
    
    return a + b + c;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
void block_example() {
    int outer = 0;
    
    { /* Block 1 */
        int inner1 = 10;
        outer += inner1;
        
        { /* Block 2 */
            int inner2 = 20;
            outer += inner2;
            
            { /* Block 3 */
                int inner3 = 30;
                outer += inner3;
            }
        }
    }
    
    /* Another block with different scope */
    {
        int temp = 100;
        for (int i = 0; i < 10; i++) {
            int loop_var = i * temp;
            outer += loop_var;
        }
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct Point {
    int x;
    int y;
    int z;
};

struct Data {
    int id;
    struct Point location;
    float values[4];
};

void constructor_example() {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor with designated initializers */
    struct Point p1 = {.x = 10, .y = 20, .z = 30};
    struct Point p2 = {10, 20, 30};
    
    /* Nested struct constructor */
    struct Data d1 = {
        .id = 1,
        .location = {.x = 100, .y = 200, .z = 300},
        .values = {1.1f, 2.2f, 3.3f, 4.4f}
    };
    
    /* Complex array constructor */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_example(int n) {
    int i;
    int sum = 0;
    int data[100];
    
    /* Initialize data */
    for (i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* Various OpenMP constructs with different clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static)
    for (i = 0; i < n; i++) {
        sum += data[i];
    }
    
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (i = 0; i < n; i++) {
            data[i] *= 2;
        }
        
        #pragma omp single
        {
            sum = 0;
        }
    }
    
    /* Parallel sections with shared variable */
    #pragma omp parallel sections shared(sum)
    {
        #pragma omp section
        {
            for (i = 0; i < n/2; i++) {
                sum += data[i];
            }
        }
        
        #pragma omp section
        {
            for (i = n/2; i < n; i++) {
                sum += data[i];
            }
        }
    }
}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchies create TREE_BINFO nodes */

/* Base class with virtual functions */
class BaseClass {
public:
    virtual void base_method() {
        /* Empty implementation */
    }
    virtual ~BaseClass() {}
    int base_data;
};

/* Intermediate class */
class IntermediateClass : virtual public BaseClass {
public:
    virtual void intermediate_method() {
        /* Empty implementation */
    }
    int intermediate_data;
};

/* Derived class with multiple inheritance */
class DerivedClass : public BaseClass, public IntermediateClass {
public:
    virtual void base_method() override {
        base_data = 42;
    }
    
    virtual void intermediate_method() override {
        intermediate_data = 24;
    }
    
    void derived_method() {
        base_method();
        intermediate_method();
    }
    
    int derived_data;
};

/* Template class for additional complexity */
template<typename T>
class TemplateClass : public BaseClass {
public:
    T template_data;
    
    virtual void base_method() override {
        template_data = T();
    }
};

void cpp_binfo_example() {
    DerivedClass derived;
    derived.derived_method();
    
    BaseClass* base_ptr = &derived;
    base_ptr->base_method();
    
    TemplateClass<int> template_obj;
    template_obj.base_method();
    
    /* Multiple objects with different types */
    IntermediateClass* inter_ptr = &derived;
    inter_ptr->intermediate_method();
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main() {
    /* Call all examples to ensure they're processed */
    identifier_function();
    
    #ifdef __GNUC__
    int vec_result = tree_vec_example();
    #else
    int vec_result = 0;
    #endif
    
    int ssa_result = ssa_name_example(50);
    block_example();
    constructor_example();
    
    #ifdef _OPENMP
    omp_example(100);
    #endif
    
    #ifdef __cplusplus
    cpp_binfo_example();
    #endif
    
    /* Compute and print something to avoid dead code elimination */
    int final_result = vec_result + ssa_result;
    
    #ifdef __cplusplus
    cout << "Test completed. Result: " << final_result << endl;
    #else
    printf("Test completed. Result: %d\n", final_result);
    #endif
    
    return 0;
}
