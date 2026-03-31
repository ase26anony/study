/* test_tree_kind.c - Comprehensive test for GCC tree node kinds */

#ifdef __cplusplus
#include <iostream>
using namespace std;
#else
#include <stdio.h>
#endif

/* ========== IDENTIFIER_NODE ========== */
/* Variable and function names create IDENTIFIER_NODE */
int global_identifier;
void function_identifier(void);

/* Helper function using multiple identifiers */
int process_data(int input_identifier, char *string_identifier) {
    static int static_identifier = 0;
    int local_identifier = input_identifier * 2;
    return local_identifier + static_identifier++;
}

/* ========== SSA_NAME ========== */
/* Complex function to force SSA form */
int ssa_generator(int n) {
    int a = 0, b = 1, c;
    
    /* Loop with multiple assignments to create SSA names */
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            a = a + i * 2;
            b = b - i;
        } else {
            a = a - i;
            b = b + i * 3;
        }
        
        /* Nested condition to create phi nodes */
        c = (a > b) ? a : b;
        
        /* Another assignment that will be converted to SSA */
        int temp = c * 2;
        a = temp / 3;
    }
    
    /* Multiple return paths for SSA */
    if (n > 100) return a;
    if (n < 0) return b;
    return c;
}

/* ========== BLOCK ========== */
/* Function with multiple nested blocks */
void block_generator(void) {
    /* Outer block */
    int x = 0;
    
    {
        /* Inner block 1 */
        int y = 10;
        x += y;
        
        {
            /* Inner block 2 */
            int z = 20;
            x += z;
            
            {
                /* Inner block 3 with its own scope */
                int w = 30;
                x += w;
            }
        }
    }
    
    /* Another block with switch */
    {
        int val = 5;
        switch (val) {
            case 1: x += 1; break;
            case 5: x += 5; break;
            default: x += 10;
        }
    }
}

/* ========== CONSTRUCTOR ========== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct point {
    int x;
    int y;
    int z;
};

union data {
    int i;
    float f;
    char c;
};

void constructor_generator(void) {
    /* Array constructor */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Struct constructor */
    struct point p1 = {.x = 1, .y = 2, .z = 3};
    struct point p2 = {1, 2, 3};
    
    /* Union constructor */
    union data d1 = {.i = 42};
    union data d2 = {42};
    
    /* Nested struct constructor */
    struct nested {
        struct point pt;
        int id;
    } n1 = {{5, 6, 7}, 100};
    
    /* Partial initialization */
    int partial[10] = {[3] = 100, [7] = 200};
}

/* ========== TREE_VEC ========== */
/* Using GCC statement expressions for TREE_VEC */
#ifdef __GNUC__
#define VEC_EXPRESSION(x) ({ \
    int _a = (x) * 2; \
    int _b = (x) * 3; \
    int _c = (x) * 4; \
    _a + _b + _c; \
})

int vec_generator(int val) {
    /* Using statement expression */
    int result = VEC_EXPRESSION(val);
    
    /* Another complex expression that might generate TREE_VEC */
    int complex = ({
        int tmp = val + 10;
        tmp * tmp - val;
    });
    
    return result + complex;
}
#else
int vec_generator(int val) {
    return val * 10;
}
#endif

/* ========== OMP_CLAUSE ========== */
/* OpenMP pragmas generate OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>

void omp_generator(int size) {
    int i;
    int sum = 0;
    int data[100];
    
    /* Initialize data */
    for (i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* Multiple OpenMP pragmas with different clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static)
    for (i = 0; i < 100; i++) {
        sum += data[i];
    }
    
    /* Another with different clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            int tid = omp_get_thread_num();
            #ifdef __cplusplus
            cout << "Thread " << tid << " executing single" << endl;
            #else
            printf("Thread %d executing single\n", tid);
            #endif
        }
        
        #pragma omp for nowait
        for (i = 0; i < 50; i++) {
            data[i] *= 2;
        }
    }
    
    /* Parallel sections */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (i = 0; i < 25; i++) data[i] += 1;
        }
        
        #pragma omp section
        {
            for (i = 25; i < 50; i++) data[i] -= 1;
        }
    }
}
#else
void omp_generator(int size) {
    /* Dummy implementation when OpenMP not available */
    (void)size;
}
#endif

/* ========== C++ Specific: TREE_BINFO ========== */
#ifdef __cplusplus
/* Class hierarchy to generate BINFO nodes */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void base_method() = 0;
    int base_data;
};

class IntermediateClass : virtual public BaseClass {
public:
    virtual void intermediate_method() {}
    int intermediate_data;
};

class DerivedClass : public IntermediateClass {
public:
    void base_method() override {}
    void derived_method() {}
    int derived_data;
};

/* Multiple inheritance for more complex BINFO */
class AnotherBase {
public:
    virtual ~AnotherBase() {}
    virtual void another_method() {}
    int another_data;
};

class MultipleDerived : public DerivedClass, public AnotherBase {
public:
    void another_method() override {}
    int multiple_data;
};

void binfo_generator(void) {
    DerivedClass d;
    MultipleDerived md;
    
    BaseClass* bp = &d;
    AnotherBase* ap = &md;
    
    bp->base_method();
    ap->another_method();
    
    /* Dynamic cast to exercise RTTI/binfo */
    DerivedClass* dp = dynamic_cast<DerivedClass*>(bp);
    if (dp) {
        dp->derived_method();
    }
}
#endif

/* ========== Main Driver ========== */
int main(void) {
    int result = 0;
    
    /* Exercise IDENTIFIER_NODE */
    global_identifier = 42;
    result += process_data(10, "test");
    
    /* Exercise SSA_NAME */
    result += ssa_generator(50);
    
    /* Exercise BLOCK */
    block_generator();
    
    /* Exercise CONSTRUCTOR */
    constructor_generator();
    
    /* Exercise TREE_VEC */
    result += vec_generator(20);
    
    /* Exercise OMP_CLAUSE */
    omp_generator(100);
    
#ifdef __cplusplus
    /* Exercise TREE_BINFO (C++ only) */
    binfo_generator();
    
    cout << "Result: " << result << endl;
    return result > 0 ? 0 : 1;
#else
    printf("Result: %d\n", result);
    return result > 0 ? 0 : 1;
#endif
}
