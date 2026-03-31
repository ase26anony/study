/* test_tree_nodes.c - Comprehensive test to trigger tree_kind classification
   for various GCC tree node types during compilation. */

#include <stdio.h>
#include <stdlib.h>

/* ==================== IDENTIFIER_NODE patterns ==================== */
/* Global variables to force identifier lookups */
int global_var_1;
double global_var_2;
char global_var_3;

/* Function declarations that require identifier resolution */
extern int external_func_1(int);
extern void external_func_2(double);

void __attribute__((noinline)) test_identifiers(void) {
    /* Local variables with distinct names */
    int local_ident_a;
    float local_ident_b;
    long local_ident_c;
    
    /* Operations that require identifier lookup */
    local_ident_a = sizeof(global_var_1);  /* sizeof on identifier */
    local_ident_b = (float)(&global_var_2); /* address-of operator */
    local_ident_c = (long)external_func_1; /* function identifier */
    
    /* Use in expressions */
    global_var_1 = local_ident_a * 2;
    global_var_3 = (char)local_ident_b;
    
    /* Prevent dead code elimination */
    volatile int sink = local_ident_a + local_ident_c;
    (void)sink;
}

/* ==================== TREE_VEC patterns ==================== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v8f __attribute__((vector_size(32)));

void __attribute__((noinline)) test_vectors(void) {
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c;
    
    /* Vector operations that generate TREE_VEC nodes */
    vec_c = vec_a + vec_b;
    vec_c = vec_a * vec_b;
    vec_c = vec_a & vec_b;
    
    /* Vector in function argument position */
    v8f vec_d = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    v8f vec_e = vec_d * 2.0f;
    
    volatile int sink = vec_c[0] + (int)vec_e[0];
    (void)sink;
}
#else
void __attribute__((noinline)) test_vectors(void) {
    /* Fallback for non-GCC compilers */
    volatile int sink = 0;
    (void)sink;
}
#endif

/* ==================== SSA_NAME patterns ==================== */
void __attribute__((noinline)) test_ssa(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Multiple loops that force SSA form generation */
    for (int i = 0; i < n; ++i) {
        x = x + i;      /* Creates SSA_NAME for x and i */
        y = y * (i + 1);
    }
    
    for (int j = 0; j < n * 2; ++j) {
        z = z - j;
        x = x + z;      /* Complex SSA web */
    }
    
    /* Conditional that creates phi nodes */
    int result = 0;
    for (int k = 0; k < n; ++k) {
        if (k % 2 == 0) {
            result += x;
        } else {
            result += y;
        }
    }
    
    volatile int sink = result + z;
    (void)sink;
}

/* ==================== BLOCK patterns ==================== */
void __attribute__((noinline)) test_blocks(void) {
    /* Outer block with local variable */
    int outer_var = 10;
    
    /* Nested block 1 */
    {
        int inner_var_1 = outer_var * 2;
        
        /* Nested block 2 */
        {
            int inner_var_2 = inner_var_1 + 5;
            outer_var = inner_var_2;
        }
    }
    
    /* GCC statement expression (creates a block) */
    int stmt_expr_result = ({
        int temp = outer_var;
        temp * temp + 1;
    });
    
    /* Label address and goto (involves block nodes) */
    void* target = &&end_label;
    
    if (stmt_expr_result > 100) {
        goto *target;
    }
    
    /* Another nested block */
    {
        volatile int block_sink = stmt_expr_result;
        (void)block_sink;
    }
    
end_label:
    return;
}

/* ==================== CONSTRUCTOR patterns ==================== */
struct ComplexStruct {
    int int_field;
    float float_field;
    double double_field;
    char char_field;
};

union MixedUnion {
    int as_int;
    float as_float;
    void* as_ptr;
};

void __attribute__((noinline)) test_constructors(void) {
    /* Structure initializer with designated initializers */
    struct ComplexStruct s1 = {
        .int_field = 42,
        .float_field = 3.14f,
        .double_field = 2.71828,
        .char_field = 'A'
    };
    
    /* Array initializer */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Compound literal for array */
    int* ptr = (int[3]){1, 2, 3};
    
    /* Compound literal for structure */
    struct ComplexStruct s2 = (struct ComplexStruct){
        .int_field = 100,
        .float_field = 1.414f,
        .char_field = 'Z'
    };
    
    /* Nested initializers */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    } nested = {
        .inner = { .int_field = 999, .float_field = 9.99f },
        .extra = 111
    };
    
    /* Union initializer */
    union MixedUnion u = { .as_int = 0xDEADBEEF };
    
    volatile int sink = s1.int_field + arr[0] + ptr[0] + s2.int_field + nested.extra + u.as_int;
    (void)sink;
}

/* ==================== OMP_CLAUSE patterns ==================== */
#ifdef _OPENMP
void __attribute__((noinline)) test_omp_clauses(int n) {
    int i;
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static, 10)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* OpenMP parallel region with firstprivate and lastprivate */
    int local_var = 5;
    #pragma omp parallel firstprivate(local_var)
    {
        local_var += omp_get_thread_num();
    }
    
    /* OpenMP sections */
    int section_result = 0;
    #pragma omp parallel sections private(i) reduction(+:section_result)
    {
        #pragma omp section
        {
            for (i = 0; i < 50; i++) {
                section_result += i;
            }
        }
        
        #pragma omp section
        {
            for (i = 50; i < 100; i++) {
                section_result += i;
            }
        }
    }
    
    /* OpenMP task with if clause */
    int task_var = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task if(n > 100) shared(task_var)
        {
            task_var = 1;
        }
    }
    
    volatile int sink = sum + section_result + task_var;
    (void)sink;
}
#else
void __attribute__((noinline)) test_omp_clauses(int n) {
    /* Fallback when OpenMP not available */
    volatile int sink = n;
    (void)sink;
}
#endif

/* ==================== TREE_BINFO patterns (C++ only) ==================== */
#ifdef __cplusplus
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

void __attribute__((noinline)) test_binfo() {
    DerivedClass derived_obj;
    BaseClass* base_ptr = &derived_obj;
    
    /* Virtual call - involves BINFO for vtable lookup */
    int result = base_ptr->method();
    
    /* Access through base pointer */
    base_ptr->base_data = 42;
    
    /* dynamic_cast would use BINFO but we avoid RTTI for simplicity */
    DerivedClass* derived_ptr = static_cast<DerivedClass*>(base_ptr);
    derived_ptr->derived_data = 84;
    
    volatile int sink = result + base_ptr->base_data + derived_ptr->derived_data;
    (void)sink;
}
#endif

/* ==================== Main driver ==================== */
int main(int argc, char** argv) {
    int n = 100;
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    
    /* Call all test functions to ensure all tree nodes are created */
    test_identifiers();
    test_vectors();
    test_ssa(n);
    test_blocks();
    test_constructors();
    test_omp_clauses(n);
    
#ifdef __cplusplus
    test_binfo();
#endif
    
    /* Compute a checksum to prevent optimization */
    volatile int final_result = 
        global_var_1 + 
        (int)global_var_2 + 
        global_var_3 +
        n;
    
    printf("Test completed with result hint: %d\n", final_result);
    
    return 0;
}
