MYDATA    SEGMENT
DATA  	DW      1234H,5678H,9D4CH,0D7H,0,-1,7D2AH,8A0EH,10F5H,645DH, 7D2BH,7D2CH,7D2DH
N   	EQU     $-DATA
MAX     DW      ?
MYDATA    ENDS
CODE    SEGMENT
        ASSUME  CS:CODE,DS:MYDATA
MAIN PROC FAR
        MOV     AX,MYDATA
        MOV     DS,AX
        MOV     CX,N-1
        MOV     BX,OFFSET DATA
        MOV     AX,[BX]
AGAIN:  INC     BX
        MOV     SI, [BX]
        TEST    SI, 01H
        JNZ     NEXT
        CMP     AX,[BX]
        JLE     NEXT
        MOV     AX,[BX]
NEXT:   LOOP    AGAIN

        MOV     MAX,AX
        MOV     BX,AX
        CALL    PRINTPROC
        RET
MAIN    ENDP

PRINTPROC   PROC
        MOV     AL,4            ;cx: 0010 0100
        MOV     CL,4
        MOV     DI,4

ROTATE: MOV     BP,CX
        ROL     BX,CL
        MOV     DL,BL
        MOV     AH,2
        AND     DL,0FH
        CMP     DL, 0AH
        JGE     ZIMU
        ADD     DL, '0'
        JMP     PRINT
ZIMU:   SUB     DL, 0AH
        ADD     DL, 'A'
PRINT:  INT     21H
        MOV     CX,BP
        DEC     AL
        CMP     AL,0
        JG      ROTATE
        RET
PRINTPROC   ENDP

CODE    ENDS
END     MAIN