#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*(*)(int, int))(int)))(void);

/* Nested array declarations with initializers */
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

/* Structure with deeply nested anonymous structures and unions */
struct OuterStruct {
    union {
        struct {
            int a:4;
            int b:4;
            struct {
                unsigned char c:2;
                unsigned char d:6;
            } nested_bits;
        };
        long long combined;
    } inner_union;
    
    struct {
        int (*func_ptr)(int, int);
        char array[2][3];
    } nested_struct;
    
    int (*member_func)(struct OuterStruct*, int);
};

/* Typedef chains with nested grouping */
typedef int (*cmp_fn)(int, int);
typedef cmp_fn (*factory_fn)(void);
typedef factory_fn (*registry_fn)(const char*);
typedef registry_fn (*manager_fn)(registry_fn*, int);

#endif /* COMPLEX_TYPES_H */
