#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Complex function pointer types with nested parentheses */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char))(void);
void (*(*(*nested_func_ptr[3])(int))(float))(char);

/* Array declarations with multiple dimensions and nested initializers */
int arr_3d[2][3][2] = {
    {
        {1, 2},
        {3, 4},
        {5, 6}
    },
    {
        {7, 8},
        {9, 10},
        {11, 12}
    }
};

/* Structures with nested anonymous structures and bit-fields */
struct OuterStruct {
    union {
        struct {
            int a:4;
            int b:4;
            int c:8 /* comment inside bitfield */;
        };
        struct {
            unsigned int x:16;
        };
        char raw[2];
    } inner_union;
    
    struct {
        int (*callback)(int, int);
        char name[20];
    } nested_struct;
    
    int d[2];
};

/* Union with deeply nested structures */
union ComplexUnion {
    struct {
        int (*func_ptr)(int, int);
        struct {
            int a;
            struct {
                char c;
                short s;
            } inner;
        } nested;
    } data;
    long long as_long;
};

/* Typedef chains with nested grouping */
typedef int (*cmp_fn)(int, int);
typedef cmp_fn (*factory_fn)(void);
typedef factory_fn (*registry_fn)(const char*);
typedef registry_fn (*manager_fn)(int, ...);

/* Function returning function pointer to function taking function pointer */
manager_fn (*get_manager_system(void))(int, registry_fn);

#endif /* COMPLEX_TYPES_H */
