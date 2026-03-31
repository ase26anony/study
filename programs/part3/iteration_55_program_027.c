/* test_tree_nodes.c - Comprehensive test to trigger tree_kind classification
   for various GCC tree node types during compilation. */

#include <stdio.h>
#include <stdlib.h>

/* ========== IDENTIFIER_NODE patterns ========== */
/* Global variables to force identifier lookups */
int global_var_1;
double global_var_2;
char global_var_3;

/* Function declarations that require identifier resolution */
extern int external_func_1(int);
extern void external_func_2(double);

void identifier_pattern(void) __attribute__((noinline));
void identifier_pattern(void) {
    /* Local variables with distinct names */
    int local_ident_a;
    float local_ident_b;
    char local_ident_c;
    
    /* Operations that require identifier lookup */
    local_ident_a = global_var_1;
    local_ident_b = global_var_2;
    local_ident_c = global_var_3;
    
    /* Taking addresses forces symbol table lookups */
    int *ptr_a = &local_ident_a;
    float *ptr_b = &local_ident_b;
    
    /* sizeof on identifiers */
    size_t sz1 = sizeof(local_ident_a);
    size_t sz2 = sizeof(global_var_1);
    
    /* Use in expressions with external functions */
    if (external_func_1) {
        local_ident_a = external_func_1(local_ident_a);
    }
    
    /* Prevent dead code elimination */
    volatile int sink = local_ident_a + *ptr_a + sz1;
    (void)sink;
}

/* ========== TREE_VEC patterns ========== */
/* Vector extension - only for GCC */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

void vector_pattern(void) __attribute__((noinline));
void vector_pattern(void) {
#ifdef __GNUC__
    /* Vector declarations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4sf vec_c = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Vector operations */
    v4si vec_sum = vec_a + vec_b;
    v4si vec_mul = vec_a * vec_b;
    v4sf vec_fmul = vec_c * 2.0f;
    
    /* Use vectors in function-like context */
    __builtin_ia32_addps(vec_c, vec_c);
    
    /* Prevent optimization */
    volatile v4si sink_vec = vec_sum + vec_mul;
    (void)sink_vec;
#else
    /* Fallback for non-GCC compilers */
    int dummy[4] = {0};
    (void)dummy;
#endif
}

/* ========== SSA_NAME patterns ========== */
void ssa_pattern(int n) __attribute__((noinline));
void ssa_pattern(int n) {
    /* Variables that will get SSA names at -O1 or higher */
    int x = 0;
    int y = 1;
    float z = 2.0f;
    
    /* Loop that forces SSA form */
    for (int i = 0; i < n; ++i) {
        x = x + i;      /* Creates phi nodes for x */
        y = y * 2;      /* Creates phi nodes for y */
        z = z + 0.5f;   /* Creates phi nodes for z */
    }
    
    /* Nested loop with different variable */
    int w = 10;
    for (int j = 0; j < n; ++j) {
        for (int k = 0; k < j; ++k) {
            w = w - k;  /* More complex SSA web */
        }
    }
    
    /* Conditional updates */
    int t = 0;
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            t = t + i;
        } else {
            t = t - i;
        }
    }
    
    /* Prevent dead code elimination */
    volatile int sink = x + y + w + t;
    (void)sink;
    (void)z;
}

/* ========== BLOCK patterns ========== */
void block_pattern(void) __attribute__((noinline));
void block_pattern(void) {
    /* Outer block with local variable */
    int outer_var = 1;
    
    /* Nested block 1 */
    {
        int inner_var_1 = 2;
        
        /* Nested block 2 */
        {
            int inner_var_2 = 3;
            outer_var += inner_var_1 + inner_var_2;
        }
        
        /* Another nested block */
        {
            float float_var = 4.5f;
            outer_var += (int)float_var;
        }
    }
    
    /* GCC statement expression (creates a block) */
    int stmt_expr_result = ({
        int temp = 10;
        temp * 2;
    });
    
    /* Labels and goto (involves block nodes) */
    void *label_ptr = &&my_label;
    
    if (outer_var > 0) {
        goto *label_ptr;
    }
    
    /* Dead code to avoid */
    outer_var = 0;
    
my_label:
    /* Block with switch */
    switch (stmt_expr_result) {
        case 10: {
            int case_var = 100;
            outer_var += case_var;
            break;
        }
        case 20: {
            int case_var = 200;
            outer_var += case_var;
            break;
        }
        default: {
            int case_var = 300;
            outer_var += case_var;
            break;
        }
    }
    
    volatile int sink = outer_var;
    (void)sink;
    (void)label_ptr;
}

/* ========== CONSTRUCTOR patterns ========== */
struct ComplexStruct {
    int int_field;
    float float_field;
    double double_field;
    char char_field;
};

union MixedUnion {
    int as_int;
    float as_float;
    void *as_ptr;
};

