#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Macro definitions that expand to balanced symbols */
#define PTR_TO_FUNC(T) T (*)(T, T)
#define ARRAY_2D(T, w, h) T arr[w][h]
#define MAKE_STRUCT(name) struct name {
#define END_STRUCT };
#define OPEN_BRACE {
#define CLOSE_BRACE }
#define OPEN_PAREN (
#define CLOSE_PAREN )
#define OPEN_BRACKET [
#define CLOSE_BRACKET ]

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*(*)(int, int)), double))(char);

/* Array declarations with multiple dimensions and nested initializers */
int matrix_3d[2][3][4] = {
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

/* Structure with nested anonymous structures and bit-fields */
struct OuterStruct {
    union {
        struct {
            int a:4;
            int b:4;
            int c:8;
        } bits;
        struct {
            char x;
            char y;
        } chars;
        long long value;
    } data;
    
    int (*callback)(int, int);
    
    struct {
        int matrix[2][2];
        struct {
            float x, y;
        } point;
    } nested;
};

/* Typedef chains with nested grouping */
typedef int (*cmp_fn)(int, int);
typedef cmp_fn (*factory_fn)(void);
typedef factory_fn (*manager_fn)(factory_fn, cmp_fn);
typedef manager_fn (*director_fn)(manager_fn (*)(int), int);

/* Macro-expanded type */
PTR_TO_FUNC(int) int_func_ptr;

/* Conditional compilation with balanced symbols */
#ifdef FEATURE_A
    /* Complex type when FEATURE_A is defined */
    struct FeatureA_Struct {
        int (*methods[3])(struct FeatureA_Struct*, int);
        union {
            struct {
                int a[2][2];
            } matrix;
            void (*handler)(int, char);
        } data;
    };
#elif defined(FEATURE_B)
    /* Alternative complex type */
    typedef void (*(*FeatureB_Func)(int (*(*)(int)), char))(double);
#else
    /* Default complex type */
    struct DefaultStruct {
        int arr[3][2] = {{1, 2}, {3, 4}, {5, 6}};
        void (*func_ptr)(int, char, double);
    };
#endif

/* Comments within balanced sequences */
int commented_array[3][2] = {
    {1, /* first element of first row */ 2},
    {3, 4}, /* second row */
    {5, 6}  /* third row */
};

void function_with_comments(
    int a, /* first parameter */
    char b, /* second parameter */
    double c /* third parameter with a very long comment that spans
                multiple lines to test comment handling */
);

#endif /* COMPLEX_TYPES_H */
