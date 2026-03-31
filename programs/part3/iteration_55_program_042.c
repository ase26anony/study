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

/* Function declarations to force identifier lookups */
extern int external_func_1(int);
extern float external_func_2(float);
extern void external_func_3(void);

/* ========== TREE_VEC patterns ========== */
#ifdef __GNUC__
/* Vector type declarations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
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
    void* as_ptr;
};

/* ========== Function prototypes ========== */
int __attribute__((noinline)) test_identifiers(void);
#ifdef __GNUC__
v4si __attribute__((noinline)) test_vectors(void);
#endif
int __attribute__((noinline)) test_ssa_names(int n);
void __attribute__((noinline)) test_blocks(void);
struct ComplexStruct __attribute__((noinline)) test_constructors(void);
int __attribute__((noinline)) test_openmp(int* arr, int n);

/* ========== IDENTIFIER_NODE function ========== */
int __attribute__((noinline)) test_identifiers(void) {
    /* Local variables with distinct names */
    int local_counter = 0;
    float local_temp = 0.0f;
    char local_char = 'Z';
    
    /* Operations that create identifier nodes */
    local_counter += sizeof(global_var_1);
    local_temp = (float)sizeof(global_var_2);
    local_char = (char)sizeof(global_var_3);
    
    /* Address-of operations */
    void* addr1 = &global_var_1;
    void* addr2 = &global_var_2;
    void* addr3 = &global_var_3;
    void* addr4 = &global_var_4;
    
    /* Use in expressions with external functions (declarations only) */
    if (external_func_1) {}
    if (external_func_2) {}
    if (external_func_3) {}
    
    /* Complex sizeof expressions with identifiers */
    local_counter += sizeof(local_temp) + sizeof(local_char);
    
    return local_counter;
}

/* ========== TREE_VEC function ========== */
#ifdef __GNUC__
v4si __attribute__((noinline)) test_vectors(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Vector arithmetic operations */
    v4si result1 = a + b;
    v4si result2 = b * c;
    v4si result3 = result1 - result2;
    
    /* Vector comparisons */
    v4si mask = a > b;
    v4si result4 = result3 & mask;
    
    /* Mixed vector operations */
    v4sf fvec = {1.0f, 2.0f, 3.0f, 4.0f};
    v4si ivec = {5, 6, 7, 8};
    
    /* Return vector result */
    return result4;
}
#endif

/* ========== SSA_NAME function ========== */
int __attribute__((noinline)) test_ssa_names(int n) {
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
        x = x ^ z;
    }
    
    /* Nested loop with phi nodes */
    int sum = 0;
    for (int k = 0; k < n; ++k) {
        for (int l = 0; l < k; ++l) {
            sum += l;
            y = y + sum;
        }
    }
    
    /* Conditional updates */
    if (x > y) {
        z = x - y;
    } else {
        z = y - x;
    }
    
    return x + y + z + sum;
}

/* ========== BLOCK function ========== */
void __attribute__((noinline)) test_blocks(void) {
    /* Outer block with variables */
    int outer_var = 100;
    
    /* Nested block 1 */
    {
        int inner_var_1 = 200;
        
        /* Deeply nested block */
        {
            int deep_var = 300;
            outer_var += inner_var_1 + deep_var;
            
            /* GCC statement expression (creates a block) */
            int stmt_expr = ({
                int temp = deep_var * 2;
                temp + 10;
            });
            outer_var += stmt_expr;
        }
    }
    
    /* Nested block 2 with different scope */
    {
        float float_var = 3.14f;
        double double_var = 2.71828;
        
        /* Another statement expression */
        double result = ({
            double temp = float_var * double_var;
            temp * 2.0;
        });
        
        outer_var += (int)result;
    }
    
    /* Labels and goto (address of label creates block nodes) */
    void* label_addr = &&my_label;
    
    if (outer_var > 500) {
        goto my_label;
    }
    
    /* Unreachable code to avoid actual goto */
    outer_var = outer_var * 2;
    
my_label:
    /* Use volatile to prevent optimization */
    volatile int final = outer_var;
    (void)final;
}

