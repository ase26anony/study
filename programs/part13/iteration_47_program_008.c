/* Compile with: g++ -O2 -fopenmp -std=c++11 -fdump-tree-all tree_test.cc -o tree_test */

#include <cstdio>
#include <cstdlib>

/* External function to prevent optimization */
extern "C" void opaque_external_function(int*);

/* Global identifiers for IDENTIFIER_NODE coverage */
int global_identifier_1;
static int static_identifier_2;
extern int extern_identifier_3;

/* Complex struct for CONSTRUCTOR nodes */
struct ComplexStruct {
    int a;
    double b;
    char c;
    int* d;
};

/* Recursive function returning struct (CONSTRUCTOR) */
ComplexStruct recursive_struct_builder(int depth, int seed) {
    ComplexStruct result;
    result.a = seed;
    result.b = seed * 1.5;
    result.c = 'A' + (seed % 26);
    result.d = &global_identifier_1;
    
    if (depth > 0) {
        ComplexStruct nested = recursive_struct_builder(depth - 1, seed + 1);
        result.a += nested.a;
        result.b += nested.b;
    }
    return result;  /* CONSTRUCTOR node for return value */
}

/* C++ classes for TREE_BINFO coverage */
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

/* Template for TREE_VEC coverage */
template<typename T, int N>
class TemplateVec {
    T data[N];
public:
    TemplateVec() {
        for (int i = 0; i < N; i++) {
            data[i] = T();
        }
    }
    T& operator[](int idx) { return data[idx]; }
};

/* Function with complex control flow for SSA_NAME and BLOCK coverage */
int complex_control_flow(int argc, char** argv) {
    volatile int input = argc;  /* Prevent optimization */
    int result = 0;
    
    /* BLOCK node with local variable */
    {
        int block_local = input * 2;
        result += block_local;
        
        /* Another nested block */
        {
            int nested_block_local = block_local + 1;
            result += nested_block_local;
            goto skip_part;  /* Jump to stress CFG */
            
            /* This code will be skipped */
            int unused = 100;
            result += unused;
        }
        
        skip_part:
        /* Continue after goto */
        result += 5;
    }
    
    /* More blocks with gotos */
    {
        int x = 10;
        if (input > 2) {
            goto label2;
        }
        x = 20;
        label2:
        result += x;
    }
    
    /* SSA_NAME generation: phi nodes */
    int ssa_var;
    if (input > 5) {
        ssa_var = 100;
    } else if (input > 3) {
        ssa_var = 200;
    } else {
        ssa_var = 300;
    }
    
    /* Use in loop to create more SSA complexity */
    for (int i = 0; i < ssa_var % 10; i++) {
        int loop_ssa;
        if (i % 2 == 0) {
            loop_ssa = i * 2;
        } else {
            loop_ssa = i * 3;
        }
        result += loop_ssa;
    }
    
    return result;
}

/* OpenMP function with multiple clauses for OMP_CLAUSE coverage */
int openmp_reduction(int size, int* data) {
    int sum = 0;
    int product = 1;
    
    #pragma omp parallel sections private(size) firstprivate(data) reduction(+:sum) reduction(*:product)
    {
        #pragma omp section
        {
            for (int i = 0; i < size; i++) {
                sum += data[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = 0; i < size; i++) {
                product *= (data[i] + 1);  /* +1 to avoid zero product */
            }
        }
    }
    
    /* Nested OpenMP with collapse clause */
    #pragma omp parallel for collapse(2) shared(data) private(size) schedule(dynamic, 4)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            data[i * 10 + j] = i + j;
        }
    }
    
    return sum + product;
}

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time optimization */
    volatile int seed = argc;
    
    /* 1. CONSTRUCTOR nodes - aggregate initialization */
    ComplexStruct cs = {1, 2.5, 'X', &global_identifier_1};
    ComplexStruct cs2 = recursive_struct_builder(3, seed);
    
    /* Array with designated initializer (more CONSTRUCTOR nodes) */
    int arr[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    /* 2. TREE_VEC nodes - template instantiation */
    TemplateVec<int, 5> tvec;
    TemplateVec<double, 3> tvec2;
    
    /* 3. Complex control flow for SSA_NAME and BLOCK */
    int cf_result = complex_control_flow(argc, argv);
    
    /* 4. OpenMP with multiple clauses */
    int omp_data[100];
    for (int i = 0; i < 100; i++) {
        omp_data[i] = (i + seed) % 7;
    }
    int omp_result = openmp_reduction(100, omp_data);
    
    /* 5. C++ class hierarchy for TREE_BINFO */
    BaseClass* base_ptr;
    if (seed % 2 == 0) {
        base_ptr = new DerivedClass();
    } else {
        base_ptr = new SecondDerived();
    }
    
    base_ptr->base_data = seed;
    if (DerivedClass* derived = dynamic_cast<DerivedClass*>(base_ptr)) {
        derived->derived_data = seed * 2;
    }
    
    int virtual_result = base_ptr->virtual_method(seed);
    
    /* 6. Call external function with various identifiers */
    opaque_external_function(&global_identifier_1);
    
    /* 7. Use all results to prevent dead code elimination */
    int final_result = cs.a + cs2.a + cf_result + omp_result + virtual_result 
                     + tvec[0] + arr[0] + static_identifier_2;
    
    printf("Final result: %d\n", final_result);
    
    delete base_ptr;
    return final_result % 256;
}

/* Dummy implementation to satisfy external reference */
extern "C" void opaque_external_function(int* p) {
    *p += 1;
}
