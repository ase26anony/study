/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* ========== IDENTIFIER_NODE patterns ========== */
/* Global variables for identifier creation */
int global_var_1 = 10;
float global_var_2 = 20.5;
char global_var_3 = 'A';
double global_var_4 = 30.75;

/* Function taking address of identifiers */
int* get_addr_int(int *p) { return p; }
float* get_addr_float(float *p) { return p; }

/* ========== TREE_VEC patterns ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

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
    char as_char[4];
};

/* ========== SSA_NAME patterns ========== */
__attribute__((noinline))
int ssa_pattern_function(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Multiple loops to create SSA_NAME nodes */
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    
    for (int j = 0; j < n; ++j) {
        z = z - j;
        x = x ^ z;  /* XOR operation */
    }
    
    /* Complex control flow for more SSA */
    int w = 0;
    while (w < n) {
        y = y + w;
        if (w % 2 == 0) {
            z = z * 2;
        } else {
            z = z / 2;
        }
        w++;
    }
    
    return x + y + z;
}

/* ========== BLOCK patterns ========== */
__attribute__((noinline))
int block_pattern_function(int val) {
    /* Outer block */
    int result = val;
    
    {
        /* Nested block 1 */
        int temp1 = result * 2;
        
        {
            /* Nested block 2 */
            int temp2 = temp1 + 10;
            
            {
                /* Nested block 3 with local variable */
                int temp3 = temp2 - 5;
                result = temp3;
            }
        }
    }
    
    /* GCC statement expression (creates BLOCK) */
    int stmt_expr = ({
        int a = result;
        int b = a * 3;
        b + 7;
    });
    
    /* Label and goto for additional BLOCK nodes */
    void* label_ptr = &&end_label;
    
    if (stmt_expr > 100) {
        goto *label_ptr;
    }
    
    result = stmt_expr;
    
end_label:
    return result;
}

/* ========== VECTOR patterns ========== */
#ifdef __GNUC__
__attribute__((noinline))
v4si vector_pattern_function(v4si a, v4si b) {
    v4si result;
    
    /* Various vector operations */
    result = a + b;
    result = result * a;
    result = result - b;
    
    /* Vector comparisons */
    v4si mask = a > b;
    result = result & mask;
    
    return result;
}

__attribute__((noinline))
v4sf float_vector_pattern(v4sf a, v4sf b) {
    v4sf result = a * b + a / b;
    return result;
}
#endif

/* ========== CONSTRUCTOR patterns function ========== */
__attribute__((noinline))
int constructor_pattern_function(void) {
    /* Structure initializer with designated initializers */
    struct ComplexStruct s1 = {
        .int_field = 42,
        .float_field = 3.14159f,
        .double_field = 2.71828,
        .char_field = 'Z'
    };
    
    /* Array initializer */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Nested structure initializer */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    } nested = {
        .inner = { .int_field = 100, .float_field = 1.5f, 
                   .double_field = 2.5, .char_field = 'X' },
        .extra = 999
    };
    
    /* Compound literals */
    int* ptr = (int[3]){1, 2, 3};
    struct ComplexStruct* sp = &(struct ComplexStruct){ 
        .int_field = 7, .float_field = 8.8f 
    };
    
    /* Union initializer */
    union MixedUnion u = { .as_int = 0xDEADBEEF };
    
    return s1.int_field + arr[2] + nested.extra + ptr[1] + sp->int_field + u.as_int;
}

/* ========== OpenMP patterns ========== */
#ifdef _OPENMP
__attribute__((noinline))
int openmp_pattern_function(int size) {
    int sum = 0;
    int product = 1;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) reduction(*:product) schedule(static, 10)
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
        product *= (arr[i] % 10) + 1;  /* Avoid overflow */
    }
    
    /* Another OpenMP section with different clauses */
    int max_val = 0;
    int min_val = 1000;
    
    #pragma omp parallel sections private(i) firstprivate(max_val, min_val) lastprivate(max_val, min_val)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                if (arr[i] < min_val) min_val = arr[i];
            }
        }
    }
    
    /* OpenMP critical section */
    #pragma omp parallel
    {
        #pragma omp critical
        {
            sum += max_val - min_val;
        }
    }
    
    return sum + product + max_val + min_val;
}
#endif

/* ========== Main driver ========== */
int main(void) {
    volatile int checksum = 0;
    
    /* 1. Trigger IDENTIFIER_NODE creation and inspection */
    checksum += (int)((void*)&global_var_1 - (void*)&global_var_2);
    checksum += sizeof(global_var_3);
    checksum += sizeof(global_var_4);
    
    /* Use identifiers in expressions */
    int local_ident_1 = global_var_1 * 2;
    float local_ident_2 = global_var_2 / 2.0f;
    checksum += local_ident_1 + (int)local_ident_2;
    
    /* Take addresses of identifiers */
    int* ptr1 = get_addr_int(&global_var_1);
    float* ptr2 = get_addr_float(&global_var_2);
    checksum += *ptr1 + (int)*ptr2;
    
    /* 2. Trigger SSA_NAME patterns */
    checksum += ssa_pattern_function(50);
    
    /* 3. Trigger BLOCK patterns */
    checksum += block_pattern_function(25);
    
    /* 4. Trigger CONSTRUCTOR patterns */
    checksum += constructor_pattern_function();
    
#ifdef __GNUC__
    /* 5. Trigger TREE_VEC patterns */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = vector_pattern_function(vec_a, vec_b);
    
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec_result = float_vector_pattern(fvec_a, fvec_b);
    
    /* Use vector results in checksum */
    int* vp = (int*)&vec_result;
    checksum += vp[0] + vp[1] + vp[2] + vp[3];
#endif

#ifdef _OPENMP
    /* 6. Trigger OMP_CLAUSE patterns */
    checksum += openmp_pattern_function(100);
#endif
    
    /* Final output to prevent optimization */
#ifdef __cplusplus
    std::cout << "Checksum: " << checksum << std::endl;
#else
    printf("Checksum: %d\n", checksum);
#endif
    
    return checksum != 0 ? 0 : 1;
}

#ifdef __cplusplus
} /* extern "C" */

/* ========== C++ TREE_BINFO patterns ========== */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int virtual_method() { return 42; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method() override { return 84; }
    int derived_data;
};

__attribute__((noinline))
int binfo_pattern_function() {
    DerivedClass derived_obj;
    BaseClass* base_ptr = &derived_obj;
    
    /* Use polymorphism to trigger BINFO inspection */
    int result = base_ptr->virtual_method();
    
    /* Access through derived pointer */
    DerivedClass* derived_ptr = static_cast<DerivedClass*>(base_ptr);
    result += derived_ptr->derived_data;
    
    /* Try dynamic_cast if RTTI is enabled */
    BaseClass* base_ptr2 = dynamic_cast<BaseClass*>(derived_ptr);
    if (base_ptr2) {
        result += base_ptr2->virtual_method();
    }
    
    return result;
}

/* C++ main */
int main(int argc, char** argv) {
    /* Call all C patterns through extern C functions */
    int checksum = ::main();
    
    /* Add C++ BINFO pattern */
    checksum += binfo_pattern_function();
    
    std::cout << "C++ Checksum: " << checksum << std::endl;
    return checksum != 0 ? 0 : 1;
}
#endif
