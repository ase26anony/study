/* test_tree_kind.c - Comprehensive test for GCC tree kind coverage */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int some_unique_identifier;
static int another_identifier;
void identifier_function(void) {
    int local_identifier = some_unique_identifier + another_identifier;
    (void)local_identifier;
}

/* ==================== TREE_VEC ==================== */
/* Using GCC statement expressions to create TREE_VEC */
#ifdef __GNUC__
#define CREATE_VEC() ({ \
    int a = 1, b = 2, c = 3; \
    (typeof(a))((a + b) * c); \
})
#else
#define CREATE_VEC() (0)
#endif

void use_tree_vec(void) {
    int result = CREATE_VEC();
    (void)result;
}

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic to force SSA form */
int ssa_test(int n) {
    int a = 0, b = 1, c = 2;
    
    /* Loop with multiple assignments to create SSA names */
    for (int i = 0; i < n; ++i) {
        a = a + i;
        b = b * a;
        c = c - b;
        
        /* Conditional to create phi nodes */
        if (i % 2 == 0) {
            a = b + c;
        } else {
            a = c - b;
        }
    }
    
    /* Multiple uses of variables */
    return a + b + c;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
void block_test(void) {
    int outer = 0;
    
    {
        /* Inner block 1 */
        int inner1 = 10;
        outer += inner1;
        
        {
            /* Inner block 2 */
            int inner2 = 20;
            outer += inner2;
            
            {
                /* Inner block 3 */
                int inner3 = 30;
                outer += inner3;
            }
        }
    }
    
    {
        /* Another block */
        int x = 5, y = 10, z = 15;
        outer += x + y + z;
    }
    
    (void)outer;
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers */
struct my_struct {
    int a;
    float b;
    char c;
};

int constructor_test(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor */
    struct my_struct s1 = {10, 3.14f, 'X'};
    
    /* Designated initializer */
    struct my_struct s2 = {.a = 20, .b = 2.71f, .c = 'Y'};
    
    /* Nested initializer */
    struct nested {
        int x;
        struct my_struct inner;
    } n = {100, {200, 4.5f, 'Z'}};
    
    return arr[0] + s1.a + s2.a + n.x;
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas */
#ifdef _OPENMP
void omp_test(int size) {
    int i;
    int sum = 0;
    int data[100];
    
    /* Initialize data */
    for (i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static)
    for (i = 0; i < size && i < 100; i++) {
        sum += data[i];
    }
    
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < 10; i++) sum += i;
        }
        
        #pragma omp section
        {
            for (i = 10; i < 20; i++) sum -= i;
        }
    }
    
    (void)sum;
}
#else
void omp_test(int size) {
    (void)size;
}
#endif

/* ==================== C++ Specific: TREE_BINFO ==================== */
#ifdef __cplusplus

/* Base class for BINFO generation */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void base_method() = 0;
    int base_data;
};

/* Another base class */
class AnotherBase {
public:
    virtual ~AnotherBase() {}
    virtual void another_method() {}
    float another_data;
};

/* Derived class with multiple inheritance to create BINFO */
class DerivedClass : public BaseClass, public AnotherBase {
public:
    void base_method() override {
        base_data = 42;
    }
    
    void another_method() override {
        another_data = 3.14f;
    }
    
    void derived_method() {
        base_method();
        another_method();
    }
    
    int derived_data;
};

/* Template class for additional complexity */
template<typename T>
class TemplateClass : public BaseClass {
public:
    void base_method() override {
        base_data = sizeof(T);
    }
    
    T template_data;
};

void cpp_binfo_test(void) {
    DerivedClass derived;
    derived.base_method();
    derived.another_method();
    derived.derived_method();
    
    TemplateClass<int> templated;
    templated.base_method();
    
    BaseClass* base_ptr = &derived;
    base_ptr->base_method();
    
    AnotherBase* another_ptr = &derived;
    another_ptr->another_method();
}

#endif /* __cplusplus */

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    /* Reference all identifiers */
    some_unique_identifier = 1;
    another_identifier = 2;
    identifier_function();
    
    /* Use TREE_VEC construct */
    use_tree_vec();
    
    /* Test SSA generation */
    int ssa_result = ssa_test(50);
    
    /* Test blocks */
    block_test();
    
    /* Test constructors */
    int constr_result = constructor_test();
    
    /* Test OpenMP */
    omp_test(50);
    
    #ifdef __cplusplus
    /* Test C++ BINFO */
    cpp_binfo_test();
    #endif
    
    /* Combine results */
    int total = ssa_result + constr_result;
    
    #ifdef __cplusplus
    cout << "Total: " << total << endl;
    #else
    printf("Total: %d\n", total);
    #endif
    
    return 0;
}
