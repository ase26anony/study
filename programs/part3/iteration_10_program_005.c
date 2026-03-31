__asm__ volatile ("# dummy" 
    : outputs      // after colon
    : inputs       // after second colon  
    : clobbers     // after third colon
);
