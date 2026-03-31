/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(int);
extern void external_func2(void);
extern volatile int external_var;

/* Prevent dead code elimination */
static volatile int sink;

/* Checksum accumulator */
static int checksum = 0;

/* ===== IDENTIFIER_NODE generation ===== */
void test_identifier_nodes(void) {
    /* Deeply nested scopes with same variable names */
    {
        int x = 1;
        checksum += x;
        
        {
            /* Different scope, same name */
            float x = 2.5f;
            checksum += (int)x;
            
            {
                /* Another nested scope */
                volatile int x = 3;
                checksum += x;
                
                {
                    /* Pointer type with same name */
                    int *x = (int[]){4, 5, 6};
                    checksum += x[0];
                }
            }
        }
    }
    
    /* Function parameter shadowing */
    {
        auto int test_shadow(int x) {
            {
                long x = (long)x * 2;  /* Different type, same name */
                return (int)x;
            }
        }
        checksum += test_shadow(10);
    }
    
    /* Multiple extern declarations */
    {
        extern int external_var;
        extern float external_var;  /* Different type */
        volatile int y = external_var;
        sink = y;
    }
}

/* ===== TREE_VEC generation ===== */
void test_tree_vec(void) {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    
    /* Store to volatile to prevent optimization */
    volatile v4si result = c;
    sink = result[0];
    
    /* Array compound literals */
    int *p = (int[]){10, 20, 30, 40};
    checksum += p[0] + p[2];
    
    /* Nested vector operations */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = f1 * (v4sf){2.0f, 2.0f, 2.0f, 2.0f};
    volatile v4sf fresult = f2;
    sink = (int)fresult[0];
}

/* ===== SSA_NAME generation ===== */
int test_ssa_name(int n) {
    /* Complex control flow with many assignments */
    int i, s = 0, t = 1, u = 2;
    
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = u + 1;
        } else {
            s *= 2;
            u = t - 1;
        }
        
        /* Nested condition */
        switch (i % 3) {
            case 0: s += 5; break;
            case 1: s -= 3; t *= 2; break;
            case 2: s /= 2; u += s; break;
        }
    }
    
    /* Another loop with phi node potential */
    int j = n;
    while (j > 0) {
        if (j % 2 == 0) {
            s += j;
            j /= 2;
        } else {
            s -= j;
            j = j * 3 + 1;
        }
    }
    
    return s;
}

/* ===== BLOCK nodes generation ===== */
void test_block_nodes(void) {
    int a = 0;
    
    /* Multiple nested blocks with labels */
    block1: {
        int b = 1;
        checksum += b;
        goto block3;
    }
    
    block2: {
        int c = 2;
        checksum += c;
        goto block4;
    }
    
    block3: {
        int d = 3;
        checksum += d;
        goto block2;
    }
    
    block4: {
        int e = 4;
        checksum += e;
        
        /* Deeply nested block */
        {
            {
                {
                    int f = 5;
                    checksum += f;
                    goto final_block;
                }
            }
        }
    }
    
    final_block:
    a = 10;
    checksum += a;
}

/* ===== CONSTRUCTOR nodes generation ===== */
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
        struct {
            int p;
            int q;
        } inner[2];
        int r;
    };
    
    struct T t1 = {
        .inner[0] = {.p = 1, .q = 2},
        .inner[1].p = 3,
        .r = 4
    };
    checksum += t1.inner[0].p + t1.r;
    
    /* Union initializer */
    union U {
        int i;
        float f;
        char c[4];
    };
    
    union U u1 = {.i = 0x12345678};
    union U u2 = {.c = {'a', 'b', 'c', '\0'}};
    checksum += u1.i & 0xFF;
    checksum += u2.c[0];
}

/* ===== OpenMP OMP_CLAUSE generation ===== */
void test_omp_clause(void) {
    int i, sum = 0;
    int array[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        array[i] = i + 1;
    }
    
    /* Multiple OpenMP pragmas with various clauses */
    #pragma omp parallel for private(i) shared(array, sum) reduction(+:sum) schedule(dynamic, 5)
    for (i = 0; i < 100; i++) {
        sum += array[i];
    }
    checksum += sum;
    
    /* Another OpenMP region with different clauses */
    int max_val = 0;
    #pragma omp parallel num_threads(4) default(none) firstprivate(array) reduction(max:max_val)
    {
        int tid = omp_get_thread_num();
        #pragma omp for collapse(2) ordered
        for (int x = 0; x < 10; x++) {
            for (int y = 0; y < 10; y++) {
                #pragma omp ordered
                {
                    if (array[x * 10 + y] > max_val) {
                        max_val = array[x * 10 + y];
                    }
                }
            }
        }
    }
    checksum += max_val;
    
    /* SIMD directive */
    #pragma omp simd aligned(array:16) linear(i:1) safelen(8)
    for (i = 0; i < 100; i++) {
        array[i] *= 2;
    }
    checksum += array[50];
}

#ifdef __cplusplus
/* ===== C++ specific: TREE_BINFO generation ===== */
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
    
    /* Access through base pointer */
    Base* bp = &d;
    bp->a = 10;
    bp->virt_func();
    
    /* Multiple inheritance-like access */
    bp = &d2;
    bp->virt_func();
    
    /* Casts that require BINFO lookups */
    Derived* dp = static_cast<Derived*>(bp);
    dp->b = 20;
    
    checksum += d.a + d.b + d2.a + d2.c;
}
#endif

/* Main function that runs all tests */
int main(void) {
    printf("Starting tree node coverage test...\n");
    
    /* Run all tests */
    test_identifier_nodes();
    test_tree_vec();
    checksum += test_ssa_name(20);
    test_block_nodes();
    test_constructor_nodes();
    test_omp_clause();
    
    #ifdef __cplusplus
    test_binfo_nodes();
    #endif
    
    /* Final checksum */
    printf("Final checksum: %d\n", checksum);
    
    /* Use external functions to create unresolved identifiers */
    if (external_var > 0) {
        external_func1(checksum);
        external_func2();
    }
    
    return checksum & 0xFF;
}
