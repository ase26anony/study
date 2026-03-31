/* test_tree.c - Comprehensive tree node coverage test */
#include <stdio.h>
#include <stdlib.h>

/* External declarations to create unresolved identifiers */
extern int external_func1(int);
extern void external_func2(void);
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

void use_binfo(Derived *d) {
    d->a = 1;        /* Accesses base member */
    d->b = 2;
    Base *b = d;     /* Upcast */
    b->vfunc();      /* Virtual call */
}
#endif

/* Function to force SSA_NAME creation */
int ssa_test(int n) {
    int i, s = 0, t = 1;
    
    /* Complex control flow for SSA */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            s += i * t;
            t *= 2;
        } else {
            s -= i / (t + 1);
            t = t > 10 ? 1 : t + 1;
        }
        
        /* Nested condition */
        switch (i % 3) {
            case 0: s += 5; break;
            case 1: s *= 2; break;
            case 2: s = s > 100 ? s / 2 : s * 3; break;
        }
    }
    
    /* Another loop with phi nodes */
    int j = 0;
    while (j < n) {
        s += (j % 2 == 0) ? j : -j;
        j++;
    }
    
    return s;
}

/* Function with blocks and labels */
void block_test(void) {
    int a = 0;
    
    /* Outer block with label */
    outer_block: {
        int b = 1;
        a += b;
        
        /* Inner block */
        {
            int c = 2;
            volatile int d = c;  /* Prevent merging */
            a += d;
            goto inner_label;    /* Jump to another block */
        }
        
        /* Unreachable code - creates interesting CFG */
        a = 999;
    }
    
    inner_label: {
        int e = 3;
        a += e;
        
        /* Jump back */
        if (a < 100) {
            goto outer_block;
        }
    }
    
    sink = a;
}

/* Constructor node tests */
struct ComplexStruct {
    int a;
    int b[4];
    struct {
        int x;
        int y;
    } nested;
    union {
        int u1;
        float u2;
    } u;
};

struct BitFieldStruct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    int d;
};

/* Union with constructor */
union TestUnion {
    int ival;
    float fval;
    char carr[4];
};

