/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(int);
extern void external_func2(void);
extern volatile int external_var;

#ifdef __cplusplus
extern "C" {
#endif

/* Helper to prevent optimization */
static volatile int sink;

/* Function to accumulate checksum */
static int checksum = 0;

/* ==================== IDENTIFIER_NODE tests ==================== */
void test_identifier_nodes(void) {
    /* Deeply nested scopes with same variable names */
    {
        int x = 1;
        checksum += x;
        
        {
            /* Shadowing in inner scope */
            int x = 2;
            checksum += x;
            
            {
                /* Another level of shadowing */
                volatile int x = 3;
                checksum += x;
                
                {
                    /* Reference to outer x via extern declaration */
                    extern int x; /* This creates a different identifier node */
                    volatile int y = (int)(long)&x; /* Use address to prevent optimization */
                    checksum += y & 0xFF;
                }
            }
        }
    }
    
    /* Multiple functions with same parameter names */
    {
        auto int local_func(int a, int b) {
            volatile int a_copy = a; /* Force separate identifier */
            volatile int b_copy = b;
            return a_copy + b_copy;
        }
        
        checksum += local_func(10, 20);
    }
    
    /* Loop variables with same name in different loops */
    for (int i = 0; i < 3; i++) {
        checksum += i;
        {
            volatile int i = i * 2; /* Self-initialization creates interesting tree */
            checksum += i;
        }
    }
}

/* ==================== TREE_VEC tests ==================== */
void test_tree_vec(void) {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = c * a;
    
    /* Store to volatile to prevent optimization */
    volatile v4si result = d;
    sink = result[0] + result[1] + result[2] + result[3];
    checksum += sink;
    
    /* Array compound literals */
    int *p = (int[]){10, 20, 30, 40};
    checksum += p[0] + p[2];
    
    /* Nested vector operations */
    v4sf f1 = {1.5f, 2.5f, 3.5f, 4.5f};
    v4sf f2 = f1 * 2.0f;
    volatile v4sf f_result = f2;
    sink = (int)f_result[0];
    checksum += sink;
}

/* ==================== SSA_NAME tests ==================== */
int test_ssa_name(int n) {
    /* Complex control flow to generate SSA */
    int i, s = 0, t = 1;
    
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t += 2;
        } else {
            s *= 3;
            t -= 1;
        }
        
        switch (i % 4) {
            case 0: s += 5; break;
            case 1: s -= 3; break;
            case 2: s *= 2; break;
            case 3: s /= 2; break;
        }
    }
    
    /* Multiple assignments to same variable */
    int x = s;
    x = x * 2 + 1;
    x = x % 100;
    x = x < 0 ? -x : x;
    
    return x;
}

/* ==================== BLOCK tests ==================== */
void test_block_nodes(void) {
    /* Nested blocks with labels and gotos */
    {
        int a = 0;
        checksum += a;
        
    block1:
        a++;
        {
            int b = a * 2;
            checksum += b;
            
            if (a < 3)
                goto block2;
            
            {
                volatile int c = b + 1;
                checksum += c;
                goto block3;
            }
        }
        
    block2:
        a += 5;
        goto block1;
    }
    
block3:
    {
        /* Deeply nested block structure */
        {
            {{{
                volatile int depth = 99;
                checksum += depth;
            }}};
        }
    }
    
    /* Block with mixed declarations */
    {
        int x = 1;
    inner_block:
        {
            volatile int y = 2;
            checksum += x + y;
            x++;
            if (x < 4)
                goto inner_block;
        }
    }
}

