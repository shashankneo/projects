.text


.globl remap_pic

remap_pic:
   

movb $0x11, %al
outb %al, $0x20     #restart PIC1
outb %al, $0xA0     #restart PIC2
 
movb $0x20,%al
outb %al, $0x21     #PIC1 now starts at 32
movb $0x28, %al
outb %al, $0xA1     #PIC2 now starts at 40
 
movb $0x04, %al
outb %al, $0x21     #ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
movb $0x02, %al
outb %al, $0xA1    #ICW3: tell Slave PIC its cascade identity (0000 0010)
 
movb $0x01,%al    # 8086/88 (MCS-80/85) mode 
outb %al, $0x21    
outb %al, $0xA1     #done!

//sti

retq