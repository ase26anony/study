/* test_targhooks.c - Combined triggers for GCC artificial symbol generation */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#ifdef __cplusplus
#include <typeinfo>
#include <exception>
#endif

/* Pattern 1: Sanitizer triggers */
#ifdef __SANITIZE_ADDRESS__
#define USE_SANITIZERS 1
#else
#define USE_SANITIZERS 0
#endif

void asan_stack_overflow() {
    int buffer[10];
    /* Deliberate overflow when sanitizers are active */
    if (USE_SANITIZERS) {
        buffer[15] = 42;  /* ASan should catch this */
    }
}

void asan_use_after_return() {
    static int* dangling_ptr = NULL;
    int local = 42;
    dangling_ptr = &local;
    /* Use after return simulation */
    if (USE_SANITIZERS && dangling_ptr) {
        *dangling_ptr = 99;
    }
}

/* Pattern 2: ThreadSanitizer data race */
static int shared_counter = 0;

void* increment_counter(void* arg) {
    for (int i = 0; i < 1000; ++i) {
        shared_counter++;  /* Data race when unsynchronized */
    }
    return NULL;
}

void tsan_data_race() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, increment_counter, NULL);
    pthread_create(&t2, NULL, increment_counter, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Shared counter (racy): %d\n", shared_counter);
}

/* Pattern 3: OpenMP artificial symbols */
void omp_reduction_symbols() {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
    printf("OpenMP reduction sum: %d\n", sum);
    
    /* Thread-private variables */
    #pragma omp parallel
    {
        static int thread_specific = 0;
        #pragma omp critical
        thread_specific++;
    }
}

/* Pattern 4: Complex inline assembly */
void inline_assembly_symbols(int x) {
    int result;
    /* Complex asm with multiple clobbers */
    asm volatile (
        "mov %1, %%eax\n\t"
        "add $42, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r" (result)
        : "r" (x)
        : "%eax", "%ebx", "%ecx", "memory", "cc"
    );
    printf("Assembly result: %d\n", result);
    
    /* asm goto for control flow */
    asm goto (
        "test %0, %0\n\t"
        "jnz %l[label]\n\t"
        : : "r" (x) : "memory" : label
    );
    printf("Did not jump\n");
    return;
    
label:
    printf("Jumped via asm goto\n");
}

#ifdef __cplusplus
/* Pattern 5: C++ features generating artificial symbols */

/* Static local in inline function */
inline int static_local_counter() {
    static int counter = 0;  /* Guard variable may be artificial */
    return ++counter;
}

/* Polymorphic class for RTTI */
class Base {
public:
    virtual ~Base() {}
    virtual void foo() = 0;
};

class Derived : public Base {
public:
    virtual void foo() override {}
    virtual ~Derived() {}
};

void cpp_artificial_symbols() {
    /* RTTI usage */
    Derived d;
    Base& b = d;
    
    const std::type_info& ti = typeid(b);
    printf("Type name: %s\n", ti.name());
    
    /* Exception handling */
    try {
        throw std::runtime_error("test");
    } catch (const std::exception& e) {
        printf("Caught: %s\n", e.what());
    }
    
    /* Static local variable */
    printf("Static counter: %d\n", static_local_counter());
}
#endif

/* Pattern 6: Profile-guided optimization counters */
void pgo_instrumentation() {
    /* Hot loop for PGO */
    volatile int sink = 0;
    for (int i = 0; i < 10000; ++i) {
        sink += i * i;
    }
    
    /* Multiple branches for edge profiling */
    for (int i = 0; i < 1000; ++i) {
        if (i % 3 == 0) {
            sink += 1;
        } else if (i % 3 == 1) {
            sink += 2;
        } else {
            sink += 3;
        }
    }
}

/* Main orchestrator */
int main(int argc, char** argv) {
    int run_sanitizers = 0;
    
    /* Check if we should trigger sanitizer errors */
    if (argc > 1 && strcmp(argv[1], "--trigger-errors") == 0) {
        run_sanitizers = 1;
    }
    
    printf("=== Testing GCC artificial symbol generation ===\n");
    
    /* Pattern 1: Sanitizers */
    if (run_sanitizers) {
        printf("\n1. Testing sanitizer instrumentation:\n");
        asan_stack_overflow();
        asan_use_after_return();
    }
    
    /* Pattern 2: ThreadSanitizer */
    printf("\n2. Testing thread-related symbols:\n");
    tsan_data_race();
    
    /* Pattern 3: OpenMP */
    printf("\n3. Testing OpenMP symbols:\n");
    omp_reduction_symbols();
    
    /* Pattern 4: Inline assembly */
    printf("\n4. Testing inline assembly symbols:\n");
    inline_assembly_symbols(100);
    
    /* Pattern 5: C++ features (if compiled as C++) */
    #ifdef __cplusplus
    printf("\n5. Testing C++ artificial symbols:\n");
    cpp_artificial_symbols();
    #endif
    
    /* Pattern 6: PGO instrumentation */
    printf("\n6. Testing PGO instrumentation:\n");
    pgo_instrumentation();
    
    printf("\n=== Test completed ===\n");
    return 0;
}
