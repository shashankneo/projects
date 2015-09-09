.text


.globl set_timer

set_timer:  
   mov $0x34,%al                 
   outb %al, $0x43
   mov $0x9B,%al                     
   outb %al,$0x40                       
   mov $0x2E,%al
   outb %al,$0x40
   retq
   