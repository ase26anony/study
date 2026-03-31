/* Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */
/* For C version: gcc -O2 -fopenmp -fdump-tree-all tree_test.c -o tree_test */

#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void opaque_external_function(int*);

/* Global identifiers (IDENTIFIER_NODE) */
int global_counter = 0;
static int static_hidden = 42;
volatile int volatile_tracker = 0;

/* Complex struct for CONSTRUCTOR nodes */
struct ComplexData {
    int values[4];
    struct {
        double x, y;
    } point;
    char tag;
};

/* Another struct for nested constructors */
struct NestedStruct {
    struct ComplexData data;
    int id;
    struct NestedStruct* next;
};

/* Recursive function returning struct (CONSTRUCTOR) */
struct ComplexData build_data(int depth, int base) {
    struct ComplexData result;
    
    /* Array constructor with partial initialization */
    int arr_init[4] = {[0] = base, [2] = base * 2};
    
    for (int i = 0; i < 4; i++) {
        result.values[i] = arr_init[i] + i;
    }
    
    result.point.x = (double)base / (depth + 1);
    result.point.y = (double)(base * 2) / (depth + 1);
    result.tag = 'A' + (depth % 26);
    
    volatile_tracker++; /* Prevent tail recursion optimization */
    
    if (depth > 0) {
        struct ComplexData nested = build_data(depth - 1, base + 1);
        /* Merge results */
        result.values[1] += nested.values[0];
    }
    
    return result; /* Returns CONSTRUCTOR node */
}

/* Function with complex control flow for SSA_NAME and BLOCK */
int complex_control_flow(int n, int* results) {
    int sum = 0;
    
    /* Outer block with local variable */
    {
        int block_local = n * 2;
        
        /* goto to create interesting control flow */
        if (n % 3 == 0) {
            goto special_case;
        }
        
        for (int i = 0; i < n; i++) {
            /* Inner block */
            {
                int inner_temp = i;
                
                /* Conditional creating phi node (SSA_NAME) */
                int value;
                if (i % 2 == 0) {
                    value = inner_temp * 3;
                } else {
                    value = inner_temp + block_local;
                }
                
                /* Another conditional for more SSA complexity */
                int final_value;
                if (value > n) {
                    final_value = value - n;
                } else {
                    final_value = value + n;
                }
                
                results[i] = final_value;
                sum += final_value;
            }
        }
        
        goto end;
        
    special_case:
        /* Different block reached by goto */
        {
            int special_local = 100;
            for (int i = 0; i < n; i++) {
                results[i] = special_local + i;
                sum += results[i];
            }
        }
    }
    
end:
    return sum;
}

/* OpenMP function with multiple clauses (OMP_CLAUSE) */
int openmp_reduction(int size) {
    int total = 0;
    int arr[100][100];
    
    /* Initialize array with constructor-like syntax */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            arr[i][j] = i * 100 + j;
        }
    }
    
    #pragma omp parallel for collapse(2) reduction(+:total) \
                private(static_hidden) firstprivate(size) \
                shared(arr, global_counter) schedule(dynamic, 10)
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            /* Nested OpenMP directive */
            #pragma omp atomic
            global_counter++;
            
            int temp = arr[i][j];
            if (temp % 7 == 0) {
                temp *= 2;
            } else if (temp % 13 == 0) {
                temp /= 2;
            }
            total += temp;
        }
    }
    
    /* Another OpenMP region with different clauses */
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            volatile_tracker = total % 1000;
        }
        
        #pragma omp barrier
        
        #pragma omp for ordered
        for (int i = 0; i < 50; i++) {
            #pragma omp ordered
            {
                static_hidden += i;
            }
        }
    }
    
    return total;
}

/* C++ specific code for TREE_BINFO nodes */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int process(int x) = 0;
    virtual BaseClass* clone() = 0;
    
protected:
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    DerivedClass(int val) : derived_data(val) {
        base_data = val * 2;
    }
    
    virtual int process(int x) override {
        return x * derived_data + base_data;
    }
    
    virtual BaseClass* clone() override {
        return new DerivedClass(derived_data);
    }
    
    /* Template method for TREE_VEC */
    template<typename T>
    T transform(T input) {
        return input * derived_data;
    }
    
private:
    int derived_data;
};

class SecondDerived : public DerivedClass {
public:
    SecondDerived(int val) : DerivedClass(val), extra(val % 10) {}
    
    virtual int process(int x) override {
        return DerivedClass::process(x) + extra;
    }
    
    virtual BaseClass* clone() override {
        return new SecondDerived(extra);
    }
    
private:
    int extra;
};

void test_cpp_features(int arg) {
    BaseClass* obj1 = new DerivedClass(arg);
    BaseClass* obj2 = new SecondDerived(arg + 1);
    
    /* dynamic_cast uses BINFO */
    DerivedClass* derived = dynamic_cast<DerivedClass*>(obj1);
    if (derived) {
        int result = derived->transform<float>(3.14f);
        volatile_tracker = (int)result;
    }
    
    /* Virtual calls */
    int r1 = obj1->process(arg);
    int r2 = obj2->process(arg);
    
    /* Template instantiation (TREE_VEC) */
    std::vector<int> vec = {arg, r1, r2};
    vec.push_back(derived->transform<int>(arg));
    
    delete obj1;
    delete obj2;
}
#endif

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    int size = (argc > 2) ? atoi(argv[2]) : 100;
    
    /* Struct initialization with constructor (CONSTRUCTOR) */
    struct ComplexData data = {
        .values = {[0] = 1, [1] = iterations, [3] = size},
        .point = {.x = 3.14, .y = 2.718},
        .tag = 'Z'
    };
    
    /* Array with designated initializer */
    int designated[10] = {[2] = 5, [5] = 10, [8] = iterations};
    
    /* Nested struct initialization */
    struct NestedStruct nested = {
        .data = data,
        .id = 1001,
        .next = NULL
    };
    
    int results[100];
    int sum = 0;
    
    /* Test complex control flow (BLOCK, SSA_NAME) */
    for (int i = 0; i < iterations; i++) {
        /* Each iteration creates new blocks */
        {
            int local_block_var = i * 10;
            sum += complex_control_flow(size % 50, results);
            
            /* Call external function with identifiers */
            opaque_external_function(&local_block_var);
        }
    }
    
    /* Test recursive constructor function */
    struct ComplexData built = build_data(3, iterations);
    sum += built.values[0] + built.values[2];
    
    /* Test OpenMP features (OMP_CLAUSE) */
    int omp_result = openmp_reduction(size % 50 + 10);
    sum += omp_result;
    
    /* C++ specific tests */
    #ifdef __cplusplus
    test_cpp_features(iterations);
    
    /* More template usage for TREE_VEC */
    std::vector<std::pair<int, double>> template_vec;
    template_vec.push_back(std::make_pair(sum, 3.14159));
    template_vec.emplace_back(omp_result, 2.71828);
    #endif
    
    /* Final result based on all computations */
    printf("Result: %d (global_counter: %d, static_hidden: %d)\n", 
           sum, global_counter, static_hidden);
    
    return sum % 255;
}
