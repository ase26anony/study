  struct {
      unsigned int flag : 1;  // bit-field: 1 bit
  } s;
  
  unsigned int *ptr = &s.flag;  // ERROR: cannot take address of bit-field
