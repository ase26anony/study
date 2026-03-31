/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */
#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global and local variables with various uses */
int global_var1 = 10;
float global_var2 = 20.5;
char global_var3 = 'A';
static int static_var = 30;
extern int extern_func(int);  /* Forces identifier lookup */

void use_identifiers(void) __attribute__((noinline));
void use_identifiers(void) {
    /* Local variables with different names */
    int local_a = 1;
    double local_b = 2.0;
    char local_c = 'z';
    
    /* Various operations that create IDENTIFIER_NODE trees */
    int *ptr1 = &global_var1;
    float *ptr2 = &global_var2;
    char *ptr3 = &global_var3;
    
    /* sizeof expressions */
    size_t s1 = sizeof(global_var1);
    size_t s2 = sizeof(local_a);
    
    /* Function call with identifiers */
    if (extern_func) {
        /* Reference to function identifier */
        local_a = extern_func(local_a);
    }
    
    /* Complex expressions with identifiers */
    local_b = global_var2 * local_b + (double)global_var1;
    local_c = global_var3 + 1;
    
    /* Prevent dead code elimination */
    volatile int dummy = local_a + (int)local_b + local_c;
    (void)dummy;
}

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
#endif

void use_vectors(void) __attribute__((noinline));
void use_vectors(void) {
#ifdef __GNUC__
    /* Vector declarations and operations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c;
    
    /* Vector arithmetic */
    vec_c = vec_a + vec_b;
    vec_c = vec_a * vec_b;
    vec_c = vec_a & vec_b;
    
    /* Float vectors */
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec_c = fvec_a * fvec_b;
    
    /* Double vectors */
    v2df dvec_a = {1.0, 2.0};
    v2df dvec_b = {3.0, 4.0};
    v2df dvec_c = dvec_a + dvec_b;
    
    /* Prevent dead code elimination */
    volatile int dummy = vec_c[0] + (int)fvec_c[0] + (int)dvec_c[0];
    (void)dummy;
#endif
}

/* Pattern 3: SSA_NAME - Loops and variable modifications */
void create_ssa_names(int n) __attribute__((noinline));
void create_ssa_names(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Multiple loops creating SSA names */
    for (int i = 0; i < n; ++i) {
        x = x + i;      /* Creates SSA_NAME for x */
        y = y * (i + 1); /* Creates SSA_NAME for y */
    }
    
    /* Nested loop with different variable */
    for (int j = 0; j < n; ++j) {
        for (int k = 0; k < j; ++k) {
            z = z + (j * k); /* Creates SSA_NAME for z */
        }
    }
    
    /* Conditional updates */
    int w = 0;
    for (int i = 0; i < n; ++i) {
        if (i % 2 == 0) {
            w = w + i;      /* Creates phi nodes in SSA */
        } else {
            w = w - i;
        }
    }
    
    /* Prevent dead code elimination */
    volatile int dummy = x + y + z + w;
    (void)dummy;
}

/* Pattern 4: BLOCK - Nested blocks and statement expressions */
void use_blocks(void) __attribute__((noinline));
void use_blocks(void) {
    /* Level 1 block */
    {
        int a = 10;
        
        /* Level 2 block */
        {
            int b = 20;
            
            /* Level 3 block */
            {
                int c = 30;
                a = b + c;
            }
            
            /* GCC statement expression (creates a block) */
            int d = ({
                int temp = a * 2;
                temp + 5;
            });
            
            /* Another nested block with its own variables */
            {
                int e = 40;
                int f = 50;
                b = e + f + d;
            }
        }
        
        /* Label and address-of-label (involves BLOCK nodes) */
        void *target = &&end_block;
        goto *target;
        
        /* Unreachable code */
        a = 100;
    }
    
end_block:
    /* Final block */
    {
        volatile int dummy = 999;
        (void)dummy;
    }
}

/* Pattern 5: CONSTRUCTOR - Structure and array initializers */
struct ComplexStruct {
    int int_field;
    float float_field;
    double double_field;
    char char_field;
    int array_field[3];
};

