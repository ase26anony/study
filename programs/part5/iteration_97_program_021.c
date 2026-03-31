#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char*))(float, double);

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
        };
        struct {
            unsigned int x:16;
        };
        char raw[2];
    } inner_union;
    
    struct {
        int (*callback)(struct OuterStruct*, int);
        int data[3];
    } nested_struct;
    
    int (*func_ptr_array[2])(int, int);
};

/* Typedef chains with nested grouping */
typedef int (*cmp_fn)(int, int);
typedef cmp_fn (*factory_fn)(void);
typedef factory_fn (*registry_fn)(const char*);
typedef registry_fn (*initializer_fn)(int, ...);

/* Function returning function pointer to function taking function pointer */
int (*(*get_handler(void))(int (*)(int)))(int, int);

#endif /* COMPLEX_TYPES_H */
