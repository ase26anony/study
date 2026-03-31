/* test_tree_kind_coverage.c - Comprehensive test to trigger all tree_kind cases */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 -x c++ test_tree_kind_coverage.c -o test_program */
/* Or separately: g++ -O2 -fopenmp test_tree_kind_coverage.c -o test_cpp_program */

#ifdef __cplusplus
#include <iostream>
using namespace std;

/* TREE_BINFO generation - requires C++ class hierarchy */
class BaseClass {
public:
    virtual void base_method() { }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void base_method() override { }
    int derived_data;
};

void test_binfo() {
    DerivedClass obj;
    BaseClass* ptr = &obj;  /* This should generate BINFO nodes */
    ptr->base_method();
}
#else
/* C version placeholder */
void test_binfo(void) { }
#endif

/* IDENTIFIER_NODE generation - variable and function names */
int some_unique_identifier_123;
void unique_function_name_456(void) { }

void test_identifier(void) {
    /* Using identifiers */
    some_unique_identifier_123 = 42;
    unique_function_name_456();
}

/* TREE_VEC generation - using GCC statement expressions */
#ifdef __GNUC__
void test_tree_vec(void) {
    /* Statement expression that creates a TREE_VEC */
    int result = ({
        int a = 5;
        int b = 10;
        int c = 15;
        a + b + c;
    });
    
    /* Another approach: using vector types (GCC extension) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
}
#else
void test_tree_vec(void) { }
#endif

/* SSA_NAME generation - complex enough for SSA optimization */
int test_ssa_name(int n) {
    int a = 0, b = 1, c = 2;
    
    /* Complex loop to generate SSA names */
    for (int i = 0; i < n; ++i) {
        a = a + i * 2;
        b = b + a / 3;
        c = c + b % 5;
        
        if (i % 2 == 0) {
            a = a - 1;
            b = b + 2;
        } else {
            a = a + 3;
            c = c - 1;
        }
        
        /* Nested loop for more complexity */
        for (int j = 0; j < 5; ++j) {
            c = c + j;
            if (j % 3 == 0) {
                a = a + j;
            }
        }
    }
    
    return a + b + c;
}

/* BLOCK generation - nested compound statements */
void test_block(void) {
    int outer = 0;
    
    {
        /* First inner block */
        int inner1 = 10;
        outer += inner1;
        
        {
            /* Second nested block */
            int inner2 = 20;
            outer += inner2;
            
            {
                /* Third nested block */
                int inner3 = 30;
                outer += inner3;
                
                if (outer > 50) {
                    /* Conditional block */
                    int conditional = 40;
                    outer += conditional;
                }
            }
        }
    }
    
    /* Switch with blocks */
    switch (outer) {
        case 100: {
            int case_var = 1000;
            outer += case_var;
            break;
        }
        default: {
            int default_var = 2000;
            outer += default_var;
        }
    }
}

/* CONSTRUCTOR generation - aggregate initializers */
struct TestStruct {
    int a;
    double b;
    char c[10];
};

void test_constructor(void) {
    /* Array constructor */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Struct constructor */
    struct TestStruct s1 = {.a = 100, .b = 3.14, .c = "hello"};
    
    /* Nested struct constructor */
    struct Nested {
        struct TestStruct inner;
        int extra;
    };
    
    struct Nested n1 = {{200, 6.28, "world"}, 999};
    
    /* Multi-dimensional array constructor */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
    /* Union constructor */
    union TestUnion {
        int x;
        float y;
    } u1 = {.y = 2.718};
}

/* OMP_CLAUSE generation - OpenMP pragmas */
#ifdef _OPENMP
void test_omp_clause(void) {
    int i, n = 100;
    int sum = 0;
    int private_var = 0;
    
    /* Multiple OpenMP clauses to generate OMP_CLAUSE nodes */
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var) \
                schedule(static, 10) num_threads(4) if(n > 50)
    for (i = 0; i < n; i++) {
        sum += i;
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel private(i) shared(n, sum) default(none)
    {
        #pragma omp for nowait
        for (i = 0; i < n; i++) {
            #pragma omp atomic
            sum++;
        }
    }
    
    /* OMP sections with clauses */
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
}
#else
void test_omp_clause(void) {
    /* Fallback without OpenMP */
    printf("OpenMP not enabled\n");
}
#endif

/* Main driver that references all test functions */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Call all test functions to ensure compiler processes them */
    test_identifier();
    test_tree_vec();
    test_binfo();
    
    result = test_ssa_name(100);
    
    test_block();
    test_constructor();
    test_omp_clause();
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", result + some_unique_identifier_123);
    
    return 0;
}
