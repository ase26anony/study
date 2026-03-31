/* test_tree_kind_coverage.c - Comprehensive test for GCC tree kind coverage */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int global_identifier_coverage;
static int static_identifier_coverage;

void function_with_identifiers(void) {
    int local_identifier = 42;
    global_identifier_coverage = local_identifier;
}

/* ==================== TREE_VEC ==================== */
/* Using GCC statement expressions to create TREE_VEC nodes */
#ifdef __GNUC__
#define CREATE_VEC(type, a, b, c) ({ \
    type __vec[] = {a, b, c}; \
    __vec[0] + __vec[1] + __vec[2]; \
})
#endif

int use_tree_vec(void) {
#ifdef __GNUC__
    return CREATE_VEC(int, 1, 2, 3);
#else
    return 6;
#endif
}

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic to force SSA form */
int ssa_name_generator(int n) {
    int a = 0, b = 1, c;
    
    /* Loop with multiple assignments to force SSA */
    for (int i = 0; i < n; ++i) {
        c = a + b;
        a = b;
        b = c;
        
        /* Additional complexity for SSA */
        if (c % 2 == 0) {
            a = a * 2;
        } else {
            b = b / 2;
        }
    }
    
    /* More SSA opportunities */
    int x = a;
    int y = b;
    int z = x * y + a - b;
    
    return z;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
int block_coverage(int val) {
    int result = 0;
    
    /* Outer block */
    {
        int temp = val * 2;
        
        /* Inner block 1 */
        {
            int inner1 = temp + 10;
            result += inner1;
        }
        
        /* Inner block 2 */
        {
            int inner2 = temp - 5;
            result *= inner2;
            
            /* Deeply nested block */
            {
                int deep = result % 100;
                result = deep;
            }
        }
    }
    
    /* Another independent block */
    {
        int final_adjust = result + 1000;
        result = final_adjust;
    }
    
    return result;
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers for arrays and structs */
struct constructor_test {
    int a;
    float b;
    char c;
};

int constructor_coverage(void) {
    /* Array constructor */
    int array_constructor[5] = {10, 20, 30, 40, 50};
    
    /* Struct constructor - designated initializer */
    struct constructor_test s1 = {.a = 1, .b = 2.0f, .c = 'x'};
    
    /* Nested struct constructor */
    struct nested {
        struct constructor_test inner;
        int extra;
    } n1 = {{2, 3.0f, 'y'}, 100};
    
    /* Complex array constructor with expressions */
    int computed_array[3] = {array_constructor[0] + s1.a, 
                             n1.inner.a * 2, 
                             block_coverage(5)};
    
    return array_constructor[0] + s1.a + n1.extra + computed_array[1];
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas generate OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>

int omp_clause_coverage(int size) {
    int sum = 0;
    int i;
    
    /* Multiple OpenMP pragmas with different clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static)
    for (i = 0; i < size; i++) {
        sum += i;
    }
    
    /* Another OpenMP region with different clauses */
    #pragma omp parallel num_threads(4) shared(sum)
    {
        #pragma omp for nowait
        for (i = 0; i < 100; i++) {
            #pragma omp atomic
            sum++;
        }
    }
    
    /* OpenMP sections */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < 50; i++) {
                #pragma omp critical
                sum += 2;
            }
        }
        
        #pragma omp section
        {
            for (i = 0; i < 50; i++) {
                #pragma omp atomic
                sum -= 1;
            }
        }
    }
    
    return sum;
}
#else
int omp_clause_coverage(int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += i;
    }
    return sum;
}
#endif

#ifdef __cplusplus
/* ==================== TREE_BINFO (C++ only) ==================== */
/* Class hierarchy to generate BINFO nodes */
class BaseClass1 {
public:
    virtual ~BaseClass1() {}
    virtual void method1() = 0;
    int base_data1;
};

class BaseClass2 {
public:
    virtual ~BaseClass2() {}
    virtual void method2() = 0;
    float base_data2;
};

/* Multiple inheritance to ensure BINFO creation */
class DerivedClass : public BaseClass1, public BaseClass2 {
public:
    virtual void method1() override {
        base_data1 = 100;
    }
    
    virtual void method2() override {
        base_data2 = 200.0f;
    }
    
    void derived_method() {
        method1();
        method2();
    }
    
private:
    char derived_data;
};

/* Template class for additional complexity */
template<typename T>
class TemplateClass : public BaseClass1 {
public:
    T template_data;
    
    virtual void method1() override {
        base_data1 = sizeof(T);
    }
};

int cpp_binfo_coverage(void) {
    DerivedClass obj;
    obj.derived_method();
    
    TemplateClass<int> tpl_obj;
    tpl_obj.method1();
    
    BaseClass1* ptr1 = &obj;
    BaseClass2* ptr2 = &obj;
    
    ptr1->method1();
    ptr2->method2();
    
    return obj.base_data1 + (int)obj.base_data2;
}
#endif

/* ==================== MAIN FUNCTION ==================== */
/* Orchestrates all test cases */
int main(void) {
    int result = 0;
    
    /* Trigger IDENTIFIER_NODE creation */
    function_with_identifiers();
    result += global_identifier_coverage;
    
    /* Trigger TREE_VEC creation */
    result += use_tree_vec();
    
    /* Trigger SSA_NAME creation */
    result += ssa_name_generator(10);
    
    /* Trigger BLOCK creation */
    result += block_coverage(20);
    
    /* Trigger CONSTRUCTOR creation */
    result += constructor_coverage();
    
    /* Trigger OMP_CLAUSE creation */
    result += omp_clause_coverage(100);
    
#ifdef __cplusplus
    /* Trigger TREE_BINFO creation (C++ only) */
    result += cpp_binfo_coverage();
#endif
    
#ifdef __cplusplus
    cout << "Final result: " << result << endl;
#else
    printf("Final result: %d\n", result);
#endif
    
    return 0;
}
