/* Test file 1: Deeply nested structs and unions with all delimiter types */

#ifndef DEEP_NESTED_STRUCTS_H
#define DEEP_NESTED_STRUCTS_H

/* Complex struct with nested anonymous unions and structs */
struct Level1 {
    struct {
        int a;
        union {
            char b;
            struct {
                long c;
                union {
                    short d;
                    struct {
                        int e:4;
                        int f:12;
                    } bits;
                } inner_union;
            } inner_struct;
        } u;
    } nested;
    
    /* Array with multiple dimensions */
    int matrix[3][4][5];
    
    /* Pointer to array of function pointers */
    void (*(*func_table)[5])(int, char);
};

/* Even more complex nesting */
struct OuterContainer {
    struct {
        union {
            struct {
                int (*(*nested_callback)(void))[10];
                char data[7][8];
            } s1;
            struct {
                float (*compute[3])(double, int);
                struct {
                    unsigned flags:8;
                    unsigned mode:4;
                } config;
            } s2;
        } choice;
        
        /* Nested designated initializer style struct */
        struct Config {
            int timeout;
            struct {
                char protocol[20];
                int port;
            } network;
            union {
                struct {
                    int cache_size;
                    int buffer_count;
                } perf;
                struct {
                    char log_level;
                    char trace_mask;
                } debug;
            } options;
        } settings;
    } inner;
    
    /* Multi-dimensional array with complex element type */
    struct Element {
        int id;
        union Value {
            int i;
            float f;
            char str[50];
            struct {
                double x, y, z;
            } coords;
        } value;
        void (*handler)(struct Element*);
    } elements[10][20];
};

/* Bit-field extravaganza */
struct BitFieldStruct {
    unsigned a:1;
    unsigned b:2;
    unsigned c:3;
    struct {
        unsigned d:4;
        unsigned e:5;
        union {
            unsigned f:6;
            struct {
                unsigned g:7;
                unsigned h:8;
            } nested_bits;
        } u;
    } inner_bits;
    
    /* Array of pointers to functions returning pointers to arrays */
    int (*(*(*callbacks[3]))(void))[5];
};

/* Mix of all delimiters in single declaration */
struct AllDelimiters {
    /* Parentheses in function pointer */
    int (*compare)(const void*, const void*);
    
    /* Brackets in arrays */
    char buffer[256][128];
    
    /* Braces in nested struct initializer (designated) */
    struct {
        int x;
        int y[10];
        struct {
            char name[50];
            int id;
        } info;
    } data[5];
    
    /* Combined: pointer to array of function pointers */
    void (*(*complex_array[7])[3])(int, float);
};

#endif /* DEEP_NESTED_STRUCTS_H */
