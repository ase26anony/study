/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func(int);
extern volatile int external_var;

/* Global variables for checksum */
static volatile int checksum = 0;

/* Helper to prevent optimization */
static void sink(int value) {
    checksum ^= value;
    asm volatile("" : : "r"(value) : "memory");
}

/* ========== IDENTIFIER_NODE tests ========== */
void test_identifier_nodes(void) {
    /* Deeply nested scopes with same variable names */
    {
        int x = 1;
        sink(x);
        {
            /* Shadowing declaration */
            int x = 2;
            sink(x);
            {
                /* Another shadow with volatile */
                volatile int x = 3;
                sink(x);
                {
                    /* External declaration in inner scope */
                    extern int external_var;
                    volatile int y = external_var + x;
                    sink(y);
                }
            }
        }
    }
    
    /* Function parameter shadowing */
    {
        auto int test_shadow(int x) {
            {
                long x = (long)x * 2;  /* Different type */
                sink((int)x);
            }
            return x + 1;
        }
        sink(test_shadow(5));
    }
    
    /* Multiple scopes in loops */
    for (int i = 0; i < 3; i++) {
        int counter = i;
        sink(counter);
        for (int i = 0; i < 2; i++) {  /* Shadow loop variable */
            volatile int counter = i * 10;
            sink(counter);
        }
    }
}

/* ========== TREE_VEC tests ========== */
void test_tree_vec(void) {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    
    /* Use vector elements to affect checksum */
    for (int i = 0; i < 4; i++) {
        sink(c[i]);
    }
    
    /* Vector operations */
    v4si d = a * b - c;
    sink(d[0] + d[1] + d[2] + d[3]);
    
    /* Float vectors */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = f1 * 2.0f;
    sink((int)f2[0] + (int)f2[1]);
    
    /* Array compound literals */
    int *arr1 = (int[]){10, 20, 30, 40};
    int *arr2 = (int[3]){50, 60, 70};
    
    sink(arr1[0] + arr1[1]);
    sink(arr2[0] + arr2[2]);
    
    /* Nested compound literals */
    struct Point { int x; int y; };
    struct Point *p = &(struct Point){ .x = 100, .y = 200 };
    sink(p->x + p->y);
}

/* ========== SSA_NAME tests ========== */
int test_ssa_name(int n) {
    /* Complex control flow with many assignments */
    int x = 0, y = 1, z = 2;
    volatile int v = 3;
    
    for (int i = 0; i < n; i++) {
        if (i & 1) {
            x += i * y;
            y = z + v;
        } else {
            x *= 2;
            z = x - y;
        }
        
        switch (i % 3) {
            case 0: x += 5; break;
            case 1: y *= 3; break;
            case 2: z = x ^ y; break;
        }
        
        v = i;  /* Volatile prevents SSA merging */
    }
    
    /* More complex flow */
    int result = 0;
    for (int i = 0; i < 10; i++) {
        int temp = i;
        for (int j = 0; j < i; j++) {
            if ((i + j) % 2 == 0) {
                temp += j;
            } else {
                temp *= 2;
            }
        }
        result += temp;
    }
    
    return result + x + y + z;
}

/* ========== BLOCK tests ========== */
void test_block_nodes(void) {
    /* Deeply nested blocks with labels and gotos */
    {
        int a = 0;
    block1:
        a++;
        {
            int b = a * 2;
        block2:
            b += 5;
            {
                volatile int c = b + 10;
                sink(c);
                if (a < 3) goto block1;
            }
            goto block3;
        }
        goto block4;
    block3:
        a += 100;
    }
    
block4:
    {
        /* More complex block structure */
        int x = 0;
        
        /* Nested blocks with declarations */
        {
            int y = 1;
        inner1:
            x += y;
            {
                int z = 2;
                if (x < 10) goto inner1;
            inner2:
                x *= 2;
                goto inner3;
            }
        }
        
    inner3:
        sink(x);
        
        /* Switch with blocks */
        switch (x % 3) {
            case 0: {
                int temp = x + 1;
                sink(temp);
                break;
            }
            case 1: {
                int temp = x * 2;
                sink(temp);
                break;
            }
            default: {
                volatile int temp = x - 1;
                sink(temp);
                break;
            }
        }
    }
}

