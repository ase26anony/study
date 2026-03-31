/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func(int);
extern volatile int external_var;

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

void use_inheritance() {
    Derived d;
    d.a = 1;  /* This should generate BINFO nodes */
    d.b = 2;
    
    Base* bp = &d;
    bp->vfunc();
    
    DeepDerived dd;
    dd.a = 3;
    dd.b = 4;
    dd.c = 5;
}
#endif

/* Function to create SSA_NAME nodes with complex control flow */
int ssa_test(int n) {
    int i, s = 0, t = 1, u = 2;
    
    /* Complex loop with multiple assignments */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = s - u;
        } else {
            s *= 2 + u;
            u = t ^ s;
        }
        
        /* Nested condition */
        if (s > 100) {
            t = s / 2;
            if (u < 50) {
                u = t * 3;
            }
        }
    }
    
    /* Another loop with switch */
    for (i = 0; i < 10; i++) {
        switch (i % 3) {
            case 0: s += t; break;
            case 1: t *= u; break;
            case 2: u -= s; break;
        }
    }
    
    return s + t + u;
}

/* Function with many blocks and labels for BLOCK nodes */
void block_test() {
    int x = 0;
    
    /* First block with label */
    block1: {
        int y = 1;
        volatile int sink = y;
        x += y;
        goto block3;
    }
    
    /* Second block (unreachable but creates BLOCK node) */
    block2: {
        int z = 2;
        volatile int sink = z;
        x += z;
        goto block4;
    }
    
    /* Third block */
    block3: {
        int a = 3;
        volatile int sink = a;
        x += a;
        goto block5;
    }
    
    /* Fourth block */
    block4: {
        int b = 4;
        volatile int sink = b;
        x += b;
        goto block6;
    }
    
    /* Fifth block with nested block */
    block5: {
        int c = 5;
        {
            int d = 6;
            volatile int sink = d;
            x += c + d;
        }
        goto block7;
    }
    
    /* Sixth block */
    block6: {
        int e = 7;
        volatile int sink = e;
        x += e;
        goto block8;
    }
    
    /* Seventh block with deeply nested blocks */
    block7: {
        int f = 8;
        {
            int g = 9;
            {
                int h = 10;
                volatile int sink = h;
                x += f + g + h;
            }
        }
        goto block9;
    }
    
    /* Eighth block */
    block8: {
        int i = 11;
        volatile int sink = i;
        x += i;
        goto block10;
    }
    
    /* Ninth block */
    block9: {
        int j = 12;
        volatile int sink = j;
        x += j;
        /* Fall through */
    }
    
    /* Tenth block */
    block10: {
        int k = 13;
        volatile int sink = k;
        x += k;
    }
    
    volatile int final_sink = x;
}

/* Function with constructor nodes */
void constructor_test() {
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
    
    /* Partial initialization */
    struct S1 s2 = {
        .b = {[1] = 7}
    };
    
    /* Array with designated initializers */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* Nested struct initialization */
    struct Outer {
        struct Inner {
            int a;
            int b;
        } inner;
        int c;
    } outer = {
        .inner = {.a = 8, .b = 9},
        .c = 10
    };
    
    /* Union initialization */
    union U {
        int i;
        float f;
        char c[4];
    } u1 = {.i = 42}, u2 = {.f = 3.14f}, u3 = {.c = {'a', 'b', 'c', '\0'}};
    
    volatile int sink = s1.a + s2.b[1] + arr[5] + outer.inner.a + u1.i;
}

/* Function with vector extensions for TREE_VEC */
void vector_test() {
    /* Various vector types */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = a + a;
    v4si c = b * a;
    
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = f1 * 2.0f;
    
    v8hi h1 = {1, 2, 3, 4, 5, 6, 7, 8};
    v8hi h2 = h1 >> 1;
    
    /* Array compound literals */
    int *p1 = (int[]){1, 2, 3, 4};
    int *p2 = (int[]){[0] = 5, [3] = 6};
    
    /* Nested vector operations */
    v4si d = (v4si){a[0] + b[0], a[1] * b[1], a[2] - b[2], a[3] / (b[3] ? b[3] : 1)};
    
    volatile int sink = b[0] + c[1] + (int)f2[2] + h2[3] + p1[0] + p2[3] + d[2];
}

/* OpenMP test for OMP_CLAUSE nodes */
void omp_test(int n) {
    int i, sum = 0, private_var = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Multiple OpenMP pragmas with various clauses */
    #pragma omp parallel for private(i) shared(arr, sum) reduction(+:sum) schedule(dynamic, 2) if(n > 1000)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    int local_sum = 0;
    #pragma omp parallel private(private_var) firstprivate(local_sum) copyin(private_var)
    {
        private_var = omp_get_thread_num();
        #pragma omp for nowait ordered schedule(static)
        for (i = 0; i < 50; i++) {
            #pragma omp ordered
            local_sum += i;
        }
    }
    
    /* Another with collapse */
    #pragma omp parallel for collapse(2) ordered(2) \
        lastprivate(private_var) linear(i:2) aligned(arr:16)
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            arr[x * 10 + y] = x * y;
            #pragma omp ordered depend(source)
            private_var = x + y;
            #pragma omp ordered depend(sink: x-1, y)
        }
    }
    
    volatile int sink = sum + local_sum + private_var;
}

/* Function to create many IDENTIFIER_NODE instances */
void identifier_test() {
    int checksum = 0;
    
    /* Deeply nested scopes with same variable names */
    {
        int x = 1;
        volatile int sink1 = x;
        checksum += x;
        
        {
            /* Shadowing x */
            float x = 2.0f;
            volatile int sink2 = (int)x;
            checksum += (int)x;
            
            {
                /* Another shadow */
                char x = 'A';
                volatile int sink3 = x;
                checksum += x;
                
                {
                    /* Pointer to x in outer scope */
                    int* ptr = &((int&)x);
                    volatile int sink4 = *ptr;
                    checksum += *ptr;
                }
            }
        }
    }
    
    /* More shadowing in loops */
    for (int i = 0; i < 3; i++) {
        double i = 3.14 * i;  /* Shadow loop variable */
        volatile int sink5 = (int)i;
        checksum += (int)i;
        
        {
            struct i { int x; };  /* Type name shadows variable */
            struct i my_i = {.x = 5};
            volatile int sink6 = my_i.x;
            checksum += my_i.x;
        }
    }
    
    /* Function scope shadowing */
    {
        /* Multiple declarations with same name */
        auto int counter = 0;
        register int counter asm("eax") = 1;  /* Different storage class */
        volatile int sink7 = counter;
        checksum += counter;
    }
    
    /* Use external identifier */
    checksum += external_var;
    
    volatile int final_sink = checksum;
}

int main() {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* Test 1: IDENTIFIER_NODE */
    identifier_test();
    checksum += 1;
    
    /* Test 2: TREE_VEC */
    vector_test();
    checksum += 2;
    
    /* Test 3: SSA_NAME */
    checksum += ssa_test(100);
    
    /* Test 4: BLOCK */
    block_test();
    checksum += 4;
    
    /* Test 5: CONSTRUCTOR */
    constructor_test();
    checksum += 5;
    
    /* Test 6: OMP_CLAUSE */
    #ifdef _OPENMP
    omp_test(1000);
    checksum += 6;
    #endif
    
    /* Test 7: C++ specific (BINFO) */
    #ifdef __cplusplus
    use_inheritance();
    checksum += 7;
    #endif
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return 0;
}
