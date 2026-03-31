/* test_nested_structs.h - Complex nested structure definitions */

#ifndef TEST_NESTED_STRUCTS_H
#define TEST_NESTED_STRUCTS_H

/* Level 1: Basic nested struct */
struct Level1 {
    int a;
    struct {
        char b;
        union {
            short c;
            long d;
            struct {
                float e;
                double f;
            } inner_inner;
        } u1;
    } nested;
};

/* Level 2: Anonymous structs and unions */
struct Level2 {
    struct {
        union {
            int x;
            struct {
                char y;
                struct {
                    unsigned z: 4;
                    unsigned w: 12;
                } bits;
            } s;
        };
        double arr[3][2];
    } anonymous_group;
    
    /* Array within nested struct */
    struct Container {
        struct Element {
            int id;
            struct {
                char name[32];
                struct {
                    int (*compare)(const void*, const void*);
                    void (*destroy)(void*);
                } ops;
            } data;
        } items[10];
        
        union {
            struct Element* ptr_array[5];
            void* generic_ptrs[8];
        } storage;
    } container;
};

/* Level 3: Deeply nested with all delimiters */
struct Level3 {
    /* Mix of all delimiters in initialization */
    int matrix[2][3] = { {1, 2, 3}, {4, 5, 6} };
    
    struct {
        /* Function pointer array */
        void (*callbacks[4])(int, char);
        
        /* Nested anonymous union */
        union {
            struct {
                int (*get_value)(void);
                void (*set_value)(int);
            } funcs;
            
            struct {
                char buffer[256];
                struct {
                    size_t (*length)(const char*);
                    char* (*copy)(char*, const char*);
                } string_ops;
            } text;
        } operations;
    } handlers;
    
    /* Bit-fields in nested struct */
    struct Flags {
        unsigned flag1: 1;
        unsigned flag2: 2;
        struct {
            unsigned nested_flag1: 3;
            unsigned nested_flag2: 4;
            struct {
                unsigned deep_flag: 1;
            } deeper;
        } nested_flags;
    } flags;
};

/* Level 4: Designated initializers with nesting */
struct Level4 {
    struct {
        int x = { .value = 42 };
        struct Point {
            int x, y;
            struct {
                float z;
                double w[2];
            } coords;
        } points[3] = {
            [0] = { .x = 1, .y = 2, .coords = { .z = 3.0f, .w = {4.0, 5.0} } },
            [1] = { .x = 6, .y = 7, .coords = { .z = 8.0f } },
            { .x = 9, .y = 10 }
        };
    } geometry;
    
    union DataUnion {
        struct Numeric {
            int ints[5];
            float floats[3][2];
        } numeric;
        
        struct Textual {
            char* strings[4];
            struct {
                char (*get_char)(int);
                int (*find)(const char*, char);
            } text_ops;
        } textual;
    } data = {
        .numeric = {
            .ints = {1, 2, 3, 4, 5},
            .floats = {{1.0f, 2.0f}, {3.0f, 4.0f}, {5.0f, 6.0f}}
        }
    };
};

#endif /* TEST_NESTED_STRUCTS_H */
