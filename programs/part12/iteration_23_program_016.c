/* test_targhooks.c - Comprehensive test to trigger artificial symbol generation */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

/* Pattern 1: Sanitizer triggers */
#ifdef __SANITIZE_ADDRESS__
void asan_trigger_stack_buffer_overflow() {
    int arr[10];
    /* Deliberate overflow - ASan should catch this */
    arr[15] = 0xdeadbeef;
}

void asan_trigger_use_after_return() {
    static volatile int* dangling_ptr = NULL;
    
    if (dangling_ptr) {
        *dangling_ptr = 42;  /* Use after return if triggered */
    }
}
#endif

/* Pattern 2: ThreadSanitizer data race */
static int shared_counter = 0;

void* thread_func(void* arg) {
    for (int i = 0; i < 1000; i++) {
        shared_counter++;  /* Data race - TSan should detect */
    }
    return NULL;
}

void tsan_trigger_data_race() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Shared counter (racy): %d\n", shared_counter);
}

/* Pattern 3: OpenMP parallel region with reduction */
#ifdef _OPENMP
#include <omp.h>
void omp_trigger_reduction() {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
    printf("OpenMP reduction sum: %d\n", sum);
}

void omp_trigger_private_vars() {
    int private_var = 0;
    #pragma omp parallel private(private_var)
    {
        private_var = omp_get_thread_num();
        #pragma omp critical
        {
            printf("Thread %d private var: %d\n", 
                   omp_get_thread_num(), private_var);
        }
    }
}
#endif

/* Pattern 4: Complex inline assembly */
void asm_trigger_clobbers() {
    int x = 42, y = 0;
    
    /* Complex asm with multiple clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (y)
        : "r" (x)
        : "%eax", "%ebx", "%ecx", "memory", "cc"
    );
    
    printf("Assembly result: %d -> %d\n", x, y);
}

/* Pattern 5: C++ features (compiled as C++ if needed) */
#ifdef __cplusplus
class Base {
public:
    virtual ~Base() {}
    virtual void foo() { printf("Base::foo\n"); }
    int base_data;
};

class Derived : public Base {
public:
    virtual void foo() override { printf("Derived::foo\n"); }
    int derived_data;
};

void cpp_trigger_rtti() {
    Base* b = new Derived();
    
    /* Use RTTI - generates typeinfo symbols */
    Derived* d = dynamic_cast<Derived*>(b);
    if (d) {
        d->foo();
    }
    
    delete b;
}

/* Static local with potential guard variable */
inline int get_static_value() {
    static int counter = 0;
    return ++counter;
}

void cpp_trigger_static_local() {
    printf("Static local values: %d %d %d\n", 
           get_static_value(), 
           get_static_value(), 
           get_static_value());
}
#endif

/* Pattern 6: Exception handling */
#ifdef __cplusplus
void exception_trigger_personality() {
    try {
        throw 42;
    } catch (int e) {
        printf("Caught exception: %d\n", e);
    }
}
#endif

/* Pattern 7: Profile-guided optimization triggers */
void pgo_trigger_hot_loop() {
    volatile int result = 0;
    for (int i = 0; i < 10000; i++) {
        result += i * i;
        if (result > 1000000) {
            result = 0;
        }
    }
    printf("PGO loop result: %d\n", result);
}

void pgo_trigger_branchy_code() {
    volatile int x = 0;
    for (int i = 0; i < 1000; i++) {
        if (i % 3 == 0) x += i;
        else if (i % 3 == 1) x -= i;
        else x *= 2;
    }
    printf("Branchy code result: %d\n", x);
}

/* Main orchestrator */
int main(int argc, char** argv) {
    int run_sanitizer_tests = 0;
    
    /* Check if we should run sanitizer-specific tests */
    if (argc > 1 && strcmp(argv[1], "--run-sanitizers") == 0) {
        run_sanitizer_tests = 1;
    }
    
    printf("=== Starting artificial symbol generation test ===\n");
    
    /* Always run these safe tests */
    tsan_trigger_data_race();
    
    #ifdef _OPENMP
    omp_trigger_reduction();
    omp_trigger_private_vars();
    #endif
    
    asm_trigger_clobbers();
    
    pgo_trigger_hot_loop();
    pgo_trigger_branchy_code();
    
    #ifdef __cplusplus
    cpp_trigger_static_local();
    cpp_trigger_rtti();
    exception_trigger_personality();
    #endif
    
    /* Only run potentially dangerous sanitizer tests if requested */
    if (run_sanitizer_tests) {
        #ifdef __SANITIZE_ADDRESS__
        printf("\nRunning ASan tests...\n");
        asan_trigger_stack_buffer_overflow();
        asan_trigger_use_after_return();
        #endif
    } else {
        printf("\nSanitizer tests skipped (use --run-sanitizers to enable)\n");
    }
    
    printf("\n=== Test completed successfully ===\n");
    return 0;
}
