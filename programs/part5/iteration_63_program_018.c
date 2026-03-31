   if (cond != 0) goto lab1 else goto lab2
   lab1:
     SIMT version of the loop
     goto lab3
   lab2:
     Original (non-SIMT) version of the loop
   lab3:
     // Continue execution
