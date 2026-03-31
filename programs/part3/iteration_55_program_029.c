/* test_tree_coverage.c - Comprehensive test to trigger tree_kind coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* ========== PATTERN 1: IDENTIFIER_NODE ========== */
/* Global variables to force identifier creation */
int global_var_1;
float global_var_2;
double global_var_3;
char global_var_4;

/* Function declarations that require identifier lookup */
extern int external_func_1(int);
extern void external_func_2(float);
extern double external_func_3(void);

__attribute__((noinline))
int pattern_identifiers(void) {
    /* Local variables with distinct names */
    int local_identifier_a;
    float local_identifier_b;
    double local_identifier_c;
    
    /* Operations that create IDENTIFIER_NODE trees */
    local_identifier_a = sizeof(global_var_1);
    local_identifier_b = sizeof(global_var_2);
    local_identifier_c = sizeof(global_var_3);
    
    /* Taking addresses */
    void *addr1 = &global_var_1;
    void *addr2 = &global_var_2;
    void *addr3 = &local_identifier_a;
    
    /* Using in expressions */
    local_identifier_a = global_var_1 + 1;
    local_identifier_b = global_var_2 * 2.0f;
    
    /* Reference to external functions */
    if (external_func_1) {}
    if (external_func_2) {}
    if (external_func_3) {}
    
    return local_identifier_a + (int)local_identifier_b;
}

/* ========== PATTERN 2: TREE_VEC ========== */
#ifdef __GNUC__
/* Vector type declarations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

__attribute__((noinline))
int pattern_vectors(void) {
    /* Vector variables */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4sf vec_f = {1.0f, 2.0f, 3.0f, 4.0f};
    v2df vec_d = {1.0, 2.0};
    
    /* Vector operations that create TREE_VEC nodes */
    v4si vec_c = vec_a + vec_b;
    v4si vec_d2 = vec_a * vec_b;
    v4sf vec_e = vec_f + vec_f;
    
    /* Extract elements */
    int sum = vec_c[0] + vec_c[1] + vec_c[2] + vec_c[3];
    sum += (int)vec_e[0];
    
    /* Vector comparisons */
    v4si mask = vec_a < vec_b;
    
    return sum + mask[0];
}
#else
__attribute__((noinline))
int pattern_vectors(void) {
    /* Fallback for non-GCC */
    int arr[4] = {1, 2, 3, 4};
    return arr[0] + arr[1];
}
#endif

/* ========== PATTERN 3: SSA_NAME ========== */
__attribute__((noinline))
int pattern_ssa(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Multiple loops to force SSA creation */
    for (int i = 0; i < n; ++i) {
        x = x + i;      /* Creates SSA_NAME for x and i */
        y = y * (i + 1);
    }
    
    for (int j = 0; j < n * 2; ++j) {
        z = z - j;      /* Creates SSA_NAME for z and j */
        x = x + z;
    }
    
    /* Conditional that creates phi nodes */
    int result = 0;
    for (int k = 0; k < n; ++k) {
        if (k % 2 == 0) {
            result += x;
        } else {
            result += y;
        }
        z = result * k; /* More SSA */
    }
    
    return result + z;
}

/* ========== PATTERN 4: BLOCK ========== */
__attribute__((noinline))
int pattern_blocks(int val) {
    int outer = val;
    
    /* Nested blocks */
    {
        int inner1 = outer + 1;
        {
            int inner2 = inner1 * 2;
            {
                int inner3 = inner2 / 3;
                outer = inner3;
            }
        }
    }
    
    /* GCC statement expression (creates BLOCK nodes) */
    int stmt_expr = ({
        int temp = outer;
        temp = temp * temp;
        temp + 10;
    });
    
    /* Labels and goto (can involve block nodes) */
    void *label_addr = &&my_label;
    
    if (val > 100) {
        goto my_label;
    }
    
    /* Another nested block with variables */
    {
        int block_var1 = 42;
        int block_var2 = block_var1 * 2;
        stmt_expr += block_var2;
    }
    
my_label:
    return stmt_expr + outer;
}

