; Prologue
lis $11
.word 1
lis $4
.word 4
; pop $31 to stack
sw $31, -4($30)
sub $30, $30, $4
lis $31
.word Fwain
jalr $31
; pop to $31 from stack
add $30, $30, $4
lw $31, -4($30)
jr $31

Ffoo:
; pop $31 to stack
sw $31, -4($30)
sub $30, $30, $4
; pop $29 to stack
sw $29, -4($30)
sub $30, $30, $4
; pop $5 to stack
sw $5, -4($30)
sub $30, $30, $4
add $29, $30, $0
; pop $1 to stack
sw $1, -4($30)
sub $30, $30, $4
; pop $2 to stack
sw $2, -4($30)
sub $30, $30, $4
lis $3
.word 0
; pop $3 to stack
sw $3, -4($30)
sub $30, $30, $4
; getVar var
lw $3, -4($29)
; pop to $5 from stack
add $30, $30, $4
lw $5, -4($30)
sub $3, $5, $3
add $30, $29, $0
; pop to $5 from stack
add $30, $30, $4
lw $5, -4($30)
; pop to $29 from stack
add $30, $30, $4
lw $29, -4($30)
; pop to $31 from stack
add $30, $30, $4
lw $31, -4($30)
jr $31

Fbar:
; pop $31 to stack
sw $31, -4($30)
sub $30, $30, $4
; pop $29 to stack
sw $29, -4($30)
sub $30, $30, $4
; pop $5 to stack
sw $5, -4($30)
sub $30, $30, $4
add $29, $30, $0
; pop $1 to stack
sw $1, -4($30)
sub $30, $30, $4
; pop $2 to stack
sw $2, -4($30)
sub $30, $30, $4
; getVar var
lw $3, -4($29)
; pop $3 to stack
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 2
; pop to $5 from stack
add $30, $30, $4
lw $5, -4($30)
mult $3, $5
mflo $3
add $30, $29, $0
; pop to $5 from stack
add $30, $30, $4
lw $5, -4($30)
; pop to $29 from stack
add $30, $30, $4
lw $29, -4($30)
; pop to $31 from stack
add $30, $30, $4
lw $31, -4($30)
jr $31

Fwain:
; pop $31 to stack
sw $31, -4($30)
sub $30, $30, $4
; pop $29 to stack
sw $29, -4($30)
sub $30, $30, $4
; pop $5 to stack
sw $5, -4($30)
sub $30, $30, $4
add $29, $30, $0
; pop $1 to stack
sw $1, -4($30)
sub $30, $30, $4
; pop $2 to stack
sw $2, -4($30)
sub $30, $30, $4
; pcall to foo
; getVar a
lw $3, -4($29)
add $1, $3, $0
lis $31
.word Ffoo
jalr $31
; pop $3 to stack
sw $3, -4($30)
sub $30, $30, $4
; pcall to bar
; getVar b
lw $3, -8($29)
add $1, $3, $0
lis $31
.word Fbar
jalr $31
; pop to $5 from stack
add $30, $30, $4
lw $5, -4($30)
sub $3, $5, $3
add $30, $29, $0
; pop to $5 from stack
add $30, $30, $4
lw $5, -4($30)
; pop to $29 from stack
add $30, $30, $4
lw $29, -4($30)
; pop to $31 from stack
add $30, $30, $4
lw $31, -4($30)
jr $31

; Epilogue
Fgetchar:
lis $3
.word 0xffff0004
lw $3, 0($3)
jr $31

Fputchar:
; pop $3 to stack
sw $3, -4($30)
sub $30, $30, $4
lis $3
.word 0xffff000c
sw $1, 0($3)
; pop to $3 from stack
add $30, $30, $4
lw $3, -4($30)
jr $31
