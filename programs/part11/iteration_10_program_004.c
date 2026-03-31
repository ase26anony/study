/* tree_node_coverage.c - Test program to exercise specific GCC tree node types */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(int);
extern void external_func2(void);
extern volatile int external_var;

/* Global variables for checksum */
static volatile int checksum = 0;

/* Helper to consume values and update checksum */
static void consume(int value) {
    checksum = (checksum * 31 + value) & 0x7FFFFFFF;
}

/* ==================== IDENTIFIER_NODE Coverage ==================== */
void test_identifier_nodes(void) {
    /* Deeply nested scopes with same variable names */
    {
        int x = 1;
        consume(x);
        {
            /* Shadowing in inner scope */
            int x = 2;
            consume(x);
            {
                /* Another level of shadowing */
                volatile int x = 3;
                consume(x);
                {
                    /* Reference to outer x via extern declaration */
                    extern int x; /* This creates a different identifier node */
                    volatile int y = x; /* Prevent merging */
                    consume(y);
                }
            }
        }
    }
    
    /* Multiple scopes in loops */
    for (int i = 0; i < 3; i++) {
        int counter = i * 10;
        {
            int counter = i * 20; /* Different identifier node */
            consume(counter);
        }
        consume(counter);
    }
    
    /* Function parameter shadowing */
    {
        auto int func_local(int x) {
            {
                int x = x * 2; /* Parameter shadowed */
                return x;
            }
        }
        consume(func_local(5));
    }
}

/* ==================== TREE_VEC Coverage ==================== */
void test_tree_vec(void) {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = a + a;
    v4si c = b * a;
    
    /* Consume vector elements */
    for (int i = 0; i < 4; i++) {
        consume(c[i]);
    }
    
    /* Array compound literals */
    int *p = (int[]){1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        consume(p[i]);
    }
    
    /* Nested compound literals */
    struct Point { int x; int y; };
    struct Point *points = (struct Point[]){{1, 2}, {3, 4}, {5, 6}};
    for (int i = 0; i < 3; i++) {
        consume(points[i].x + points[i].y);
    }
    
    /* Vector operations */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = f1 * 2.0f;
    volatile v4sf sink = f2; /* Prevent optimization */
}

/* ==================== SSA_NAME Coverage ==================== */
int test_ssa_name(void) {
    /* Complex control flow with many assignments */
    int i, s = 0, t = 1;
    
    /* Loop with conditional updates */
    for (i = 0; i < 20; i++) {
        if (i & 1) {
            s += i * t;
            t++;
        } else {
            s *= 2;
            t--;
        }
        
        /* Nested condition */
        if (s > 100) {
            s /= 2;
        } else if (s < 0) {
            s = 0;
        }
    }
    
    /* Another loop with phi node potential */
    int x = 0, y = 1;
    for (int j = 0; j < 15; j++) {
        if (j % 3 == 0) {
            x = y + j;
        } else if (j % 3 == 1) {
            y = x * 2;
        } else {
            int temp = x;
            x = y;
            y = temp;
        }
        consume(x + y);
    }
    
    return s;
}

/* ==================== BLOCK Node Coverage ==================== */
void test_block_nodes(void) {
    /* Deeply nested blocks with labels and gotos */
    int a = 0;
    
    block1: {
        int b = 1;
        consume(a + b);
        {
            int c = 2;
            goto block3; /* Jump to inner block */
    block2:
            consume(b + c);
            goto block4;
        }
    }
    
    goto block5; /* Skip block3 */
    
    block3: {
        int d = 3;
        consume(d);
        goto block2;
    }
    
    block4: {
        int e = 4;
        consume(e);
    }
    
    block5: {
        /* Empty block with label */
        ;
    }
    
    /* Switch with blocks */
    switch (a) {
        case 0: {
            int block_var = 10;
            consume(block_var);
            break;
        }
        case 1: {
            int block_var = 20; /* Different identifier node */
            consume(block_var);
            break;
        }
        default: {
            int block_var = 30;
            consume(block_var);
        }
    }
}

