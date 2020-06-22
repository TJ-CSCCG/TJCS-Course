DATA SEGMENT

FILESTR  DB        '1.txt',0
HANDLE   DW        0
BUF      DB        100 DUP(0)
ERRMSG1  DB        13,10,'file operation error 1! $'
ERRMSG2  DB        13,10,'file operation error 2! $'
ERRMSG3  DB        13,10,'file operation error 3! $'
NUM      DW        0

DATA ENDS

CODE  SEGMENT
    ASSUME    CS:CODE
START:  ;PUSH      CS
        ;POP       DS
        ;PUSH      CS
        ;POP       ES
 
        MOV       AH,3DH      ; 打开文件
        LEA       DX,FILESTR
        MOV       AL,0
        INT       21H
        MOV       HANDLE,AX
 
@M0:
        MOV       AH,3FH      ; 单字节读文件
        MOV       BX,HANDLE
        MOV       CX,1
        LEA       DX,BUF
        INT       21H
        CMP       AX,1
        JNE       @FILEEND
        INC       NUM
        JMP       @M0
 
@FILEEND:
        MOV       AH,3EH      ; 关闭文件
        MOV       BX,HANDLE
        INT       21H
              
        MOV       AX, NUM
        MOV       BL, 9
        DIV       BL
@EXIT:
        MOV       AH,4CH
        INT       21H
 
 
CODE  ENDS
END   START