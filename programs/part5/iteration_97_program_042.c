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

/* Macro chains producing balanced symbols */
#define BEGIN_STRUCT struct S
#define STRUCT_BODY OPEN_BRACE int x; CLOSE_BRACE
#define DECLARE_FUNC(name, ret) ret name OPEN_PAREN void CLOSE_PAREN

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*handler)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char*))(float, double);

/* Array declarations with multiple dimensions and nested initializers */
int matrix[3][4] = {
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12}
};

/* Multi-dimensional array with comments inside braces */
int cube[2][2][2] = {
    {
        {1 /* depth 0 */, 2},
        {3, 4}
    },
    {
        {5, 6},
        {7, 8 /* last element */}
    }
};

/* Structures with nested anonymous structures and bit-fields */
struct Outer {
    union {
        struct {
            int a:4;
            int b:4 /* two 4-bit fields */;
        };
        char c;
    } inner;
    int d[2];
    struct {
        struct {
            int x:1;
            int y:7;
        } nested_bits;
        float z;
    } another;
};

/* Typedef chains with nested grouping */
typedef int (*cmp_fn)(int, int);
typedef cmp_fn (*factory_fn)(void);
typedef factory_fn (*manager_fn)(factory_fn, cmp_fn);

/* Function returning function pointer to function taking function pointer */
int (*(*get_processor(void))(int (*)(int)))(char*);

#endif /* COMPLEX_TYPES_H */
