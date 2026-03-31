/* tree_node_coverage.c - Test program to trigger specific tree node creation */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(int);
extern void external_func2(void);
extern volatile int global_ext_var;

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
    void vfunc() override {}
};

void use_hierarchy(Base *b) {
    b->a = 1;
    b->vfunc();
}
#endif

/* Function to create SSA_NAME nodes with complex control flow */
int create_ssa_names(int n) {
    int i, s = 0, t = 1, u = 2;
    
    /* Complex loop with multiple branches */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t = u ^ i;
        } else {
            s *= 2 + u;
            u = t + i;
        }
        
        /* Nested condition */
        if (s > 100) {
            t = s % 7;
            if (u < 50) {
                u = u * 2 + 1;
            }
        } else {
            u = u / 2;
        }
    }
    
    /* Another loop with switch */
    for (i = 0; i < n; i++) {
        switch (i % 4) {
            case 0: s += t; break;
            case 1: s -= u; break;
            case 2: s *= 2; t++; break;
            case 3: s /= 2; u--; break;
        }
    }
    
    return s + t + u;
}

/* Function with deeply nested blocks and labels */
void create_blocks(void) {
    int level1 = 0;
    
    /* Level 1 block with label */
    block1: {
        int a = 1;
        volatile int prevent_merge1 = a;
        
        /* Level 2 block */
        {
            int a = 2;  /* Same name, different scope */
            volatile int prevent_merge2 = a;
            
            /* Level 3 block with label */
            block3: {
                int a = 3;  /* Another same name */
                volatile int prevent_merge3 = a;
                goto block4;
            }
            
            /* Unreachable but creates tree nodes */
            int unreachable1 = 99;
        }
    }
    
    block4: {
        int b = 4;
        volatile int prevent_merge4 = b;
        
        /* Jump back */
        if (level1++ < 3) {
            goto block1;
        }
    }
    
    /* Final block with computed goto simulation */
    {
        void *labels[] = { &&lab1, &&lab2, &&lab3 };
        int choice = level1 % 3;
        goto *labels[choice];
        
        lab1: sink = 1; goto end;
        lab2: sink = 2; goto end;
        lab3: sink = 3; goto end;
        end: ;
    }
}

/* Function to create CONSTRUCTOR nodes */
struct ComplexStruct {
    int x;
    int y;
    struct {
        int a;
        int b;
        int c;
    } nested;
    int arr[5];
};

union MixedUnion {
    int i;
    float f;
    char bytes[4];
    struct {
        short s1;
        short s2;
    } shorts;
};

void create_constructors(void) {
    /* Designated initializers with nesting */
    struct ComplexStruct cs1 = {
        .x = 1,
        .y = 2,
        .nested = {
            .a = 10,
            .b = 20,
            .c = 30
        },
        .arr = { [0] = 100, [2] = 200, [4] = 400 }
    };
    
    /* Partial initialization */
    struct ComplexStruct cs2 = {
        .nested.b = 99,
        .arr = { [1] = 111, [3] = 333 }
    };
    
    /* Union initializers */
    union MixedUnion u1 = { .i = 0x12345678 };
    union MixedUnion u2 = { .f = 3.14f };
    union MixedUnion u3 = { .shorts = { .s1 = 0xAAAA, .s2 = 0xBBBB } };
    union MixedUnion u4 = { .bytes = { 'a', 'b', 'c', 'd' } };
    
    /* Array with designated range (GCC extension) */
    int arr[10] = { [0 ... 4] = 1, [5 ... 9] = 2 };
    
    /* Nested array in struct */
    struct {
        int matrix[3][3];
    } mat = { .matrix = { {1,2,3}, {4,5,6}, {7,8,9} } };
    
    sink = cs1.x + cs2.nested.b + u1.i + arr[3] + mat.matrix[1][1];
}

