/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */
#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* ========== Pattern 1: IDENTIFIER_NODE ========== */
/* Global variables for identifier creation */
int global_var_1 = 10;
int global_var_2 = 20;
float global_float = 3.14;
char global_char = 'A';

/* Function using identifiers in various ways */
__attribute__((noinline))
int identifier_pattern(void) {
    /* Local identifiers */
    int local_int = 5;
    float local_float = 2.71;
    
    /* Operations that create identifier nodes */
    int *ptr1 = &global_var_1;
    int *ptr2 = &local_int;
    
    /* sizeof uses identifiers */
    size_t s1 = sizeof(global_var_1);
    size_t s2 = sizeof(local_float);
    
    /* Complex expression with multiple identifiers */
    int result = global_var_1 * local_int + global_var_2 / 2;
    
    /* Function-like macro that expands to identifiers */
#define USE_IDENT(x) ((x) + 1)
    result = USE_IDENT(global_var_1);
    
    /* Multiple scopes with identifiers */
    {
        int inner_var = 100;
        result += inner_var;
    }
    
    return result;
}

/* ========== Pattern 2: TREE_VEC ========== */
#ifdef __GNUC__
/* Vector type declaration */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

__attribute__((noinline))
int vector_pattern(void) {
    /* Vector variables */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4sf vec_f = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Vector operations */
    v4si vec_c = vec_a + vec_b;
    v4si vec_d = vec_a * vec_b;
    v4si vec_e = vec_c - vec_d;
    
    /* Vector comparisons */
    v4si mask = vec_a > vec_b;
    
    /* Vector shuffle/extract */
    int elem = vec_c[0] + vec_c[1] + vec_c[2] + vec_c[3];
    
    /* Mixed vector operations */
    v4si vec_g = vec_a + 5;
    
    return elem + vec_g[0];
}
#else
__attribute__((noinline))
int vector_pattern(void) {
    /* Fallback for non-GCC */
    int arr[4] = {1, 2, 3, 4};
    return arr[0] + arr[1];
}
#endif

/* ========== Pattern 3: TREE_BINFO (C++ only) ========== */
#ifdef __cplusplus

class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int method() { return 42; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int method() override { return 84; }
    int derived_data;
};

class AnotherDerived : public BaseClass {
public:
    virtual int method() override { return 168; }
    int another_data;
};

__attribute__((noinline))
int binfo_pattern(void) {
    DerivedClass derived_obj;
    BaseClass* base_ptr = &derived_obj;
    
    /* Access through base pointer - involves BINFO */
    int result = base_ptr->method();
    
    /* Create another object */
    AnotherDerived another_obj;
    BaseClass* base_ptr2 = &another_obj;
    result += base_ptr2->method();
    
    /* Cast operations */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        result += derived_ptr->derived_data;
    }
    
    /* Multiple inheritance would create more BINFO nodes */
    return result;
}

#else
/* C version - no BINFO nodes */
__attribute__((noinline))
int binfo_pattern(void) {
    return 0;
}
#endif

/* ========== Pattern 4: SSA_NAME ========== */
__attribute__((noinline))
int ssa_pattern(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Multiple loops creating SSA variables */
    for (int i = 0; i < n; ++i) {
        x = x + i;      /* Creates SSA for x and i */
        y = y * (i + 1);
    }
    
    /* Another loop with different variable */
    for (int j = 0; j < n; ++j) {
        z = z - j;
        x = x + z;
    }
    
    /* Conditional updates creating phi nodes */
    int w = 0;
    for (int k = 0; k < n; ++k) {
        if (k % 2 == 0) {
            w = w + k;
        } else {
            w = w - k;
        }
    }
    
    /* Complex expression with multiple assignments */
    int a = 1, b = 2, c = 3;
    for (int i = 0; i < 10; ++i) {
        a = b + c;
        b = c + a;
        c = a + b;
    }
    
    return x + y + z + w + a + b + c;
}

