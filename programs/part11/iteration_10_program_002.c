/* test_tree.c - Comprehensive tree node coverage test */
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
    d->a = 1;        /* Accesses through inheritance */
    dd->b = 2;       /* Accesses through multiple inheritance */
    Base *b = d;     /* Upcast */
    b->vfunc();      /* Virtual call */
}
#endif

/* Function to generate SSA_NAME nodes with complex control flow */
int ssa_generator(int n) {
    int i, j, k, result = 0;
    
    /* Complex loop with multiple conditional updates */
    for (i = 0; i < n; i++) {
        if (i & 1) {
            result += i * 2;
            for (j = 0; j < i; j++) {
                if (j % 3 == 0) {
                    result -= j;
                } else {
                    result += j * 3;
                }
                /* Nested condition */
                k = (j > 5) ? j * 2 : j / 2;
                result ^= k;
            }
        } else {
            result *= 2;
            if (i > n / 2) {
                result /= 3;
            }
        }
        
        /* Switch to add more complexity */
        switch (i % 4) {
            case 0: result += 1; break;
            case 1: result -= 2; break;
            case 2: result *= 3; break;
            case 3: result /= 2; break;
        }
    }
    
    return result;
}

/* Function with deeply nested scopes for IDENTIFIER_NODE */
void identifier_generator(void) {
    int x = 1;
    sink = x;
    
    {
        /* Shadow outer x */
        volatile int x = 2;
        sink = x;
        
        {
            /* Another shadow in deeper scope */
            extern int x;  /* Declaration only */
            volatile int y = x;  /* Use of unresolved identifier */
            sink = y;
            
            {
                /* Function scope shadow */
                int (*x)(void) = external_func1;
                sink = x();
            }
        }
    }
    
    /* Loop scope shadows */
    for (int i = 0; i < 3; i++) {
        int x = i * 10;  /* Different x in loop scope */
        sink = x;
        
        for (int j = 0; j < 2; j++) {
            volatile int x = j * 100;  /* Yet another x */
            sink = x;
        }
    }
    
    /* Switch case scope */
    switch (sink) {
        case 1: {
            int x = 1000;
            sink = x;
            break;
        }
        case 2: {
            volatile int x = 2000;
            sink = x;
            break;
        }
    }
}

/* Function to generate TREE_VEC nodes using GCC extensions */
void vector_generator(void) {
    /* Vector types using GCC extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef float v4sf __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    /* Vector operations */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    /* Mixed vector operations */
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = f * 2.0f;
    
    /* Array compound literals (also generate TREE_VEC) */
    int *arr1 = (int[]){1, 2, 3, 4, 5};
    int *arr2 = (int[3]){10, 20, 30};
    
    /* Nested array initializers */
    int *arr3 = (int[]){[0] = 100, [2] = 200, [4] = 300};
    
    /* Prevent dead code elimination */
    sink = c[0] + d[1] + e[2] + (int)g[0] + arr1[0] + arr2[1] + arr3[2];
}

/* Function with complex blocks for BLOCK nodes */
void block_generator(void) {
    int a = 0;
    
    /* Label and goto for block creation */
    start:
    {
        int b = 1;
        sink = b;
        goto middle;
    }
    
    /* Unreachable block with its own scope */
    {
        int c = 2;
        sink = c;
    }
    
    middle:
    {
        int d = 3;
        {
            int e = 4;
            {
                int f = 5;
                sink = f;
                goto end;
            }
        }
        /* Dead code after goto */
        int g = 6;
        sink = g;
    }
    
    end:
    {
        int h = 7;
        sink = h;
        
        /* Nested blocks with labels */
        inner:
        {
            int i = 8;
            sink = i;
            if (a++ < 3) {
                goto inner;
            }
        }
    }
    
    /* Switch with compound statements */
    switch (a) {
        case 0: {
            int j = 9;
            sink = j;
            break;
        }
        case 1: {
            int k = 10;
            {
                int l = 11;
                sink = l;
            }
            break;
        }
        default: {
            int m = 12;
            sink = m;
        }
    }
}

