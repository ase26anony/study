#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char))(void);

/* Multi-dimensional arrays with nested braces */
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

/* Structures with deeply nested anonymous members */
struct Outer {
    union {
        struct {
            int a:4;
            int b:4;
            int c:8;
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
    
    int (*callback)(struct Outer*, int);
};

/* Function with complex return type containing parentheses */
int (*(*get_factory(void))(int, int))(float, double);

#endif /* COMPLEX_TYPES_H */
