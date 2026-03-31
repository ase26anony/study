/* test_tree_kind.c - Comprehensive test for GCC tree node kinds */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* Helper to avoid unused variable warnings */
#define USE(V) ((void)(V))

/* ========== IDENTIFIER_NODE ========== */
/* Variable and function names create IDENTIFIER_NODE */
int global_identifier_test = 42;
void function_identifier_test(void) {
    int local_identifier = global_identifier_test;
    USE(local_identifier);
}

/* ========== TREE_VEC ========== */
/* GCC statement expressions with multiple elements can create TREE_VEC */
#ifdef __GNUC__
int tree_vec_test(void) {
    /* Using statement expression with multiple elements */
    int result = ({ 
        int a = 1, b = 2, c = 3;
        a + b + c; 
    });
    return result;
}
#endif

/* ========== SSA_NAME ========== */
/* Complex enough code to trigger SSA formation */
int ssa_name_test(int n) {
    int a = 0, b = 1, c;
    
    /* Loop with arithmetic to force SSA */
    for (int i = 0; i < n; ++i) {
        a = a + i;
        b = b * 2;
        c = a + b;
        
        /* Conditional to create phi nodes */
        if (c > 100) {
            a = c / 2;
        } else {
            a = c * 2;
        }
    }
    
    /* Another loop with induction variable */
    int sum = 0;
    for (int j = 0; j < n; ++j) {
        sum += j * j;
        if (sum > 1000) break;
    }
    
    return a + b + c + sum;
}

/* ========== BLOCK ========== */
/* Nested blocks with local variables */
int block_test(int x) {
    /* Outer block */
    int outer = x * 2;
    
    {
        /* Inner block 1 */
        int inner1 = outer + 10;
        
        {
            /* Inner block 2 */
            int inner2 = inner1 * 3;
            outer = inner2;
        }
        
        {
            /* Another inner block */
            int temp = 0;
            for (int i = 0; i < 5; i++) {
                temp += i;
            }
            outer += temp;
        }
    }
    
    return outer;
}

/* ========== CONSTRUCTOR ========== */
/* Aggregate initializers */
struct my_struct {
    int a;
    float b;
    char c;
};

int constructor_test(void) {
    /* Array constructor */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Struct constructor (designated initializer) */
    struct my_struct s1 = {.a = 1, .b = 2.5f, .c = 'x'};
    
    /* Nested struct constructor */
    struct nested {
        struct my_struct inner;
        int extra;
    } n1 = {{2, 3.14f, 'y'}, 100};
    
    /* Complex array constructor with mixed expressions */
    int arr2[3] = {arr[0] + s1.a, block_test(5), ssa_name_test(3)};
    
    return arr[0] + s1.a + n1.extra + arr2[1];
}

/* ========== OMP_CLAUSE ========== */
#ifdef _OPENMP
void omp_clause_test(int size) {
    int i;
    int sum = 0;
    int private_var = 0;
    
    /* Multiple OpenMP pragmas with different clauses */
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var)
    for (i = 0; i < size; i++) {
        sum += i * i;
    }
    
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            int section_var = 1;
            USE(section_var);
        }
        #pragma omp section
        {
            int section_var = 2;
            USE(section_var);
        }
    }
    
    USE(sum);
}
#endif

/* ========== C++ Specific: TREE_BINFO ========== */
#ifdef __cplusplus
/* Class hierarchy to generate BINFO nodes */
class BaseClass1 {
public:
    virtual void method1() { cout << "Base1" << endl; }
    int base_data1;
};

class BaseClass2 {
public:
    virtual void method2() { cout << "Base2" << endl; }
    float base_data2;
};

/* Multiple inheritance to ensure BINFO creation */
class DerivedClass : public BaseClass1, public BaseClass2 {
public:
    virtual void method1() override { cout << "Derived1" << endl; }
    virtual void method2() override { cout << "Derived2" << endl; }
    
    /* Virtual base class */
    class VirtualBase {
    public:
        virtual ~VirtualBase() {}
        int virtual_data;
    };
    
    class DeepDerived : public virtual VirtualBase {
    public:
        int deep_data;
    };
    
    void use_binfo() {
        BaseClass1* b1 = this;
        BaseClass2* b2 = this;
        b1->method1();
        b2->method2();
        
        DeepDerived dd;
        dd.virtual_data = 42;
        dd.deep_data = 100;
    }
    
    int derived_data;
};

/* Template class with inheritance */
template<typename T>
class TemplateBase {
public:
    virtual T process(T x) { return x * 2; }
};

class ConcreteDerived : public TemplateBase<int> {
public:
    virtual int process(int x) override { return x * 3; }
};

void cpp_binfo_test() {
    DerivedClass obj;
    obj.use_binfo();
    
    ConcreteDerived cd;
    int result = cd.process(10);
    cout << "Template result: " << result << endl;
    
    /* Dynamic cast to exercise BINFO lookups */
    BaseClass1* ptr = &obj;
    DerivedClass* derived = dynamic_cast<DerivedClass*>(ptr);
    if (derived) {
        derived->derived_data = 999;
    }
}
#endif

/* ========== Main Driver ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Exercise all test functions */
    function_identifier_test();
    
    #ifdef __GNUC__
    result += tree_vec_test();
    #endif
    
    result += ssa_name_test(10);
    result += block_test(7);
    result += constructor_test();
    
    #ifdef _OPENMP
    omp_clause_test(100);
    #endif
    
    #ifdef __cplusplus
    cpp_binfo_test();
    cout << "C++ mode active" << endl;
    #else
    printf("C mode active\n");
    #endif
    
    printf("Final result: %d\n", result);
    
    /* Use command line arguments to prevent optimization removal */
    if (argc > 1) {
        return result % 256;
    }
    return 0;
}
