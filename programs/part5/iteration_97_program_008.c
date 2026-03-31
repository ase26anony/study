#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* ====== 1. Complex Type Declarations with Nested Parentheses ====== */

/* Function pointer with nested parameter lists */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char*))(double);

/* Multi-dimensional arrays with nested initializers */
int matrix_3d[2][3][2] = {
    { /* Layer 0 */
        {1, 2},  /* Row 0 */
        {3, 4},  /* Row 1 */
        {5, 6}   /* Row 2 */
    },
    { /* Layer 1 */
        {7, 8},
        {9, 10},
        {11, 12}
    }
};

/* Structures with nested anonymous structures and bit-fields */
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
        char raw[4];
    } data;
    
    struct {
        int (*callback)(struct OuterStruct*, int);
        int values[2][2];
    } handler;
    
    int d[2];
};

/* ====== 2. Macros Expanding to Balanced Symbols ====== */

/* Type alias macros */
#define PTR_TO_FUNC(T) T (*)(T, T)
#define ARRAY_2D(T, w, h) T arr[w][h]
#define NESTED_PTR(T) T (*(*)(void))(void)

/* Macros that generate balanced symbols */
#define OPEN_PAREN (
#define CLOSE_PAREN )
#define OPEN_BRACE {
#define CLOSE_BRACE }
#define OPEN_BRACK [
#define CLOSE_BRACK ]

/* Macro chains producing balanced symbols */
#define BEGIN_STRUCT struct
#define END_STRUCT ;
#define STRUCT_FIELD(T, n) T n

/* Using macro chains */
BEGIN_STRUCT MacroStruct OPEN_BRACE
    STRUCT_FIELD(int, x);
    STRUCT_FIELD(char, y[10]);
    STRUCT_FIELD(PTR_TO_FUNC(int), func_ptr);
END_STRUCT

/* ====== 3. Typedef Chains with Nested Grouping ====== */

typedef int (*cmp_fn)(int, int);
typedef cmp_fn (*factory_fn)(void);
typedef factory_fn (*registry_fn)(const char*);
typedef registry_fn (*dispatcher_fn)(int, ...);

/* Complex typedef with multiple levels */
typedef void (*(*(*complex_callback)(int, cmp_fn))(double))(char*);

#endif /* TEST_TYPES_H */
