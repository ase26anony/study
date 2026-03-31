/* test_tree_kind.c - Coverage test for GCC's get_kind function */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_tree_kind */
/* For C++ mode: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind.c -o test_tree_kind_cpp */

#include <stdio.h>
#include <stdlib.h>

/* ===== IDENTIFIER_NODE ===== */
/* Any variable/function name creates IDENTIFIER_NODE */
int global_identifier = 42;
void function_identifier(void) {
    int local_identifier = 10;
    local_identifier += global_identifier;
}

/* ===== TREE_VEC ===== */
/* Using GCC statement expressions with multiple elements */
#ifdef __GNUC__
int tree_vec_example(void) {
    /* This creates a TREE_VEC in GCC's internal representation */
    int result = ({ 
        int a = 5; 
        int b = 10; 
        int c = a + b; 
        c; 
    });
    return result;
}
#endif

/* ===== SSA_NAME ===== */
/* Complex enough to trigger SSA form creation */
int ssa_name_example(int n) {
    int a = 0, b = 1, c;
    
    /* Loop with arithmetic to force SSA */
    for (int i = 0; i < n; ++i) {
        a = a + i;
        b = b * 2;
        c = a + b;
    }
    
    /* Conditional to create phi nodes */
    if (n > 0) {
        a = c;
    } else {
        a = b;
    }
    
    return a + b + c;
}

/* ===== BLOCK ===== */
/* Nested blocks with local variables */
void block_example(void) {
    int outer = 1;
    
    {
        /* Inner block 1 */
        int inner1 = 2;
        outer += inner1;
        
        {
            /* Inner block 2 */
            int inner2 = 3;
            outer += inner2;
            
            {
                /* Deeply nested block */
                int inner3 = 4;
                outer += inner3;
            }
        }
    }
    
    printf("Block result: %d\n", outer);
}

/* ===== CONSTRUCTOR ===== */
/* Aggregate initializers */
struct my_struct {
    int x;
    double y;
    char z[10];
};

int constructor_example(void) {
    /* Array constructor */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Struct constructor */
    struct my_struct s = {.x = 10, .y = 3.14, .z = "hello"};
    
    /* Nested constructor */
    struct { int a; int b; int c; } nested = {1, {2}, 3};
    
    return arr[0] + s.x;
}

/* ===== OMP_CLAUSE ===== */
#ifdef _OPENMP
void omp_clause_example(int n) {
    int i;
    int sum = 0;
    
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static)
    for (i = 0; i < n; i++) {
        sum += i;
    }
    
    printf("OpenMP sum: %d\n", sum);
    
    /* Additional OpenMP constructs */
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("Thread %d executing single\n", omp_get_thread_num());
        }
        
        #pragma omp barrier
        
        #pragma omp for nowait
        for (i = 0; i < 5; i++) {
            #pragma omp atomic
            sum++;
        }
    }
}
#endif

/* ===== C++ Specific: TREE_BINFO ===== */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void method() = 0;
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void method() override {
        base_data = 42;
    }
    int derived_data;
};

class MultiBase1 {
public:
    virtual void m1() {}
    int data1;
};

class MultiBase2 {
public:
    virtual void m2() {}
    int data2;
};

class MultipleDerived : public MultiBase1, public MultiBase2 {
public:
    virtual void m1() override {}
    virtual void m2() override {}
    int data3;
};

void tree_binfo_example(void) {
    DerivedClass d;
    d.method();
    
    MultipleDerived md;
    md.m1();
    md.m2();
    
    BaseClass* ptr = &d;
    ptr->method();
}
#endif

/* ===== Main driver ===== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Trigger IDENTIFIER_NODE */
    function_identifier();
    
    /* Trigger TREE_VEC */
    #ifdef __GNUC__
    result += tree_vec_example();
    #endif
    
    /* Trigger SSA_NAME */
    result += ssa_name_example(100);
    
    /* Trigger BLOCK */
    block_example();
    
    /* Trigger CONSTRUCTOR */
    result += constructor_example();
    
    /* Trigger OMP_CLAUSE */
    #ifdef _OPENMP
    omp_clause_example(1000);
    #endif
    
    /* Trigger TREE_BINFO (C++ only) */
    #ifdef __cplusplus
    tree_binfo_example();
    #endif
    
    printf("Final result: %d\n", result);
    
    /* Additional complexity to ensure middle-end processing */
    volatile int* volatile_ptr = &result;
    for (int i = 0; i < 10; i++) {
        *volatile_ptr += i * i;
        
        /* Switch statement for additional tree node variety */
        switch (i % 4) {
            case 0: result += 1; break;
            case 1: result += 2; break;
            case 2: result += 3; break;
            case 3: result += 4; break;
        }
    }
    
    return result > 0 ? 0 : 1;
}

/* Additional functions to increase compilation complexity */
void extra_complexity_1(int x) {
    /* Recursive-like pattern without actual recursion */
    int arr[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i * j + x;
        }
    }
    
    /* Pointer arithmetic */
    int* ptr = &arr[0][0];
    for (int i = 0; i < 100; i++) {
        ptr[i] = ptr[i] * 2;
    }
}

void extra_complexity_2(void) {
    /* Function pointers */
    void (*func_ptr)(int) = extra_complexity_1;
    func_ptr(42);
    
    /* Union with different types */
    union {
        int i;
        float f;
        char c[4];
    } u;
    
    u.i = 0x12345678;
    u.f = 3.14f;
}
