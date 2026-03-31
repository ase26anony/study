#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include standard headers for types */
#include <stddef.h>

/* Struct with parentheses - triggers case '(' */
struct GTY(()) StructWithParens {
    /* Function pointer type - contains parentheses */
    int (*callback)(int, char*);
    
    /* Bitfield with parenthesized expression */
    unsigned int bits: (sizeof(int) * 8 - 1);
    
    /* Another function pointer with complex signature */
    void (*complex_callback)(struct StructWithParens*, int (*)(void));
};

/* Union with brackets - triggers case '[' */
union GTY(()) UnionWithBrackets {
    /* Fixed-size array */
    int fixed_array[10];
    
    /* Multi-dimensional array */
    double matrix[3][3];
    
    /* Zero-length array (GCC extension) */
    char flexible_array[0];
    
    /* Array with computed size */
    long variable_array[(sizeof(void*) == 8) ? 16 : 8];
};

/* Struct with braces - triggers case '{' */
struct GTY(()) StructWithBraces {
    int id;
    
    /* Anonymous nested union with braces */
    union {
        int as_int;
        float as_float;
        void* as_ptr;
    } GTY((tag("0"))) data;
    
    /* Nested struct definition */
    struct {
        int x;
        int y;
    } GTY((skip)) point;
};

/* Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Function pointer returning pointer to array - has both () and [] */
    int (*(*get_array_func)(void))[10];
    
    /* Array of function pointers - has both [] and () */
    void (*handlers[5])(struct ComplexType*);
    
    /* Nested struct with bitfield */
    struct {
        unsigned int flags: (8);
        char name[32];
    } GTY((desc("%1.flags"))) info;
    
    /* Union containing array of structs with function pointers */
    union {
        struct {
            int (*compare)(const void*, const void*);
            void* items[20];
        } GTY((tag("1"))) sorted;
        
        struct {
            int count;
            /* Flexible array member at end */
            struct ComplexType* list[];
        } GTY((tag("2"))) dynamic;
    } GTY((desc("%1.info.flags & 1"))) container;
};

/* Another struct with deeply nested balanced tokens */
struct GTY(()) DeeplyNested {
    /* Pointer to function returning pointer to array of pointers to functions */
    void (*(*(*deep_func)(int))[5])(void);
    
    /* Array of structs containing arrays */
    struct {
        int values[4];
        char* names[];
    } GTY((length("%h.count"))) items[8];
    
    /* Complex bitfield expression */
    unsigned int control: ((sizeof(long) * 8) - 4);
};

#endif /* TEST_GTY_H */
