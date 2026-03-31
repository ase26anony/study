   struct tagged_string {
       int length_bits;  // Fixed size (typically 4 bytes)
       char data[];      // Variable size, starts immediately after length_bits
   };