/* ========== Pattern 5: BLOCK ========== */
__attribute__((noinline))
int block_pattern(void) {
    int result = 0;
    
    /* Nested blocks */
    {
        int block_var1 = 10;
        {
            int block_var2 = 20;
            {
                int block_var3 = 30;
                result = block_var1 + block_var2 + block_var3;
            }
        }
    }
    
    /* GCC statement expression (creates BLOCK nodes) */
    result += ({
        int temp = 5;
        temp * 2;
    });
    
    /* More nested scopes */
    if (result > 0) {
        int if_var = 100;
        {
            int inner_if_var = 200;
            result += if_var + inner_if_var;
        }
    }
    
    /* Switch with blocks */
    switch (result % 3) {
        case 0: {
            int case_var = 300;
            result += case_var;
            break;
        }
        case 1: {
            int case_var = 400;
            result += case_var;
            break;
        }
        default: {
            int case_var = 500;
            result += case_var;
            break;
        }
    }
    
    return result;
}

/* ========== Pattern 6: CONSTRUCTOR ========== */
__attribute__((noinline))
int constructor_pattern(void) {
    /* Structure with designated initializer */
    struct Point {
        int x;
        int y;
        float z;
    };
    
    struct Point p1 = { .x = 1, .y = 2, .z = 3.0 };
    struct Point p2 = { .y = 4, .x = 3, .z = 5.0 };
    
    /* Array initializer */
    int arr1[5] = {1, 2, 3, 4, 5};
    int arr2[] = {6, 7, 8, 9, 10};
    
    /* Nested structure initializer */
    struct Rectangle {
        struct Point top_left;
        struct Point bottom_right;
    };
    
    struct Rectangle rect = {
        .top_left = { .x = 0, .y = 0, .z = 0.0 },
        .bottom_right = { .x = 10, .y = 10, .z = 0.0 }
    };
    
    /* Compound literals */
    int sum = 0;
    sum += ((int[3]){1, 2, 3})[0];
    sum += ((struct Point){ .x = 5, .y = 6, .z = 7.0 }).x;
    
    /* Union initializer */
    union Data {
        int i;
        float f;
        char str[20];
    };
    
    union Data data = { .i = 42 };
    
    /* Complex nested initializer */
    struct Complex {
        int a;
        struct {
            int b;
            int c;
        } inner;
        int d[2];
    };
    
    struct Complex comp = {
        .a = 1,
        .inner = { .b = 2, .c = 3 },
        .d = {4, 5}
    };
    
    return p1.x + p2.y + arr1[0] + arr2[0] + sum + data.i + comp.a;
}

/* ========== Pattern 7: OMP_CLAUSE ========== */
#ifdef _OPENMP
#include <omp.h>

__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int product = 1;
    int* arr = (int*)__builtin_alloca(size * sizeof(int));
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static)
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP directive with different clauses */
    #pragma omp parallel sections private(product)
    {
        #pragma omp section
        {
            product = 1;
            #pragma omp parallel for reduction(*:product)
            for (int i = 1; i <= 5; i++) {
                product *= i;
            }
        }
        
        #pragma omp section
        {
            int local_sum = 0;
            #pragma omp parallel for reduction(+:local_sum) collapse(2)
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    local_sum += i * j;
                }
            }
            sum += local_sum;
        }
    }
    
    /* OpenMP task with clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: arr[0]) firstprivate(size)
            {
                arr[0] = size * 2;
            }
            
            #pragma omp task depend(in: arr[0])
            {
                sum += arr[0];
            }
        }
    }
    
    return sum + product;
}
#else
__attribute__((noinline))
int omp_pattern(int size) {
    /* Fallback without OpenMP */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += i + 1;
    }
    return sum;
}
#endif

/* ========== Main function ========== */
int main(void) {
    volatile int result = 0;
    
    /* Call all pattern functions */
    result += identifier_pattern();
    result += vector_pattern();
    result += binfo_pattern();
    result += ssa_pattern(100);
    result += block_pattern();
    result += constructor_pattern();
    result += omp_pattern(50);
    
    /* Prevent dead code elimination */
#ifdef __cplusplus
    std::cout << "Final result: " << result << std::endl;
#else
    printf("Final result: %d\n", result);
#endif
    
    return result != 0 ? 0 : 1;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
