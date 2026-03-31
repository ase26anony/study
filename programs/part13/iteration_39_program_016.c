/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Force emulated TLS even if platform supports native TLS */
#pragma GCC target("tls")  /* Some GCC versions support this to force emulation */

#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* ========== TLS VARIABLES WITH DIVERSE ATTRIBUTES ========== */

/* 1. Weak TLS variable with hidden visibility */
__thread __attribute__((weak, visibility("hidden"))) 
int tls_weak_hidden = 0x1234;

/* 2. Public TLS with default visibility (implicitly external) */
extern __thread int tls_public_extern;
__thread int tls_public_extern = 0x5678;

/* 3. Common linkage TLS (tentative definition) */
__thread int tls_common __attribute__((common));

/* 4. Protected visibility TLS */
__thread __attribute__((visibility("protected"))) 
int tls_protected = 0x9ABC;

/* 5. DLL import simulation (Windows-specific attribute) */
#ifdef _WIN32
__thread __declspec(dllimport) int tls_dllimport;
#else
/* Simulate with weak external on non-Windows */
__thread __attribute__((weak)) int tls_dllimport;
#endif

/* 6. Static TLS inside a function context (tests DECL_CONTEXT) */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 0xDEF0;
    volatile int* volatile ptr = &tls_func_static;
    *ptr += 1;  /* Force usage through volatile */
}

/* 7. TLS with used attribute to ensure preservation */
__thread __attribute__((used)) int tls_used_preserve = 0x1111;

/* 8. TLS in different scopes for context testing */
namespace TLS_Namespace {
    __thread int tls_in_namespace = 0x2222;
    
    class Container {
    public:
        static __thread int tls_in_class;
        __thread static int tls_in_class_public;
    };
    
    __thread int Container::tls_in_class = 0x3333;
    __thread int Container::tls_in_class_public = 0x4444;
}

/* ========== NOINLINE HELPER FUNCTIONS ========== */

NOINLINE static void use_tls_weak_hidden(volatile int** out) {
    *out = &tls_weak_hidden;
    tls_weak_hidden += 1;  /* Modify to ensure it's not optimized away */
}

NOINLINE static void use_tls_public_extern(int increment) {
    volatile int* ptr = &tls_public_extern;
    *ptr += increment;
}

NOINLINE static void* get_tls_common_addr(void) {
    /* Taking address forces TLS control structure generation */
    return (void*)&tls_common;
}

NOINLINE static void modify_tls_protected(void) {
    tls_protected ^= 0xF0F0;  /* XOR modification */
}

NOINLINE static void use_tls_dllimport_like(void) {
    /* External reference - may be unresolved until link time */
    extern __thread int tls_dllimport;
    volatile int* ptr = &tls_dllimport;
    if (ptr) {
        *ptr = 0x5555;
    }
}

NOINLINE static int use_namespace_tls(void) {
    using namespace TLS_Namespace;
    volatile int* ptr1 = &tls_in_namespace;
    volatile int* ptr2 = &Container::tls_in_class;
    
    *ptr1 += *ptr2;
    return *ptr1;
}

/* ========== CONSTRUCTOR/DESTRUCTOR INTERACTIONS ========== */

CONSTRUCTOR static void init_tls_in_constructor(void) {
    /* This should run before main, testing DECL_PRESERVE_P */
    tls_public_extern = 0xAAAA;
    tls_used_preserve = 0xBBBB;
    
    /* Access namespace TLS */
    TLS_Namespace::tls_in_namespace = 0xCCCC;
}

DESTRUCTOR static void cleanup_tls_in_destructor(void) {
    /* Final modification to ensure TLS stays alive */
    tls_public_extern ^= 0xDEAD;
}

/* ========== VOLATILE CONTROL FLOW ========== */

NOINLINE static uintptr_t volatile_tls_access(volatile int selector) {
    volatile uintptr_t result = 0;
    volatile int* ptrs[4];
    
    /* Volatile array of TLS pointers */
    ptrs[0] = &tls_weak_hidden;
    ptrs[1] = &tls_public_extern;
    ptrs[2] = &tls_protected;
    ptrs[3] = &tls_used_preserve;
    
    /* Volatile loop prevents optimization */
    for (volatile int i = 0; i < selector; i++) {
        if (i < 4) {
            result += (uintptr_t)ptrs[i];
            *ptrs[i] += i;  /* Modify based on index */
        }
    }
    
    return result;
}

/* ========== MAIN TEST FUNCTION ========== */

int main(void) {
    volatile int counter = 0;
    uintptr_t checksum = 0;
    
    /* 1. Force usage of all TLS variables through noinline functions */
    volatile int* weak_hidden_ptr;
    use_tls_weak_hidden(&weak_hidden_ptr);
    checksum += *weak_hidden_ptr;
    
    use_tls_public_extern(5);
    checksum += tls_public_extern;
    
    checksum += (uintptr_t)get_tls_common_addr();
    
    modify_tls_protected();
    checksum += tls_protected;
    
    use_tls_dllimport_like();
    
    /* 2. Call function with static TLS */
    func_with_static_tls();
    
    /* 3. Use namespace/class TLS */
    checksum += use_namespace_tls();
    checksum += TLS_Namespace::Container::tls_in_class_public;
    
    /* 4. Volatile control flow with TLS access */
    for (volatile int i = 0; i < 10; i++) {
        checksum += volatile_tls_access(i & 3);
        counter++;  /* Volatile to prevent loop unrolling */
    }
    
    /* 5. Compute final checksum using all TLS values */
    checksum += tls_weak_hidden;
    checksum += tls_public_extern;
    checksum += tls_protected;
    checksum += tls_used_preserve;
    checksum += TLS_Namespace::tls_in_namespace;
    checksum += TLS_Namespace::Container::tls_in_class;
    checksum += TLS_Namespace::Container::tls_in_class_public;
    
    /* 6. Print checksum to prevent dead code elimination */
    printf("TLS checksum: 0x%lx\n", (unsigned long)checksum);
    
    /* 7. Verify emulated TLS by checking if addresses look like structures */
    void* addr1 = &tls_public_extern;
    void* addr2 = &tls_weak_hidden;
    printf("TLS address difference: %ld\n", 
           (long)((uintptr_t)addr2 - (uintptr_t)addr1));
    
    return (checksum & 0xFF);
}
