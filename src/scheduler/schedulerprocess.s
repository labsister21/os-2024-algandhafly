global fly_to_the_sky ;
; Load struct Context (CPU GP-register) then jump to the sky
; Function Signature: void process_context_switch(struct Context ctx);

fly_to_the_sky:
    ; Using iret (return instruction for interrupt) technique for privilege change
    lea  ecx, [esp+0x04] ; Save the base address for struct Context ctx
    mov  edi, [ecx + 4]
    mov  esi, [ecx + 8]
    mov  ebp, [ecx + 12]
    mov  esp, [ecx + 16]
    mov  ebx, [ecx + 20]
    mov  edx, [ecx + 24]

    mov  ax, [ecx + 36]
    mov  gs, ax
    mov  ax, [ecx + 38]
    mov  fs, ax
    mov  ax, [ecx + 40]
    mov  es, ax
    mov  ax, [ecx + 42]
    mov  ds, ax
    mov  eax, [ecx + 32]

    push dword [ecx + 52]
    push dword [ecx + 56]
    push dword [ecx]
    mov  ecx, [ecx + 28]
    iret