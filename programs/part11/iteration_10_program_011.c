/* tree_node_coverage.c - Test program to exercise specific GCC tree node types */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifier nodes */
extern int external_func1(void);
extern void external_func2(int);
extern volatile int external_var;

/* Sink function to prevent optimization */
static volatile int sink;

/* Function to create SSA_NAME nodes with complex control flow */
static int create_ssa_names(int n) {
    int i, s = 0, t = 1;
    
    /* Complex loop with multiple assignments to create SSA */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = s % 7;
        } else {
            s *= 2;
            t = (t + i) % 5;
        }
        
        /* Nested condition to create more SSA merge points */
        if (s > 100) {
            s = s / 3;
            t = t * 2;
        } else if (s < 0) {
            s = -s;
            t = t + 1;
        }
    }
    
    /* Another loop with switch to create more SSA complexity */
    for (i = 0; i < n; i++) {
        switch (i % 4) {
            case 0: s += t; break;
            case 1: s -= t; t++; break;
            case 2: s *= t; t--; break;
            case 3: s /= (t ? t : 1); break;
        }
    }
    
    return s;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE creation */
static int create_identifier_nodes(void) {
    int result = 0;
    
    /* Level 1 scope */
    {
        int x = 1;
        result += x;
        
        /* Level 2 scope */
        {
            /* Different x in inner scope */
            float x = 2.5f;
            result += (int)x;
            
            /* Level 3 scope */
            {
                /* Another x with different type */
                volatile int x = 3;
                result += x;
                
                /* Level 4 scope - reference outer x */
                {
                    /* Use all three x variables through pointers/computation */
                    int *ptr = &((int){x});  /* Use compound literal */
                    result += *ptr;
                }
            }
        }
        
        /* Another sibling scope with same name */
        {
            double x = 4.75;
            result += (int)x;
        }
    }
    
    /* Function scope with parameter shadowing */
    {
        auto int func_local(int x) {
            /* Nested block in nested function */
            {
                long x = (long)x * 2;  /* Different type */
                return (int)x;
            }
        }
        result += func_local(5);
    }
    
    return result;
}

/* Function to create TREE_VEC nodes using vector extensions */
static int create_tree_vec_nodes(void) {
    int result = 0;
    
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    /* Store to volatile to prevent optimization */
    volatile v4si cv = c;
    volatile v4si dv = d;
    
    /* Access elements */
    for (int i = 0; i < 4; i++) {
        result += cv[i];
        result += dv[i];
    }
    
    /* More vector operations */
    v4si e = {result, result+1, result+2, result+3};
    v4si f = e > (v4si){10, 10, 10, 10};
    volatile v4si fv = f;
    
    /* Array compound literals (also create TREE_VEC) */
    int *arr1 = (int[]){1, 2, 3, 4, 5};
    int *arr2 = (int[3]){result, result*2, result*3};
    
    for (int i = 0; i < 5; i++) result += arr1[i];
    for (int i = 0; i < 3; i++) result += arr2[i];
    
    /* Nested array initializer */
    int *arr3 = (int[]){[0] = 1, [2] = 3, [4] = 5, [9] = 10};
    for (int i = 0; i < 10; i++) {
        if (arr3[i]) result += arr3[i];
    }
    
    return result;
}

/* Function with complex blocks and labels for BLOCK nodes */
static int create_block_nodes(void) {
    int result = 0;
    int a = 0;
    
    /* Block 1 */
    {
        int b = 1;
        result += b;
        
    block1_label:
        b++;
        if (b < 5) goto block1_label;
    }
    
    /* Block 2 with nested blocks */
    {
        int c = 10;
        
        {
            int d = 20;
        block2_inner_label:
            d += c;
            result += d;
            
            if (d < 50) {
                goto block2_inner_label;
            }
        }
        
    block2_outer_label:
        c *= 2;
        result += c;
        
        {
            int e = 100;
            if (c < 1000) {
                goto block2_outer_label;
            }
            
        block2_deep_label:
            e /= 2;
            result += e;
            if (e > 10) goto block2_deep_label;
        }
    }
    
    /* Block 3 with computed goto (indirect) */
    {
        static void *labels[] = { &&label1, &&label2, &&label3 };
        int i = 0;
        
        goto *labels[i];
        
    label1:
        result += 1;
        i = 1;
        goto *labels[i];
        
    label2:
        result += 2;
        i = 2;
        goto *labels[i];
        
    label3:
        result += 3;
        /* fall through */
    }
    
    return result;
}

