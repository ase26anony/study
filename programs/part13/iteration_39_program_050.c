/* tls_emulation_test.c - Test GCC's emulated TLS attribute propagation */

/* Prevent inlining to force TLS variable usage */
#define NOINLINE __attribute__((noinline))
#define CONSTRUCTOR __attribute__((constructor))
#define DESTRUCTOR __attribute__((destructor))

/* TLS variables with various attributes */
/* 1. Weak TLS variable with hidden visibility */
__thread __attribute__((weak, visibility("hidden"))) int tls_weak_hidden = 42;

/* 2. Public TLS variable with default visibility */
__thread int tls_public_default = 100;

/* 3. Protected visibility TLS variable */
__thread __attribute__((visibility("protected"))) int tls_protected = 200;

/* 4. Common linkage (tentative definition) - should become DECL_COMMON */
__thread int tls_common;

/* 5. External TLS declaration (will be defined elsewhere) */
extern __thread int tls_external;

/* 6. Weak external TLS declaration */
extern __thread __attribute__((weak)) int tls_weak_external;

/* 7. DLL import simulation (using visibility attributes) */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_dllimport;
#else
__thread __attribute__((visibility("default"), dllimport)) int tls_dllimport;
#endif

/* 8. Static TLS inside a function context */
static void func_with_static_tls(void) {
    static __thread int tls_func_static = 300;
    volatile int* volatile ptr = &tls_func_static;
    *ptr += 1;  /* Force usage through volatile pointer */
}

/* 9. TLS with preserve attribute (used in constructor) */
__thread int tls_preserve CONSTRUCTOR;

/* Block-scoped TLS variable */
void block_scoped_tls(void) {
    __thread int tls_block = 500;
    volatile int* vptr = &tls_block;
    *vptr = 501;  /* Access through volatile pointer */
}

/* Non-inlineable functions that take addresses of TLS variables */
NOINLINE static void use_tls_weak_hidden(volatile int* counter) {
    volatile int* ptr = &tls_weak_hidden;
    *ptr += *counter;
}

NOINLINE static void use_tls_public_default(volatile int* counter) {
    volatile int* ptr = &tls_public_default;
    *ptr -= *counter;
}

NOINLINE static void use_tls_protected(void) {
    volatile int* ptr = &tls_protected;
    *ptr *= 2;
}

NOINLINE static void use_tls_common(volatile int* counter) {
    volatile int* ptr = &tls_common;
    *ptr = *counter;
}

/* Constructor that uses TLS */
CONSTRUCTOR static void init_tls_vars(void) {
    /* This should set DECL_PRESERVE_P for tls_preserve */
    tls_preserve = 999;
    
    /* Also use other TLS variables */
    volatile int* ptr = &tls_public_default;
    *ptr = 1234;
}

/* Destructor that verifies TLS is still accessible */
DESTRUCTOR static void verify_tls_vars(void) {
    volatile int check = tls_preserve;
    (void)check;  /* Use variable */
}

/* Function that takes TLS addresses and performs operations */
NOINLINE static unsigned long compute_tls_checksum(void) {
    unsigned long sum = 0;
    volatile int* ptr;
    
    /* Access all TLS variables through volatile pointers */
    ptr = &tls_weak_hidden;
    sum += *ptr;
    
    ptr = &tls_public_default;
    sum += *ptr;
    
    ptr = &tls_protected;
    sum += *ptr;
    
    ptr = &tls_common;
    sum += *ptr;
    
    ptr = &tls_preserve;
    sum += *ptr;
    
    /* Force external TLS reference (even if undefined) */
    extern __thread int tls_external;
    sum += (unsigned long)&tls_external;  /* Use address */
    
    return sum;
}

/* Conditional TLS access based on volatile selector */
NOINLINE static void conditional_tls_access(volatile int selector) {
    volatile int* ptr = NULL;
    
    switch (selector % 4) {
        case 0:
            ptr = &tls_weak_hidden;
            break;
        case 1:
            ptr = &tls_public_default;
            break;
        case 2:
            ptr = &tls_protected;
            break;
        case 3:
            ptr = &tls_common;
            break;
    }
    
    if (ptr) {
        *ptr += selector;
    }
}

/* Main test function */
int main(void) {
    volatile int counter = 0;
    unsigned long checksum = 0;
    
    /* 1. Call functions that take TLS addresses */
    use_tls_weak_hidden(&counter);
    use_tls_public_default(&counter);
    use_tls_protected();
    use_tls_common(&counter);
    
    /* 2. Conditional TLS access in a loop */
    for (volatile int i = 0; i < 10; i++) {
        conditional_tls_access(i);
        counter++;
    }
    
    /* 3. Function with static TLS */
    func_with_static_tls();
    
    /* 4. Block-scoped TLS */
    block_scoped_tls();
    
    /* 5. Compute checksum of all TLS values */
    checksum = compute_tls_checksum();
    
    /* 6. Use the checksum to prevent optimization */
    if (checksum != 0) {
        /* Print something to ensure side effects */
        printf("TLS checksum: %lu\n", checksum);
    }
    
    /* 7. Force reference to weak external (even if undefined) */
    if (&tls_weak_external != NULL) {
        counter++;
    }
    
    /* 8. DLL import reference */
    volatile long dll_ref = (long)&tls_dllimport;
    (void)dll_ref;
    
    return (int)(checksum & 0x7FFFFFFF);
}

/* External TLS definition (for the external declaration) */
__thread int tls_external = 600;

/* Weak external TLS definition */
__thread __attribute__((weak)) int tls_weak_external = 700;