union MixedUnion {
    int as_int;
    float as_float;
    char as_chars[4];
};

void use_constructors(void) __attribute__((noinline));
void use_constructors(void) {
    /* Structure with designated initializer */
    struct ComplexStruct s1 = {
        .int_field = 42,
        .float_field = 3.14f,
        .double_field = 2.71828,
        .char_field = 'X',
        .array_field = {1, 2, 3}
    };
    
    /* Array with initializer */
    int arr1[5] = {10, 20, 30, 40, 50};
    
    /* Compound literal for array */
    int *ptr = (int[]){100, 200, 300, 400};
    
    /* Compound literal for structure */
    struct ComplexStruct *sptr = &(struct ComplexStruct){
        .int_field = 99,
        .float_field = 1.414f,
        .char_field = 'Z'
    };
    
    /* Nested initializers */
    struct Nested {
        struct Inner {
            int a;
            int b;
        } inner;
        int outer;
    } nested = {
        .inner = {.a = 1, .b = 2},
        .outer = 3
    };
    
    /* Union initializer */
    union MixedUnion u1 = {.as_int = 0xDEADBEEF};
    union MixedUnion u2 = {.as_float = 1.234f};
    
    /* Prevent dead code elimination */
    volatile int dummy = s1.int_field + arr1[0] + ptr[0] + sptr->int_field + 
                         nested.outer + u1.as_int;
    (void)dummy;
}

/* Pattern 6: OMP_CLAUSE - OpenMP pragmas with various clauses */
#ifdef _OPENMP
void use_openmp(int size) __attribute__((noinline));
void use_openmp(int size) {
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
    #pragma omp parallel for private(i) shared(arr) reduction(max:max_val)
    for (i = 0; i < size; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel num_threads(4) default(none) shared(arr, size, sum, max_val)
    {
        #pragma omp single
        {
            /* Just to create more OMP_CLAUSE nodes */
            volatile int dummy = 0;
            (void)dummy;
        }
    }
    
    free(arr);
    
    /* Prevent dead code elimination */
    volatile int dummy = sum + max_val;
    (void)dummy;
}
#endif

/* Pattern 7: TREE_BINFO - C++ class inheritance (only in C++ mode) */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int get_value() const { return 10; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int get_value() const override { return 20; }
    int derived_data;
};

class SecondDerived : public DerivedClass {
public:
    virtual int get_value() const override { return 30; }
    int second_data;
};

void use_inheritance(void) __attribute__((noinline));
void use_inheritance(void) {
    DerivedClass derived;
    SecondDerived second;
    
    BaseClass* base_ptr = &derived;
    BaseClass* base_ptr2 = &second;
    
    /* Virtual calls - involve BINFO lookups */
    int val1 = base_ptr->get_value();
    int val2 = base_ptr2->get_value();
    
    /* Dynamic casting (involves BINFO) */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    SecondDerived* second_ptr = dynamic_cast<SecondDerived*>(base_ptr2);
    
    /* Typeid (involves BINFO) */
    const std::type_info& type1 = typeid(*base_ptr);
    const std::type_info& type2 = typeid(*base_ptr2);
    
    /* Prevent dead code elimination */
    volatile int dummy = val1 + val2 + (derived_ptr != 0) + (second_ptr != 0);
    (void)dummy;
}
#endif

/* Main function that calls all patterns */
int main(int argc, char** argv) {
    int n = 100;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 100;
    }
    
    /* Call all pattern functions */
    use_identifiers();
    use_vectors();
    create_ssa_names(n);
    use_blocks();
    use_constructors();
    
#ifdef _OPENMP
    use_openmp(n);
#endif
    
#ifdef __cplusplus
    use_inheritance();
#endif
    
    /* Compute a checksum to prevent optimization */
    volatile int checksum = 
        global_var1 + 
        (int)global_var2 + 
        global_var3 + 
        static_var +
        n;
    
    printf("Tree node test completed. Checksum: %d\n", checksum);
    
    return 0;
}

/* Dummy extern function declaration */
extern int extern_func(int x) {
    return x * 2;
}
