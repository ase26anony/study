#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*(*)(int, int))(int)))(void);

/* Multi-dimensional arrays with nested initializers */
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

/* Structures with deeply nested anonymous members */
struct Outer {
    union {
        struct {
            int a:4;
            int b:4;
            struct {
                unsigned char x:2;
                unsigned char y:2;
                unsigned char z:4;
            } bits;
        };
        char raw[3];
    } inner;
    
    int (*callback)(struct Outer*, int);
    int arr[2][3];
};

/* Function with complex parameter list containing nested parentheses */
int (*(*register_callback(
    int id,
    void (*(*get_handler)(int, char*))(void),
    int (*validator)(int, int, int)
))(int, int));

#endif /* COMPLEX_TYPES_H */
