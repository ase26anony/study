struct {
    unsigned int flag : 1;
} s;

unsigned int *ptr = &s.flag;  // ERROR: cannot take address of bitfield
