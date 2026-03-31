/* test_tree_nodes.c - Comprehensive test to trigger tree_kind classification */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables with various uses */
int global_var1 = 10;
float global_var2 = 20.5;
char global_var3 = 'A';
double global_var4 = 30.75;

/* Helper function to force identifier usage */
static int use_identifiers(void) __attribute__((noinline));
static int use_identifiers(void) {
    /* Take addresses of globals */
    int *p1 = &global_var1;
    float *p2 = &global_var2;
    char *p3 = &global_var3;
    double *p4 = &global_var4;
    
    /* sizeof expressions */
    size_t s1 = sizeof(global_var1);
    size_t s2 = sizeof(global_var2);
    size_t s3 = sizeof(global_var3);
    size_t s4 = sizeof(global_var4);
    
    /* Use in expressions */
    int result = global_var1 + (int)global_var2 + (int)global_var3 + (int)global_var4;
    
    /* External function declaration forces identifier lookup */
    extern int atoi(const char *);
    extern double atof(const char *);
    
    return result + s1 + s2 + s3 + s4 + (int)(p1 != 0);
}

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

static int use_vectors(void) __attribute__((noinline));
static int use_vectors(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf f3 = f1 * f2;
    
    v2df d1 = {1.0, 2.0};
    v2df d2 = {3.0, 4.0};
    v2df d3 = d1 + d2;
    
    /* Use vectors in function-like context */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += c[i];
        sum += (int)f3[i];
    }
    
    return sum + (int)d3[0] + (int)d3[1];
}
#else
static int use_vectors(void) {
    return 0;
}
#endif

/* Pattern 3: SSA_NAME - Loops that force SSA form */
static int use_ssa(int n) __attribute__((noinline));
static int use_ssa(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Multiple loops creating SSA variables */
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    
    for (int j = 0; j < n * 2; ++j) {
        z = z - j;
        x = x + z;
    }
    
    int w = 0;
    for (int k = 0; k < n; k += 2) {
        w = w + k;
        for (int l = 0; l < k; ++l) {
            w = w - l;
        }
    }
    
    return x + y + z + w;
}

/* Pattern 4: BLOCK - Nested blocks and statement expressions */
static int use_blocks(void) __attribute__((noinline));
static int use_blocks(void) {
    int result = 0;
    
    /* Level 1 block */
    {
        int a = 10;
        
        /* Level 2 block */
        {
            int b = 20;
            
            /* Level 3 block */
            {
                int c = 30;
                result = a + b + c;
                
                /* GCC statement expression creates a block */
                int stmt_expr = ({
                    int temp = a * b;
                    temp + c;
                });
                result += stmt_expr;
            }
        }
    }
    
    /* Another block with label */
    {
        void *label_ptr = &&my_label;
        goto *label_ptr;
        
        my_label:
        result += 100;
    }
    
    /* Switch with blocks */
    switch (result % 3) {
        case 0: {
            int x = 5;
            result += x;
            break;
        }
        case 1: {
            int y = 10;
            result += y;
            break;
        }
        case 2: {
            int z = 15;
            result += z;
            break;
        }
    }
    
    return result;
}

/* Pattern 5: CONSTRUCTOR - Structure and array initializers */
static int use_constructors(void) __attribute__((noinline));
static int use_constructors(void) {
    /* Structure with designated initializer */
    struct Point {
        int x;
        int y;
        float z;
    };
    
    struct Point p1 = { .x = 1, .y = 2, .z = 3.0f };
    struct Point p2 = { 4, 5, 6.0f };
    
    /* Array initializer */
    int arr1[5] = {1, 2, 3, 4, 5};
    int arr2[] = {6, 7, 8, 9, 10};
    
    /* Compound literal */
    int sum = 0;
    sum += ((int[]){1, 2, 3})[0];
    sum += ((int[]){4, 5, 6})[1];
    
    /* Nested structure initializer */
    struct Nested {
        struct Point p;
        int id;
        char name[4];
    };
    
    struct Nested n1 = {
        .p = { .x = 10, .y = 20, .z = 30.0f },
        .id = 100,
        .name = { 'A', 'B', 'C', '\0' }
    };
    
    /* Union initializer */
    union Data {
        int i;
        float f;
        char str[4];
    };
    
    union Data d1 = { .i = 42 };
    union Data d2 = { .f = 3.14f };
    
    return sum + p1.x + p2.y + arr1[0] + arr2[1] + n1.p.x + d1.i + (int)d2.f;
}

/* Pattern 6: OpenMP clauses - OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>

static int use_openmp(int size) __attribute__((noinline));
static int use_openmp(int size) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) schedule(static, 4)
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    int max_val = 0;
    int min_val = 1000;
    
    /* Another parallel region with different clauses */
    #pragma omp parallel sections private(arr) firstprivate(sum) reduction(max:max_val) reduction(min:min_val)
    {
        #pragma omp section
        {
            max_val = sum > max_val ? sum : max_val;
        }
        
        #pragma omp section
        {
            min_val = sum < min_val ? sum : min_val;
        }
    }
    
    /* Single directive with copyprivate */
    int shared_var = 0;
    #pragma omp parallel private(shared_var)
    {
        #pragma omp single copyprivate(shared_var)
        {
            shared_var = 42;
        }
    }
    
    return sum + max_val + min_val + shared_var;
}
#else
static int use_openmp(int size) {
    return size * 2;
}
#endif

#ifdef __cplusplus
} /* extern "C" */

/* Pattern 7: TREE_BINFO - C++ class inheritance (only in C++) */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int get_value() const { return base_value; }
    int base_value = 100;
};

class DerivedClass : public BaseClass {
public:
    virtual int get_value() const override { return derived_value; }
    int derived_value = 200;
};

static int use_binfo(void) __attribute__((noinline));
static int use_binfo(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call through base pointer */
    int val1 = base_ptr->get_value();
    
    /* Access through reference */
    BaseClass& base_ref = derived;
    int val2 = base_ref.get_value();
    
    /* Try dynamic_cast (requires RTTI) */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    int val3 = derived_ptr ? derived_ptr->derived_value : 0;
    
    return val1 + val2 + val3;
}

#endif /* __cplusplus */

/* Main function that combines all patterns */
int main(void) {
    volatile int result = 0;  /* volatile to prevent optimization */
    
    /* Call all pattern functions */
    result += use_identifiers();
    result += use_vectors();
    result += use_ssa(100);
    result += use_blocks();
    result += use_constructors();
    result += use_openmp(50);
    
#ifdef __cplusplus
    result += use_binfo();
#endif
    
    /* Print result to ensure code is live */
#ifdef __cplusplus
    std::cout << "Result: " << result << std::endl;
#else
    printf("Result: %d\n", result);
#endif
    
    return result != 0 ? 0 : 1;
}
