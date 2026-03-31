/* test_tree_kind.c - Coverage test for GCC's get_kind function */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_tree_kind */
/* For C++ nodes: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind.c -o test_tree_kind_cpp */

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
#include <vector>
#endif

/* ==================== IDENTIFIER_NODE ==================== */
/* Any variable/function name creates IDENTIFIER_NODE */
void identifier_test(void) {
    int some_unique_identifier = 42;
    int another_identifier = some_unique_identifier * 2;
    printf("Identifier test: %d\n", another_identifier);
}

/* ==================== TREE_VEC ==================== */
/* GCC statement expressions can create TREE_VEC nodes */
int tree_vec_test(void) {
    /* Using statement expression (GCC extension) */
    int result = ({
        int a = 5;
        int b = 10;
        int c = 15;
        a + b + c;
    });
    
    /* Another TREE_VEC example with multiple values */
    int vec_result = ({
        int x = 1, y = 2, z = 3;
        x * y + z;
    });
    
    return result + vec_result;
}

#ifdef __cplusplus
/* ==================== TREE_BINFO ==================== */
/* C++ class hierarchy creates BINFO nodes */
class BaseClass {
public:
    virtual void base_method() { printf("Base method\n"); }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    void base_method() override { printf("Derived method\n"); }
};

void binfo_test(void) {
    DerivedClass obj;
    BaseClass* ptr = &obj;
    ptr->base_method();
}
#endif

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic with loops creates SSA_NAME nodes */
int ssa_name_test(int n) {
    int a = 0, b = 1, c;
    
    /* Loop with arithmetic to force SSA form */
    for (int i = 0; i < n; ++i) {
        c = a + b;
        a = b;
        b = c;
        
        /* Additional operations to create more SSA names */
        int temp = c * 2;
        a = temp - b;
        b = temp + a;
    }
    
    /* Conditional to create phi nodes */
    int result = (a > 100) ? a : b;
    
    /* More SSA opportunities */
    for (int j = 0; j < 10; ++j) {
        result += j * j;
        if (result % 2 == 0) {
            result /= 2;
        } else {
            result = result * 3 + 1;
        }
    }
    
    return result;
}

/* ==================== BLOCK ==================== */
/* Nested blocks create BLOCK nodes */
int block_test(int x) {
    /* Outer block */
    int outer = x;
    
    {
        /* Inner block 1 */
        int inner1 = outer * 2;
        
        {
            /* Inner block 2 */
            int inner2 = inner1 + 5;
            
            {
                /* Deeply nested block */
                int inner3 = inner2 - 3;
                outer = inner3;
            }
        }
        
        /* Another block with locals */
        {
            int temp = 100;
            outer += temp;
        }
    }
    
    /* Final block with computation */
    {
        int final_val = outer;
        for (int i = 0; i < 5; ++i) {
            final_val += i;
        }
        return final_val;
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct Point {
    int x;
    int y;
    int z;
};

int constructor_test(void) {
    /* Array constructor */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Struct constructor */
    struct Point p1 = {.x = 1, .y = 2, .z = 3};
    struct Point p2 = {5, 6, 7};
    
    /* Nested struct constructor */
    struct Line {
        struct Point start;
        struct Point end;
    };
    
    struct Line line = {
        .start = {0, 0, 0},
        .end = {10, 10, 10}
    };
    
    /* Complex constructor with expressions */
    int matrix[2][3] = {
        {1*2, 2*2, 3*2},
        {4*2, 5*2, 6*2}
    };
    
    return arr[0] + p1.x + line.end.x + matrix[1][2];
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_clause_test(int size) {
    int i;
    int sum = 0;
    int* data = (int*)malloc(size * sizeof(int));
    
    if (!data) return;
    
    /* Initialize data */
    for (i = 0; i < size; ++i) {
        data[i] = i;
    }
    
    /* OpenMP parallel region with various clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static, 10)
    for (i = 0; i < size; ++i) {
        sum += data[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(i) shared(sum, data)
    {
        #pragma omp section
        {
            for (i = 0; i < size/2; ++i) {
                data[i] *= 2;
            }
        }
        
        #pragma omp section
        {
            for (i = size/2; i < size; ++i) {
                data[i] /= 2;
            }
        }
    }
    
    printf("OMP sum: %d\n", sum);
    free(data);
}
#else
void omp_clause_test(int size) {
    printf("OpenMP not enabled\n");
}
#endif

/* ==================== MAIN FUNCTION ==================== */
/* Orchestrates all tests to ensure all tree nodes are created */
int main(void) {
    printf("Testing GCC tree node coverage...\n");
    
    /* Call all test functions to ensure compiler processes them */
    identifier_test();
    
    int vec_result = tree_vec_test();
    printf("Tree vec result: %d\n", vec_result);
    
#ifdef __cplusplus
    binfo_test();
#endif
    
    int ssa_result = ssa_name_test(20);
    printf("SSA name result: %d\n", ssa_result);
    
    int block_result = block_test(42);
    printf("Block result: %d\n", block_result);
    
    int constructor_result = constructor_test();
    printf("Constructor result: %d\n", constructor_result);
    
    omp_clause_test(100);
    
    /* Additional complex code to ensure middle-end passes run */
    volatile int final = 0;
    for (int i = 0; i < 1000; ++i) {
        final += i * i;
        if (final > 1000000) {
            final /= 2;
        }
    }
    
    printf("Final result: %d\n", final);
    printf("All tests completed.\n");
    
    return 0;
}
