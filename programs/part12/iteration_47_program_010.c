   struct tagged_string {
       int length_bits;  // 4 bytes (typically)
       char data[];      // Variable length, starts immediately after length_bits
   };
