/* Test program to exercise tree node dispatch in GCC */
#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void opaque_external_function(int*);

/* Global identifiers for IDENTIFIER_NODE coverage */
int global_var_1;
static int static_var_1;
extern int extern_var_1;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    double b;
    char c;
    int* d;
};

/* Array for TREE_VEC-like representations */
int multi_array[2][3][4];

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_struct_builder(int depth, int* counter) {
    struct ComplexStruct result;
    result.a = *counter;
    result.b = (*counter) * 1.5;
    result.c = 'A' + (*counter % 26);
    result.d = counter;
    
    (*counter)++;
    
    if (depth > 0) {
        struct ComplexStruct nested = recursive_struct_builder(depth - 1, counter);
        result.a += nested.a;
        result.b += nested.b;
    }
    
    return result;  /* CONSTRUCTOR node generated here */
}

/* Function with complex control flow for SSA_NAME */
int ssa_generator(int x, int y) {
    volatile int result;  /* Prevent optimization */
    
    /* Complex conditional for SSA phi nodes */
    if (x > 0) {
        if (y < 0) {
            result = x * y;
        } else {
            result = x + y;
        }
    } else {
        if (y > 100) {
            result = y - x;
        } else {
            result = x * 2 - y;
        }
    }
    
    /* Loop with SSA */
    for (int i = 0; i < x; i++) {
        if (i % 2 == 0) {
            result += i * 2;
        } else {
            result -= i;
        }
    }
    
    return result;
}

/* Function with BLOCK nodes and goto */
int block_and_goto_test(int val) {
    int a = val;
    
    /* First block with local variable */
    {
        int hidden_in_block = a * 2;
        a += hidden_in_block;
        
        if (a > 100) {
            goto skip_section;
        }
        
        /* This section might be skipped */
        int another_local = 50;
        a += another_local;
    }
    
skip_section:
    
    /* Another block */
    {
        int final_adjust = 10;
        a += final_adjust;
    }
    
    return a;
}

#ifdef __cplusplus
/* C++ specific code for TREE_BINFO */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int virtual_method(int x) {
        return x * 2;
    }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method(int x) override {
        return x * 3 + base_data;
    }
    int derived_data;
};

class SecondDerived : public DerivedClass {
public:
    virtual int virtual_method(int x) override {
        return x * 4 + base_data + derived_data;
    }
};

void cpp_binfo_test() {
    DerivedClass* obj1 = new DerivedClass();
    SecondDerived* obj2 = new SecondDerived();
    BaseClass* base_ptr;
    
    obj1->base_data = 10;
    obj1->derived_data = 20;
    
    obj2->base_data = 30;
    obj2->derived_data = 40;
    
    /* Dynamic dispatch for BINFO */
    base_ptr = obj1;
    int result1 = base_ptr->virtual_method(5);
    
    base_ptr = dynamic_cast<BaseClass*>(obj2);
    int result2 = base_ptr->virtual_method(5);
    
    printf("C++ BINFO test results: %d, %d\n", result1, result2);
    
    delete obj1;
    delete obj2;
}
#endif

/* Main function with OpenMP for OMP_CLAUSE */
int main(int argc, char** argv) {
    /* Use argc to prevent optimization */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    
    /* CONSTRUCTOR: Initialize struct with designators */
    struct ComplexStruct cs = {
        .a = 42,
        .b = 3.14159,
        .c = 'X',
        .d = &iterations
    };
    
    /* CONSTRUCTOR: Array with designators (may create TREE_VEC) */
    int designated_array[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* TREE_VEC: Complex nested initializer */
    int nested_init[2][3] = {{1, 2, 3}, {4, 5, 6}};
    
    /* BLOCK test */
    int block_result = block_and_goto_test(iterations);
    
    /* SSA_NAME test */
    int ssa_result = ssa_generator(iterations, iterations / 2);
    
    /* Recursive struct builder (CONSTRUCTOR) */
    int counter = 1;
    struct ComplexStruct recursive_result = 
        recursive_struct_builder(5, &counter);
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE) */
    int sum = 0;
    int private_var = 0;
    
    #pragma omp parallel for reduction(+:sum) \
            private(private_var) firstprivate(iterations) \
            shared(designated_array) collapse(2) \
            schedule(dynamic)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            private_var = i * j;
            sum += private_var + designated_array[i % 10];
            
            /* Nested OpenMP for more clauses */
            #pragma omp atomic
            designated_array[i % 10]++;
        }
    }
    
    /* Another OpenMP with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) \
            num_threads(4) if(iterations > 50)
    for (int i = 0; i < iterations; i++) {
        int local_max = ssa_generator(i, i % 10);
        if (local_max > max_val) {
            max_val = local_max;
        }
    }
    
    /* Call external function with various identifiers */
    opaque_external_function(&sum);
    
    /* Use all results to prevent dead code elimination */
    int final_result = 
        block_result + 
        ssa_result + 
        recursive_result.a + 
        sum + 
        max_val +
        designated_array[0] +
        nested_init[0][0];
    
    #ifdef __cplusplus
    /* C++ BINFO test if compiled as C++ */
    cpp_binfo_test();
    #endif
    
    printf("Final checksum: %d\n", final_result);
    
    /* More IDENTIFIER_NODE usage */
    {
        int local_ident_1 = final_result;
        static int static_local_ident;
        static_local_ident = local_ident_1;
        
        /* Use goto with label (more BLOCK nodes) */
        if (local_ident_1 > 1000) {
            goto large_result;
        }
        
        printf("Result is moderate\n");
        goto finish;
        
large_result:
        printf("Result is large\n");
        
finish:
        /* Empty statement for label */
        ;
    }
    
    return final_result % 256;
}
