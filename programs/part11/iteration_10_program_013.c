/* test_tree.c - Comprehensive test for GCC tree node coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(void);
extern void external_func2(int);
extern volatile int external_var;

/* Helper to prevent optimization */
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
    d->a = 1;        // Accesses through BINFO
    dd->b = 2;       // Accesses through inheritance chain
    Base *b = d;     // Upcast
    b->vfunc();      // Virtual call
}
#endif

/* Function to generate SSA_NAME nodes with complex control flow */
int ssa_generator(int n) {
    int i, j, k = 0;
    int result = 0;
    
    /* Complex loop with multiple assignments */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            for (j = 0; j < i; j++) {
                k = k * 2 + j;
                if (k > 1000) {
                    k = k / 3;
                } else {
                    k = k + i - j;
                }
            }
            result += k;
        } else {
            k = k - i * 2;
            result -= k;
        }
        
        /* Switch to create more SSA complexity */
        switch (i % 4) {
            case 0: k = k | 0xFF; break;
            case 1: k = k & 0x0F; break;
            case 2: k = k ^ 0x55; break;
            case 3: k = ~k; break;
        }
    }
    
    /* Multiple return paths */
    if (result > 1000000) {
        return result % 1000;
    } else if (result < -1000000) {
        return -result % 1000;
    }
    return result;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
int identifier_generator(void) {
    int checksum = 0;
    
    /* Level 1 */
    {
        int x = 1;
        checksum += x;
        
        /* Level 2 */
        {
            /* Different x in inner scope */
            float x = 2.5f;
            checksum += (int)x;
            
            /* Level 3 */
            {
                /* Another x */
                volatile int x = 3;
                checksum += x;
                
                /* Level 4 - extern declaration */
                {
                    extern int x;  /* Unresolved identifier */
                    volatile int y = 0;
                    /* Force use of extern x */
                    y = (int)&x;   /* Take address to prevent optimization */
                    checksum += y & 0xFF;
                }
            }
        }
        
        /* Another block reusing x name */
        {
            char x = 'A';
            checksum += x;
        }
    }
    
    /* Function scope with same name */
    {
        /* Use vector type to also trigger TREE_VEC */
        typedef int v4si __attribute__((vector_size(16)));
        v4si x = {1, 2, 3, 4};
        sink = x[0];  /* Use volatile sink */
        checksum += 5;
    }
    
    return checksum;
}

/* Function to generate TREE_VEC nodes */
int vector_generator(void) {
    int checksum = 0;
    
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    /* Use all elements to prevent optimization */
    for (int i = 0; i < 4; i++) {
        checksum += c[i];
        checksum += d[i];
        checksum += e[i];
    }
    
    /* Float vectors */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = f1 * 2.0f;
    sink = (int)f2[0];
    
    /* Array compound literals */
    int *arr1 = (int[]){1, 2, 3, 4, 5};
    int *arr2 = (int[]){[0] = 10, [2] = 20, [4] = 30};
    
    for (int i = 0; i < 5; i++) {
        checksum += arr1[i];
        if (i < 5) checksum += arr2[i];
    }
    
    /* Nested array initializer with designators */
    struct Point {
        int x;
        int y;
    };
    
    struct Point points[3] = {
        [0] = {.x = 1, .y = 2},
        [2] = {.x = 5, .y = 6}
    };
    
    checksum += points[0].x + points[2].y;
    
    return checksum;
}

/* Function with complex blocks for BLOCK nodes */
int block_generator(void) {
    int checksum = 0;
    int a = 0;
    
    /* Block 1 */
    {
        int b = 1;
        lab1:
        a += b;
        
        /* Nested block */
        {
            int c = 2;
            goto lab3;  /* Jump forward */
            
            lab2:
            c *= 2;
            a += c;
        }
        
        lab3:
        b++;
        goto lab4;
    }
    
    /* Block 2 */
    {
        int d = 3;
        lab4:
        a += d;
        
        /* Deeply nested blocks with labels */
        {
            {
                {
                    lab5:
                    d *= 2;
                    goto lab6;
                }
            }
        }
        
        goto lab2;  /* Jump backward - will need to create BLOCK nodes */
    }
    
    lab6:
    checksum = a;
    return checksum;
}

