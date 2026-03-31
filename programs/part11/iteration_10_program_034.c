/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(void);
extern void external_func2(int);
extern double external_func3(float);

/* Volatile sink to prevent optimization */
volatile int volatile_sink = 0;

/* Function to accumulate checksum */
static int checksum = 0;

/* ==================== IDENTIFIER_NODE Coverage ==================== */
void test_identifier_nodes(void) {
    /* Deeply nested scopes with same variable names */
    {
        int x = 1;
        volatile_sink = x;
        checksum += x;
        
        {
            /* Shadowing in inner scope */
            int x = 2;
            volatile_sink = x;
            checksum += x;
            
            {
                /* Another level of shadowing */
                volatile int x = 3;
                volatile_sink = x;
                checksum += x;
                
                {
                    /* External declaration in innermost scope */
                    extern int x;  /* Creates another identifier node */
                    volatile int y = 4;
                    volatile_sink = y;
                    checksum += y;
                }
            }
        }
    }
    
    /* Function parameter shadowing */
    {
        auto int test_shadow(int x) {
            {
                int x = x * 2;  /* Parameter shadowed by local */
                return x;
            }
        }
        checksum += test_shadow(5);
    }
    
    /* Multiple scopes in loops */
    for (int i = 0; i < 3; i++) {
        int counter = i;
        {
            int counter = counter * 2;  /* Shadows outer counter */
            volatile_sink = counter;
            checksum += counter;
        }
    }
}

/* ==================== TREE_VEC Coverage ==================== */
void test_tree_vec(void) {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    /* Store to volatile to prevent optimization */
    volatile v4si v_volatile = c;
    volatile_sink = v_volatile[0] + v_volatile[1];
    checksum += v_volatile[0] + v_volatile[1] + v_volatile[2] + v_volatile[3];
    
    /* Array compound literals */
    int *p = (int[]){1, 2, 3, 4, 5};
    int *q = (int[]){[0] = 10, [2] = 20, [4] = 30};
    
    for (int i = 0; i < 5; i++) {
        checksum += p[i] + q[i];
    }
    
    /* Nested compound literals */
    struct Point { int x; int y; };
    struct Point *points = (struct Point[]){
        {.x = 1, .y = 2},
        {.x = 3, .y = 4},
        {.x = 5, .y = 6}
    };
    
    for (int i = 0; i < 3; i++) {
        checksum += points[i].x + points[i].y;
    }
}

/* ==================== SSA_NAME Coverage ==================== */
int test_ssa_name(void) {
    /* Complex control flow with many assignments */
    int i, s = 0, t = 1, u = 2;
    
    /* Loop with conditional updates */
    for (i = 0; i < 20; i++) {
        if (i & 1) {
            s += i * t;
            t = t ^ u;
        } else {
            s *= 2 + u;
            u = t + 1;
        }
        
        /* Nested condition */
        if (i > 10) {
            t = s % 7;
            if (i == 15) {
                u = t * 3;
                s = u - 5;
            }
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
            int tmp = x + y;
            x = y;
            y = tmp;
        }
        checksum += x + y;
    }
    
    checksum += s + t + u;
    return s;
}

/* ==================== BLOCK Coverage ==================== */
void test_block_nodes(void) {
    /* Nested blocks with labels and gotos */
    int a = 0;
    
    block1: {
        int b = 10;
        a += b;
        {
            int c = 20;
            volatile_sink = c;
            goto block3;
        }
    }
    
    block2: {
        int d = 30;
        a += d;
        goto block4;
    }
    
    block3: {
        int e = 40;
        a += e;
        goto block2;
    }
    
    block4: {
        int f = 50;
        a += f;
        
        /* More nesting */
        {
            {
                {
                    int deeply_nested = 100;
                    a += deeply_nested;
                }
            }
        }
    }
    
    /* Switch with blocks */
    switch (a % 4) {
        case 0: {
            int case_var = 1;
            a += case_var;
            break;
        }
        case 1: {
            int case_var = 2;  /* Same name, different scope */
            a += case_var;
            break;
        }
        default: {
            int case_var = 3;
            a += case_var;
            break;
        }
    }
    
    checksum += a;
}

