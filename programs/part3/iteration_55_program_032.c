/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* ========== PATTERN 1: IDENTIFIER_NODE ========== */
/* Global variables for identifier pattern */
int global_var_1 = 10;
int global_var_2 = 20;
float global_float = 3.14;
char global_char = 'A';

/* Function using multiple identifiers */
__attribute__((noinline))
int identifier_pattern(void) {
    /* Local variables with distinct names */
    int local_counter = 0;
    int local_sum = 0;
    float local_float = 2.718;
    
    /* Operations that require identifier lookups */
    local_counter = global_var_1 + global_var_2;
    local_sum = sizeof(global_char) + sizeof(local_float);
    
    /* Taking addresses of identifiers */
    int *ptr1 = &global_var_1;
    float *ptr2 = &global_float;
    char *ptr3 = &global_char;
    
    /* Using identifiers in sizeof expressions */
    local_sum += sizeof(global_var_1) + sizeof(global_var_2);
    
    /* External function declaration forces identifier lookup */
    extern int atoi(const char *);
    local_sum += atoi("123");
    
    return local_counter + local_sum;
}

/* ========== PATTERN 2: TREE_VEC ========== */
#ifdef __GNUC__
/* Vector type declaration */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

__attribute__((noinline))
int vector_pattern(void) {
    /* Vector variables */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4sf vec_f = {1.0, 2.0, 3.0, 4.0};
    
    /* Vector arithmetic operations */
    v4si vec_c = vec_a + vec_b;
    v4si vec_d = vec_a * vec_b;
    v4sf vec_g = vec_f + vec_f;
    
    /* Extract elements */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += vec_c[i];
        sum += (int)vec_g[i];
    }
    
    /* Vector in function argument (simulated) */
    v4si *vec_ptr = &vec_c;
    sum += (*vec_ptr)[0];
    
    return sum;
}
#else
__attribute__((noinline))
int vector_pattern(void) {
    /* Fallback for non-GCC compilers */
    int arr[4] = {1, 2, 3, 4};
    int sum = 0;
    for (int i = 0; i < 4; i++) sum += arr[i];
    return sum;
}
#endif

/* ========== PATTERN 3: TREE_BINFO (C++ only) ========== */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int get_value() { return 10; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int get_value() override { return 20; }
    int derived_data;
};

__attribute__((noinline))
int binfo_pattern(void) {
    DerivedClass derived_obj;
    BaseClass* base_ptr = &derived_obj;
    
    /* Access through base pointer triggers BINFO lookup */
    int value = base_ptr->get_value();
    base_ptr->base_data = 100;
    
    /* Create another derived object */
    DerivedClass* derived_ptr = new DerivedClass();
    value += derived_ptr->get_value();
    delete derived_ptr;
    
    return value;
}
#endif

/* ========== PATTERN 4: SSA_NAME ========== */
__attribute__((noinline))
int ssa_pattern(int n) {
    int x = 0;
    int y = 0;
    int z = 0;
    
    /* Multiple loops that force SSA generation */
    for (int i = 0; i < n; ++i) {
        x = x + i;      /* Creates SSA_NAME for x */
        y = y * i + 1;  /* Creates SSA_NAME for y */
    }
    
    /* Another loop with different variable */
    for (int j = 0; j < n; ++j) {
        z = z + j * 2;
        x = x - j;      /* Modifies x again */
    }
    
    /* Conditional that creates phi nodes */
    int result = 0;
    if (x > 0) {
        result = y;
    } else {
        result = z;
    }
    
    /* Complex expression with multiple assignments */
    int temp = result;
    for (int k = 0; k < 10; k++) {
        temp = temp * 2 + k;
    }
    
    return result + temp;
}

/* ========== PATTERN 5: BLOCK ========== */
__attribute__((noinline))
int block_pattern(void) {
    int outer = 0;
    
    /* Level 1 block */
    {
        int inner1 = 10;
        outer += inner1;
        
        /* Level 2 nested block */
        {
            int inner2 = 20;
            outer += inner2;
            
            /* Level 3 nested block */
            {
                int inner3 = 30;
                outer += inner3;
            }
        }
    }
    
    /* GCC statement expression (creates a block) */
    int stmt_expr = ({
        int a = 5;
        int b = 10;
        a + b;
    });
    outer += stmt_expr;
    
    /* Labels and goto (creates block nodes) */
    void* label_ptr = &&my_label;
    goto *label_ptr;
    
my_label:
    outer += 100;
    
    /* Another block with variable declaration */
    {
        volatile int block_var = 50;
        outer += block_var;
    }
    
    return outer;
}

/* ========== PATTERN 6: CONSTRUCTOR ========== */
struct ComplexStruct {
    int int_field;
    float float_field;
    double double_field;
    char char_field;
};

union MixedUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

__attribute__((noinline))
int constructor_pattern(void) {
    /* Structure with designated initializer */
    struct ComplexStruct s1 = {
        .int_field = 42,
        .float_field = 3.14159,
        .double_field = 2.71828,
        .char_field = 'X'
    };
    
    /* Array initializer */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Union initializer */
    union MixedUnion u1 = {.as_int = 0x12345678};
    
    /* Compound literal for array */
    int sum = 0;
    int* dynamic_arr = (int[3]){1, 2, 3};
    for (int i = 0; i < 3; i++) {
        sum += dynamic_arr[i];
    }
    
    /* Compound literal for struct */
    struct ComplexStruct* s2 = &(struct ComplexStruct){
        .int_field = 100,
        .float_field = 1.234,
        .double_field = 5.678,
        .char_field = 'Z'
    };
    sum += s2->int_field;
    
    /* Nested initializers */
    struct Nested {
        int a;
        int b[3];
        struct {
            int x;
            int y;
        } inner;
    } nested = {
        .a = 1,
        .b = {2, 3, 4},
        .inner = {.x = 5, .y = 6}
    };
    sum += nested.a + nested.inner.x;
    
    return sum + s1.int_field + arr[0] + u1.as_int;
}

/* ========== PATTERN 7: OMP_CLAUSE ========== */
#ifdef _OPENMP
#include <omp.h>

__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int* arr = (int*)__builtin_alloca(size * sizeof(int));
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static)
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP directive with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) collapse(2)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int val = i * j;
            if (val > max_val) max_val = val;
        }
    }
    
    /* OpenMP parallel region with private and firstprivate */
    int private_var = 0;
    #pragma omp parallel private(private_var) firstprivate(max_val)
    {
        private_var = omp_get_thread_num();
        sum += private_var + max_val;
    }
    
    return sum;
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

/* ========== MAIN FUNCTION ========== */
int main(void) {
    volatile int total = 0;
    
    /* Call all pattern functions */
    total += identifier_pattern();
    total += vector_pattern();
    
#ifdef __cplusplus
    total += binfo_pattern();
#endif
    
    total += ssa_pattern(100);
    total += block_pattern();
    total += constructor_pattern();
    total += omp_pattern(50);
    
    /* Prevent dead code elimination */
#ifdef __cplusplus
    std::cout << "Total checksum: " << total << std::endl;
#else
    printf("Total checksum: %d\n", total);
#endif
    
    return total > 0 ? 0 : 1;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
