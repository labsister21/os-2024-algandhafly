global fly_to_the_sky ;
; Load struct Context (CPU GP-register) then jump to the sky
; Function Signature: void process_context_switch(struct Context ctx);

fly_to_the_sky:
    ; Using iret (return instruction for interrupt) technique for privilege change
    lea  ecx, [esp+0x04] ; Save the base address for struct Context ctx
    

    mov  edi, [ecx + 4]
    mov  esi, [ecx + 8]
    mov  ebp, [ecx + 12]
    
    mov  ebx, [ecx + 20]
    mov  edx, [ecx + 24]

    mov  ax, [ecx + 36]
    mov  gs, ax
    mov  ax, [ecx + 40]
    mov  fs, ax
    mov  ax, [ecx + 44]
    mov  es, ax
    mov  ax, [ecx + 48]
    mov  ds, ax
    
    push 0x23               ; ss
    push dword [ecx + 16]   ; esp
    push dword [ecx + 52]   ; eflags
    push dword [ecx + 56]   ; cs
    push dword [ecx]        ; eip

    mov  eax, [ecx + 32]
    mov  ecx, [ecx + 28]

    iret