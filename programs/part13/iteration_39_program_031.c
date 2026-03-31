/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Prevent inlining to force TLS address usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* Force preservation of symbols */
#define USED __attribute__((used))

/* TLS variables with diverse attributes */
/* 1. Weak TLS variable with hidden visibility */
__thread int tls_weak_hidden __attribute__((weak, visibility("hidden"))) = 42;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 100;

/* 3. Common linkage TLS (tentative definition) */
__thread int tls_common;

/* 4. External TLS declaration (will be defined elsewhere) */
extern __thread int tls_external;

/* 5. Weak external TLS */
extern __thread int tls_weak_external __attribute__((weak));

/* 6. Protected visibility TLS */
__thread int tls_protected __attribute__((visibility("protected"))) = 200;

/* 7. DLL import simulation (using visibility attributes) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
__thread int tls_dllimport __attribute__((visibility("default")));
#endif

/* Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 300;
    volatile int* volatile ptr = &tls_func_static;
    *ptr += 1;
}

/* Block-scoped TLS (C++ only in separate file) */

/* Non-inlineable functions that take TLS addresses */
NOINLINE static void use_tls_weak_hidden(volatile int* ptr) {
    *ptr += 1;
}

NOINLINE static void use_tls_public_default(volatile int* ptr) {
    *ptr *= 2;
}

NOINLINE static void use_tls_common(volatile int* ptr) {
    *ptr = *ptr + 3;
}

NOINLINE static void use_tls_protected(volatile int* ptr) {
    *ptr -= 4;
}

/* Constructor that interacts with TLS */
CONSTRUCTOR static void init_tls_values(void) {
    /* This should force DECL_PRESERVE_P to be set */
    tls_common = 999;
    tls_public_default = 1234;
    
    /* Take address to force usage */
    volatile int* volatile ptr1 = &tls_weak_hidden;
    volatile int* volatile ptr2 = &tls_protected;
    (void)ptr1;
    (void)ptr2;
}

/* Destructor that also uses TLS */
DESTRUCTOR static void cleanup_tls(void) {
    volatile int* volatile ptr = &tls_public_default;
    *ptr = 0;
}

/* Main test function */
int main(void) {
    volatile int selector = 0;
    int checksum = 0;
    
    /* Force TREE_USED on all TLS variables by taking addresses */
    volatile int* volatile addrs[] = {
        &tls_weak_hidden,
        &tls_public_default,
        &tls_common,
        &tls_protected,
        &tls_dllimport
    };
    
    /* Use volatile loop to prevent optimization */
    for (volatile int i = 0; i < 5; i++) {
        selector = i % 5;
        
        switch (selector) {
            case 0:
                use_tls_weak_hidden(&tls_weak_hidden);
                break;
            case 1:
                use_tls_public_default(&tls_public_default);
                break;
            case 2:
                use_tls_common(&tls_common);
                break;
            case 3:
                use_tls_protected(&tls_protected);
                break;
            case 4:
                func_with_static_tls();
                break;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < 5; i++) {
        checksum += *addrs[i];
    }
    
    /* Use checksum in a way that can't be optimized out */
    volatile int result = checksum;
    
    return result != 0 ? 0 : 1;
}

/* External TLS definition (for the extern declaration) */
__thread int tls_external = 555;

/* Weak external TLS definition */
__thread int tls_weak_external __attribute__((weak)) = 777;
