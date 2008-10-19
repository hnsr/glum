; TODO: Calling conventions on win64 are different, this probably won't work

segment .data

segment .bss

segment .text
    global glum_clear_asm
    global _glum_clear_asm

    global glum_clear_asm2
    global _glum_clear_asm2

; Using 'rep stosd' to clear framebuffer to color
glum_clear_asm:
_glum_clear_asm:

    push    rbp
    mov     rbp, rsp

; rdi == *fb
; esi == pixels
; edx == color

    mov     ecx, esi
    mov     eax, edx
    rep stosd

    leave
    ret

; Using stosq isn't really any faster and has the additional requirement for pixels to be an even
; number to work properly. Not using this!
glum_clear_asm2:
_glum_clear_asm2:

    push    rbp
    mov     rbp, rsp

; rdi == *fb
; esi == pixels
; edx == color

    mov     ecx, esi
    ; divide pixels by two since we're copying twice as much data
    shr     ecx, 1

    mov     eax, edx ; copy color into lower 32bits of rax
    shl     eax, 32  ; move them to the upper 32bits, 
    add     rax, rdx ; add color to it to fill the lower 32 bits
    rep stosq

    leave
    ret
