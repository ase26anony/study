/* test_tree_codes.c - Comprehensive test for GCC tree node coverage */

/* Prevent excessive optimization */
volatile int global_volatile = 0;

/* Function to prevent dead code elimination */
void use(void* p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* ========== IDENTIFIER_NODE ========== */
/* Generate many unique identifiers */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

void test_identifiers(void) {
    /* Generate multiple unique identifiers */
    int MAKE_ID(0) = 1;
    int MAKE_ID(1) = 2;
    int MAKE_ID(2) = 3;
    int MAKE_ID(3) = 4;
    int MAKE_ID(4) = 5;
    int MAKE_ID(5) = 6;
    int MAKE_ID(6) = 7;
    int MAKE_ID(7) = 8;
    int MAKE_ID(8) = 9;
    int MAKE_ID(9) = 10;
    
    /* Use them in a way that prevents optimization */
    int sum = 0;
    sum += MAKE_ID(0);
    sum += MAKE_ID(1);
    sum += MAKE_ID(2);
    sum += MAKE_ID(3);
    sum += MAKE_ID(4);
    sum += MAKE_ID(5);
    sum += MAKE_ID(6);
    sum += MAKE_ID(7);
    sum += MAKE_ID(8);
    sum += MAKE_ID(9);
    
    global_volatile = sum;
}

/* ========== TREE_VEC ========== */
/* Use GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

void test_vectors(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    
    /* Force vector operations to be kept */
    use(&c);
    use(&d);
    use(&e);
    
    /* More complex vector operations */
    v4sf f = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf g = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf h = f * g + f - g;
    use(&h);
}

/* ========== SSA_NAME ========== */
/* Create complex control flow for SSA */
int test_ssa(int n) {
    int x = 0;
    int y = 1;
    
    /* Loop with multiple assignments to create phi nodes */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            x = x + i;
        } else {
            x = x - i;
        }
        
        /* Nested condition */
        if (i % 3 == 0) {
            y = y * 2;
        } else if (i % 3 == 1) {
            y = y + 1;
        } else {
            y = y - 1;
        }
    }
    
    /* Another SSA pattern */
    int z = x;
    for (int j = 0; j < 10; j++) {
        z = z + (j % 2 ? x : y);
    }
    
    return x + y + z;
}

/* ========== BLOCK ========== */
void test_blocks(void) {
    /* Outer block with variable */
    int outer = 10;
    
    /* Nested block 1 */
    {
        int inner1 = outer + 5;
        
        /* Deeper nested block */
        {
            int inner2 = inner1 * 2;
            outer = inner2;
            
            /* Even deeper */
            {
                volatile int deepest = inner2 + 1;
                outer += deepest;
            }
        }
    }
    
    /* Another block in loop */
    for (int i = 0; i < 5; i++) {
        /* Block inside loop */
        {
            int loop_var = i * i;
            outer += loop_var;
            
            /* Conditional block */
            if (loop_var > 5) {
                int cond_var = loop_var / 2;
                outer -= cond_var;
            }
        }
    }
    
    global_volatile = outer;
}

/* ========== CONSTRUCTOR ========== */
struct Point {
    int x;
    int y;
    int z;
};

struct Data {
    struct Point p;
    int values[4];
    char tag;
};

int get_value(void) {
    return global_volatile + 1;
}

void test_constructors(void) {
    /* Non-constant struct initializer */
    struct Point p1 = { get_value(), get_value() + 1, get_value() * 2 };
    
    /* Designated initializer */
    struct Point p2 = { .y = get_value(), .x = get_value() - 1, .z = get_value() + 2 };
    
    /* Array with non-constant initializers */
    int arr[3] = { get_value(), get_value() * 2, get_value() + 5 };
    
    /* Nested struct initializer */
    struct Data d1 = {
        .p = { get_value(), get_value(), get_value() },
        .values = { get_value(), get_value() + 1, get_value() + 2, get_value() + 3 },
        .tag = 'A'
    };
    
    /* More complex constructor */
    struct Data d2 = {
        .p = p1,
        .values = { arr[0], arr[1], arr[2], p2.x },
        .tag = 'B' + (get_value() % 26)
    };
    
    use(&p1);
    use(&p2);
    use(&arr);
    use(&d1);
    use(&d2);
}

/* ========== OMP_CLAUSE ========== */
#ifdef _OPENMP
#include <omp.h>

void test_omp_clauses(void) {
    int i;
    int n = 100;
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < n; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP with multiple clauses */
    #pragma omp parallel for private(i) shared(arr, n) reduction(+:sum) schedule(dynamic, 4) num_threads(2)
    for (i = 0; i < n; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP construct with different clauses */
    int max_val = 0;
    #pragma omp parallel sections private(i) shared(arr, max_val) 
    {
        #pragma omp section
        {
            int local_max = 0;
            for (i = 0; i < n/2; i++) {
                if (arr[i] > local_max) local_max = arr[i];
            }
            #pragma omp critical
            {
                if (local_max > max_val) max_val = local_max;
            }
        }
        
        #pragma omp section
        {
            int local_max = 0;
            for (i = n/2; i < n; i++) {
                if (arr[i] > local_max) local_max = arr[i];
            }
            #pragma omp critical
            {
                if (local_max > max_val) max_val = local_max;
            }
        }
    }
    
    /* Task with clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            for (i = 0; i < 10; i++) {
                #pragma omp task firstprivate(i) shared(sum)
                {
                    sum += i;
                }
            }
        }
    }
    
    global_volatile = sum + max_val;
}
#endif

/* ========== TREE_BINFO (C++ version) ========== */
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

class AnotherDerived : public BaseClass {
public:
    virtual int method() override { return 3; }
    int another_data;
};

void test_binfo(void) {
    DerivedClass d;
    AnotherDerived ad;
    BaseClass* b1 = &d;
    BaseClass* b2 = &ad;
    
    /* Virtual calls to use vtable */
    int result = b1->method() + b2->method();
    
    /* Casts that might use BINFO */
    DerivedClass* d2 = dynamic_cast<DerivedClass*>(b1);
    AnotherDerived* ad2 = dynamic_cast<AnotherDerived*>(b2);
    
    /* Access through base pointer */
    b1->base_data = result;
    b2->base_data = result * 2;
    
    global_volatile = result + (d2 != nullptr) + (ad2 != nullptr);
}
#endif

/* ========== Main function ========== */
int main(int argc, char** argv) {
    /* Force execution of all tests */
    test_identifiers();
    test_vectors();
    
    int ssa_result = test_ssa(argc > 1 ? 100 : 50);
    test_blocks();
    test_constructors();
    
    #ifdef _OPENMP
    test_omp_clauses();
    #endif
    
    #ifdef __cplusplus
    test_binfo();
    #endif
    
    /* Use results to prevent optimization */
    int final_result = global_volatile + ssa_result;
    
    /* Return something based on all computations */
    return final_result > 0 ? 0 : 1;
}
