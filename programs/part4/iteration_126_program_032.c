#ifndef TEST_GTY_PARSER_H
#define TEST_GTY_PARSER_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointers) */
struct GTY(()) StructWithParens {
  /* Function pointer with parameters in parentheses */
  int (*callback_func)(int, char*);
  
  /* Bitfield with expression in parentheses */
  unsigned int bits: (sizeof(int) * 8 - 1);
  
  /* Nested function pointer type */
  void (*(*complex_callback)(void))(int);
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
  /* Fixed-size array */
  int fixed_array[10];
  
  /* Multi-dimensional array */
  double matrix[3][3];
  
  /* Zero-length array at end (flexible array member) */
  char name[];
  
  /* Array with computed size using parentheses */
  unsigned char data[(sizeof(int) + 7) & ~7];
};

/* Test case 3: Struct with braces (nested types) */
struct GTY(()) StructWithBraces {
  int id;
  
  /* Anonymous nested union with braces */
  union {
    int as_int;
    float as_float;
    void* as_ptr;
  } GTY((tag("0"))) value;
  
  /* Nested struct definition */
  struct {
    int x;
    int y;
  } GTY((skip)) point;
};

/* Test case 4: Complex type combining all bracket types */
struct GTY(()) ComplexType {
  /* Function pointer returning pointer to array (mixes parens and brackets) */
  int (*(*get_matrix)(int size))[10];
  
  /* Array of function pointers */
  void (*handlers[5])(void*);
  
  /* Nested struct with bitfield using parentheses */
  struct {
    unsigned int flags: (8 * sizeof(unsigned int) - 4);
    char name[32];
  } GTY((desc("%1.flags"))) config;
  
  /* Union with anonymous struct */
  union {
    struct {
      int a;
      int b;
    };
    long long combined;
  } GTY((desc("0"))) data;
};

/* Test case 5: Type with deeply nested brackets */
typedef struct GTY(()) DeeplyNested {
  /* Pointer to array of function pointers */
  int (*(*(*deep_array)[5])(int))[3];
  
  /* Struct containing union containing array */
  struct {
    union {
      int vec[4];
      struct {
        int x, y, z, w;
      };
    };
  } GTY((skip)) coordinates;
} DeeplyNestedType;

/* Test case 6: Template-like structure (simulating C++ templates in C) */
#define DECLARE_VECTOR(T) \
  struct GTY(()) Vector_##T { \
    T* GTY((length("%h.size"))) items; \
    size_t size; \
    size_t capacity; \
    int (*compare)(const T*, const T*); \
  }

/* Instantiate some vector types */
DECLARE_VECTOR(int);
DECLARE_VECTOR(void*);
DECLARE_VECTOR(char);

/* Clean up macro */
#undef DECLARE_VECTOR

#endif /* TEST_GTY_PARSER_H */
