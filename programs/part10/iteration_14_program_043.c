/* Main file with various TLS declarations and attribute manipulations */

/* Force emulated TLS */
#ifdef __APPLE__
/* macOS doesn't support __thread in emulated mode well, so use _Thread_local */
#define TLS _Thread_local
#else
#define TLS __thread
#endif

/* Declare with various attributes to set the flags we need copied */
__attribute__((used, visibility("default"), weak))
TLS int weak_public_tls = 42;

__attribute__((used, visibility("hidden")))
static TLS long hidden_static_tls;

/* DLL import simulation for non-Windows */
#ifndef _WIN32
#define __declspec(dllimport) __attribute__((dllimport))
#endif

/* Try to trigger DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) TLS double imported_tls;
#else
/* Non-Windows fallback - use a different attribute */
__attribute__((weak, visibility("protected"))) TLS double imported_tls;
#endif

/* Common linkage TLS */
TLS char common_tls;

/* External declaration (will be defined in another file) */
extern TLS unsigned long external_tls;

/* Function to take address and force declaration usage */
static __attribute__((noinline, used)) 
void use_tls_addresses(void *addr1, void *addr2, void *addr3) {
    /* Opaque operations to prevent optimization */
    __asm__ volatile ("" : : "r"(addr1), "r"(addr2), "r"(addr3) : "memory");
}

/* Constructor to initialize TLS with non-constant values */
__attribute__((constructor))
static void init_tls_values(void) {
    /* Use time/random for non-constant initialization */
    hidden_static_tls = (long)&init_tls_values; /* Use address as non-constant */
    common_tls = 'A' + ((long)&common_tls % 26); /* Non-constant based on address */
}

/* Helper that uses statement expression with TLS address */
static unsigned long tls_checksum(void) {
    /* Statement expression that takes TLS address */
    unsigned long sum = ({
        unsigned long s = 0;
        /* Take address of TLS vars in complex context */
        void *addr1 = &weak_public_tls;
        void *addr2 = &hidden_static_tls;
        void *addr3 = &common_tls;
        
        /* Call external function through pointer to prevent optimization */
        extern unsigned long external_func(unsigned long);
        unsigned long (*func_ptr)(unsigned long) = external_func;
        
        s += *(int*)addr1;
        s += *(long*)addr2;
        s += *(char*)addr3;
        
        /* Use function pointer to create opaque context */
        s = func_ptr(s);
        s;
    });
    
    return sum;
}

int main(void) {
    /* Local TLS variable - may trigger declaration in local scope */
    TLS int local_tls = 123;
    
    /* Take addresses of multiple TLS variables */
    void *addrs[] = {
        &weak_public_tls,
        &hidden_static_tls,
        &common_tls,
        &local_tls,
        &imported_tls
    };
    
    /* Force use through noinline function */
    use_tls_addresses(addrs[0], addrs[1], addrs[2]);
    
    /* Use statement expression with TLS */
    unsigned long sum = tls_checksum();
    
    /* Use local TLS in another statement expression */
    int result = ({
        int x = local_tls;
        x += weak_public_tls;
        /* Force address taken in nested context */
        use_tls_addresses(&x, &local_tls, &weak_public_tls);
        x;
    });
    
    /* Prevent dead code elimination */
    __asm__ volatile ("" : : "r"(sum), "r"(result) : "memory");
    
    return (int)(sum % 256);
}