void constructor_pattern(void) __attribute__((noinline));
void constructor_pattern(void) {
    /* Structure initializer with designated initializers */
    struct ComplexStruct s1 = {
        .int_field = 42,
        .float_field = 3.14f,
        .double_field = 2.71828,
        .char_field = 'A'
    };
    
    /* Array initializer */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Nested structure initializer */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    } nested = {
        .inner = { .int_field = 10, .float_field = 1.5f, .double_field = 3.0, .char_field = 'B' },
        .extra = 100
    };
    
    /* Union initializer */
    union MixedUnion u1 = { .as_int = 0xDEADBEEF };
    
    /* Compound literals */
    int *ptr_arr = (int[3]){10, 20, 30};
    struct ComplexStruct *ptr_struct = &(struct ComplexStruct){
        .int_field = 99,
        .float_field = 9.9f,
        .double_field = 8.8,
        .char_field = 'Z'
    };
    
    /* Multi-dimensional array initializer */
    int matrix[2][3] = {
        {1, 2, 3},
        {4, 5, 6}
    };
    
    /* String literal (also a constructor) */
    char str[] = "Hello, World!";
    
    /* Use the constructors to prevent optimization */
    volatile int sink = s1.int_field + arr[0] + nested.extra + u1.as_int + 
                       ptr_arr[0] + ptr_struct->int_field + matrix[0][0] + str[0];
    (void)sink;
}

/* ========== OMP_CLAUSE patterns ========== */
#ifdef _OPENMP
#include <omp.h>

void omp_pattern(int size) __attribute__((noinline));
void omp_pattern(int size) {
    int i;
    int sum = 0;
    int *arr = (int*)malloc(size * sizeof(int));
    
    if (!arr) return;
    
    /* Initialize array */
    for (i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static)
    for (i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP directive with different clauses */
    int max_val = 0;
    #pragma omp parallel for private(i) shared(arr) reduction(max:max_val) collapse(2)
    for (i = 0; i < size; i++) {
        for (int j = 0; j < 10; j++) {
            if (arr[i] > max_val) {
                max_val = arr[i];
            }
        }
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel num_threads(4) default(none) shared(arr, size, sum) private(i)
    {
        #pragma omp for nowait
        for (i = 0; i < size; i++) {
            arr[i] *= 2;
        }
        
        #pragma omp barrier
        
        #pragma omp single
        {
            sum = 0;
        }
    }
    
    /* OpenMP sections */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < size/2; i++) {
                arr[i] += 1;
            }
        }
        
        #pragma omp section
        {
            for (i = size/2; i < size; i++) {
                arr[i] -= 1;
            }
        }
    }
    
    free(arr);
    
    volatile int sink = sum + max_val;
    (void)sink;
}
#else
void omp_pattern(int size) __attribute__((noinline));
void omp_pattern(int size) {
    /* Fallback when OpenMP not available */
    volatile int sink = size;
    (void)sink;
}
#endif

/* ========== TREE_BINFO patterns (C++ only) ========== */
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

class SecondDerived : public DerivedClass {
public:
    virtual int method() override { return 3; }
    int second_data;
};

void binfo_pattern(void) __attribute__((noinline));
void binfo_pattern(void) {
    DerivedClass derived_obj;
    BaseClass* base_ptr = &derived_obj;
    
    /* Virtual call - involves BINFO for vtable lookup */
    int result = base_ptr->method();
    
    /* Dynamic cast - involves BINFO for type checking */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    
    /* Access through hierarchy */
    if (derived_ptr) {
        derived_ptr->base_data = 10;
        derived_ptr->derived_data = 20;
    }
    
    /* Multiple inheritance-like pattern */
    SecondDerived second_obj;
    BaseClass* base_ptr2 = &second_obj;
    DerivedClass* derived_ptr2 = &second_obj;
    
    /* Cross-casting */
    int result2 = base_ptr2->method();
    int result3 = derived_ptr2->method();
    
    volatile int sink = result + (derived_ptr ? 1 : 0) + result2 + result3;
    (void)sink;
}

#else

/* C version - dummy implementation */
void binfo_pattern(void) __attribute__((noinline));
void binfo_pattern(void) {
    /* In C, we can't create BINFO nodes, but we need something */
    volatile int sink = 0;
    (void)sink;
}

#endif

/* ========== Main driver ========== */
int main(int argc, char** argv) {
    int size = 100;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = 100;
    }
    
    /* Call all pattern functions */
    identifier_pattern();
    vector_pattern();
    ssa_pattern(size);
    block_pattern();
    constructor_pattern();
    omp_pattern(size);
    binfo_pattern();
    
    /* Compute a checksum to ensure all code is live */
    int checksum = 
        global_var_1 + 
        (int)global_var_2 + 
        global_var_3;
    
    printf("Tree node test completed. Checksum: %d\n", checksum);
    
    return 0;
}
