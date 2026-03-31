#ifndef TEST_GTY_PARSER_H
#define TEST_GTY_PARSER_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointer) */
struct GTY(()) StructWithParens {
    int value;
    /* This will trigger case '(': */
    int (*callback)(int, char*);
    /* More parentheses in bitfield */
    unsigned int flags: (sizeof(int) * 8 - 1);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
    /* These will trigger case '[': */
    int fixed_array[10];
    char string_array[256];
    /* Variable length array in pointer */
    int (*ptr_to_array)[];
    /* Multi-dimensional array */
    double matrix[3][3];
};

/* Test case 3: Struct with braces (nested anonymous union) */
struct GTY(()) StructWithBraces {
    int id;
    /* This will trigger case '{': with anonymous union */
    union {
        int as_int;
        float as_float;
        void* as_ptr;
    } GTY((tag("0"))) data;
    
    /* Another nested struct with braces */
    struct {
        int x;
        int y;
    } GTY((skip)) point;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
    /* Parentheses: function pointer */
    void (*init_func)(struct ComplexType*);
    
    /* Brackets: array of function pointers */
    int (*handlers[5])(void);
    
    /* Braces: nested struct */
    struct {
        /* Nested array with parentheses in size expression */
        char buffer[(256 + 64)];
        
        /* Pointer to array */
        int (*matrix_ptr)[4];
        
        /* Function pointer field */
        void (*cleanup)(void);
    } GTY((desc("%1.buffer"))) nested;
    
    /* Flexible array member with brackets */
    int flexible_array[];
};

/* Test case 5: Typedef with GTY and complex type */
typedef struct GTY(()) {
    /* Parentheses in cast-like bitfield */
    unsigned int mask: (8 * sizeof(unsigned int) - 4);
    
    /* Array with computed size */
    unsigned char data[sizeof(void*) * 2];
    
    /* Nested anonymous struct */
    struct {
        int counter;
        int (*compare)(const void*, const void*);
    } GTY((skip)) helper;
} ComplexTypedef;

/* Test case 6: Union with all bracket types */
union GTY(()) AllBracketsUnion {
    /* Parentheses in function pointer array */
    void (*func_array[3])(void);
    
    /* Struct with all brackets */
    struct {
        /* Array declaration */
        int items[16];
        
        /* Function pointer with parameters in parentheses */
        int (*validator)(int, const char*);
        
        /* Anonymous union */
        union {
            long as_long;
            double as_double;
        };
    } GTY((desc("0"))) container;
    
    /* Simple array */
    char raw_data[128];
};

/* Forward declaration with GTY */
struct GTY(()) ForwardDecl;

/* Struct using forward declaration with pointer */
struct GTY(()) UsesForwardDecl {
    struct ForwardDecl* GTY((skip)) next;
    
    /* Array of pointers */
    struct ForwardDecl* GTY((length("count"))) *items;
    
    int count;
    
    /* Function pointer that takes forward decl */
    void (*processor)(struct ForwardDecl*);
};

/* Complete the forward declaration */
struct GTY(()) ForwardDecl {
    int id;
    char* GTY((length("strlen(name)+1"))) name;
    
    /* Self-referential pointer */
    struct ForwardDecl* GTY((skip)) child;
    
    /* Array with parentheses in size (macro expansion) */
    unsigned char metadata[sizeof(int) * 4];
};

#endif /* TEST_GTY_PARSER_H */
