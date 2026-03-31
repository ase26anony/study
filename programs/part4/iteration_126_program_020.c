#ifndef TEST_GTY_COVERAGE_H
#define TEST_GTY_COVERAGE_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointers) */
struct GTY(()) StructWithParens {
  /* Function pointer with parentheses */
  int (*callback)(int, char*);
  
  /* Bitfield with parentheses in expression */
  unsigned int bits: (sizeof(int) * 8 - 1);
  
  /* Complex function pointer */
  void (*(*complex_callback[5])(int))(float);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
  /* Simple array */
  int simple_array[10];
  
  /* Multi-dimensional array */
  double matrix[3][4];
  
  /* Flexible array member */
  char flexible_array[];
  
  /* Array with computed size */
  long computed_size[sizeof(double) > 4 ? 8 : 4];
};

/* Test case 3: Struct with braces (nested types) */
struct GTY(()) StructWithBraces {
  /* Anonymous nested union with braces */
  union {
    int as_int;
    float as_float;
    struct {
      char a;
      char b;
    } as_struct;
  } nested_data;
  
  /* Named nested struct */
  struct GTY(()) InnerStruct {
    int x;
    int y;
  } inner;
  
  /* Another anonymous struct */
  struct {
    short s;
    long l;
  } another_nested;
};

/* Test case 4: Complex type combining all bracket types */
struct GTY(()) ComplexType {
  /* Function pointer returning pointer to array (mixes parens and brackets) */
  int (*(*get_array_ptr)(int size))[];
  
  /* Array of function pointers */
  void (*callbacks[5])(void);
  
  /* Nested anonymous union with array */
  union {
    int (*int_funcs[3])(int);
    float (*float_funcs[2])(float);
  } func_union;
  
  /* Struct with bitfield using parentheses */
  struct {
    unsigned int flags: (8 * sizeof(unsigned int) - 4);
    int data[(10 + 2) * 3];  /* Array with parenthesized expression */
  } data_block;
};

/* Test case 5: Pointer chain with all bracket types */
typedef struct GTY(()) Node {
  struct Node* GTY((skip)) next;
  struct Node* GTY((skip)) prev;
  
  /* Mixed usage in a single declaration */
  union {
    /* Array of pointers to functions */
    int (*(*func_array[4])(int))[2];
    
    /* Pointer to array of structs */
    struct GTY(()) Element {
      int id;
      char name[32];
    } (*element_array)[10];
  } data;
  
  /* Direct nested struct definition */
  struct {
    int depth;
    struct Node* GTY((skip)) parent;
  } tree_info;
} Node;

/* Test case 6: Template-like structure (for C++) */
#ifdef __cplusplus
extern "C" {
#endif

struct GTY(()) TemplateLike {
  /* Function pointer with complex signature */
  void* (*(*allocator)(size_t size, void* context))(void);
  
  /* Nested struct with array */
  struct Buffer {
    unsigned char data[256];
    size_t GTY((skip)) length;
  } buffer;
  
  /* Union with anonymous struct */
  union {
    struct {
      int x, y;
    } point;
    int coordinates[2];
  } position;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_COVERAGE_H */
