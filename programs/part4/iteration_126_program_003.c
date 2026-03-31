#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include for size_t if needed */
#include <stddef.h>

/* Struct with parentheses - triggers case '(' */
struct GTY(()) StructWithParens {
  /* Function pointer with parentheses */
  int (*callback)(int, char*);
  
  /* Bitfield with parenthesized expression */
  unsigned int bits: (sizeof(int) * 8 - 1);
  
  /* Nested function pointer type */
  void (*complex_callback)(int (*)(double), char);
};

/* Union with brackets - triggers case '[' */
union GTY(()) UnionWithBrackets {
  /* Fixed-size array */
  int fixed_array[100];
  
  /* Multi-dimensional array */
  double matrix[10][20];
  
  /* Zero-length array (GCC extension) */
  char flexible_array[0];
  
  /* Array with computed size */
  unsigned char sized_array[sizeof(long double)];
};

/* Struct with braces - triggers case '{' */
struct GTY(()) StructWithBraces {
  int id;
  
  /* Anonymous nested union with braces */
  union {
    int as_int;
    float as_float;
    struct {
      char a;
      char b;
    } as_struct;
  } data;
  
  /* Nested struct definition */
  struct inner {
    int x;
    int y;
  } point;
};

/* Complex type combining all three bracket types */
struct GTY(()) ComplexType {
  /* Parentheses: function pointer */
  char* (*allocator)(size_t);
  
  /* Brackets: array of function pointers */
  int (*handlers[5])(void);
  
  /* Braces: nested anonymous struct */
  struct {
    /* Parentheses in bitfield */
    unsigned flags: (8 * sizeof(char));
    
    /* Brackets in array */
    int values[4];
    
    /* More braces: union */
    union {
      long l;
      void* p;
    } storage;
  } context;
  
  /* Multi-dimensional array with parenthesized size */
  double grid[(sizeof(void*) == 8 ? 64 : 32)][16];
};

/* Another type with deeply nested brackets */
typedef struct GTY(()) DeeplyNested {
  /* Array of pointers to functions returning pointers to arrays */
  int (*(*func_table[10])(int))[20];
  
  /* Struct containing union containing struct... */
  struct level1 {
    union level2 {
      struct level3 {
        int (*(*deep_func)(void))[5];
        char data[100];
      } deepest;
      float alternate;
    } choice;
  } container;
} DeeplyNestedType;

#endif /* TEST_GTY_H */