/* ========== CONSTRUCTOR tests ========== */
void test_constructor_nodes(void) {
    /* Designated initializers */
    struct S {
        int a;
        int b[3];
        struct {
            int x;
            int y;
        } nested;
    };
    
    /* Partial and nested initialization */
    struct S s1 = { 
        .a = 1, 
        .b = {[0] = 10, [2] = 30},
        .nested = {.x = 100, .y = 200}
    };
    
    sink(s1.a + s1.b[0] + s1.b[2] + s1.nested.x);
    
    /* More complex designated initializers */
    struct S s2 = {
        .b = {5, [2] = 7},
        .a = 2,
        .nested.y = 300
    };
    
    sink(s2.a + s2.b[0] + s2.b[2] + s2.nested.y);
    
    /* Array with designated initializers */
    int arr[5] = {[0] = 1, [2] = 3, [4] = 5};
    sink(arr[0] + arr[2] + arr[4]);
    
    /* Union initializers */
    union U {
        int i;
        float f;
        char c[4];
    };
    
    union U u1 = { .i = 0x12345678 };
    union U u2 = { .f = 3.14f };
    union U u3 = { .c = {'a', 'b', 'c', 'd'} };
    
    sink(u1.i + (int)u2.f + u3.c[0]);
    
    /* Nested struct with array */
    struct Outer {
        struct Inner {
            int values[4];
            char tag;
        } inner;
        int count;
    };
    
    struct Outer outer = {
        .inner = {
            .values = {[1] = 42, [3] = 84},
            .tag = 'X'
        },
        .count = 2
    };
    
    sink(outer.inner.values[1] + outer.inner.values[3] + outer.count);
}

/* ========== OpenMP tests ========== */
void test_omp_clause_nodes(void) {
    int i, sum = 0;
    int array[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        array[i] = i + 1;
    }
    
    /* OpenMP with multiple clauses */
    #pragma omp parallel for private(i) shared(array, sum) \
        reduction(+:sum) schedule(dynamic, 4) if(1000 > 100) \
        num_threads(2)
    for (i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    sink(sum);
    
    /* More OpenMP constructs */
    int a = 0, b = 0, c = 0;
    
    #pragma omp parallel sections private(i) \
        firstprivate(a) lastprivate(b) nowait
    {
        #pragma omp section
        {
            a = 1;
            for (i = 0; i < 50; i++) {
                #pragma omp atomic
                c += i;
            }
        }
        
        #pragma omp section
        {
            a = 2;
            for (i = 50; i < 100; i++) {
                #pragma omp atomic
                c += i;
            }
            b = c;
        }
    }
    
    sink(a + b + c);
    
    /* SIMD directive */
    int x[100], y[100], z[100];
    
    #pragma omp simd aligned(x, y: 16) linear(i:1) \
        safelen(8) simdlen(4)
    for (i = 0; i < 100; i++) {
        z[i] = x[i] + y[i];
    }
    
    /* Collapse clause */
    int matrix[10][10];
    int total = 0;
    
    #pragma omp parallel for collapse(2) reduction(+:total) \
        ordered
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            #pragma omp ordered
            matrix[i][j] = i * 10 + j;
            total += matrix[i][j];
        }
    }
    
    sink(total);
}

#ifdef __cplusplus
/* ========== C++ BINFO tests ========== */
class Base {
public:
    int a;
    virtual void foo() { a = 1; }
    Base() : a(0) {}
};

class Derived : public Base {
public:
    int b;
    virtual void foo() override { a = 2; b = 3; }
    Derived() : b(0) {}
};

class Derived2 : public Derived {
public:
    int c;
    virtual void foo() override { a = 4; b = 5; c = 6; }
    Derived2() : c(0) {}
};

void test_binfo_nodes() {
    Derived d;
    Derived2 d2;
    
    /* Access through base pointer */
    Base* bp = &d;
    bp->foo();
    bp->a = 10;
    
    /* Multiple inheritance-like access */
    Derived* dp = &d2;
    dp->foo();
    dp->a = 20;
    dp->b = 30;
    
    /* Casts that require BINFO lookups */
    Base& br = d2;
    br.foo();
    
    sink(d.a + d.b + d2.a + d2.b + d2.c);
}

/* More complex hierarchy */
template<typename T>
class TemplateBase {
public:
    T value;
    virtual T get() { return value; }
};

class Concrete : public TemplateBase<int>, public Base {
public:
    int extra;
    virtual int get() override { return value + a + extra; }
};

void test_template_binfo() {
    Concrete c;
    c.value = 100;
    c.a = 200;
    c.extra = 300;
    
    TemplateBase<int>* tb = &c;
    Base* b = &c;
    
    sink(tb->get() + b->a);
}
#endif

/* ========== Main test driver ========== */
int main(void) {
    printf("Starting tree node coverage tests...\n");
    
    /* Run all tests */
    test_identifier_nodes();
    test_tree_vec();
    
    int ssa_result = test_ssa_name(20);
    sink(ssa_result);
    
    test_block_nodes();
    test_constructor_nodes();
    
    /* OpenMP test if supported */
    #ifdef _OPENMP
    test_omp_clause_nodes();
    #else
    printf("OpenMP not supported, skipping OMP_CLAUSE tests\n");
    #endif
    
    #ifdef __cplusplus
    test_binfo_nodes();
    test_template_binfo();
    #else
    printf("C++ not enabled, skipping BINFO tests\n");
    #endif
    
    /* Final checksum */
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
