/* Complex type declarations to exercise consume_balanced parsing */
#include "complex_types.h"
#include <stdio.h>

/* Function pointer with nested parameter list */
int (*(*global_callback)(int (*)(float, double), char *))(void) = NULL;

/* Multi-dimensional array with nested initializer */
int tensor[2][3][4] = {
    {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    },
    {
        {13, 14, 15, 16},
        {17, 18, 19, 20},
        {21, 22, 23, 24}
    }
};

/* Structure with deeply nested anonymous structs and bit-fields */
struct OuterContainer {
    union {
        struct {
            int flags : 5;
            int mode : 3;
            int : 4;  /* unnamed bit-field */
        } bits;
        struct {
            long data;
            char tag;
        } payload;
    } inner_union;
    
    /* Array of function pointers returning pointers to arrays */
    int (*(*func_table[3])(int, ...))[5];
    
    /* Nested structure with function pointer member */
    struct {
        void (*signal_handler)(int, struct siginfo_t *, void *);
        char buffer[256];
    } nested;
} __attribute__((aligned(64), packed));

/* Typedef with parentheses for grouping */
typedef int (*Comparator)(const void *, const void *);
typedef void (*(*FactoryMethod)(int arg_count, ...))(void);

/* GCC attributes with nested parentheses */
int debug_printf(const char *format, ...) 
    __attribute__((format(printf, 1, 2), nonnull(1)));

/* Macro to generate complex types conditionally */
#ifdef GENERATE_COMPLEX
#define MAKE_PTR_ARRAY(n) int (*(*ptr_array_##n)[n])(char (*)[n], ...)
MAKE_PTR_ARRAY(8) complex_ptr_array;
#endif

/* Compound type in sizeof context */
size_t get_complex_size(void) {
    return sizeof(int (*(*)[10])(char (*)[5]));
}

/* Cast expression with nested delimiters */
void *complex_cast(void) {
    return (void *(*(*)[3])(int, ...))0xDEADBEEF;
}

/* Function with complex parameter type */
void process_matrix(int (*matrix)[10][20], 
                    void (*callback)(int, int, double (*)[5])) {
    /* Use attributes in parameter */
    callback __attribute__((nonnull)) (0, 0, NULL);
}

/* Nested switch in initializer (triggers braces) */
static const struct {
    enum { CASE_A, CASE_B, CASE_C } tag;
    union {
        int (*func_ptr)(int, float);
        double matrix[2][2];
        struct {
            char *str;
            int len;
        } text;
    } data;
} variant = {
    .tag = CASE_A,
    .data = {
        .func_ptr = NULL
    }
};

/* Main function - minimal but uses complex types */
int main(void) {
    /* Declare local variable with complex type */
    int (*(*local_var)(int (*)(float)))[10] = NULL;
    
    /* Use sizeof on complex array type */
    size_t s1 = sizeof(int [10][20][30]);
    size_t s2 = sizeof(struct { int a; double b; char c[100]; });
    
    /* Compound literal */
    struct Point3D p = (struct Point3D){
        .x = 1.0,
        .y = 2.0,
        .z = 3.0,
        .color = {255, 0, 0, 255}
    };
    
    /* Array of function pointers initialization */
    int (*funcs[5])(int, char *) = {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };
    
    /* Nested initializer with designators */
    struct Config cfg = {
        .enabled = 1,
        .threshold = 0.5,
        .filters = {
            { .type = FILTER_LOWPASS, .cutoff = 1000 },
            { .type = FILTER_HIGHPASS, .cutoff = 5000 }
        },
        .callback = NULL
    };
    
    printf("Program compiled successfully\n");
    return 0;
}
