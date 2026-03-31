/* complex-gty-test.c - Test file for gengtype delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with deeply nested parameter lists */
typedef void (*GTY((user)) complex_func_ptr)(
    int (*GTY((skip)) nested_callback)(
        char param[10],
        struct GTY(()) inner_struct { int x; } *arg
    ),
    double matrix[3][4]
);

/* Test 2: Structure with multiple nested delimiter types */
struct GTY((for_user)) outer_struct {
    /* Array with complex size expression */
    int GTY((length("(count > 0 ? count : 1)"))) variable_array[
        (10 + sizeof(struct GTY(()) temp { short s; }))
    ];
    
    /* Function pointer array */
    void (*GTY((skip)) func_array[5])(
        int GTY((tag("1"))) param1,
        char *GTY((length("strlen(%h)"))) strings[]
    );
    
    /* Nested anonymous union with bitfields */
    union {
        struct {
            unsigned int flags: (sizeof(int) * 8 - 2);
            unsigned int : 2;  /* unnamed bitfield */
        } GTY((skip)) bits;
        
        long GTY((skip)) all_bits;
    } GTY((desc("%1.bits.flags"))) bitfield_union;
    
    /* Pointer to array of function pointers */
    int (*(*GTY((chain_next, chain_prev)) complex_ptr)[10])(
        void *GTY((skip)) data,
        struct GTY(()) config { int id; } cfg
    );
};

/* Test 3: Template-like macro with nested delimiters */
#define DECLARE_VECTOR(TYPE, SIZE) \
    struct GTY(()) vector_##TYPE { \
        TYPE GTY((length("%0.capacity"))) *data; \
        size_t size; \
        size_t capacity; \
        void (*GTY((skip)) resize)(struct vector_##TYPE *v, size_t new_size); \
    }

DECLARE_VECTOR(int, 100);
DECLARE_VECTOR(struct outer_struct *, 50);

/* Test 4: Structure with initializer-like nested braces */
struct GTY(()) tree_like_node {
    enum node_type { LEAF, INTERNAL } type;
    
    union {
        /* Leaf node with array initializer pattern */
        struct {
            int GTY((length("%0.leaf.count"))) values[10];
            int count;
        } GTY((skip)) leaf;
        
        /* Internal node with nested structure */
        struct {
            struct tree_like_node *GTY((reorder("tree_node_cmp"))) children[4];
            int child_count;
            void (*GTY((skip)) traverse)(
                struct tree_like_node *node,
                void (*GTY((skip)) visit)(int value, void *context),
                void *context
            );
        } GTY((skip)) internal;
    } GTY((desc("%0.type == LEAF ? \"leaf\" : \"internal\""))) u;
    
    /* Multi-dimensional array with computed sizes */
    char GTY((length("sizeof(struct { char x[(10+5)*2]; })"))) buffer[
        (sizeof(int[3]) * 2) + offsetof(struct outer_struct, bitfield_union)
    ];
};

/* Test 5: Deeply nested parentheses in type expressions */
typedef struct GTY(()) recursive_list {
    void *GTY((tag("0"))) data;
    struct recursive_list *GTY((chain_next)) next;
    
    /* Function with nested parameter containing function pointer */
    void (*GTY((skip)) processor)(
        int (*GTY((skip)) comparator)(
            const void *a,
            const void *b,
            struct { int case_sensitive; } *options
        ),
        struct recursive_list *list
    );
} *GTY((user)) list_ptr;

/* Test 6: Complex array declarations with nested brackets */
struct GTY(()) matrix_container {
    /* Three-dimensional array */
    double GTY((skip)) space[3][4][5];
    
    /* Array of pointers to arrays */
    int (*GTY((length("%0.row_count"))) rows[10])[];
    
    /* Pointer to array of function pointers returning arrays */
    float (*(*GTY((skip)) transform)[3])(int index, float params[][2])[4];
    
    /* Nested structure with flexible array member */
    struct {
        size_t len;
        struct GTY(()) element {
            int id;
            char data[];
        } GTY((length("%0.len"))) elements[];
    } GTY((skip)) flexible;
};

/* Test 7: Union containing all delimiter types */
union GTY((user)) all_delimiters {
    /* Parentheses */
    void (*GTY((skip)) func)(int a[10], struct { int x; } s);
    
    /* Brackets */
    int GTY((skip)) multi_array[2][(3 + sizeof(long))];
    
    /* Braces */
    struct GTY(()) nested {
        int a;
        union {
            short b;
            long c;
        } u;
    } GTY((skip)) nested_struct;
    
    /* Combination */
    void (*(*GTY((skip)) complex[2])[3])(
        char *strings[],
        int counts[][4]
    );
};

/* Test 8: Attribute with nested parentheses */
struct GTY((
    user,
    desc("%0.type == 1 ? \"Type1\" : \"Type2\""),
    maybe_undef
)) attributed_struct {
    int type;
    
    /* Field with chain_next/prev in nested position */
    struct attributed_struct *GTY((
        chain_next,
        chain_prev,
        reorder("attr_struct_cmp")
    )) next;
    
    /* Callback with GTY attributes in parameter */
    void (*GTY((skip)) callback)(
        struct GTY((user)) param_struct *p,
        int GTY((tag("1"))) flags
    );
};

/* Test 9: Deep nesting of all three delimiters */
typedef void (*(*GTY((user)) ultra_complex)(
    struct {
        int (*(*GTY((skip)) member1)[10])(
            char *args[],
            void *context
        );
        struct GTY(()) { int x; } member2;
    } *config
)[20])(
    int matrix[][10][20],
    struct GTY(()) { 
        union { 
            short a; 
            long b[5]; 
        } u; 
    } param
);

/* Test 10: Multiple levels of nested structures/unions */
struct GTY(()) level1 {
    struct GTY(()) level2 {
        union GTY(()) level3 {
            struct GTY(()) level4 {
                int (*(*GTY((skip)) func_ptr)(
                    struct level1 *l1,
                    struct level2 *l2
                ))[10];
                
                struct GTY(()) level5 {
                    char data[100];
                    struct level5 *GTY((chain_next)) next;
                } *GTY((skip)) chain;
            } deepest;
            
            int simple;
        } choice;
        
        int array[(sizeof(struct level1) + 15) / sizeof(int)];
    } inner;
    
    void (*GTY((skip)) operation)(
        struct level2 *l2,
        int values[],
        size_t count
    );
};

/* Main function to make the file compilable */
int main(void) {
    return 0;
}
