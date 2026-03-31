/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */
/* Compile with: gcc -O2 -fopenmp -fprofile-arcs -ftest-coverage test_tree_nodes.c -o test_tree_nodes.exe */
/* For C++ features: g++ -O2 -fopenmp -fprofile-arcs -ftest-coverage test_tree_nodes.cc -o test_tree_nodes.exe */

#ifdef __cplusplus
#include <iostream>
#else
#include <stdio.h>
#endif

/* ========== IDENTIFIER_NODE patterns ========== */
/* Global variables to force identifier creation */
int global_var_1 = 10;
float global_var_2 = 20.5;
double global_var_3 = 30.7;
char global_var_4 = 'A';

/* Function taking address of identifiers */
int* get_address_of_global(void) __attribute__((noinline));
int* get_address_of_global(void) {
    return &global_var_1;
}

/* Function using sizeof on identifiers */
size_t get_size_of_globals(void) __attribute__((noinline));
size_t get_size_of_globals(void) {
    return sizeof(global_var_1) + sizeof(global_var_2) + 
           sizeof(global_var_3) + sizeof(global_var_4);
}

/* ========== TREE_VEC patterns ========== */
#ifdef __GNUC__
/* Vector type declaration */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Vector operations */
v4si vector_operations(void) __attribute__((noinline));
v4si vector_operations(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    /* Also test float vectors */
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf h = f + g;
    
    return e;
}
#endif

/* ========== SSA_NAME patterns ========== */
/* Complex loops to force SSA generation */
int ssa_pattern_1(int n) __attribute__((noinline));
int ssa_pattern_1(int n) {
    int x = 0;
    int y = 1;
    
    /* Multiple variables modified in loop */
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
        if (i % 2 == 0) {
            x = x - y;
        } else {
            y = y + x;
        }
    }
    
    /* Nested loop with different variable */
    int z = 0;
    for (int j = 0; j < n; ++j) {
        for (int k = 0; k < j; ++k) {
            z = z + k * j;
        }
    }
    
    return x + y + z;
}

/* Another SSA pattern with conditional updates */
float ssa_pattern_2(int iterations) __attribute__((noinline));
float ssa_pattern_2(int iterations) {
    float acc = 0.0f;
    float factor = 1.0f;
    
    for (int i = 0; i < iterations; ++i) {
        acc = acc + (i * factor);
        if (acc > 100.0f) {
            factor = factor * 0.5f;
            acc = acc * 0.9f;
        } else {
            factor = factor * 1.1f;
        }
    }
    
    return acc;
}

/* ========== BLOCK patterns ========== */
/* Nested blocks and statement expressions */
int block_patterns(void) __attribute__((noinline));
int block_patterns(void) {
    int result = 0;
    
    /* Level 1 block */
    {
        int a = 10;
        
        /* Level 2 block */
        {
            int b = 20;
            
            /* Level 3 block */
            {
                int c = 30;
                result = a + b + c;
            }
        }
    }
    
    /* GCC statement expression (creates a block) */
    result += ({
        int x = 5;
        int y = 10;
        x * y;
    });
    
    /* More nested scopes with variable declarations */
    {
        int temp1 = 100;
        {
            int temp2 = 200;
            {
                int temp3 = 300;
                result += temp1 + temp2 + temp3;
            }
        }
    }
    
    return result;
}

