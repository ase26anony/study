/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* ==================== IDENTIFIER_NODE patterns ==================== */
/* Global variables to force identifier creation */
int global_var_1;
float global_var_2;
double global_var_3;
char global_var_4;

/* Function declarations that require identifier lookup */
extern int external_func_1(int);
extern void external_func_2(float);
extern double external_func_3(double, int);

/* Complex identifier usage patterns */
static volatile int* volatile_ptr;
const char* const_string = "test";

/* ==================== BLOCK patterns ==================== */
/* Function with nested blocks and statement expressions */
int __attribute__((noinline)) test_blocks(int n) {
    int result = 0;
    
    /* Level 1 block */
    {
        int a = n * 2;
        
        /* Level 2 block with statement expression */
        result = ({
            int b = a + 5;
            /* Level 3 block */
            {
                int c = b * 3;
                c;  /* Return from statement expression */
            }
        });
        
        /* Another block with label and goto */
        {
            int d = result;
            if (d > 100) {
                goto skip;
            }
            d += 10;
        skip:
            result = d;
        }
    }
    
    /* Additional block with local variable */
    {
        int e = result % 7;
        result = e * 2;
    }
    
    return result;
}

/* ==================== CONSTRUCTOR patterns ==================== */
struct ComplexStruct {
    int int_field;
    float float_field;
    double double_field;
    char char_field;
};

union TestUnion {
    int as_int;
    float as_float;
    struct {
        int x, y;
    } coords;
};

int __attribute__((noinline)) test_constructors(void) {
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
        .inner = { .int_field = 10, .float_field = 1.5f },
        .extra = 99
    };
    
    /* Compound literals */
    int* ptr = (int[]){10, 20, 30, 40};
    struct ComplexStruct* sp = &(struct ComplexStruct){
        .int_field = 100,
        .float_field = 9.8f
    };
    
    /* Union initializer */
    union TestUnion u1 = { .as_int = 0xDEADBEEF };
    union TestUnion u2 = { .coords = { .x = 1, .y = 2 } };
    
    return s1.int_field + arr[2] + nested.extra + ptr[1] + sp->int_field;
}

/* ==================== SSA_NAME patterns ==================== */
int __attribute__((noinline)) test_ssa(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Multiple loops to force SSA creation */
    for (int i = 0; i < n; ++i) {
        x = x + i;      /* Creates SSA for x and i */
        y = y * (i + 1); /* Creates SSA for y */
    }
    
    /* Another loop with different variable */
    for (int j = n; j > 0; --j) {
        z = z - j;      /* Creates SSA for z and j */
        x = x + z;      /* Complex SSA web */
    }
    
    /* Conditional with phi nodes */
    int result;
    if (x > y) {
        result = x * 2;
    } else {
        result = y * 3;
    }
    
    /* Loop with multiple exits */
    int k = 0;
    while (1) {
        if (k > 10) break;
        result += k;
        k++;
    }
    
    return result + z;
}

/* ==================== VECTOR (TREE_VEC) patterns ==================== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

int __attribute__((noinline)) test_vectors(void) {
    /* Vector declarations and initializations */
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c;
    
    /* Vector arithmetic operations */
    c = a + b;          /* Creates TREE_VEC nodes */
    v4si d = a * b;
    v4si e = b - a;
    
    /* Vector comparisons */
    v4si mask = a > b;
    
    /* Float vectors */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf f3 = f1 * f2;
    
    /* Double vectors */
    v2df d1 = {1.0, 2.0};
    v2df d2 = {3.0, 4.0};
    v2df d3 = d1 + d2;
    
    /* Extract elements */
    int sum = c[0] + c[1] + c[2] + c[3];
    sum += (int)f3[0];
    sum += (int)d3[0];
    
    return sum;
}
#else
int __attribute__((noinline)) test_vectors(void) {
    /* Fallback for non-GCC compilers */
    int arr[4] = {1, 2, 3, 4};
    return arr[0] + arr[1] + arr[2] + arr[3];
}
#endif

/* ==================== OpenMP (OMP_CLAUSE) patterns ==================== */
#ifdef _OPENMP
#include <omp.h>

int __attribute__((noinline)) test_openmp(int size) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static, 10)
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    int max_val = 0;
    /* Another OpenMP directive with different clauses */
    #pragma omp parallel for reduction(max:max_val) collapse(2) if(size > 1000)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int val = i * 10 + j;
            if (val > max_val) {
                max_val = val;
            }
        }
    }
    
    /* OpenMP sections */
    int section_result = 0;
    #pragma omp parallel sections private(i) firstprivate(section_result)
    {
        #pragma omp section
        {
            section_result = 1;
        }
        #pragma omp section
        {
            section_result = 2;
        }
    }
    
    return sum + max_val + section_result;
}
#else
int __attribute__((noinline)) test_openmp(int size) {
    /* Fallback without OpenMP */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += i + 1;
    }
    return sum;
}
#endif

/* ==================== Main function ==================== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Use identifiers in various ways */
    global_var_1 = 10;
    global_var_2 = 3.14f;
    volatile_ptr = &global_var_1;
    
    /* sizeof expressions with identifiers */
    result += sizeof(global_var_1);
    result += sizeof(global_var_2);
    result += sizeof(global_var_3);
    result += sizeof(global_var_4);
    
    /* Address-of operations */
    result += (int)(long)&global_var_1 % 1000;
    result += (int)(long)&global_var_2 % 1000;
    
    /* Call pattern functions */
    result += test_blocks(20);
    result += test_constructors();
    result += test_ssa(15);
    result += test_vectors();
    result += test_openmp(2000);
    
    /* Prevent dead code elimination */
    volatile int final_result = result;
    
#ifdef __cplusplus
    std::cout << "Result: " << final_result << std::endl;
#else
    printf("Result: %d\n", final_result);
#endif
    
    return final_result % 256;
}

#ifdef __cplusplus
} /* extern "C" */

/* ==================== C++ BINFO patterns ==================== */
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

int __attribute__((noinline)) test_cpp_polymorphism() {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    SecondDerived second;
    
    /* Virtual calls that involve BINFO */
    int result = base_ptr->method();
    result += derived.method();
    result += second.method();
    
    /* Casts that involve class hierarchy */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        result += derived_ptr->derived_data;
    }
    
    /* Multiple inheritance-like access */
    BaseClass& base_ref = second;
    result += base_ref.method();
    
    return result;
}

/* C++ main wrapper */
int cpp_main(int argc, char** argv) {
    int result = main(argc, argv);
    result += test_cpp_polymorphism();
    
    std::cout << "C++ Result: " << result << std::endl;
    return result % 256;
}

/* Override main for C++ compilation */
#ifndef TEST_C_ONLY
#define main cpp_main
#endif

#endif /* __cplusplus */
