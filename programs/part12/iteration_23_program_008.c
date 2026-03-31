/* Combined test program to trigger artificial symbol creation in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/* Pattern 1: AddressSanitizer triggers */
#ifdef __SANITIZE_ADDRESS__
void asan_stack_overflow() {
    int arr[10];
    arr[15] = 0;  /* Deliberate stack buffer overflow */
}

void asan_use_after_return() {
    int *ptr;
    {
        int local = 42;
        ptr = &local;
    }
    *ptr = 100;  /* Use after return */
}
#endif

/* Pattern 2: ThreadSanitizer triggers */
volatile int shared_counter = 0;

void* thread_func(void* arg) {
    for (int i = 0; i < 1000; i++) {
        shared_counter++;  /* Data race */
    }
    return NULL;
}

void create_data_race() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Shared counter (racy): %d\n", shared_counter);
}

/* Pattern 3: OpenMP artificial symbols */
#ifdef _OPENMP
#include <omp.h>
void openmp_reduction() {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
    printf("OpenMP reduction sum: %d\n", sum);
}

void openmp_private_vars() {
    int private_var = 0;
    #pragma omp parallel private(private_var)
    {
        private_var = omp_get_thread_num();
        #pragma omp critical
        printf("Thread %d private_var: %d\n", omp_get_thread_num(), private_var);
    }
}
#endif

/* Pattern 4: Complex inline assembly */
void inline_asm_test() {
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
    printf("Inline asm result: %d\n", y);
}

/* Pattern 5: C++ features (compile as C++) */
#ifdef __cplusplus
class Base {
public:
    virtual void foo() { printf("Base::foo\n"); }
    virtual ~Base() {}
};

class Derived : public Base {
public:
    virtual void foo() override { printf("Derived::foo\n"); }
};

void cpp_rtti_test() {
    Base* b = new Derived();
    
    /* Trigger RTTI usage */
    Derived* d = dynamic_cast<Derived*>(b);
    if (d) {
        printf("Dynamic cast successful\n");
    }
    
    /* Use typeid */
    const std::type_info& ti = typeid(*b);
    printf("Type name: %s\n", ti.name());
    
    delete b;
}

/* Static local with potential guard variable */
inline int get_counter() {
    static int counter = 0;  /* May generate guard variable */
    return ++counter;
}

void static_local_test() {
    for (int i = 0; i < 5; i++) {
        printf("Counter: %d\n", get_counter());
    }
}

/* Exception handling */
void exception_test() {
    try {
        throw 42;
    } catch (int e) {
        printf("Caught exception: %d\n", e);
    }
}
#endif

/* Pattern 6: Profile-guided optimization counters */
void pgo_heavy_loop() {
    long long sum = 0;
    for (int i = 0; i < 100000; i++) {
        sum += i * i;
        if (sum % 1000 == 0) {
            sum /= 2;
        }
    }
    printf("PGO loop sum: %lld\n", sum);
}

/* Main orchestrator */
int main(int argc, char** argv) {
    int run_sanitizer_tests = 0;
    
    /* Check if we should run sanitizer tests */
    if (argc > 1 && argv[1][0] == 's') {
        run_sanitizer_tests = 1;
    }
    
    printf("=== Starting artificial symbol trigger tests ===\n");
    
    /* PGO instrumentation */
    pgo_heavy_loop();
    
    /* OpenMP patterns */
#ifdef _OPENMP
    openmp_reduction();
    openmp_private_vars();
#endif
    
    /* Inline assembly */
    inline_asm_test();
    
    /* ThreadSanitizer pattern (always compile, but conditionally execute) */
    if (run_sanitizer_tests) {
        create_data_race();
    } else {
        printf("Skipping data race test (run with 's' argument to enable)\n");
    }
    
    /* C++ patterns */
#ifdef __cplusplus
    cpp_rtti_test();
    static_local_test();
    exception_test();
#endif
    
    /* AddressSanitizer patterns */
#ifdef __SANITIZE_ADDRESS__
    if (run_sanitizer_tests) {
        asan_stack_overflow();
        asan_use_after_return();
    }
#endif
    
    printf("=== Tests completed ===\n");
    return 0;
}