/* Function to generate CONSTRUCTOR nodes */
int constructor_generator(void) {
    int checksum = 0;
    
    /* Complex struct with nested arrays */
    struct Nested {
        int a;
        struct {
            int x;
            int y;
        } inner;
        int arr[4];
    };
    
    /* Designated initializers with nesting */
    struct Nested n1 = {
        .a = 1,
        .inner = {
            .x = 2,
            .y = 3
        },
        .arr = {[0] = 4, [2] = 5, [3] = 6}
    };
    
    checksum += n1.a + n1.inner.x + n1.inner.y;
    for (int i = 0; i < 4; i++) {
        checksum += n1.arr[i];
    }
    
    /* Partial initialization */
    struct Nested n2 = {
        .inner.y = 7,
        .arr = {[1] = 8}
    };
    checksum += n2.inner.y + n2.arr[1];
    
    /* Union with designated initializer */
    union U {
        int i;
        float f;
        struct {
            short a;
            short b;
        } s;
    };
    
    union U u1 = {.i = 100};
    union U u2 = {.f = 3.14f};
    union U u3 = {.s = {.a = 1, .b = 2}};
    
    checksum += u1.i + (int)u2.f + u3.s.a + u3.s.b;
    
    /* Array of structs with designators */
    struct Point {
        int x;
        int y;
    } points[4] = {
        [0] = {.x = 1, .y = 2},
        [2] = {.x = 3},
        [3] = {.y = 4, .x = 5}
    };
    
    for (int i = 0; i < 4; i++) {
        checksum += points[i].x + points[i].y;
    }
    
    return checksum;
}

/* Function with OpenMP pragmas for OMP_CLAUSE nodes */
int omp_generator(int n) {
    int sum = 0;
    int i;
    
    /* Complex OpenMP pragma with many clauses */
    #pragma omp parallel for private(i) shared(sum) \
            reduction(+:sum) schedule(dynamic, 2) \
            num_threads(4) if(n > 1000)
    for (i = 0; i < n; i++) {
        sum += i * i;
    }
    
    int arr[100];
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Another OpenMP with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) \
            collapse(2) ordered
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            #pragma omp ordered
            {
                int val = arr[x * 10 + y];
                if (val > max_val) {
                    max_val = val;
                }
            }
        }
    }
    
    /* OpenMP sections */
    int section_result = 0;
    #pragma omp parallel sections private(i) \
            firstprivate(n) lastprivate(section_result)
    {
        #pragma omp section
        {
            for (i = 0; i < n/2; i++) {
                section_result += i;
            }
        }
        
        #pragma omp section
        {
            for (i = n/2; i < n; i++) {
                section_result += i * 2;
            }
        }
    }
    
    return sum + max_val + section_result;
}

int main(void) {
    int total_checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* Test 1: IDENTIFIER_NODE generation */
    printf("Testing IDENTIFIER_NODE...\n");
    total_checksum += identifier_generator();
    
    /* Test 2: TREE_VEC generation */
    printf("Testing TREE_VEC...\n");
    total_checksum += vector_generator();
    
    /* Test 3: SSA_NAME generation (with optimization) */
    printf("Testing SSA_NAME...\n");
    total_checksum += ssa_generator(100);
    
    /* Test 4: BLOCK node generation */
    printf("Testing BLOCK...\n");
    total_checksum += block_generator();
    
    /* Test 5: CONSTRUCTOR node generation */
    printf("Testing CONSTRUCTOR...\n");
    total_checksum += constructor_generator();
    
    /* Test 6: OMP_CLAUSE generation */
    printf("Testing OMP_CLAUSE...\n");
    total_checksum += omp_generator(500);
    
    /* Test 7: Call external functions for unresolved identifiers */
    printf("Testing external identifiers...\n");
    sink = external_var;
    external_func2(sink);
    
#ifdef __cplusplus
    /* Test 8: TREE_BINFO generation (C++ only) */
    printf("Testing TREE_BINFO (C++ only)...\n");
    Derived d;
    DeepDerived dd;
    use_hierarchy(&d, &dd);
    total_checksum += d.a + dd.b;
#endif
    
    printf("Total checksum: %d\n", total_checksum);
    printf("Test completed.\n");
    
    return total_checksum != 0 ? 0 : 1;
}
