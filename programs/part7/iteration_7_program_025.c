/* test_rich_types.c - Comprehensive type coverage for gengtype-state.cc */
/* This program defines a diverse set of type constructs to exercise all
   type categories in GCC's GGC type state serialization. */

#include <stddef.h>
#include <string.h>

/* ========== TYPE_UNDEFINED / TYPE_LANG_STRUCT ========== */
/* Forward declaration for undefined type */
struct opaque;                     /* TYPE_UNDEFINED candidate */

/* GCC-specific vector type extension */
typedef float __attribute__((vector_size(32))) float32x8_t;  /* TYPE_LANG_STRUCT */

/* ========== TYPE_CALLBACK ========== */
/* Function pointer typedefs */
typedef void (*event_handler)(int, void*);                    /* TYPE_CALLBACK */
typedef int (*comparator_fn)(const void*, const void*);      /* TYPE_CALLBACK */
typedef float (*transform_fn)(float32x8_t);                  /* TYPE_CALLBACK */

/* Struct with function pointer members */
struct Plugin {                                               /* TYPE_STRUCT */
    const char* name;                                         /* TYPE_POINTER, TYPE_STRING */
    int (*init)(void);                                        /* TYPE_CALLBACK */
    void (*process)(int);                                     /* TYPE_CALLBACK */
    event_handler on_error;                                   /* TYPE_CALLBACK */
};

/* ========== TYPE_UNION with nested anonymous struct ========== */
union Variant {                                               /* TYPE_UNION */
    int as_int;                                               /* TYPE_SCALAR */
    void* as_ptr;                                             /* TYPE_POINTER */
    struct {                                                  /* Anonymous TYPE_STRUCT */
        short len;                                            /* TYPE_SCALAR */
        char buf[];                                           /* TYPE_ARRAY (flexible) */
    } as_string;
    float32x8_t as_vector;                                    /* TYPE_LANG_STRUCT */
};

/* ========== TYPE_STRUCT with nested struct/union ========== */
struct Container {                                            /* TYPE_STRUCT */
    struct {                                                  /* Nested TYPE_STRUCT */
        int a;                                                /* TYPE_SCALAR */
        char b:4;                                             /* Bit-field TYPE_SCALAR */
        unsigned flag:1;                                      /* Bit-field TYPE_SCALAR */
    } inner __attribute__((packed));
    
    union {                                                   /* Nested TYPE_UNION */
        long x;                                               /* TYPE_SCALAR */
        double y;                                             /* TYPE_SCALAR */
        struct opaque* opaque_ptr;                            /* TYPE_POINTER to undefined */
    } data;
    
    struct Plugin* plugins[5];                                /* TYPE_ARRAY of TYPE_POINTER */
};

/* ========== Complex array and pointer types ========== */
struct Node {                                                 /* TYPE_STRUCT */
    int value;                                                /* TYPE_SCALAR */
    struct Node* next;                                        /* TYPE_POINTER */
    struct Node* children[3];                                 /* TYPE_ARRAY of TYPE_POINTER */
};

/* Multi-dimensional array of pointers */
struct Node* adjacency_matrix[10][10];                        /* TYPE_ARRAY of TYPE_ARRAY of TYPE_POINTER */

/* Pointer to array of function pointers */
int (*(*signal_processor)(float[][256], int**))();           /* TYPE_POINTER to TYPE_CALLBACK */

/* Triple pointer chain */
int ***triple_ptr_chain;                                      /* TYPE_POINTER */

/* ========== String and scalar types ========== */
const char* error_messages[] = {                              /* TYPE_ARRAY of TYPE_STRING */
    "Error",
    "Warning",
    "Info",
    NULL
};

/* Complex scalar types */
_Complex float cf;                                            /* TYPE_SCALAR */
_Complex double cd;                                           /* TYPE_SCALAR */
volatile _Bool status_flag;                                   /* TYPE_SCALAR */

/* ========== Global instances with initialization ========== */
struct Plugin plugin_registry[3] = {                          /* TYPE_ARRAY of TYPE_STRUCT */
    {"alpha", NULL, NULL, NULL},
    {"beta", NULL, NULL, NULL},
    {"gamma", NULL, NULL, NULL}
};

union Variant global_variant;                                 /* TYPE_UNION */

/* ========== Function definitions ========== */
/* Callback function implementations */
static int default_init(void) {
    return 0;
}

static void default_process(int val) {
    /* Do nothing */
}

static void sample_handler(int code, void* data) {
    /* Empty handler */
}

static float vector_transform(float32x8_t v) {
    return v[0] + v[7];
}

/* Function using complex pointer types */
static void process_matrix(void) {
    volatile struct Node* node_ptr = NULL;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            adjacency_matrix[i][j] = node_ptr;
        }
    }
}

/* Function processing union with runtime condition */
static int process_variant(union Variant *v, int type) {
    switch (type) {
        case 0:
            return v->as_int;
        case 1:
            return (int)(ptrdiff_t)v->as_ptr;
        case 2:
            return v->as_string.len;
        default:
            return (int)v->as_vector[0];
    }
}

/* Function using GCC vector type */
static float32x8_t vector_operation(float32x8_t a, float32x8_t b) {
    return a + b;
}

/* ========== Main function ========== */
int main(void) {
    int result = 0;
    
    /* 1. Initialize struct with nested types */
    struct Container container = {
        .inner = { .a = 42, .b = 3, .flag = 1 },
        .data = { .y = 3.14159 }
    };
    
    /* 2. Setup function pointers and callbacks */
    plugin_registry[0].init = default_init;
    plugin_registry[0].process = default_process;
    plugin_registry[0].on_error = sample_handler;
    
    if (plugin_registry[0].init) {
        result += plugin_registry[0].init();
    }
    
    /* 3. Use GCC vector type */
    float32x8_t vec_a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float32x8_t vec_b = {8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    float32x8_t vec_c = vector_operation(vec_a, vec_b);
    result += (int)vec_c[0];
    
    /* 4. Process multi-dimensional array */
    process_matrix();
    result += (adjacency_matrix[0][0] == NULL) ? 1 : 0;
    
    /* 5. Process union type */
    global_variant.as_int = 100;
    result += process_variant(&global_variant, 0);
    
    global_variant.as_ptr = &container;
    result += process_variant(&global_variant, 1);
    
    /* 6. Complex pointer chain */
    int value = 42;
    int *ptr1 = &value;
    int **ptr2 = &ptr1;
    triple_ptr_chain = &ptr2;
    result += ***triple_ptr_chain;
    
    /* 7. String array usage */
    for (int i = 0; error_messages[i] != NULL; i++) {
        result += (int)error_messages[i][0];  /* Add first char of each string */
    }
    
    /* 8. Complex scalar types */
    cf = 1.0f + 2.0fi;
    cd = 3.0 + 4.0i;
    result += (int)(__real__ cf + __imag__ cd);
    
    /* 9. Flexible array member simulation */
    char buffer[100] = "Test string";
    union Variant *str_var = (union Variant*)malloc(sizeof(union Variant) + strlen(buffer) + 1);
    if (str_var) {
        str_var->as_string.len = (short)strlen(buffer);
        strcpy(str_var->as_string.buf, buffer);
        result += str_var->as_string.len;
        free(str_var);
    }
    
    /* Return deterministic result based on all operations */
    return result % 256;
}
