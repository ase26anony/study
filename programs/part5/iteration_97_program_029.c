#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* ==================== MACROS FOR BALANCED SYMBOLS ==================== */

/* Macros that expand to contain balanced parentheses */
#define PTR_TO_FUNC(T) T (*)(T, T)
#define FUNC_PTR_NAME(name, T) T (*name)(T, T)
#define CAST_TO_FUNC_PTR (void (*)(int))

/* Macros for array declarations with balanced brackets */
#define MAKE_ARRAY(T, sz) T arr[sz]
#define MATRIX(T, r, c) T matrix[r][c]
#define INIT_3D_ARRAY T arr3d[2][3][4]

/* Macros that produce balanced braces */
#define BEGIN_STRUCT struct {
#define END_STRUCT }
#define OPEN_BRACE {
#define CLOSE_BRACE }
#define STRUCT_WITH(T) struct { T field; }

/* Macro chains that ultimately produce balanced symbols */
#define OPEN_PAREN (
#define CLOSE_PAREN )
#define OPEN_BRACK [
#define CLOSE_BRACK ]
#define OPEN_CURLY {
#define CLOSE_CURLY }

/* ==================== COMPLEX TYPE DECLARATIONS ==================== */

/* Function pointer with deeply nested parameter lists */
void (*signal(int sig, void (*handler)(int)))(int);

/* Even more complex: function returning pointer to function with nested params */
int (*(*complex_func1(int x))(int, int))(float, double);

/* Multi-dimensional function pointer array */
void (*(*func_array[3][2])(int, int))(void);

/* Nested function pointers in typedefs */
typedef int (*comparator)(int, int);
typedef comparator (*factory_fn)(void);
typedef factory_fn (*registry_fn)(const char*);

/* ==================== ARRAYS WITH NESTED INITIALIZERS ==================== */

/* Multi-dimensional array with nested initializers */
int matrix_3d[2][3][2] = {
    { /* First 3x2 matrix */
        {1, 2},
        {3, 4},
        {5, 6}
    },
    { /* Second 3x2 matrix */
        {7, 8},
        {9, 10},
        {11, 12}
    }
};

/* Array with mixed nesting and comments */
char* string_table[3][2] = {
    {"hello" /* greeting */, "world"},
    {"foo", "bar"},
    OPEN_CURLY
        "nested" /* via macro */,
        "macro"
    CLOSE_CURLY
};

/* ==================== STRUCTURES/UNIONS WITH NESTING ==================== */

/* Structure with anonymous nested structures and bit-fields */
struct OuterStruct {
    union {
        struct {
            int a:4;
            int b:4;
            int c:8;
        } bits;
        struct {
            long x;
            long y;
        } coords;
        char data[16];
    } inner_union;
    
    struct {
        int (*compare)(int, int);
        void (*print)(const char*);
    } operations;
    
    int d[2];
};

/* Union containing structures with function pointers */
union ComplexUnion {
    struct {
        int (*func1)(int, int);
        void (**func_array)(void);
    } funcs;
    struct {
        int matrix[2][2];
        char* (*get_name)(void);
    } data;
};

/* ==================== PREPROCESSOR CONDITIONAL BLOCKS ==================== */

/* Test #if 0 block with balanced symbols (should be skipped) */
#if 0
/* This entire block contains balanced symbols but should be skipped */
void (*skipped_func(int x))(int) {
    return NULL;
}

int skipped_array[3][2] = {
    {1, 2},
    {3, 4},
    {5, 6}
};

struct SkippedStruct {
    int (*method)(int, int);
};
#endif

/* Conditional compilation with different balanced symbol patterns */
#ifdef FEATURE_A
/* Complex function prototype if FEATURE_A is defined */
int (*(*feature_a_func(double d))(int, float))(char*);
#else
/* Different nested structure if FEATURE_A is not defined */
struct FeatureBStruct {
    struct {
        int (*calc)(int[2][2], int);
        void (*init)(struct { int x; int y; }*);
    } methods;
};
#endif

/* Nested conditional blocks */
#ifndef DISABLE_COMPLEX
    #ifdef USE_ARRAYS
        int conditional_array[][3] = {
            {1, 2, 3},
            {4, 5, 6},
            {7, 8, 9}
        };
    #elif defined(USE_FUNCTIONS)
        void (*(*conditional_func)(int))(void) = NULL;
    #else
        struct ConditionalStruct {
            int field;
        };
    #endif
#endif

/* ==================== TYPEDEF CHAINS ==================== */

/* Chain of typedefs leading to complex types */
typedef int Integer;
typedef Integer* IntPtr;
typedef IntPtr (*IntPtrFactory)(Integer, Integer);
typedef IntPtrFactory (*RegistryEntry)(const char*);

/* Typedef for function pointer with nested parameters */
typedef void (*SimpleHandler)(int);
typedef SimpleHandler (*HandlerFactory)(int priority);
typedef HandlerFactory (*FactoryRegistry)[2];

/* ==================== DECLARATIONS WITH COMMENTS IN BALANCED SEQUENCES ==================== */

/* Function prototype with comments between parameters */
void process_data(
    int count, /* number of items */
    char** data, /* array of strings */
    void (*callback)(int, /* status */ char*) /* completion callback */
);

/* Array with comments inside initializers */
int commented_matrix[2][2] = {
    { /* First row */
        1, /* column 1 */
        2  /* column 2 */
    },
    { /* Second row */
        3, /* with comment */
        4  /* another comment */
    }
};

/* Structure with commented nested elements */
struct CommentedStruct {
    struct {
        int x; /* x coordinate */
        int y; /* y coordinate */
    } point /* geometric point */;
    
    union {
        int (*func1)(int, int); /* function pointer */
        void (*func2)(void); /* alternative function */
    } operation /* operation union */;
};

#endif /* COMPLEX_TYPES_H */