/* ==================== CONSTRUCTOR Node Coverage ==================== */
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
    
    /* Partial initialization with designators */
    struct S s1 = { 
        .a = 1, 
        .b = {[1] = 5, [0] = 3},
        .nested = {.x = 7, .y = 8}
    };
    consume(s1.a + s1.b[0] + s1.b[1] + s1.nested.x);
    
    /* Nested array initialization */
    int arr[2][3] = {[0][1] = 1, [1][2] = 2};
    consume(arr[0][1] + arr[1][2]);
    
    /* Union initializers */
    union U {
        int i;
        float f;
        char c[4];
    };
    
    union U u1 = {.i = 0x12345678};
    union U u2 = {.f = 3.14f};
    union U u3 = {.c = {'a', 'b', 'c', '\0'}};
    
    consume(u1.i);
    
    /* Zero initialization with designators */
    struct S s2 = {.b = {0}};
    consume(s2.a);
    
    /* Array of structs with designators */
    struct S sarray[2] = {
        [0] = {.a = 1, .b = {2, 3, 4}},
        [1] = {.a = 5, .nested = {.x = 6}}
    };
    consume(sarray[0].b[1] + sarray[1].a);
}

/* ==================== OpenMP Clause Coverage ==================== */
void test_omp_clause_nodes(void) {
    int i;
    int sum = 0;
    int array[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        array[i] = i + 1;
    }
    
    /* OpenMP pragma with multiple clauses */
    #pragma omp parallel for private(i) shared(array, sum) \
            reduction(+:sum) schedule(dynamic, 5) \
            num_threads(4) if(100 > 50)
    for (i = 0; i < 100; i++) {
        sum += array[i];
    }
    
    consume(sum);
    
    /* Additional OpenMP constructs */
    int a = 0, b = 0;
    
    #pragma omp parallel sections private(i) \
            firstprivate(a) lastprivate(b) \
            copyin(a) copyprivate(b)
    {
        #pragma omp section
        {
            a = 1;
            b = 2;
        }
        
        #pragma omp section
        {
            a = 3;
            b = 4;
        }
    }
    
    consume(a + b);
    
    /* OMP critical with clause */
    #pragma omp critical (my_critical) hint(omp_sync_hint_contended)
    {
        sum++;
    }
    
    consume(sum);
}

#ifdef __cplusplus
/* ==================== TREE_BINFO Coverage (C++ only) ==================== */
class Base {
public:
    int a;
    virtual void vfunc() { a = 1; }
    Base() : a(0) {}
};

class Derived : public Base {
public:
    int b;
    virtual void vfunc() override { a = 2; b = 3; }
    Derived() : b(0) {}
};

class Derived2 : public Derived {
public:
    int c;
    virtual void vfunc() override { a = 4; b = 5; c = 6; }
    Derived2() : c(0) {}
};

void test_binfo_nodes() {
    Derived d;
    Derived2 d2;
    
    /* Access base class members */
    d.a = 10;
    d.b = 20;
    
    /* Pointer to base class */
    Base* bp = &d;
    bp->vfunc(); /* Virtual call */
    
    /* Multiple inheritance-like access */
    Derived* dp = &d2;
    dp->a = 30;
    dp->b = 40;
    
    /* Reference to base */
    Base& br = d2;
    br.a = 50;
    
    consume(d.a + d.b + d2.c);
}

/* C++ specific constructor nodes */
struct CPPStruct {
    int x;
    double y;
    char z;
    
    CPPStruct() : x(1), y(2.0), z('a') {}
    CPPStruct(int a, double b, char c) : x(a), y(b), z(c) {}
};

void test_cpp_constructors() {
    /* Various C++ initializations */
    CPPStruct s1;
    CPPStruct s2 = {10, 20.0, 'b'};
    CPPStruct s3{30, 40.0, 'c'};
    CPPStruct s4 = CPPStruct(50, 60.0, 'd');
    
    consume(s1.x + s2.x + s3.x + s4.x);
}
#endif

/* ==================== Main Function ==================== */
int main(void) {
    printf("Starting tree node coverage test...\n");
    
    /* Test all node types */
    test_identifier_nodes();
    test_tree_vec();
    consume(test_ssa_name());
    test_block_nodes();
    test_constructor_nodes();
    
    /* Test OpenMP if available */
    #ifdef _OPENMP
    test_omp_clause_nodes();
    #endif
    
    /* Test C++ specific nodes if in C++ mode */
    #ifdef __cplusplus
    test_binfo_nodes();
    test_cpp_constructors();
    #endif
    
    /* Call external functions to create unresolved identifiers */
    external_func1(checksum);
    external_func2();
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
