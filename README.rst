SVM (Simple Virtual Machine)
============================

A simple virtual machine aimed to be a base for educational programming systems
like `popa <https://github.com/AmalIrfan/popa>`_.

I have started taking inspiration from `colorforth <https://colorforth.github.io/inst.htm>_`.

Spec aim:
---------
- Memory: 65536 (0000 to FFFF)
- Data stack: 0000 to 00FF
- Return stack: 0100 to 01FF
- General: 0200 to FFFF

Ops
^^^

+---------+--------------------------------------------------------------------+
| ``NOP`` | no operation                                                       |
+---------+--------------------------------------------------------------------+
| ``HLT`` | stop the cpu                                                       |
+---------+--------------------------------------------------------------------+
| ``CAL`` | go to address, push current address to return stack                |
+---------+--------------------------------------------------------------------+
| ``RET`` | go to address in top of return stack, consumes                     |
+---------+--------------------------------------------------------------------+
| ``BNZ`` | branch if top of data stack is not zero, consumes                  |
+---------+--------------------------------------------------------------------+
| ``BNG`` | branch if top of data stack is less than zero, consumes            |
+---------+--------------------------------------------------------------------+
| ``LIT B`` | load byte from code                                              |
+---------+--------------------------------------------------------------------+
| ``LAD A`` | load address from code                                           |
+---------+--------------------------------------------------------------------+
| ``FCH`` | fetch data at the address from the top of data stack, consumes     |
+---------+--------------------------------------------------------------------+
| ``PUT`` | put data at the address from the top of data stack, consumes       |
+---------+--------------------------------------------------------------------+
| ``POP`` | pop top of return stack and push it to top of data stack, produces |
+---------+--------------------------------------------------------------------+
| ``PSH`` | pop top of data stack and push it to top of return stack, consumes |
+---------+--------------------------------------------------------------------+

Examples
^^^^^^^^

::

    NOP       ; u8
    CAL 0005  ; u8 u16
    HLT       ; u8
    LIT 00    ; u8 u8
    BNZ +00   ; u8 i8
    LIT 00    ; u8 u8
    BNG +00   ; u8 u8
    LAD 0000  ; u8 u16
    FCH       ; u8
    LIT 00    ; u8
    LAD 0000  ; u8 u16
    PUT       ; u8
    POP       ; u8
    PSH       ; u8
    RET       ; u8
