SET_DEST(x) → Check ZERO_EXTRACT/STRICT_LOW_PART → If true: mark recursively
               ↓ If false
               Check SUBREG → If true: get underlying register
               ↓ If false  
               Check MEM_P → If true: mark address recursively
               ↓
               Return
