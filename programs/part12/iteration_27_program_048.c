/* test_tree_kind.c - Comprehensive test for GCC tree node kinds */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int unique_identifier_1;
void identifier_function() {
    int local_identifier = 42;
    unique_identifier_1 = local_identifier;
}

/* ==================== TREE_VEC ==================== */
/* GCC statement expressions with multiple elements create TREE_VEC */
#ifdef __GNUC__
int tree_vec_example() {
    /* Using GCC statement expression extension */
    int result = ({ 
        int a = 5; 
        int b = 10; 
        int c = 15; 
        a + b + c; 
    });
    return result;
}
#endif

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic with loops creates SSA_NAME nodes */
int ssa_name_example(int n) {
    int sum = 0;
    int i, j;
    
    /* Multiple operations to force SSA form */
    for (i = 0; i < n; ++i) {
        for (j = 0; j < i; ++j) {
            sum = sum + (i * j) - (i / (j + 1));
            sum = sum ^ (i & j);
        }
        sum = sum * 2 + 1;
    }
    
    /* Conditional to create phi nodes */
    int result = (sum > 1000) ? sum : sum * 2;
    return result;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
void block_example() {
    int outer = 10;
    {
        int inner1 = outer + 5;
        {
            int inner2 = inner1 * 2;
            {
                int inner3 = inner2 / 3;
                outer = inner3;
            }
        }
    }
    
    /* Another deeply nested block */
    {
        int a = 1;
        {
            int b = a + 1;
            {
                int c = b + 1;
                outer = c;
            }
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
    int array[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor */
    struct Point p1 = {10, 20, 30};
    
    /* Nested struct with designated initializers */
    struct Data d1 = {
        .id = 100,
        .location = {.x = 1, .y = 2, .z = 3},
        .values = {1.1f, 2.2f, 3.3f, 4.4f}
    };
    
    /* 2D array constructor */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_example(int size) {
    int i;
    int sum = 0;
    int array[1000];
    
    /* Initialize array */
    for (i = 0; i < 1000; i++) {
        array[i] = i;
    }
    
    /* Multiple OpenMP pragmas with different clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static, 10)
    for (i = 0; i < 1000; i++) {
        sum += array[i];
    }
    
    /* Another with different clauses */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < 500; i++) {
                array[i] *= 2;
            }
        }
        #pragma omp section
        {
            for (i = 500; i < 1000; i++) {
                array[i] /= 2;
            }
        }
    }
    
    /* Parallel region with shared and firstprivate */
    int shared_var = 0;
    int firstprivate_var = 42;
    #pragma omp parallel shared(shared_var) firstprivate(firstprivate_var)
    {
        #pragma omp atomic
        shared_var++;
    }
}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchies create TREE_BINFO nodes */

class BaseClass1 {
public:
    virtual void method1() {
        int x = 0;
    }
    int data1;
};

class BaseClass2 {
public:
    virtual void method2() {
        int y = 0;
    }
    virtual ~BaseClass2() {}
    int data2;
};

class DerivedClass : public BaseClass1, public BaseClass2 {
public:
    virtual void method1() override {
        data1 = 10;
    }
    virtual void method2() override {
        data2 = 20;
    }
    virtual void method3() {
        int z = data1 + data2;
    }
    int extra_data;
};

/* Template class for additional complexity */
template<typename T>
class TemplateBase {
public:
    virtual T process(T value) {
        return value * 2;
    }
};

class ConcreteDerived : public TemplateBase<int> {
public:
    virtual int process(int value) override {
        return value + 100;
    }
};

void cpp_binfo_example() {
    DerivedClass obj;
    BaseClass1* ptr1 = &obj;
    BaseClass2* ptr2 = &obj;
    
    ptr1->method1();
    ptr2->method2();
    
    ConcreteDerived template_obj;
    int result = template_obj.process(50);
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main() {
    /* Reference all examples to ensure they're compiled */
    identifier_function();
    
    #ifdef __GNUC__
    int vec_result = tree_vec_example();
    #endif
    
    int ssa_result = ssa_name_example(100);
    block_example();
    constructor_example();
    
    #ifdef _OPENMP
    omp_example(1000);
    #endif
    
    #ifdef __cplusplus
    cpp_binfo_example();
    #endif
    
    /* Compute and print something to avoid dead code elimination */
    int final_result = ssa_result;
    
    #ifdef __cplusplus
    cout << "Test completed. Result: " << final_result << endl;
    #else
    printf("Test completed. Result: %d\n", final_result);
    #endif
    
    return 0;
}
