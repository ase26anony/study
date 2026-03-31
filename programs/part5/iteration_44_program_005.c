/* Test file for gengtype parser - default case triggers */
%{
#include "config.h"
#include "system.h"
%}

%typedef unsigned int size_t;
%typedef void* ptr_t;

/* Various punctuation that should trigger default case */
%struct default_test {
  int simple_field;      /* ; triggers default */
  char *string_ptr;      /* * triggers default */
  const int& ref_field;  /* & triggers default */
  int (*func_ptr)(int, char**);  /* * and , trigger default */
  struct inner {
    int x, y;  /* , triggers default */
  } nested;
  int array[10];  /* [ will be balanced, but 10 and ] trigger default */
};

/* Preprocessor directives */
#ifdef SPECIAL_FEATURE
%struct special {
  int feature_enabled;
};
#endif

/* Multiple comments to test lexer */
/* This is a /* nested */ comment test */  /* Should handle gracefully */

%var struct default_test global_instance;
