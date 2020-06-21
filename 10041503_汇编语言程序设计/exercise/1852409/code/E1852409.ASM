MYDATA    SEGMENT
DATA  	DW      1234H,5678H,9D4CH,0D7H,0,-1,7D2AH,8A0EH,10F5H,645DH
N   	EQU     $-DATA
MAX     DW      ?
MYDATA    ENDS
CODE    SEGMENT
        ASSUME  CS:CODE,DS:MYDATA
START:  MOV     AX,MYDATA
        MOV     DS,AX
        MOV     CX,N-1
        MOV     BX,OFFSET DATA
        MOV     AX,[BX]
AGAIN:  INC     BX
        MOV     SI, [BX]
        TEST    SI, 01H
        JNZ     NEXT
        CMP     AX,[BX]
        JGE     NEXT
        MOV     AX,[BX]
NEXT:   LOOP    AGAIN
        MOV     MAX,AX
        MOV     BX,AX
        MOV     CH,2
        MOV     CL,4            ;cx: 0010 0100
ROTATE: ROR     BX,CL
        MOV     AX,BX
        AND     AX,0FH
        ADD     AX,30H
        CMP     AX,3AH
        JL      PRINTIT
        ADD     AX,7
PRINTIT:MOV     DX,AX
        MOV     AH,2
        INT     21H
        DEC     CH
        JNZ     ROTATE
        MOV     AH,2
        MOV     DL,48H
        INT     21H
        MOV     AH,4CH
        INT     21H
CODE    ENDS
       	END	START