/* Function to create CONSTRUCTOR nodes with various initializers */
static int create_constructor_nodes(void) {
    int result = 0;
    
    /* Struct with designated initializers */
    struct S1 {
        int a;
        int b[3];
        struct {
            int x;
            int y;
        } nested;
    };
    
    struct S1 s1 = { 
        .a = 1, 
        .b = {[0] = 2, [2] = 4},
        .nested = {.x = 5, .y = 6}
    };
    result += s1.a + s1.b[0] + s1.b[2] + s1.nested.x + s1.nested.y;
    
    /* Partial array initialization */
    int arr1[10] = {[0] = 1, [5] = 2, [9] = 3};
    for (int i = 0; i < 10; i++) result += arr1[i];
    
    /* Nested designated initializers */
    struct S2 {
        struct {
            int a[2];
            int b;
        } inner;
        int c;
    };
    
    struct S2 s2 = {
        .inner = {
            .a = {[1] = 7},
            .b = 8
        },
        .c = 9
    };
    result += s2.inner.a[0] + s2.inner.a[1] + s2.inner.b + s2.c;
    
    /* Union initializers */
    union U {
        int i;
        float f;
        struct {
            char a;
            char b;
        } chars;
    };
    
    union U u1 = { .i = 100 };
    union U u2 = { .f = 3.14f };
    union U u3 = { .chars = {.a = 'x', .b = 'y'} };
    
    result += u1.i + (int)u2.f + u3.chars.a + u3.chars.b;
    
    /* Array of structs with designated init */
    struct Point {
        int x, y;
    } points[3] = {[0].x = 1, [0].y = 2, [2] = {3, 4}};
    
    for (int i = 0; i < 3; i++) {
        result += points[i].x + points[i].y;
    }
    
    return result;
}

#ifdef __cplusplus
/* C++ specific code for TREE_BINFO nodes */
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
    virtual void vfunc1() override { a = 3; c = 4; }
    virtual void vfunc2() override { b = 5; c = 6; }
};

static int create_binfo_nodes(void) {
    int result = 0;
    Derived d;
    Base1* b1 = &d;
    Base2* b2 = &d;
    
    b1->a = 10;
    b2->b = 20;
    d.c = 30;
    
    b1->vfunc1();
    b2->vfunc2();
    
    result = d.a + d.b + d.c;
    
    /* Multiple inheritance access */
    Derived* pd = static_cast<Derived*>(b1);
    pd->c = 40;
    result += pd->c;
    
    /* Virtual base class (more complex hierarchy) */
    class VirtualBase {
    public:
        int v;
        VirtualBase() : v(100) {}
    };
    
    class WithVirtual : virtual public VirtualBase {
    public:
        int w;
    };
    
    WithVirtual wv;
    result += wv.v + wv.w;
    
    return result;
}
#endif

/* Function with OpenMP pragmas for OMP_CLAUSE nodes */
static int create_omp_clause_nodes(void) {
    int result = 0;
    int i;
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel private(i) shared(result) num_threads(2) if(1)
    {
        int local_sum = 0;
        #pragma omp for schedule(dynamic, 2) reduction(+:local_sum) nowait
        for (i = 0; i < 100; i++) {
            local_sum += i;
        }
        
        #pragma omp atomic
        result += local_sum;
    }
    
    /* Sections with private/firstprivate/lastprivate */
    #pragma omp parallel sections private(i) firstprivate(result) lastprivate(i)
    {
        #pragma omp section
        {
            i = 1;
            result += i * 10;
        }
        
        #pragma omp section
        {
            i = 2;
            result += i * 20;
        }
    }
    
    /* Single construct with copyprivate */
    int shared_var = 0;
    #pragma omp parallel
    {
        int local_var;
        #pragma omp single copyprivate(local_var)
        {
            local_var = 42;
        }
        
        #pragma omp atomic
        shared_var += local_var;
    }
    result += shared_var;
    
    /* Task with depend clauses */
    int a = 0, b = 0, c = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: a)
        { a = 1; }
        
        #pragma omp task depend(in: a) depend(out: b)
        { b = a + 1; }
        
        #pragma omp task depend(in: b) depend(out: c)
        { c = b + 1; }
        
        #pragma omp task depend(in: c)
        { result += a + b + c; }
    }
    
    /* Target offload with map clauses */
    #pragma omp target map(tofrom:result) device(0)
    {
        result *= 2;
    }
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing tree node coverage...\n");
    
    /* 1. Test IDENTIFIER_NODE creation */
    checksum += create_identifier_nodes();
    printf("IDENTIFIER_NODE test: %d\n", checksum);
    
    /* 2. Test TREE_VEC creation */
    checksum += create_tree_vec_nodes();
    printf("TREE_VEC test: %d\n", checksum);
    
    /* 3. Test BLOCK node creation */
    checksum += create_block_nodes();
    printf("BLOCK test: %d\n", checksum);
    
    /* 4. Test CONSTRUCTOR node creation */
    checksum += create_constructor_nodes();
    printf("CONSTRUCTOR test: %d\n", checksum);
    
    /* 5. Test SSA_NAME creation (needs optimization) */
    checksum += create_ssa_names(50);
    printf("SSA_NAME test: %d\n", checksum);
    
    /* 6. Test OMP_CLAUSE creation */
    #ifdef _OPENMP
    checksum += create_omp_clause_nodes();
    printf("OMP_CLAUSE test: %d\n", checksum);
    #else
    printf("OMP_CLAUSE test skipped (no OpenMP support)\n");
    #endif
    
    /* 7. Test TREE_BINFO creation (C++ only) */
    #ifdef __cplusplus
    checksum += create_binfo_nodes();
    printf("TREE_BINFO test: %d\n", checksum);
    #else
    printf("TREE_BINFO test skipped (not C++)\n");
    #endif
    
    /* Use external identifiers */
    checksum += external_var;
    external_func2(checksum);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Prevent dead code elimination */
    sink = checksum;
    
    return checksum != 0 ? 0 : 1;
}
