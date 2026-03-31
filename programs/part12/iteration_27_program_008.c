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
    /* Using statement expression - creates TREE_VEC internally */
    return ({
        int a = 5;
        int b = 10;
        int c = a + b;
        c;
    });
}
#endif

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic with loops creates SSA_NAME nodes */
int ssa_name_example(int n) {
    int result = 0;
    /* This loop forces SSA form */
    for (int i = 0; i < n; ++i) {
        result = result + i * 2;  /* Creates phi nodes in SSA */
    }
    
    /* More SSA complexity */
    int x = 10, y = 20;
    for (int j = 0; j < 5; ++j) {
        x = x + y;
        y = y - 1;
        result += x;
    }
    
    return result;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables create BLOCK nodes */
void block_example() {
    /* Outer block */
    int outer = 1;
    
    {
        /* Inner block 1 */
        int inner1 = outer + 1;
        
        {
            /* Inner block 2 */
            int inner2 = inner1 * 2;
            {
                /* Deeply nested block */
                int inner3 = inner2 + outer;
                (void)inner3;
            }
        }
    }
    
    /* Another block with control flow */
    if (outer > 0) {
        int conditional_var = 100;
        for (int i = 0; i < 3; i++) {
            int loop_var = conditional_var + i;
            (void)loop_var;
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
    int array_constructor[5] = {10, 20, 30, 40, 50};
    
    /* Struct constructor */
    struct Point p1 = {.x = 1, .y = 2, .z = 3};
    
    /* Nested struct with array constructor */
    struct Data d1 = {
        .id = 100,
        .location = {.x = 5, .y = 6, .z = 7},
        .values = {1.1f, 2.2f, 3.3f, 4.4f}
    };
    
    /* Complex constructor with designators */
    struct Data d2 = {
        .id = 200,
        .location = {.x = 0, .y = 0, .z = 0},
        .values = {[0] = 5.5f, [3] = 6.6f}
    };
    
    (void)array_constructor;
    (void)p1;
    (void)d1;
    (void)d2;
}

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_clause_example(int size) {
    int i;
    int sum = 0;
    int array[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static, 10)
    for (i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    /* More OpenMP constructs */
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (i = 0; i < 50; i++) {
            array[i] *= 2;
        }
        
        #pragma omp single
        {
            int tid = omp_get_thread_num();
            (void)tid;
        }
    }
    
    /* OpenMP sections with clause */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < 25; i++) array[i] += 1;
        }
        
        #pragma omp section
        {
            for (i = 25; i < 50; i++) array[i] -= 1;
        }
    }
}
#endif

/* ==================== C++ Specific: TREE_BINFO ==================== */
#ifdef __cplusplus
/* Class hierarchies create TREE_BINFO nodes */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void base_method() = 0;
    int base_data;
};

class Derived1 : public virtual BaseClass {
public:
    void base_method() override {}
    int derived1_data;
};

class Derived2 : public virtual BaseClass {
public:
    void base_method() override {}
    int derived2_data;
};

class MultipleDerived : public Derived1, public Derived2 {
public:
    void base_method() override {}
    int multiple_data;
};

void tree_binfo_example() {
    MultipleDerived md;
    BaseClass* bp = &md;
    Derived1* d1p = &md;
    Derived2* d2p = &md;
    
    bp->base_method();
    d1p->base_method();
    d2p->base_method();
    
    /* Use virtual inheritance to ensure BINFO creation */
    MultipleDerived* mdp = dynamic_cast<MultipleDerived*>(bp);
    (void)mdp;
}

/* Template with inheritance for more BINFO complexity */
template<typename T>
class TemplateBase {
public:
    virtual T get_value() = 0;
};

class ConcreteClass : public TemplateBase<int> {
public:
    int get_value() override { return 42; }
};
#endif

/* ==================== Main Driver ==================== */
int main() {
    /* Ensure all constructs are used */
    identifier_function();
    
    #ifdef __GNUC__
    int vec_result = tree_vec_example();
    #else
    int vec_result = 15;
    #endif
    
    int ssa_result = ssa_name_example(20);
    block_example();
    constructor_example();
    
    #ifdef _OPENMP
    omp_clause_example(100);
    #endif
    
    #ifdef __cplusplus
    tree_binfo_example();
    #endif
    
    /* Combine results to prevent optimization removal */
    int final_result = vec_result + ssa_result;
    
    #ifdef __cplusplus
    cout << "Result: " << final_result << endl;
    #else
    printf("Result: %d\n", final_result);
    #endif
    
    return final_result > 0 ? 0 : 1;
}
