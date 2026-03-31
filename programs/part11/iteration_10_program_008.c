/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(int);
extern void external_func2(void);
extern volatile int external_var;

/* Prevent optimization of results */
static volatile int sink;

#ifdef __cplusplus
/* C++ specific code for BINFO nodes */
struct Base {
    int a;
    virtual void vfunc() {}
};

struct Derived : Base {
    int b;
    void vfunc() override {}
};

struct DeepDerived : Derived {
    int c;
};

void use_hierarchy(Derived *d, DeepDerived *dd) {
    d->a = 1;        /* Accesses through inheritance */
    dd->b = 2;       /* Multiple inheritance levels */
    Base *b = d;     /* Upcast */
    b->vfunc();      /* Virtual call */
}
#endif

/* Function to create complex control flow for SSA */
int ssa_test(int n) {
    int i, s = 0, t = 1;
    
    /* Complex loop with multiple updates */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t++;
        } else {
            s -= i / (t + 1);
            t *= 2;
        }
        
        /* Nested condition */
        switch (i % 3) {
            case 0: s ^= 0xFF; break;
            case 1: s |= 0xAA; break;
            case 2: s &= 0x55; break;
        }
    }
    
    /* Another loop with phi nodes */
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

/* Function with deeply nested scopes for IDENTIFIER_NODE */
void identifier_test(void) {
    int x = 1;
    sink = x;
    
    {
        /* Shadow outer x */
        int x = 2;
        volatile int y = x;
        sink = y;
        
        {
            /* Another shadow in deeper scope */
            extern int x;  /* External declaration */
            volatile int z = x;
            sink = z;
            
            {
                /* Yet another shadow */
                static int x = 3;
                volatile int w = x;
                sink = w;
            }
        }
    }
    
    /* Multiple blocks with same variable names */
    for (int i = 0; i < 2; i++) {
        int counter = i * 10;
        sink = counter;
        
        {
            int counter = i * 20;  /* Shadows loop counter */
            volatile int tmp = counter;
            sink = tmp;
        }
    }
    
    /* Function parameters as identifiers */
    auto lambda = [](int param) -> int {
        int param_copy = param;  /* Shadows parameter */
        return param_copy;
    };
    sink = lambda(42);
}

/* Vector operations for TREE_VEC */
void vector_test(void) {
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    /* Store to volatile to prevent optimization */
    volatile v4si result = e;
    sink = result[0];
    
    /* Float vectors */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = f1 * 2.0f;
    volatile v4sf fresult = f2;
    sink = (int)fresult[0];
    
    /* Array compound literals */
    int *arr1 = (int[]){1, 2, 3, 4, 5};
    int *arr2 = (int[3]){10, 20, 30};
    sink = arr1[0] + arr2[0];
    
    /* Nested array initializers */
    struct Point { int x; int y; };
    struct Point *points = (struct Point[]){
        {1, 2}, {3, 4}, {5, 6}
    };
    sink = points[0].x;
}

/* Block and label tests */
void block_test(void) {
    int a = 0;
    
    /* Deeply nested blocks with labels */
    {
        int b = 1;
    lab1:
        b++;
        {
            int c = 2;
        lab2:
            c++;
            {
                int d = 3;
                goto lab4;  /* Jump forward */
            }
        }
    }
    
    {
        int e = 4;
    lab3:
        e++;
        goto lab5;
    }
    
lab4:
    a = 10;
    goto lab3;
    
lab5:
    a = 20;
    
    /* Switch with nested blocks */
    switch (a) {
        case 10: {
            int x = 100;
            sink = x;
            break;
        }
        case 20: {
            int x = 200;  /* Same name, different scope */
            sink = x;
            break;
        }
        default: {
            int x = 300;
            sink = x;
        }
    }
}

/* Constructor nodes */
void constructor_test(void) {
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
    sink = s1.a + s1.b[0] + s1.nested.x;
    
    /* Partial initialization */
    struct S s2 = {
        .b = {[1] = 5},
        .nested.y = 50
    };
    sink = s2.b[1] + s2.nested.y;
    
    /* Array with designated initializers */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3};
    sink = arr[0] + arr[5] + arr[9];
    
    /* Nested struct initialization */
    struct Outer {
        struct Inner {
            int a;
            int b;
        } inner;
        int c;
    };
    
    struct Outer outer = {
        .inner = {.a = 1, .b = 2},
        .c = 3
    };
    sink = outer.inner.a + outer.c;
    
    /* Union initializer */
    union U {
        int i;
        float f;
        char c[4];
    };
    
    union U u1 = {.i = 0x12345678};
    union U u2 = {.f = 3.14f};
    union U u3 = {.c = {'a', 'b', 'c', '\0'}};
    sink = u1.i + (int)u2.f + u3.c[0];
}

/* OpenMP tests */
void omp_test(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    
    /* Multiple clauses in single pragma */
    #pragma omp parallel for private(i) shared(sum) \
        reduction(+:sum) schedule(dynamic, 2) \
        num_threads(4) if(n > 1000)
    for (i = 0; i < n; i++) {
        sum += i * i;
    }
    sink = sum;
    
    /* Another with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) \
        ordered collapse(2) nowait
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            #pragma omp ordered
            {
                int val = x * 100 + y;
                if (val > max_val) max_val = val;
            }
        }
    }
    sink = max_val;
    
    /* Sections with private/firstprivate */
    #pragma omp parallel sections private(private_var) \
        firstprivate(n)
    {
        #pragma omp section
        {
            private_var = n * 2;
            sink = private_var;
        }
        
        #pragma omp section
        {
            private_var = n * 3;
            sink = private_var;
        }
    }
    
    /* Task with depend clause */
    int a = 0, b = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: a)
        { a = 1; }
        
        #pragma omp task depend(in: a) depend(out: b)
        { b = a + 2; }
        
        #pragma omp task depend(in: b)
        { sink = b; }
    }
}

int main(void) {
    int checksum = 0;
    
    /* Test 1: IDENTIFIER_NODE */
    identifier_test();
    checksum += 1;
    
    /* Test 2: TREE_VEC */
    vector_test();
    checksum += 2;
    
    /* Test 3: BLOCK nodes */
    block_test();
    checksum += 3;
    
    /* Test 4: CONSTRUCTOR nodes */
    constructor_test();
    checksum += 4;
    
    /* Test 5: SSA_NAME nodes (with optimization) */
    checksum += ssa_test(100);
    
    /* Test 6: OMP_CLAUSE nodes */
    omp_test(1000);
    checksum += 6;
    
#ifdef __cplusplus
    /* Test 7: TREE_BINFO nodes (C++ only) */
    Derived d;
    DeepDerived dd;
    use_hierarchy(&d, &dd);
    checksum += 7;
#endif
    
    /* Use external identifiers */
    checksum += external_func1(checksum);
    external_func2();
    checksum += external_var;
    
    printf("Final checksum: %d\n", checksum);
    return checksum & 0xFF;
}
