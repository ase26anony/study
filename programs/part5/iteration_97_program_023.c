#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Macro definitions that expand to balanced symbols */
#define PTR_TO_FUNC(T) T (*)(T, T)
#define ARRAY_2D(T, w, h) T [w][h]
#define MAKE_ARRAY(T, sz) T arr[sz]
#define OPEN_BRACE {
#define CLOSE_BRACE }
#define OPEN_PAREN (
#define CLOSE_PAREN )
#define OPEN_BRACKET [
#define CLOSE_BRACKET ]

/* Typedef chains with nested grouping */
typedef int (*cmp_fn)(int, int);
typedef cmp_fn (*factory_fn)(void);
typedef factory_fn (*registry_fn)(const char*);
typedef registry_fn (*dispatcher_fn)(int, registry_fn);

/* Complex function pointer types with deeply nested parentheses */
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_func1(void))[5])(float, double);
char *(*(**(*complex_func2(int x))[][8])())(void);

/* Nested array declarations */
int matrix_3d[2][3][4] = {
    { /* Layer 0 */
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    },
    { /* Layer 1 */
        {13, 14, 15, 16},
        {17, 18, 19, 20},
        {21, 22, 23, 24}
    }
};

/* Structure with deeply nested anonymous structures and unions */
struct OuterStruct {
    union {
        struct {
            int a:4;
            int b:4;
            int c:8;
            int d:16;
        } bits;
        unsigned int value;
    } inner_union;
    
    struct {
        struct {
            int x;
            int y;
        } point;
        char name[32];
    } nested_struct;
    
    int (*callback)(struct OuterStruct*, int);
};

/* Macro usage creating balanced symbols */
PTR_TO_FUNC(int) func_ptr_var;
ARRAY_2D(double, 5, 5) matrix_var;

#endif /* COMPLEX_TYPES_H */