/* ==================== CONSTRUCTOR Coverage ==================== */
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
    
    struct S s1 = { .a = 1, .b = {[0] = 2, [2] = 4}, .nested = {.x = 5, .y = 6} };
    struct S s2 = { .a = 10, .b = {[1] = 20}, .nested.x = 30 };
    
    checksum += s1.a + s1.b[0] + s1.b[2] + s1.nested.x + s1.nested.y;
    checksum += s2.a + s2.b[1] + s2.nested.x;
    
    /* Array with designated initializers */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3, [2 ... 4] = 7};
    for (int i = 0; i < 10; i++) {
        checksum += arr[i];
    }
    
    /* Union initializers */
    union U {
        int i;
        float f;
        char c[4];
    };
    
    union U u1 = { .i = 0x12345678 };
    union U u2 = { .f = 3.14f };
    union U u3 = { .c = {'a', 'b', 'c', 'd'} };
    
    checksum += u1.i + (int)u2.f + u3.c[0];
    
    /* Nested designated initializers */
    struct Outer {
        struct Inner {
            int a;
            int b;
        } inner;
        int c;
    };
    
    struct Outer outer = { .inner = {.a = 1, .b = 2}, .c = 3 };
    struct Outer outer2 = { .inner.a = 4, .c = 5 };
    
    checksum += outer.inner.a + outer.inner.b + outer.c;
    checksum += outer2.inner.a + outer2.c;
}

/* ==================== OpenMP Clause Coverage ==================== */
void test_omp_clause_nodes(void) {
    int i, sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    
    /* Multiple clauses in single pragma */
    #pragma omp parallel for private(i) shared(shared_var) reduction(+:sum) schedule(dynamic, 2) if(1)
    for (i = 0; i < 100; i++) {
        sum += i;
        shared_var++;
    }
    
    checksum += sum + shared_var;
    
    /* More complex OpenMP constructs */
    int a[100], b[100], c[100];
    
    #pragma omp parallel private(private_var)
    {
        private_var = omp_get_thread_num();
        
        #pragma omp for nowait
        for (int j = 0; j < 100; j++) {
            a[j] = j;
        }
        
        #pragma omp barrier
        
        #pragma omp for collapse(2) ordered
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                b[j * 10 + k] = j + k;
            }
        }
    }
    
    /* OMP sections with different clauses */
    #pragma omp parallel sections private(private_var) firstprivate(sum)
    {
        #pragma omp section
        {
            private_var = 1;
            checksum += private_var + sum;
        }
        
        #pragma omp section
        {
            private_var = 2;
            checksum += private_var + sum;
        }
    }
    
    /* SIMD directive */
    #pragma omp simd aligned(a, b, c: 16) linear(i:1) safelen(8)
    for (i = 0; i < 100; i++) {
        c[i] = a[i] + b[i];
        checksum += c[i];
    }
}

#ifdef __cplusplus
/* ==================== TREE_BINFO Coverage (C++ only) ==================== */
class Base {
public:
    int a;
    virtual void virt_func() { a = 1; }
    Base() : a(0) {}
};

class Derived : public Base {
public:
    int b;
    virtual void virt_func() override { a = 2; b = 3; }
    Derived() : b(0) {}
};

class Derived2 : public Derived {
public:
    int c;
    virtual void virt_func() override { a = 4; b = 5; c = 6; }
    Derived2() : c(0) {}
};

void test_binfo_nodes() {
    Derived d;
    Derived2 d2;
    
    /* Access base class members */
    d.a = 10;
    d.b = 20;
    
    d2.a = 30;
    d2.b = 40;
    d2.c = 50;
    
    /* Pointer to base class */
    Base* bp = &d;
    bp->virt_func();
    
    bp = &d2;
    bp->virt_func();
    
    /* References */
    Derived& dr = d;
    dr.a = 100;
    
    /* Multiple inheritance-like access patterns */
    checksum += d.a + d.b + d2.a + d2.b + d2.c;
}

/* More complex hierarchy */
struct Base2 {
    int x;
    virtual ~Base2() {}
};

struct Middle : virtual Base, virtual Base2 {
    int y;
};

struct Bottom : Middle {
    int z;
    void test() {
        a = 1;  // From Base
        x = 2;  // From Base2
        y = 3;  // From Middle
        z = 4;
    }
};

void test_virtual_inheritance() {
    Bottom b;
    b.test();
    checksum += b.a + b.x + b.y + b.z;
}
#endif

/* ==================== Main Function ==================== */
int main(void) {
    printf("Starting tree node coverage test...\n");
    
    /* Test all node types */
    test_identifier_nodes();
    printf("  IDENTIFIER_NODE test completed\n");
    
    test_tree_vec();
    printf("  TREE_VEC test completed\n");
    
    checksum += test_ssa_name();
    printf("  SSA_NAME test completed\n");
    
    test_block_nodes();
    printf("  BLOCK test completed\n");
    
    test_constructor_nodes();
    printf("  CONSTRUCTOR test completed\n");
    
    test_omp_clause_nodes();
    printf("  OMP_CLAUSE test completed\n");
    
#ifdef __cplusplus
    test_binfo_nodes();
    printf("  TREE_BINFO test completed\n");
    
    test_virtual_inheritance();
    printf("  Virtual inheritance test completed\n");
#endif
    
    /* Call external functions to create unresolved identifiers */
    checksum += external_func1();
    external_func2(checksum);
    checksum += (int)external_func3(checksum);
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed successfully!\n");
    
    return checksum != 0 ? 0 : 1;
}
