global fly_to_the_sky ;
; Load struct Context (CPU GP-register) then jump
; Function Signature: void process_context_switch(struct Context ctx);

fly_to_the_sky:
    ; Using iret (return instruction for interrupt) technique for privilege change
    lea  ecx, [esp+0x04] ; Save the base address for struct Context ctx
    
    push eax ; Stack segment selector (GDT_USER_DATA_SELECTOR), user privilege
    mov  eax, ecx
    add  eax, 0x400000 - 4
    push eax ; User space stack pointer (esp), move it into last 4 MiB
    pushf    ; eflags register state, when jump inside user program
    mov  eax, 0x18 | 0x3
    push eax ; Code segment selector (GDT_USER_CODE_SELECTOR), user privilege
    mov  eax, ecx
    push eax ; eip register to jump back
    iret