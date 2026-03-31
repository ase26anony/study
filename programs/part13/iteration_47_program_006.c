/* Test program to trigger tree_kind dispatch for various TREE_CODE cases */
#ifdef __cplusplus
#include <vector>
#include <iostream>
extern "C" {
#endif

/* External function to prevent optimization */
extern void use(void*);
extern int get_value(void);

/* Global identifiers (IDENTIFIER_NODE) */
int global_var = 42;
static int static_var = 100;
extern int extern_var;

/* Struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    int b;
    int c;
};

/* Array with designator (TREE_VEC in C mode) */
int designated_array[10] = {[2] = 5, [5] = 10, [9] = 20};

#ifdef __cplusplus
/* C++ class hierarchy for TREE_BINFO */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int virtual_method() { return 1; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method() override { return 2; }
    int derived_data;
};

template<typename T>
class TemplateClass {
    T data;
public:
    TemplateClass(T d) : data(d) {}
    T get() { return data; }
};
#endif

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexStruct recursive_struct_builder(int depth, int value) {
    struct ComplexStruct result = {value, value * 2, value * 3};
    if (depth > 0) {
        struct ComplexStruct inner = recursive_struct_builder(depth - 1, value + 1);
        result.a += inner.a;
        result.b += inner.b;
        result.c += inner.c;
    }
    return result;
}

/* Function with complex control flow for SSA_NAME and BLOCK */
int complex_control_flow(int argc, char** argv) {
    int result = 0;
    volatile int vol = get_value(); /* Prevent optimization */
    
    /* BLOCK with local variable and goto */
    {
        int block_local = vol * 2;
        if (argc > 1) {
            goto skip_init;
        }
        block_local += 10;
    skip_init:
        result += block_local;
    }
    
    /* Another BLOCK with nested goto */
    {
        int x = vol;
        if (x > 100) {
            goto middle;
        }
        x += 50;
        goto end;
    middle:
        x -= 25;
    end:
        result += x;
    }
    
    /* SSA_NAME generation through phi nodes */
    int ssa_var;
    for (int i = 0; i < vol; i++) {
        if (i % 2 == 0) {
            ssa_var = i * 2;  /* Assignment in one path */
        } else {
            ssa_var = i * 3;  /* Assignment in another path - creates phi */
        }
        
        /* Use ssa_var to prevent elimination */
        if (ssa_var % 7 == 0) {
            result += ssa_var;
        }
    }
    
    /* More SSA complexity with nested loops */
    int outer = vol;
    while (outer-- > 0) {
        int inner = 5;
        int temp;
        do {
            if (inner % 2) {
                temp = inner * outer;
            } else {
                temp = inner + outer;
            }
            result += temp;
        } while (inner-- > 0);
    }
    
    return result;
}

/* OpenMP function with multiple clauses (OMP_CLAUSE) */
int openmp_reduction(int size) {
    int sum = 0;
    int arr[100][100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * j;
        }
    }
    
    /* Multiple OpenMP clauses */
    #pragma omp parallel for private(i) firstprivate(size) shared(arr) reduction(+:sum) collapse(2) schedule(dynamic)
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            if ((i + j) % size == 0) {
                sum += arr[i][j];
            }
        }
    }
    
    /* Nested OpenMP with more clauses */
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < 50; i++) {
            #pragma omp atomic
            sum += i;
        }
        
        #pragma omp single
        {
            sum *= 2;
        }
    }
    
    return sum;
}

int main(int argc, char** argv) {
    int result = 0;
    
    /* Use command line args to prevent optimization */
    int iterations = (argc > 1) ? get_value() : 10;
    
    /* CONSTRUCTOR nodes through aggregate initialization */
    struct ComplexStruct cs1 = {1, 2, 3};
    struct ComplexStruct cs2 = {.a = 4, .c = 6, .b = 5};
    struct ComplexStruct cs3 = recursive_struct_builder(3, iterations);
    
    result += cs1.a + cs1.b + cs1.c;
    result += cs2.a + cs2.b + cs2.c;
    result += cs3.a + cs3.b + cs3.c;
    
    /* Use designated array (TREE_VEC) */
    for (int i = 0; i < 10; i++) {
        result += designated_array[i];
    }
    
    /* Complex control flow for SSA_NAME and BLOCK */
    result += complex_control_flow(argc, argv);
    
    /* OpenMP computation */
    result += openmp_reduction(iterations % 7 + 1);
    
    /* Call external function with various identifiers */
    use(&global_var);
    use(&static_var);
    use(&result);
    
    #ifdef __cplusplus
    /* C++ specific code for TREE_BINFO */
    BaseClass* base_ptr;
    DerivedClass derived_obj;
    base_ptr = &derived_obj;
    
    /* Virtual call through base pointer */
    result += base_ptr->virtual_method();
    
    /* Template instantiation (TREE_VEC in C++) */
    TemplateClass<int> tc1(42);
    TemplateClass<double> tc2(3.14);
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    result += tc1.get();
    result += (int)tc2.get();
    for (int v : vec) {
        result += v;
    }
    #endif
    
    /* Final output to ensure all code is live */
    printf("Result: %d\n", result);
    
    return result % 256;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
