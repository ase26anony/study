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
    } inner_union;
    
    struct {
        int (*callback)(int, char);
        void *data;
    } handler;
    
    int array[2][3];
};

/* Nested union with anonymous struct */
union ComplexUnion {
    struct {
        int (*compare)(const void *, const void *);
        void (*cleanup)(void *);
    } ops;
    struct {
        int values[3];
        char name[32];
    } data;
};

#endif /* COMPLEX_TYPES_H */
