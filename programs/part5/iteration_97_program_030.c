#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Macro definitions that expand to balanced symbols */
#define PTR_TO_FUNC(T) T (*)(T, T)
#define ARRAY_OF_PTR(T, n) T *arr[n]
#define NESTED_ARRAY(T, d1, d2) T arr[d1][d2]
#define OPEN_BRACE {
#define CLOSE_BRACE }
#define OPEN_PAREN (
#define CLOSE_PAREN )
#define OPEN_BRACKET [
#define CLOSE_BRACKET ]

/* Typedef chains with nested grouping */
typedef int (*cmp_fn)(int, int);
typedef cmp_fn (*factory_fn)(void);
typedef factory_fn (*registry_fn)(const char *name);
typedef registry_fn (*lookup_fn)(int id);

/* Complex function pointer with nested parameter lists */
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char *))(double, float);

/* Nested structure with anonymous members */
struct Outer {
    union {
        struct {
            int a:4;
            int b:4;
            /* Comment inside nested structure */
            unsigned c:8;
        };
        struct {
            long d;
            short e;
        } inner;
        char data[16];
    };
    
    int matrix[2][3] = {
        {1, 2, 3},
        {4, 5, 6}
    };
    
    /* Function pointer array */
    void (*callbacks[5])(int, void *);
};

/* Macro-based type alias */
typedef PTR_TO_FUNC(double) double_func_ptr_t;

/* Conditional compilation blocks */
#ifdef FEATURE_A
    /* Complex type when FEATURE_A is defined */
    struct FeatureA_Struct {
        int (*methods[3])(
            struct FeatureA_Struct *self,  /* First parameter */
            int arg1,                      /* Second parameter */
            void *data                     /* Third parameter */
        );
        union {
            struct {
                int x[2][2];
            };
            long y;
        } u;
    };
#elif defined(FEATURE_B)
    /* Alternative complex type */
    typedef struct {
        int (*(*get_processor(void))[5])(int, int);
        char buffer[256];
    } FeatureB_Type;
#else
    /* Default complex type */
    typedef union {
        struct {
            int a;
            char b;
        };
        struct {
            long c;
            short d;
        } nested;
        ARRAY_OF_PTR(void, 10) pointers;
    } DefaultType;
#endif

/* Array with nested initializers and comments */
int complex_array[3][2][2] = {
    {   /* Layer 0 */
        {1 /* row0 col0 */, 2},
        {3, 4}
    },
    {   /* Layer 1 */
        {5, 6},
        {7, 8}
    },
    {   /* Layer 2 */
        {9, 10},
        {11, 12}
    }
};

/* Function with complex return type and parameters */
int (*(*register_callback(
    const char *name,                     /* Callback name */
    void (*callback)(int, void *),        /* The callback */
    int priority                          /* Priority level */
))[5])(int, int);

/* Nested macro expansion */
#define CREATE_NESTED() \
    struct Nested { \
        int (*funcs[2])(int, int); \
        union { \
            struct { \
                int a; \
                char b; \
            }; \
            long c; \
        } data; \
    }

CREATE_NESTED();

#endif /* COMPLEX_TYPES_H */