/* ========== PATTERN 5: CONSTRUCTOR ========== */
__attribute__((noinline))
int pattern_constructors(void) {
    /* Structure with designated initializer */
    struct ComplexStruct {
        int a;
        float b;
        double c;
        char d;
    };
    
    struct ComplexStruct s1 = {
        .a = 10,
        .b = 20.5f,
        .c = 30.75,
        .d = 'X'
    };
    
    /* Array initializer */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Compound literal */
    int *ptr = (int[3]){10, 20, 30};
    
    /* Nested structure initializer */
    struct Inner {
        int x;
        int y;
    };
    
    struct Outer {
        struct Inner i;
        float f;
    };
    
    struct Outer o = {
        .i = {.x = 100, .y = 200},
        .f = 300.0f
    };
    
    /* Union initializer */
    union MyUnion {
        int as_int;
        float as_float;
    } u = {.as_int = 1234};
    
    return s1.a + arr[0] + ptr[1] + o.i.x + u.as_int;
}

/* ========== PATTERN 6: OPENMP CLAUSES ========== */
#ifdef _OPENMP
__attribute__((noinline))
int pattern_openmp(int size) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static, 10)
    for (int i = 0; i < size && i < 100; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP directive with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) collapse(2) if(size > 50)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int idx = i * 10 + j;
            if (idx < 100 && arr[idx] > max_val) {
                max_val = arr[idx];
            }
        }
    }
    
    /* OpenMP sections */
    int section_result = 0;
    #pragma omp parallel sections private(i) firstprivate(size)
    {
        #pragma omp section
        {
            for (int i = 0; i < size/2 && i < 50; i++) {
                section_result += arr[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = size/2; i < size && i < 100; i++) {
                section_result += arr[i];
            }
        }
    }
    
    return sum + max_val + section_result;
}
#else
__attribute__((noinline))
int pattern_openmp(int size) {
    /* Fallback without OpenMP */
    return size * 2;
}
#endif

/* ========== MAIN FUNCTION ========== */
int main(void) {
    volatile int result = 0;
    
    /* Call all pattern functions */
    result += pattern_identifiers();
    result += pattern_vectors();
    result += pattern_ssa(50);
    result += pattern_blocks(42);
    result += pattern_constructors();
    result += pattern_openmp(100);
    
    /* Prevent dead code elimination */
#ifdef __cplusplus
    std::cout << "Result: " << result << std::endl;
#else
    printf("Result: %d\n", result);
#endif
    
    return result > 0 ? 0 : 1;
}

#ifdef __cplusplus
} /* extern "C" */

/* ========== PATTERN 7: TREE_BINFO (C++ only) ========== */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int virtual_method() { return 42; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method() override { return base_data * 2; }
    int derived_data;
};

__attribute__((noinline))
int pattern_binfo(void) {
    DerivedClass derived_obj;
    derived_obj.base_data = 21;
    derived_obj.derived_data = 84;
    
    BaseClass* base_ptr = &derived_obj;
    
    /* Virtual call - involves BINFO for vtable lookup */
    int result = base_ptr->virtual_method();
    
    /* Access through base pointer */
    result += base_ptr->base_data;
    
    /* Cast (implicit and explicit) */
    DerivedClass* derived_ptr = static_cast<DerivedClass*>(base_ptr);
    result += derived_ptr->derived_data;
    
    return result;
}

/* C++ main that includes BINFO pattern */
int cpp_main(void) {
    volatile int result = 0;
    
    result += pattern_identifiers();
    result += pattern_vectors();
    result += pattern_ssa(50);
    result += pattern_blocks(42);
    result += pattern_constructors();
    result += pattern_openmp(100);
    result += pattern_binfo();  /* C++ specific */
    
    std::cout << "C++ Result: " << result << std::endl;
    return result > 0 ? 0 : 1;
}
#endif
