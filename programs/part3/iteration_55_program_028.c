/* test_tree_nodes.cc - Comprehensive test for tree node coverage */
#ifdef __cplusplus
#include <iostream>
using namespace std;

/* Pattern for TREE_BINFO - C++ class inheritance */
class BaseClass {
public:
    virtual int base_method() { return 42; }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual int base_method() override { return 84; }
    int derived_method() { return 168; }
};

void use_polymorphism() __attribute__((noinline));
void use_polymorphism() {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    volatile int result = base_ptr->base_method();  /* TREE_BINFO nodes here */
    (void)result;
}
#endif

/* Pattern for IDENTIFIER_NODE - Global variables with various uses */
int global_var_1 = 100;
float global_var_2 = 200.5;
double global_var_3 = 300.75;
char global_var_4 = 'X';

void use_identifiers() __attribute__((noinline));
void use_identifiers() {
    /* Taking addresses of globals */
    int* ptr1 = &global_var_1;
    float* ptr2 = &global_var_2;
    
    /* sizeof expressions with identifiers */
    volatile size_t s1 = sizeof(global_var_1);
    volatile size_t s2 = sizeof(global_var_2);
    volatile size_t s3 = sizeof(global_var_3);
    volatile size_t s4 = sizeof(global_var_4);
    
    /* Local identifiers */
    int local_var_1 = 10;
    float local_var_2 = 20.5;
    double local_var_3 = 30.75;
    char local_var_4 = 'Y';
    
    /* More address operations */
    (void)&local_var_1;
    (void)&local_var_2;
    (void)&local_var_3;
    (void)&local_var_4;
    
    /* Function declarations using identifiers */
    extern int external_func(int, float);
    (void)external_func;
}

/* Pattern for TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

void use_vectors() __attribute__((noinline));
void use_vectors() {
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;      /* Vector arithmetic */
    v4si vec4 = vec1 * vec2;
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fvec3 = fvec1 + fvec2;
    
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v2df dvec3 = dvec1 * dvec2;
    
    /* Use vectors in function-like context */
    volatile v4si temp = vec3;
    (void)temp;
    (void)fvec3;
    (void)dvec3;
}
#else
void use_vectors() __attribute__((noinline));
void use_vectors() {
    /* Fallback for non-GCC compilers */
    int vec[4] = {1, 2, 3, 4};
    (void)vec;
}
#endif

/* Pattern for SSA_NAME - Loops with variable modifications */
void create_ssa_names(int n) __attribute__((noinline));
void create_ssa_names(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Multiple loops creating SSA names */
    for (int i = 0; i < n; ++i) {
        x = x + i;      /* SSA_NAME for x and i */
        y = y * (i + 1);
    }
    
    for (int j = 0; j < n * 2; ++j) {
        z = z - j;      /* SSA_NAME for z and j */
        x = x ^ z;      /* Complex SSA web */
    }
    
    /* Nested loops */
    for (int k = 0; k < 10; ++k) {
        for (int l = 0; l < 5; ++l) {
            y = y + k * l;  /* More SSA names */
        }
    }
    
    volatile int result = x + y + z;
    (void)result;
}

/* Pattern for BLOCK - Nested blocks and statement expressions */
void use_blocks() __attribute__((noinline));
void use_blocks() {
    /* Level 1 block */
    {
        int block_var_1 = 10;
        
        /* Level 2 block */
        {
            int block_var_2 = 20;
            
            /* Level 3 block */
            {
                int block_var_3 = 30;
                volatile int sum = block_var_1 + block_var_2 + block_var_3;
                (void)sum;
            }
        }
    }
    
    /* GCC statement expression (creates a block) */
    int stmt_expr_result = ({
        int a = 5;
        int b = 10;
        int c = a * b;
        c + 15;
    });
    
    /* Label address and goto (involves blocks) */
    void* label_ptr = &&my_label;
    goto *label_ptr;
    
my_label:
    volatile int final = stmt_expr_result;
    (void)final;
    
    /* Switch statement with blocks */
    int switch_var = 2;
    switch (switch_var) {
        case 1: {
            int case_var = 100;
            (void)case_var;
            break;
        }
        case 2: {
            int case_var = 200;
            (void)case_var;
            break;
        }
        default: {
            int case_var = 300;
            (void)case_var;
            break;
        }
    }
}

/* Pattern for CONSTRUCTOR - Structure and array initializers */
void use_constructors() __attribute__((noinline));
void use_constructors() {
    /* Structure with designated initializers */
    struct ComplexStruct {
        int int_field;
        float float_field;
        double double_field;
        char char_field;
        int array_field[3];
    };
    
    struct ComplexStruct s1 = {
        .int_field = 1,
        .float_field = 2.0f,
        .double_field = 3.0,
        .char_field = 'A',
        .array_field = {4, 5, 6}
    };
    
    /* Array initializer */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Compound literals */
    int* ptr = (int[3]){100, 200, 300};
    struct ComplexStruct* s2 = &(struct ComplexStruct){
        .int_field = 7,
        .float_field = 8.0f,
        .double_field = 9.0,
        .char_field = 'B',
        .array_field = {10, 11, 12}
    };
    
    /* Nested initializers */
    struct Outer {
        struct Inner {
            int a;
            int b;
        } inner;
        int c;
    };
    
    struct Outer outer = {
        .inner = {.a = 13, .b = 14},
        .c = 15
    };
    
    volatile int check = s1.int_field + arr[0] + ptr[0] + s2->int_field + outer.inner.a;
    (void)check;
}

/* Pattern for OMP_CLAUSE - OpenMP pragmas with various clauses */
#ifdef _OPENMP
#include <omp.h>

void use_openmp(int n) __attribute__((noinline));
void use_openmp(int n) {
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
    
    /* Another OpenMP directive with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) collapse(2)
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            int val = x * 10 + y;
            if (val > max_val) {
                max_val = val;
            }
        }
    }
    
    /* OpenMP sections */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            i = 1;
        }
        #pragma omp section
        {
            i = 2;
        }
    }
    
    volatile int result = sum + max_val;
    (void)result;
}
#else
void use_openmp(int n) __attribute__((noinline));
void use_openmp(int n) {
    /* Fallback without OpenMP */
    volatile int dummy = n;
    (void)dummy;
}
#endif

/* Main function that calls all patterns */
int main(int argc, char** argv) {
    int n = 100;
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    
    /* Call all pattern functions */
    use_identifiers();
    
#ifdef __cplusplus
    use_polymorphism();
#endif
    
    use_vectors();
    create_ssa_names(n);
    use_blocks();
    use_constructors();
    use_openmp(n);
    
    /* Compute a final checksum to prevent dead code elimination */
    volatile int checksum = global_var_1 + (int)global_var_2;
    checksum += n * 2;
    
#ifdef __cplusplus
    cout << "Test completed with checksum: " << checksum << endl;
#else
    printf("Test completed with checksum: %d\n", checksum);
#endif
    
    return 0;
}
