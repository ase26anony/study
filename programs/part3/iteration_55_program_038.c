/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* ========== IDENTIFIER_NODE patterns ========== */
/* Global variables for identifier nodes */
int global_var_1;
float global_var_2;
double global_var_3;
char global_var_4;

/* Function declarations using identifiers */
extern int external_func_1(int);
extern void external_func_2(float);
extern double external_func_3(void);

/* Function that uses identifiers in various ways */
__attribute__((noinline))
int identifier_pattern(void) {
    /* Local variables */
    int local_var_1 = 1;
    float local_var_2 = 2.0f;
    double local_var_3 = 3.0;
    
    /* Operations that create identifier nodes */
    int *ptr1 = &global_var_1;
    float *ptr2 = &global_var_2;
    size_t sz1 = sizeof(global_var_3);
    size_t sz2 = sizeof(local_var_1);
    
    /* Using identifiers in expressions */
    global_var_1 = local_var_1 * 2;
    global_var_2 = local_var_2 + 1.0f;
    
    /* Function calls with identifiers */
    int result = external_func_1(global_var_1);
    external_func_2(global_var_2);
    
    return result + local_var_1 + (int)local_var_2;
}

/* ========== TREE_VEC patterns ========== */
#ifdef __GNUC__
/* Vector type declaration */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

__attribute__((noinline))
int vector_pattern(void) {
    /* Vector variables */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Vector operations */
    v4si vec_sum = vec1 + vec2;
    v4si vec_mul = vec1 * vec2;
    v4sf fvec_sum = fvec1 + fvec2;
    v4sf fvec_mul = fvec1 * fvec2;
    
    /* Extract elements */
    int sum = vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3];
    sum += (int)fvec_sum[0] + (int)fvec_sum[1];
    
    return sum;
}
#else
__attribute__((noinline))
int vector_pattern(void) {
    /* Fallback for non-GCC compilers */
    int arr[4] = {1, 2, 3, 4};
    return arr[0] + arr[1] + arr[2] + arr[3];
}
#endif

/* ========== SSA_NAME patterns ========== */
__attribute__((noinline))
int ssa_pattern(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Multiple loops to create SSA names */
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    
    for (int j = 0; j < n * 2; ++j) {
        z = z - j;
        x = x + z;
    }
    
    /* Complex control flow */
    int w = 0;
    for (int k = 0; k < n; ++k) {
        if (k % 2 == 0) {
            w = w + k;
        } else {
            w = w - k;
        }
    }
    
    return x + y + z + w;
}

/* ========== BLOCK patterns ========== */
__attribute__((noinline))
int block_pattern(void) {
    int result = 0;
    
    /* Nested blocks */
    {
        int a = 10;
        {
            int b = 20;
            {
                int c = 30;
                result = a + b + c;
            }
        }
    }
    
    /* GCC statement expression (creates a block) */
    int val = ({
        int temp = 5;
        temp * 2;
    });
    result += val;
    
    /* Labels and gotos (involve block nodes) */
    void *label_ptr = &&my_label;
    
    if (result > 0) {
        goto *label_ptr;
    }
    
    return result;
    
my_label:
    return result * 2;
}

/* ========== CONSTRUCTOR patterns ========== */
__attribute__((noinline))
int constructor_pattern(void) {
    /* Structure with designated initializer */
    struct Point {
        int x;
        int y;
        float z;
    };
    
    struct Point p1 = { .x = 1, .y = 2, .z = 3.0f };
    struct Point p2 = { .y = 4, .x = 3, .z = 5.0f };
    
    /* Array initializer */
    int arr1[5] = {1, 2, 3, 4, 5};
    int arr2[] = {6, 7, 8, 9, 10};
    
    /* Compound literals */
    int sum = ((int[3]){p1.x, p2.x, arr1[0]})[0] +
              ((int[3]){p1.y, p2.y, arr2[0]})[1];
    
    /* Nested initializers */
    struct Nested {
        struct Point p;
        int values[3];
    };
    
    struct Nested n = {
        .p = { .x = 10, .y = 20, .z = 30.0f },
        .values = {100, 200, 300}
    };
    
    sum += n.p.x + n.values[1];
    
    return sum;
}

/* ========== OMP_CLAUSE patterns ========== */
#ifdef _OPENMP
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) schedule(static)
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP directive with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) collapse(2)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int idx = i * 10 + j;
            if (arr[idx] > max_val) {
                max_val = arr[idx];
            }
        }
    }
    
    /* OpenMP sections */
    #pragma omp parallel sections private(arr)
    {
        #pragma omp section
        {
            arr[0] = 1;
        }
        #pragma omp section
        {
            arr[1] = 2;
        }
    }
    
    return sum + max_val;
}
#else
__attribute__((noinline))
int omp_pattern(int size) {
    /* Fallback without OpenMP */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += i;
    }
    return sum;
}
#endif

#ifdef __cplusplus
} /* extern "C" */

/* ========== TREE_BINFO patterns (C++ only) ========== */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int method() { return 1; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int method() override { return 2; }
    int derived_data;
};

__attribute__((noinline))
int binfo_pattern(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Use polymorphism */
    int result = base_ptr->method();
    
    /* Access through base pointer */
    base_ptr->base_data = 10;
    
    /* Dynamic cast (involves BINFO) */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        derived_ptr->derived_data = 20;
        result += derived_ptr->derived_data;
    }
    
    return result + base_ptr->base_data;
}
#endif

/* ========== Main function ========== */
int main(void) {
    volatile int total = 0;
    
    /* Call all pattern functions */
    total += identifier_pattern();
    total += vector_pattern();
    total += ssa_pattern(10);
    total += block_pattern();
    total += constructor_pattern();
    total += omp_pattern(100);
    
#ifdef __cplusplus
    total += binfo_pattern();
    std::cout << "Total: " << total << std::endl;
#else
    printf("Total: %d\n", total);
#endif
    
    return total > 0 ? 0 : 1;
}