/* ========== CONSTRUCTOR function ========== */
struct ComplexStruct __attribute__((noinline)) test_constructors(void) {
    /* Structure initializers */
    struct ComplexStruct s1 = {
        .int_field = 42,
        .float_field = 3.14159f,
        .double_field = 2.71828,
        .char_field = 'X'
    };
    
    /* Array initializer */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Nested structure initializer */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    } nested = {
        .inner = {100, 2.5f, 3.5, 'Y'},
        .extra = 999
    };
    
    /* Compound literals */
    int* ptr = (int[3]){1, 2, 3};
    struct ComplexStruct* sp = &(struct ComplexStruct){55, 4.5f, 6.7, 'Z'};
    
    /* Union initializer */
    union MixedUnion u1 = {.as_int = 1234};
    union MixedUnion u2 = {.as_float = 5.67f};
    
    /* Update s1 using values from compound literals */
    s1.int_field += ptr[0];
    s1.float_field += sp->float_field;
    
    return s1;
}

/* ========== OMP_CLAUSE function ========== */
int __attribute__((noinline)) test_openmp(int* arr, int n) {
    int sum = 0;
    int i;
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static)
    for (i = 0; i < n; i++) {
        sum += arr[i];
    }
    
    int max_val = 0;
    int min_val = 0;
    
    /* Another OpenMP region with different clauses */
    #pragma omp parallel sections private(i) shared(arr, n) reduction(max:max_val) reduction(min:min_val)
    {
        #pragma omp section
        {
            max_val = arr[0];
            for (i = 1; i < n; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
        
        #pragma omp section
        {
            min_val = arr[0];
            for (i = 1; i < n; i++) {
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
    
    return sum;
}

#ifdef __cplusplus
} /* extern "C" */

/* ========== TREE_BINFO patterns (C++ only) ========== */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int get_value() const { return base_value; }
    virtual void set_value(int v) { base_value = v; }
    
private:
    int base_value = 100;
};

class DerivedClass : public BaseClass {
public:
    virtual int get_value() const override { return derived_value; }
    virtual void set_value(int v) override { derived_value = v * 2; }
    
    int get_double() const { return derived_value * 2; }
    
private:
    int derived_value = 200;
};

class SecondDerived : public DerivedClass {
public:
    virtual int get_value() const override { return second_value; }
    
private:
    int second_value = 300;
};

int __attribute__((noinline)) test_binfo(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    SecondDerived second;
    
    /* Virtual function calls through base pointer */
    base_ptr->set_value(42);
    int val1 = base_ptr->get_value();
    
    /* Dynamic cast (involves BINFO) */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        val1 += derived_ptr->get_double();
    }
    
    /* Multiple inheritance-like access */
    BaseClass* base_ptr2 = &second;
    int val2 = base_ptr2->get_value();
    
    return val1 + val2;
}
#endif

/* ========== Main function ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Test IDENTIFIER_NODE */
    result += test_identifiers();
    
    /* Test TREE_VEC */
    #ifdef __GNUC__
    v4si vec_result = test_vectors();
    volatile int* vptr = (int*)&vec_result;
    result += vptr[0] + vptr[1] + vptr[2] + vptr[3];
    #endif
    
    /* Test SSA_NAME */
    result += test_ssa_names(100);
    
    /* Test BLOCK */
    test_blocks();
    
    /* Test CONSTRUCTOR */
    struct ComplexStruct cs = test_constructors();
    result += cs.int_field + (int)cs.float_field;
    
    /* Test OMP_CLAUSE */
    int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    result += test_openmp(arr, 100);
    
    #ifdef __cplusplus
    /* Test TREE_BINFO (C++ only) */
    result += test_binfo();
    #endif
    
    /* Prevent dead code elimination */
    volatile int final_result = result;
    
    #ifdef __cplusplus
    std::cout << "Result: " << final_result << std::endl;
    #else
    printf("Result: %d\n", final_result);
    #endif
    
    return final_result > 0 ? 0 : 1;
}
