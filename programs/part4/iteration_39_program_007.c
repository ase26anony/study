/* test_tree_classification.c - Comprehensive test for GCC tree node classification */

/* Prevent unwanted optimizations */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

#include <stdio.h>
#include <stdlib.h>

/* ========== IDENTIFIER_NODE generation ========== */
/* Generate many unique identifiers using macros */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(var_, n)
#define MAKE_FUNC(n) CONCAT(func_, n)

/* Generate multiple identifiers */
#define GEN_IDENTS(n) \
    int MAKE_ID(n) = n; \
    void MAKE_FUNC(n)(void) { volatile int t = MAKE_ID(n); (void)t; }

/* Generate 20 identifiers to ensure IDENTIFIER_NODE creation */
GEN_IDENTS(0) GEN_IDENTS(1) GEN_IDENTS(2) GEN_IDENTS(3) GEN_IDENTS(4)
GEN_IDENTS(5) GEN_IDENTS(6) GEN_IDENTS(7) GEN_IDENTS(8) GEN_IDENTS(9)
GEN_IDENTS(10) GEN_IDENTS(11) GEN_IDENTS(12) GEN_IDENTS(13) GEN_IDENTS(14)
GEN_IDENTS(15) GEN_IDENTS(16) GEN_IDENTS(17) GEN_IDENTS(18) GEN_IDENTS(19)

/* ========== TREE_VEC generation ========== */
/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE USED void test_vectors(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Multiple vector operations to generate TREE_VEC nodes */
    v4si r1 = a + b;
    v4si r2 = a * b;
    v4si r3 = r1 - c;
    v4si r4 = r2 / (v4si){2, 2, 2, 2};
    
    /* Use volatile to prevent optimization */
    volatile v4si vr = r1 + r2 + r3 + r4;
    (void)vr;
    
    /* More vector operations with different types */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fr = f1 * f2 + f1 - f2;
    
    volatile v4sf vfr = fr;
    (void)vfr;
}

/* ========== SSA_NAME generation ========== */
NOINLINE USED int test_ssa(int n) {
    int x = 0, y = 0, z = 0;
    volatile int trigger = 1;
    
    /* Complex loop with multiple branches to create SSA form */
    for (int i = 0; i < n; i++) {
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
        
        if (trigger & 1) {
            x = i * 2;
            y = x + i;
        } else {
            x = i / 2;
            y = x - i;
        }
        
        if (i % 3 == 0) {
            z = x + y;
        } else if (i % 3 == 1) {
            z = x - y;
        } else {
            z = x * y;
        }
        
        /* Use all variables to prevent dead code elimination */
        trigger += z;
    }
    
    return x + y + z + trigger;
}

/* ========== BLOCK generation ========== */
NOINLINE USED void test_blocks(void) {
    int outer = 0;
    
    /* Level 1 block */
    {
        int level1 = 1;
        
        /* Level 2 block */
        {
            int level2 = 2;
            
            /* Level 3 block inside if */
            if (outer < 10) {
                int level3 = 3;
                outer += level1 + level2 + level3;
                
                /* Level 4 block inside loop */
                for (int i = 0; i < 5; i++) {
                    int level4 = i;
                    {
                        /* Level 5 block */
                        int level5 = level4 * 2;
                        outer += level5;
                        
                        /* Level 6 block in switch */
                        switch (i % 3) {
                            case 0: {
                                int level6 = 100;
                                outer += level6;
                                break;
                            }
                            case 1: {
                                int level6 = 200;
                                outer += level6;
                                break;
                            }
                            default: {
                                int level6 = 300;
                                outer += level6;
                                break;
                            }
                        }
                    }
                }
            }
        }
        
        /* Another block after the nested ones */
        {
            int after_block = 999;
            outer += after_block;
        }
    }
    
    volatile int result = outer;
    (void)result;
}