/* Function to generate CONSTRUCTOR nodes */
void constructor_generator(void) {
    /* Struct with designated initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Nested {
        struct Point p;
        int id;
        float data[4];
    };
    
    /* Various initializers */
    struct Point p1 = { .x = 1, .y = 2, .z = 3 };
    struct Point p2 = { .y = 20, .z = 30 };  /* Partial */
    struct Point p3 = { 100, 200 };          /* Traditional */
    
    /* Nested designated initializers */
    struct Nested n1 = {
        .p = { .x = 1, .y = 2 },
        .id = 100,
        .data = { [0] = 1.0f, [2] = 3.0f }
    };
    
    /* Array with designated initializers */
    int arr1[10] = { [0] = 1, [5] = 2, [9] = 3 };
    int arr2[5] = { 1, 2, [3] = 4, 5 };
    
    /* Union initializers */
    union Data {
        int i;
        float f;
        char str[4];
    };
    
    union Data u1 = { .i = 42 };
    union Data u2 = { .f = 3.14f };
    union Data u3 = { .str = "ABC" };
    
    /* Complex nested initializer */
    struct Complex {
        struct {
            int a;
            int b;
        } inner;
        int arr[3][2];
    };
    
    struct Complex c1 = {
        .inner = { .a = 1, .b = 2 },
        .arr = { {1, 2}, {3, 4}, {[1] = {5, 6}} }
    };
    
    /* Prevent optimization */
    sink = p1.x + p2.y + p3.z + n1.id + arr1[0] + arr2[3] + u1.i + c1.inner.a;
}

/* OpenMP function for OMP_CLAUSE nodes */
void omp_generator(int n) {
    int i, sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    
    /* Multiple OpenMP pragmas with various clauses */
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel private(i) shared(sum, shared_var) \
                         firstprivate(private_var) if(n > 100)
    {
        #pragma omp for reduction(+:sum) schedule(dynamic, 2) \
                         collapse(2) ordered
        for (i = 0; i < n; i++) {
            for (int j = 0; j < 10; j++) {
                #pragma omp ordered
                sum += i * j;
            }
        }
        
        #pragma omp barrier
        
        #pragma omp sections private(private_var) nowait
        {
            #pragma omp section
            {
                private_var = 1;
                #pragma omp atomic
                shared_var += private_var;
            }
            
            #pragma omp section
            {
                private_var = 2;
                #pragma omp atomic
                shared_var += private_var;
            }
        }
        
        #pragma omp single copyprivate(private_var)
        {
            private_var = shared_var;
        }
    }
    
    /* Another parallel region with different clauses */
    #pragma omp parallel for simd aligned(sum:16) \
                         linear(i:1) safelen(8) \
                         lastprivate(private_var)
    for (i = 0; i < n; i++) {
        sum += i;
        private_var = i;
    }
    
    /* Task construct */
    #pragma omp task depend(inout: sum) priority(1) untied mergeable
    {
        sum *= 2;
    }
    
    /* Taskloop */
    #pragma omp taskloop grainsize(10) nogroup num_tasks(4)
    for (i = 0; i < n; i++) {
        #pragma omp atomic update
        sum += 1;
    }
    
    sink = sum + shared_var + private_var;
}

int main(void) {
    int checksum = 0;
    
    printf("Starting tree node coverage test...\n");
    
    /* 1. Generate IDENTIFIER_NODE cases */
    printf("Testing IDENTIFIER_NODE generation...\n");
    identifier_generator();
    checksum += sink;
    
    /* 2. Generate TREE_VEC nodes */
    printf("Testing TREE_VEC generation...\n");
    vector_generator();
    checksum += sink;
    
    /* 3. Generate SSA_NAME nodes */
    printf("Testing SSA_NAME generation...\n");
    sink = ssa_generator(50);
    checksum += sink;
    
    /* 4. Generate BLOCK nodes */
    printf("Testing BLOCK generation...\n");
    block_generator();
    checksum += sink;
    
    /* 5. Generate CONSTRUCTOR nodes */
    printf("Testing CONSTRUCTOR generation...\n");
    constructor_generator();
    checksum += sink;
    
    /* 6. Generate OMP_CLAUSE nodes */
    printf("Testing OMP_CLAUSE generation...\n");
    omp_generator(100);
    checksum += sink;
    
#ifdef __cplusplus
    /* 7. Generate TREE_BINFO nodes (C++ only) */
    printf("Testing TREE_BINFO generation...\n");
    Derived d;
    DeepDerived dd;
    use_hierarchy(&d, &dd);
    checksum += d.a + dd.b;
#endif
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum != 0 ? 0 : 1;
}
