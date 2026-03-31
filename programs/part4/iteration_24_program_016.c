Original:
    jump L1
    nop            # delay slot
    add r1, r2, r3
L1: mul r4, r5, r6

After optimization:
    jump L1
    mul r4, r5, r6  # moved from target into delay slot
    add r1, r2, r3
L1: # label now points here
