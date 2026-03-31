#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*(*)(int, int))(int)))(void);

/* Array declarations with multiple dimensions and nested initializers */
int arr_3d[2][3][4] = {
    { /* First 3x4 block */
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    },
    { /* Second 3x4 block */
        {13, 14, 15, 16},
        {17, 18, 19, 20},
        {21, 22, 23, 24}
    }
};

/* Structures with nested anonymous structures and bit-fields */
struct OuterStruct {
    union {
        struct {
            int a:4;    /* Lower 4 bits */
            int b:4;    /* Next 4 bits */
            int c:8;    /* Next 8 bits */
        };
        unsigned short packed;
    } inner_union;
    
    struct {
        int x;
        int y;
        struct {
            float f;
            double d;
        } nested_point;
    } point_data;
    
    int d[2][3];
};

/* Union with deeply nested structures */
union ComplexUnion {
    struct {
        int (*callback)(int, char **);
        void (*cleanup)(void *);
    } funcs;
    struct {
        int matrix[2][2];
        float vector[3];
    } data;
    char raw[64];
};

#endif /* COMPLEX_TYPES_H */
