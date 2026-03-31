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

/* Nested structures with anonymous members and bit-fields */
struct Outer {
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
        char buffer[16];
    } data;
    
    struct {
        int (*callback)(int, char **);
        void (*cleanup)(void *);
    } handlers;
    
    int arr[2][3];
};

/* Function with complex return type */
struct Outer *(*get_processor(void))(int, char **);

#endif /* COMPLEX_TYPES_H */