/* ========== CONSTRUCTOR patterns ========== */
/* Structure and array initializers */
int constructor_patterns(void) __attribute__((noinline));
int constructor_patterns(void) {
    /* Structure with designated initializer */
    struct ComplexStruct {
        int id;
        float values[4];
        char name[16];
        double extra;
    };
    
    struct ComplexStruct s1 = {
        .id = 1001,
        .values = {1.1f, 2.2f, 3.3f, 4.4f},
        .name = "test",
        .extra = 99.99
    };
    
    /* Array with initializer */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Compound literal */
    int* ptr = (int[3]){1, 2, 3};
    
    /* Nested structure initializer */
    struct Inner {
        int x, y;
    };
    
    struct Outer {
        struct Inner a;
        struct Inner b;
        int z;
    };
    
    struct Outer o = {
        .a = {.x = 1, .y = 2},
        .b = {.x = 3, .y = 4},
        .z = 5
    };
    
    /* Union with initializer */
    union Data {
        int i;
        float f;
        char str[20];
    };
    
    union Data d1 = {.i = 42};
    union Data d2 = {.f = 3.14f};
    
    return s1.id + arr[0] + ptr[1] + o.z + d1.i;
}

/* ========== OMP_CLAUSE patterns ========== */
/* OpenMP directives with various clauses */
void omp_patterns(int size) __attribute__((noinline));
void omp_patterns(int size) {
    int i;
    int sum = 0;
    int arr[1000];
    
    /* Initialize array */
    for (i = 0; i < 1000; i++) {
        arr[i] = i;
    }
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static, 16)
    for (i = 0; i < 1000; i++) {
        sum += arr[i];
    }
    
    /* Another parallel region with different clauses */
    int max_val = 0;
    #pragma omp parallel for private(i) reduction(max:max_val) collapse(2)
    for (i = 0; i < 100; i++) {
        for (int j = 0; j < 10; j++) {
            int idx = i * 10 + j;
            if (arr[idx] > max_val) {
                max_val = arr[idx];
            }
        }
    }
    
    /* Sections with private and firstprivate */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            int local_sum = 0;
            for (i = 0; i < 500; i++) {
                local_sum += arr[i];
            }
        }
        
        #pragma omp section
        {
            int local_prod = 1;
            for (i = 500; i < 1000; i++) {
                local_prod *= (arr[i] + 1);
            }
        }
    }
    
    /* Single directive with copyprivate */
    int shared_var = 0;
    #pragma omp parallel private(i)
    {
        #pragma omp single copyprivate(shared_var)
        {
            shared_var = 42;
        }
    }
    
    volatile int prevent_opt = sum + max_val + shared_var;
    (void)prevent_opt;
}

#ifdef __cplusplus
/* ========== TREE_BINFO patterns (C++ only) ========== */
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

class AnotherDerived : public BaseClass {
public:
    virtual int get_value() const override { return 30; }
    int another_data;
};

int binfo_patterns(void) __attribute__((noinline));
int binfo_patterns(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call through base pointer */
    int val1 = base_ptr->get_value();
    
    /* Dynamic cast */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    int val2 = derived_ptr ? derived_ptr->get_value() : 0;
    
    /* Multiple inheritance-like usage */
    AnotherDerived another;
    BaseClass* base_ptr2 = &another;
    int val3 = base_ptr2->get_value();
    
    /* Array of base pointers */
    BaseClass* objects[3];
    objects[0] = &derived;
    objects[1] = &another;
    
    int total = 0;
    for (int i = 0; i < 2; i++) {
        total += objects[i]->get_value();
    }
    
    return val1 + val2 + val3 + total;
}
#endif

/* ========== Main function ========== */
int main(void) {
    int result = 0;
    volatile int output; /* volatile to prevent optimization */
    
    /* Call all pattern functions */
    result += (int)get_size_of_globals();
    
#ifdef __GNUC__
    v4si vec_result = vector_operations();
    result += vec_result[0] + vec_result[1];
#endif
    
    result += ssa_pattern_1(100);
    result += (int)ssa_pattern_2(50);
    result += block_patterns();
    result += constructor_patterns();
    
    /* OpenMP patterns */
    omp_patterns(1000);
    
#ifdef __cplusplus
    result += binfo_patterns();
#endif
    
    /* Use result to prevent dead code elimination */
    output = result;
    
#ifdef __cplusplus
    std::cout << "Result: " << output << std::endl;
#else
    printf("Result: %d\n", output);
#endif
    
    return 0;
}
