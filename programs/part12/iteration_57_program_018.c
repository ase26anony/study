// Before
*ptr = value;
ptr++;

// After (using post-increment addressing)
*(ptr++) = value;