/* ========== CONSTRUCTOR generation ========== */
struct ComplexStruct {
    int a, b, c;
    float f;
    double d;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

NOINLINE USED struct NestedStruct test_constructors(int base) {
    /* Non-constant initializer function */
    int get_value(void) {
        static int counter = 0;
        return counter++ + base;
    }
    
    /* Complex designated initializer with non-constant expressions */
    struct ComplexStruct cs = {
        .a = get_value(),
        .b = get_value() * 2,
        .c = get_value() + base,
        .f = get_value() / 3.0f,
        .d = get_value() * 1.5
    };
    
    /* Array with non-constant initializers */
    int arr[5] = {
        get_value(),
        get_value() + 1,
        get_value() * 2,
        get_value() - 3,
        get_value() / 2
    };
    
    /* Nested struct initializer */
    struct NestedStruct ns = {
        .inner = {
            .a = arr[0],
            .b = arr[1],
            .c = arr[2],
            .f = arr[3] * 0.5f,
            .d = arr[4] * 0.25
        },
        .extra = get_value()
    };
    
    /* Use volatile to prevent optimization */
    volatile struct NestedStruct vns = ns;
    (void)vns;
    
    return ns;
}

/* ========== OMP_CLAUSE generation ========== */
#ifdef _OPENMP
#include <omp.h>

NOINLINE USED int test_omp_clauses(int size) {
    int sum = 0;
    int* array = (int*)malloc(size * sizeof(int));
    
    if (!array) return -1;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel shared(array) private(size) reduction(+:sum) \
        num_threads(4) if(size > 100) default(none)
    {
        #pragma omp for schedule(dynamic, 4) nowait
        for (int i = 0; i < size; i++) {
            sum += array[i];
        }
        
        /* Nested OpenMP sections */
        #pragma omp sections firstprivate(sum) lastprivate(array)
        {
            #pragma omp section
            {
                sum += 1;
            }
            #pragma omp section
            {
                sum += 2;
            }
        }
        
        /* OpenMP task with dependencies */
        #pragma omp task depend(inout: sum) final(size > 1000)
        {
            sum *= 2;
        }
        
        #pragma omp taskwait
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: sum) collapse(2) aligned(array: 64) \
        linear(i:1) safelen(8) simdlen(4)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++) {
            sum += array[i] * j;
        }
    }
    
    free(array);
    return sum;
}
#else
NOINLINE USED int test_omp_clauses(int size) {
    /* Fallback without OpenMP */
    return size * (size + 1) / 2;
}
#endif

/* ========== TREE_BINFO generation (C++ version) ========== */
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

class MultiDerived : public DerivedClass {
public:
    virtual int method() override { return 3; }
    int multi_data;
};

NOINLINE USED int test_binfo(void) {
    BaseClass* obj1 = new DerivedClass();
    BaseClass* obj2 = new MultiDerived();
    DerivedClass* obj3 = new MultiDerived();
    
    int result = obj1->method() + obj2->method() + obj3->method();
    
    /* Use dynamic_cast to trigger BINFO lookups */
    if (DerivedClass* d = dynamic_cast<DerivedClass*>(obj2)) {
        result += d->derived_data;
    }
    
    delete obj1;
    delete obj2;
    delete obj3;
    
    return result;
}
#else
/* C version using LTO - may generate BINFO-like structures */
struct BaseStruct {
    int type_id;
    void* vtable;
};

struct DerivedStruct {
    struct BaseStruct base;
    int extra_data;
};

NOINLINE USED int test_binfo(void) {
    struct DerivedStruct ds = { {1, NULL}, 42 };
    volatile int result = ds.base.type_id + ds.extra_data;
    return result;
}
#endif

/* ========== Main function combining everything ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Call all test functions to generate various tree nodes */
    
    /* 1. Test identifiers - call generated functions */
    func_0(); func_1(); func_2(); func_3(); func_4();
    func_5(); func_6(); func_7(); func_8(); func_9();
    
    /* 2. Test vectors */
    test_vectors();
    
    /* 3. Test SSA with complex control flow */
    result += test_ssa(argc > 1 ? atoi(argv[1]) : 100);
    
    /* 4. Test nested blocks */
    test_blocks();
    
    /* 5. Test constructors with non-constant initializers */
    struct NestedStruct ns = test_constructors(result);
    result += ns.extra;
    
    /* 6. Test OpenMP clauses */
    result += test_omp_clauses(1000);
    
    /* 7. Test BINFO (C++ classes or LTO structures) */
    result += test_binfo();
    
    /* 8. More identifier usage */
    for (int i = 0; i < 20; i++) {
        switch (i % 4) {
            case 0: result += var_0 + var_4 + var_8; break;
            case 1: result += var_1 + var_5 + var_9; break;
            case 2: result += var_2 + var_6 + var_10; break;
            default: result += var_3 + var_7 + var_11; break;
        }
    }
    
    /* Final result to prevent optimization of entire program */
    printf("Final result: %d\n", result);
    
    return result == 0 ? 0 : 1;
}