/* Function using GCC vector extensions */
void create_tree_vec(void) {
    /* Basic vector types */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    /* Vector operations */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    /* Mixed operations */
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = f * 2.0f;
    
    /* Array compound literals */
    int *p1 = (int[]){1, 2, 3, 4, 5};
    int *p2 = (int[]){[0] = 10, [2] = 30, [4] = 50};
    
    /* Nested compound literals */
    struct {
        int *ptr;
        int val;
    } s1 = { .ptr = (int[]){100, 200}, .val = 300 };
    
    /* Vector with compound literal */
    v4si v_lit = (v4si){9, 8, 7, 6};
    
    /* Complex vector expression */
    v4si result = (a + b) * (c - d) + v_lit;
    
    sink = result[0] + p1[1] + p2[2] + s1.val;
}

/* OpenMP section for OMP_CLAUSE nodes */
void create_omp_clauses(void) {
    int i, sum = 0;
    int array[100];
    int private_var = 0;
    static int shared_var = 0;
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        array[i] = i;
    }
    
    /* Multiple OpenMP pragmas with different clauses */
    
    /* Parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(array, shared_var) reduction(+:sum) schedule(dynamic, 4) if(1000 > 100)
    for (i = 0; i < 100; i++) {
        sum += array[i];
        shared_var++;
    }
    
    /* Parallel region with firstprivate/lastprivate */
    #pragma omp parallel firstprivate(private_var) lastprivate(private_var) num_threads(4)
    {
        private_var = omp_get_thread_num();
        #pragma omp critical
        {
            shared_var += private_var;
        }
    }
    
    /* Sections with nowait */
    #pragma omp parallel
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            {
                for (i = 0; i < 50; i++) {
                    array[i] *= 2;
                }
            }
            #pragma omp section
            {
                for (i = 50; i < 100; i++) {
                    array[i] /= 2;
                }
            }
        }
    }
    
    /* Single with copyprivate */
    #pragma omp parallel
    {
        int local_val = 0;
        #pragma omp single copyprivate(local_val)
        {
            local_val = 42;
        }
        /* local_val should be 42 in all threads */
        sum += local_val;
    }
    
    /* Task with depend clauses */
    int x = 0, y = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: x)
        { x = 1; }
        
        #pragma omp task depend(in: x) depend(out: y)
        { y = x + 1; }
        
        #pragma omp task depend(in: y)
        { sum += y; }
    }
    
    sink = sum + shared_var;
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    
    printf("Testing tree node coverage...\n");
    
    /* 1. Create IDENTIFIER_NODE cases with name reuse */
    {
        int x = 1;
        volatile int y = x;
        
        {
            /* Same name in inner scope */
            int x = 2;
            volatile int z = x;
            
            {
                /* And another */
                extern int x;  /* External declaration */
                volatile int w = x;
                checksum += w;
            }
            
            checksum += z;
        }
        
        /* Function scope reuse */
        {
            auto int x = 3;  /* C++ auto or C auto storage */
            volatile int v = x;
            checksum += v;
        }
        
        checksum += y;
    }
    
    /* 2. Create TREE_VEC nodes */
    create_tree_vec();
    checksum += sink;
    
    /* 3. Create SSA_NAME nodes */
    checksum += create_ssa_names(20);
    
    /* 4. Create BLOCK nodes */
    create_blocks();
    checksum += sink;
    
    /* 5. Create CONSTRUCTOR nodes */
    create_constructors();
    checksum += sink;
    
    /* 6. Create OMP_CLAUSE nodes */
    #ifdef _OPENMP
    create_omp_clauses();
    checksum += sink;
    #endif
    
    /* 7. Use external identifiers */
    checksum += external_func1(checksum);
    external_func2();
    checksum += global_ext_var;
    
    /* 8. C++ specific: BINFO nodes */
    #ifdef __cplusplus
    {
        Derived d;
        DeepDerived dd;
        Base *b1 = &d;
        Base *b2 = &dd;
        
        use_hierarchy(b1);
        use_hierarchy(b2);
        
        checksum += d.a + dd.b;
    }
    #endif
    
    printf("Final checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