/* ==================== CONSTRUCTOR tests ==================== */
void test_constructor_nodes(void) {
    /* Struct with designated initializers */
    struct S {
        int a;
        int b[3];
        struct {
            int x;
            int y;
        } nested;
    };
    
    struct S s1 = { 
        .a = 1, 
        .b = {[0] = 10, [2] = 30},
        .nested = {.x = 100, .y = 200}
    };
    checksum += s1.a + s1.b[0] + s1.nested.x;
    
    /* Partial array initialization */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3};
    checksum += arr[5];
    
    /* Nested designated initializers */
    struct T {
        struct S s;
        int extra;
    } t = {
        .s = {.a = 5, .b = {[1] = 50}},
        .extra = 999
    };
    checksum += t.s.b[1] + t.extra;
    
    /* Union initializers */
    union U {
        int i;
        float f;
        char c[4];
    } u1 = {.i = 0x12345678}, u2 = {.f = 3.14f};
    checksum += u1.c[0] + (int)u2.f;
    
    /* Array of structs with designated init */
    struct S s_array[2] = {
        [0] = {.a = 1, .b = {1, 2, 3}},
        [1] = {.a = 2, .b = {[2] = 9}}
    };
    checksum += s_array[1].b[2];
}

/* ==================== OpenMP tests ==================== */
#ifdef _OPENMP
void test_omp_clause_nodes(void) {
    int i, sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    
    /* Multiple clauses in single pragma */
    #pragma omp parallel for private(i) shared(shared_var) \
            reduction(+:sum) schedule(dynamic, 2) \
            firstprivate(private_var) lastprivate(private_var) \
            collapse(1) ordered
    for (i = 0; i < 100; i++) {
        sum += i;
        shared_var++;
        private_var = i;
    }
    
    checksum += sum + shared_var;
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(i) \
            num_threads(2) default(none) \
            copyin(shared_var)
    {
        #pragma omp section
        {
            i = 1;
            checksum += i;
        }
        
        #pragma omp section
        {
            i = 2;
            checksum += i;
        }
    }
    
    /* SIMD directive with clauses */
    int arr[100];
    #pragma omp simd aligned(arr:16) safelen(8) \
            linear(i:1) reduction(+:sum)
    for (i = 0; i < 100; i++) {
        arr[i] = i * 2;
        sum += arr[i];
    }
    
    checksum += sum;
}
#endif

#ifdef __cplusplus
} /* extern "C" */

/* ==================== C++ BINFO tests ==================== */
class Base1 {
public:
    int a;
    virtual void vfunc1() { a = 1; }
};

class Base2 {
public:
    int b;
    virtual void vfunc2() { b = 2; }
};

class Derived : public Base1, public Base2 {
public:
    int c;
    virtual void vfunc1() override { a = 3; }
    virtual void vfunc2() override { b = 4; }
    void derived_func() { c = a + b; }
};

void test_binfo_nodes() {
    Derived d;
    Base1* b1 = &d;
    Base2* b2 = &d;
    
    b1->a = 10;
    b2->b = 20;
    d.derived_func();
    
    checksum += d.a + d.b + d.c;
    
    /* Multiple inheritance hierarchy */
    class DeepDerived : public Derived {
    public:
        int deep;
        void deep_func() { deep = a * b * c; }
    };
    
    DeepDerived dd;
    dd.deep_func();
    checksum += dd.deep;
}

#endif /* __cplusplus */

/* ==================== Main driver ==================== */
int main(void) {
    printf("Starting tree node coverage test...\n");
    
    /* Test each node type */
    test_identifier_nodes();
    printf("  IDENTIFIER_NODE test complete\n");
    
    test_tree_vec();
    printf("  TREE_VEC test complete\n");
    
    checksum += test_ssa_name(20);
    printf("  SSA_NAME test complete\n");
    
    test_block_nodes();
    printf("  BLOCK test complete\n");
    
    test_constructor_nodes();
    printf("  CONSTRUCTOR test complete\n");
    
#ifdef _OPENMP
    test_omp_clause_nodes();
    printf("  OMP_CLAUSE test complete\n");
#endif
    
#ifdef __cplusplus
    test_binfo_nodes();
    printf("  BINFO test complete\n");
#endif
    
    /* Use external identifiers */
    checksum += (int)(long)&external_func1;
    checksum += (int)(long)&external_func2;
    checksum += external_var;
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum == 0 ? 0 : 1;
}
