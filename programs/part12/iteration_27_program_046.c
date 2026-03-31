/* test_tree_kind.c - Coverage test for GCC's get_kind function */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_tree_kind */
/* For C++ mode: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind.c -o test_tree_kind_cpp */

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
#include <vector>
#endif

/* ===== IDENTIFIER_NODE ===== */
/* Variable and function names create IDENTIFIER_NODE */
int some_unique_identifier_1;
double another_identifier_for_coverage;
static const char* identifier_string = "test";

void use_identifiers(void) {
    some_unique_identifier_1 = 42;
    another_identifier_for_coverage = 3.14;
}

/* ===== SSA_NAME ===== */
/* Loop and arithmetic operations create SSA_NAME nodes */
int create_ssa_names(int n) {
    int a = 0;
    int b = 1;
    int c;
    
    /* Complex enough to trigger SSA optimization passes */
    for (int i = 0; i < n; ++i) {
        a = a + i;
        b = b * 2;
        if (i % 2 == 0) {
            c = a + b;
        } else {
            c = a - b;
        }
        a = c * 3;
    }
    
    /* Nested loop for more SSA complexity */
    for (int j = 0; j < 10; ++j) {
        for (int k = 0; k < 5; ++k) {
            a += j * k;
        }
    }
    
    return a + b;
}

/* ===== BLOCK ===== */
/* Nested blocks with local variables */
void nested_blocks(void) {
    int outer = 10;
    
    {
        /* Inner block 1 */
        int inner1 = outer * 2;
        {
            /* Inner block 2 */
            int inner2 = inner1 + 5;
            {
                /* Inner block 3 */
                volatile int inner3 = inner2 * 3;
                (void)inner3; /* Use variable to avoid warnings */
            }
        }
    }
    
    /* Switch with blocks */
    switch (outer) {
        case 10: {
            int case_var = 100;
            outer += case_var;
            break;
        }
        default: {
            int default_var = 200;
            outer += default_var;
        }
    }
}

/* ===== CONSTRUCTOR ===== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct test_struct {
    int a;
    double b;
    char c;
};

union test_union {
    int x;
    float y;
};

void use_constructors(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor */
    struct test_struct s1 = {.a = 10, .b = 20.5, .c = 'X'};
    struct test_struct s2 = {10, 20.5, 'Y'};
    
    /* Union constructor */
    union test_union u1 = {.x = 42};
    union test_union u2 = {100};
    
    /* Nested constructor */
    struct nested {
        int values[3];
        struct test_struct inner;
    } nested_obj = {
        .values = {9, 8, 7},
        .inner = {5, 6.7, 'Z'}
    };
    
    /* Prevent optimization */
    volatile int* p = arr;
    (void)p;
    (void)s1;
    (void)s2;
    (void)u1;
    (void)u2;
    (void)nested_obj;
}

/* ===== OMP_CLAUSE ===== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void openmp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var)
    for (i = 0; i < n; i++) {
        sum += i * 2;
        private_var = i;
    }
    
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            { sum += 1; }
            
            #pragma omp section
            { sum += 2; }
        }
    }
    
    printf("OpenMP sum: %d\n", sum);
}
#else
void openmp_test(int n) {
    printf("OpenMP not enabled\n");
    (void)n;
}
#endif

/* ===== TREE_VEC ===== */
/* GCC extensions for TREE_VEC */
#ifdef __GNUC__
void tree_vec_examples(void) {
    /* Using statement expressions - creates TREE_VEC */
    int a = ({ 
        int x = 5; 
        int y = 10; 
        x + y; 
    });
    
    /* Another statement expression */
    int b = ({
        volatile int tmp = 20;
        for (int i = 0; i < 3; i++) {
            tmp += i;
        }
        tmp;
    });
    
    /* Using typeof extension */
    typeof(a) c = a + b;
    
    (void)c;
}
#else
void tree_vec_examples(void) {
    /* Fallback for non-GCC compilers */
    printf("GCC extensions not available\n");
}
#endif

#ifdef __cplusplus
/* ===== TREE_BINFO ===== (C++ only) */
/* Class hierarchies create TREE_BINFO nodes */

class BaseClass1 {
public:
    virtual ~BaseClass1() {}
    virtual void method1() { printf("Base1\n"); }
    int base_data1;
};

class BaseClass2 {
public:
    virtual ~BaseClass2() {}
    virtual void method2() { printf("Base2\n"); }
    double base_data2;
};

/* Multiple inheritance for more complex binfo */
class DerivedClass : public BaseClass1, public BaseClass2 {
public:
    virtual ~DerivedClass() {}
    virtual void method1() override { printf("Derived1\n"); }
    virtual void method2() override { printf("Derived2\n"); }
    void derived_method() { printf("Derived specific\n"); }
    char derived_data;
};

/* Template class for additional complexity */
template<typename T>
class TemplateBase {
public:
    virtual T get_value() = 0;
    virtual ~TemplateBase() {}
};

class ConcreteClass : public TemplateBase<int> {
public:
    virtual int get_value() override { return 42; }
};

void use_cpp_classes(void) {
    DerivedClass obj;
    BaseClass1* ptr1 = &obj;
    BaseClass2* ptr2 = &obj;
    
    ptr1->method1();
    ptr2->method2();
    
    ConcreteClass concrete;
    int val = concrete.get_value();
    
    printf("C++ value: %d\n", val);
    
    /* Use STL vector which may create additional tree nodes */
    std::vector<int> vec = {1, 2, 3, 4, 5};
    vec.push_back(6);
}
#endif

/* ===== MAIN FUNCTION ===== */
/* Orchestrates all test cases */
int main(int argc, char** argv) {
    int n = 100;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 100;
    }
    
    /* Trigger all cases */
    use_identifiers();
    
    int ssa_result = create_ssa_names(n);
    printf("SSA result: %d\n", ssa_result);
    
    nested_blocks();
    
    use_constructors();
    
    openmp_test(n);
    
    tree_vec_examples();
    
#ifdef __cplusplus
    use_cpp_classes();
#endif
    
    /* Complex final computation using all results */
    volatile int final_result = ssa_result;
    for (int i = 0; i < 10; i++) {
        final_result = (final_result * 3 + i) % 1000;
    }
    
    printf("Final result: %d\n", final_result);
    
    return 0;
}
