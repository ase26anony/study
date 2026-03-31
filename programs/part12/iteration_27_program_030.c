/* test_tree_kind_coverage.c - Comprehensive test for GCC tree kind coverage */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 -x c++ test_tree_kind_coverage.c -o test_program */
/* Or separately: gcc -O2 -fopenmp -std=gnu99 test.c -o test_c_program */
/*                g++ -O1 -fno-rtti -fopenmp test.cpp -o test_cpp_program */

#ifdef __cplusplus
#include <iostream>
using namespace std;

/* TREE_BINFO: C++ class hierarchy for base information nodes */
class BaseClass {
public:
    virtual void base_method() { /* virtual method for vtable */ }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void base_method() override { /* override */ }
    int derived_data;
};

void test_binfo() {
    DerivedClass obj;
    BaseClass* ptr = &obj;  /* This should generate TREE_BINFO nodes */
    ptr->base_method();
}
#else
/* C version placeholder */
void test_binfo(void) {
    /* Empty for C compilation */
}
#endif

/* IDENTIFIER_NODE: Various identifiers */
int some_unique_identifier;
double another_identifier_123;
void function_identifier(void);

/* SSA_NAME: Complex function to force SSA form */
int ssa_test_function(int n) {
    int a = 0, b = 1, c;
    
    /* Loop with operations that create SSA names */
    for (int i = 0; i < n; ++i) {
        a = a + i;          /* Creates SSA_NAME for 'a' */
        b = b * 2;          /* Creates SSA_NAME for 'b' */
        c = a + b;          /* Creates SSA_NAME for 'c' */
        
        /* Conditional to create phi nodes */
        if (c > 100) {
            a = c / 2;
        } else {
            a = c * 2;
        }
    }
    
    /* More complex control flow */
    int result = 0;
    for (int j = 0; j < a; ++j) {
        result += (j % 2 == 0) ? j : -j;
    }
    
    return result + b + c;
}

/* BLOCK: Nested blocks with local variables */
void block_test(void) {
    int outer = 10;
    
    {  /* BLOCK 1 */
        int inner1 = outer * 2;
        
        {  /* BLOCK 2 */
            int inner2 = inner1 + 5;
            
            {  /* BLOCK 3 - deeply nested */
                int inner3 = inner2 * 3;
                outer = inner3;
            }
        }
    }
    
    /* Another block with different scope */
    {
        int temp = 0;
        for (int i = 0; i < 5; ++i) {
            int loop_var = i * 2;  /* BLOCK inside loop */
            temp += loop_var;
        }
        outer += temp;
    }
}

/* CONSTRUCTOR: Aggregate initializers */
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

void constructor_test(void) {
    /* Array constructor */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Struct constructor */
    struct Point p1 = {.x = 1, .y = 2, .z = 3};
    struct Point p2 = {5, 6, 7};
    
    /* Nested struct constructor */
    struct Data d1 = {
        .id = 100,
        .location = {.x = 10, .y = 20, .z = 30},
        .values = {1.1f, 2.2f, 3.3f, 4.4f}
    };
    
    /* Complex array constructor with designators */
    int matrix[2][3] = {
        [0] = {1, 2, 3},
        [1] = {4, 5, 6}
    };
}

/* TREE_VEC: Using GCC statement expressions (GNU extension) */
#ifdef __GNUC__
#define VEC_OPERATION(a, b, c) ({ \
    typeof(a) _a = (a); \
    typeof(b) _b = (b); \
    typeof(c) _c = (c); \
    (_a + _b) * _c; \
})

void tree_vec_test(void) {
    /* Using statement expression - should generate TREE_VEC */
    int result = VEC_OPERATION(5, 3, 2);
    
    /* Another complex statement expression */
    int x = ({ 
        int temp = 10; 
        for (int i = 0; i < 5; i++) temp += i; 
        temp; 
    });
}
#else
void tree_vec_test(void) {
    /* Fallback without GNU extensions */
    int result = (5 + 3) * 2;
}
#endif

/* OMP_CLAUSE: OpenMP pragmas */
#ifdef _OPENMP
void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 42;
    
    /* Multiple OpenMP pragmas with different clauses */
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var)
    for (i = 0; i < n; i++) {
        sum += i + private_var;
    }
    
    /* Another OpenMP region with different clauses */
    #pragma omp parallel
    {
        #pragma omp sections private(i)
        {
            #pragma omp section
            {
                for (i = 0; i < 10; i++) {
                    sum += i;
                }
            }
            #pragma omp section
            {
                for (i = 10; i < 20; i++) {
                    sum += i;
                }
            }
        }
    }
    
    /* OMP critical with clause */
    #pragma omp parallel
    {
        #pragma omp critical (update_lock)
        {
            sum += 1;
        }
    }
}
#else
void omp_test(int n) {
    /* Fallback without OpenMP */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += i;
    }
}
#endif

/* Main function that exercises all test cases */
int main(int argc, char *argv[]) {
    /* Force usage of identifier */
    some_unique_identifier = 42;
    another_identifier_123 = 3.14;
    
    /* Test SSA_NAME generation */
    int ssa_result = ssa_test_function(100);
    
    /* Test BLOCK generation */
    block_test();
    
    /* Test CONSTRUCTOR generation */
    constructor_test();
    
    /* Test TREE_VEC generation */
    tree_vec_test();
    
    /* Test OMP_CLAUSE generation */
    omp_test(1000);
    
    /* Test TREE_BINFO generation (C++ only) */
    test_binfo();
    
    /* Use results to prevent optimization removal */
    int final_result = ssa_result + some_unique_identifier;
    
#ifdef __cplusplus
    cout << "Test completed. Result: " << final_result << endl;
#else
    printf("Test completed. Result: %d\n", final_result);
#endif
    
    return final_result > 0 ? 0 : 1;
}