int main(void) {
    int checksum = 0;
    
    /* ============================== */
    /* 1. IDENTIFIER_NODE generation  */
    /* ============================== */
    {
        /* Deeply nested scopes with same variable names */
        int x = 1;
        checksum += x;
        
        {
            /* Different scope, same name */
            float x = 2.5f;
            checksum += (int)x;
            
            {
                /* Pointer with same name */
                volatile int *x = (int[]){1, 2, 3};
                checksum += x[0];
                
                {
                    /* Array with same name */
                    char x[] = "test";
                    checksum += x[0];
                    
                    {
                        /* External declaration with same name */
                        extern int external_x;  /* Unresolved identifier */
                        volatile int y = 10;
                        checksum += y;
                    }
                }
            }
        }
        
        /* Function scope test */
        {
            auto int func_local(void) {
                static int counter = 0;
                int x = counter++;  /* Different x in function scope */
                return x;
            }
            checksum += func_local();
        }
    }
    
    /* ============================== */
    /* 2. TREE_VEC node generation    */
    /* ============================== */
    {
        /* GCC vector extensions */
        typedef int v4si __attribute__((vector_size(16)));
        typedef float v4sf __attribute__((vector_size(16)));
        
        v4si a = {1, 2, 3, 4};
        v4si b = {5, 6, 7, 8};
        v4si c = a + b;
        v4si d = a * b;
        
        /* Use vector elements */
        for (int i = 0; i < 4; i++) {
            checksum += c[i];
            checksum += d[i];
        }
        
        /* Float vectors */
        v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
        v4sf fb = {0.5f, 1.5f, 2.5f, 3.5f};
        v4sf fc = fa * fb;
        
        /* Array compound literals */
        int *p = (int[]){10, 20, 30, 40};
        checksum += p[0] + p[2];
        
        /* Nested array initializer */
        int arr[2][3] = {{1, 2, 3}, {4, 5, 6}};
        checksum += arr[1][2];
    }
    
    /* ============================== */
    /* 3. TREE_BINFO node generation  */
    /* ============================== */
#ifdef __cplusplus
    {
        Derived d;
        DeepDerived dd;
        
        use_binfo(&d);
        
        /* Multiple inheritance access */
        dd.a = 1;    /* Base::a */
        dd.b = 2;    /* Derived::b */
        dd.c = 3;    /* DeepDerived::c */
        
        checksum += dd.a + dd.b + dd.c;
        
        /* Pointer casts through hierarchy */
        Base *bp = &dd;
        Derived *dp = &dd;
        checksum += (bp != NULL);
        checksum += (dp != NULL);
    }
#endif
    
    /* ============================== */
    /* 4. SSA_NAME node generation    */
    /* ============================== */
    {
        int result = ssa_test(20);
        checksum += result;
        
        /* Additional SSA patterns */
        int x = 0, y = 1, z = 2;
        
        /* Complex conditional updates */
        for (int i = 0; i < 10; i++) {
            if (i % 3 == 0) {
                x = y + z;
                y = x * 2;
            } else if (i % 3 == 1) {
                z = x - y;
                x = z / 2;
            } else {
                y = z * x;
                z = y + 1;
            }
            
            /* Switch with variable modifications */
            switch (i % 4) {
                case 0: x++; break;
                case 1: y += x; break;
                case 2: z = x * y; break;
                case 3: x = y = z = (x + y + z); break;
            }
        }
        
        checksum += x + y + z;
    }
    
    /* ============================== */
    /* 5. BLOCK node generation       */
    /* ============================== */
    {
        block_test();
        
        /* Additional block structures */
        {
            int block_var1 = 1;
            label1: {
                int block_var2 = 2;
                checksum += block_var1 + block_var2;
                
                {
                    int block_var3 = 3;
                    if (checksum < 1000) {
                        goto label2;  /* Jump to different block */
                    }
                    checksum += block_var3;
                }
            }
            
            label2: {
                int block_var4 = 4;
                checksum += block_var4;
                
                /* Computed goto (GCC extension) */
                static void *jump_table[] = { &&case0, &&case1, &&case2 };
                int selector = checksum % 3;
                goto *jump_table[selector];
                
                case0: checksum += 10; goto end;
                case1: checksum += 20; goto end;
                case2: checksum += 30; goto end;
            }
        }
        end: ;
    }
    
    /* ============================== */
    /* 6. CONSTRUCTOR node generation */
    /* ============================== */
    {
        /* Designated initializers */
        struct ComplexStruct cs = {
            .a = 1,
            .b = {[0] = 10, [2] = 30, [3] = 40},  /* Partial array init */
            .nested = { .x = 100, .y = 200 },
            .u = { .u2 = 3.14f }
        };
        checksum += cs.a + cs.b[2] + cs.nested.x;
        
        /* Bitfield constructor */
        struct BitFieldStruct bfs = {
            .a = 5,     /* 3 bits */
            .b = 20,    /* 5 bits */
            .c = 100,   /* 8 bits */
            .d = -1
        };
        checksum += bfs.a + bfs.b + bfs.c + bfs.d;
        
        /* Union constructor */
        union TestUnion tu1 = { .ival = 12345 };
        union TestUnion tu2 = { .fval = 2.718f };
        union TestUnion tu3 = { .carr = "ABC" };
        
        checksum += tu1.ival + (int)tu2.fval + tu3.carr[0];
        
        /* Nested constructors */
        struct {
            struct {
                int a;
                int b;
            } inner[2];
            int outer;
        } nested_struct = {
            .inner = {[0] = {.a = 1, .b = 2}, [1] = {.a = 3}},
            .outer = 99
        };
        checksum += nested_struct.inner[1].a + nested_struct.outer;
        
        /* Zero initialization */
        struct ComplexStruct zero = {0};
        checksum += zero.a;  /* Should be 0 */
    }
    
    /* ============================== */
    /* 7. OMP_CLAUSE node generation  */
    /* ============================== */
#ifdef _OPENMP
    {
        int i, sum = 0;
        int array[100];
        
        /* Initialize array */
        for (i = 0; i < 100; i++) {
            array[i] = i + 1;
        }
        
        /* OpenMP with multiple clauses */
        #pragma omp parallel for private(i) shared(array) reduction(+:sum) \
                schedule(dynamic, 4) num_threads(2) if(checksum > 0)
        for (i = 0; i < 100; i++) {
            sum += array[i];
        }
        checksum += sum;
        
        /* More OpenMP constructs */
        #pragma omp parallel
        {
            #pragma omp single nowait
            {
                checksum += 1;
            }
            
            #pragma omp for collapse(2) ordered
            for (int x = 0; x < 10; x++) {
                for (int y = 0; y < 10; y++) {
                    #pragma omp ordered
                    checksum += x * y;
                }
            }
            
            #pragma omp sections
            {
                #pragma omp section
                { checksum += 100; }
                
                #pragma omp section
                { checksum += 200; }
            }
        }
        
        /* OpenMP task with dependencies */
        int a = 0, b = 0;
        #pragma omp parallel
        #pragma omp single
        {
            #pragma omp task depend(out: a)
            { a = 1; }
            
            #pragma omp task depend(in: a) depend(out: b)
            { b = a + 1; }
            
            #pragma omp task depend(in: b)
            { checksum += b; }
        }
    }
#endif
    
    /* Final checksum output */
    printf("Final checksum: %d\n", checksum);
    
    /* Use external identifiers */
    if (external_var > 0) {
        checksum = external_func1(checksum);
    }
    
    return checksum % 256;
}
