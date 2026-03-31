/* Test for EMUTLS attribute copying - Main file */

/* Force EMUTLS transformation by targeting ARM without hardware TLS */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fvisibility=hidden */

/* TLS variable with used attribute (DECL_PRESERVE_P) */
__thread int tls_used __attribute__((used)) = 42;

/* Public TLS variable (TREE_PUBLIC) */
__thread int tls_public = 100;

/* Static (non-public) TLS variable */
static __thread int tls_static = 200;

/* Weak TLS variable (DECL_WEAK) */
__thread int tls_weak __attribute__((weak)) = 300;

/* TLS variable with hidden visibility (DECL_VISIBILITY) */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 400;

/* TLS variable with default visibility (DECL_VISIBILITY_SPECIFIED) */
__thread int tls_default_vis __attribute__((visibility("default"))) = 500;

/* Common TLS variable without initializer (DECL_COMMON) */
__thread int tls_common;

/* External TLS variable declaration (DECL_EXTERNAL) */
extern __thread int tls_external;

/* Function-scoped TLS variable (tests DECL_CONTEXT) */
void use_function_tls(void) {
    __thread int tls_function_local = 700;
    tls_function_local += 1;  /* Ensure TREE_USED */
}

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_all_tls(void) {
    /* Read and write to each TLS variable */
    int sum = 0;
    
    sum += tls_used;          /* Read used variable */
    tls_public = sum;         /* Write to public variable */
    sum += tls_static;        /* Read static variable */
    tls_weak = sum;           /* Write to weak variable */
    sum += tls_hidden;        /* Read hidden variable */
    tls_default_vis = sum;    /* Write to default visibility variable */
    sum += tls_common;        /* Read common variable */
    tls_common = sum;         /* Write to common variable */
    sum += tls_external;      /* Read external variable */
    
    use_function_tls();       /* Use function-local TLS */
}

int main(void) {
    /* Initialize common TLS variable */
    tls_common = 600;
    
    /* Reference all TLS variables */
    reference_all_tls();
    
    /* Perform additional operations to ensure variables are used */
    tls_used += 1;
    tls_public *= 2;
    tls_static -= 10;
    tls_weak /= 2;
    tls_hidden <<= 1;
    tls_default_vis >>= 1;
    
    return 0;
}
