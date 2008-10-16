segment .data

segment .bss

segment .text
    global glum_clear_asm
    global _glum_clear_asm

glum_clear_asm:
_glum_clear_asm:

    push    ebp
    mov     ebp, esp

    push    edi         ; preserve caller's edi

; [ebp] == caller's ebp 
; [ebp+4] == return address
; [ebp+8] == fb
; [ebp+12] == pixels
; [ebp+16] == color

    mov     ecx, [ebp+12]
    mov     eax, [ebp+16]
    mov     edi, [ebp+8]
    rep stosd

    pop     edi
    pop     ebp         ; restore caller's ebp
    ret                 ; return to caller
