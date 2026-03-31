#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char*))(double);
void (*(*(**nested_func_ptrs[5])(int))(void))(char);

/* Array declarations with nested initializers */
int matrix[3][4] = {
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12}
};

struct point {
    int x;
    int y;
};

struct point points[2][2] = {
    {
        {1, 2},
        {3, 4}
    },
    {
        {5, 6},
        {7, 8}
    }
};

/* Nested structures with anonymous members and bit-fields */
struct outer {
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
        char data[4];
    } inner_union;
    
    struct {
        int (*callback)(struct outer*, int);
        void* data;
    } handler;
    
    int array[2][3];
};

/* Typedef chains with nested grouping */
typedef int (*cmp_fn)(int, int);
typedef cmp_fn (*factory_fn)(void);
typedef factory_fn (*registry_fn)(const char*);
typedef registry_fn (*manager_fn)(int, registry_fn*);

/* Macro definitions that expand to balanced symbols */
#define PTR_TO_FUNC(T) T (*)(T, T)
#define ARRAY_2D(T, w, h) T arr[w][h]
#define NESTED_INIT { {0, 1}, {2, 3} }
#define OPEN_PAREN (
#define CLOSE_PAREN )
#define OPEN_BRACE {
#define CLOSE_BRACE }
#define OPEN_BRACKET [
#define CLOSE_BRACKET ]

/* Macro that generates complex type */
#define MAKE_COMPLEX_TYPE(name, T) \
    T (*name OPEN_PAREN int, PTR_TO_FUNC(T) CLOSE_PAREN) OPEN_PAREN T, T CLOSE_PAREN

/* Use the macro */
MAKE_COMPLEX_TYPE(complex_var, int);

#endif /* TEST_TYPES_H */
