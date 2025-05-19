SVM (Simple Virtual Machine)
============================

A simple virtual machine aimed to be a base for educational programming systems
like `popa <https://github.com/AmalIrfan/popa>`_.

Spec aim:
---------
- Memory: 65536 (0000 to FFFF)
- Data stack: 0000 to 2FFF
- Return stack: 3000 to 4FFF
- General: 5000 to FFFF

Ops
^^^

+-----------+------------------------------------------------------------------+
| ``NOP``   | no operation. advances like other ops                            |
+-----------+------------------------------------------------------------------+
| ``CALL X`` | store current address on return stack, and jump to address X    |
+-----------+------------------------------------------------------------------+
| ``EXIT``  | pop the return address from return stack and jump to it          |
+-----------+------------------------------------------------------------------+
| ``LIT X`` | push `X` to stack                                                |
+-----------+------------------------------------------------------------------+
| ``DUP``   | duplicate stack top                                              |
+-----------+------------------------------------------------------------------+
| ``DROP``  | pop stack top                                                    |
+-----------+------------------------------------------------------------------+
| ``SWAP``  | exchange two values on the top of stack                          |
+-----------+------------------------------------------------------------------+
| ``OVER``  | copy the value second to top of stack to the top                 |
+-----------+------------------------------------------------------------------+
| ``READ``  | push user input to stack                                         |
+-----------+------------------------------------------------------------------+
| ``WRITE`` | pop the value (character) on the stack and write it to output    |
+-----------+------------------------------------------------------------------+
| ``SUB``   | pop two values from stack and push the difference onto stack     |
+-----------+------------------------------------------------------------------+
| ``JNZ X`` | pop condition off the stack and jump to address X if condition   |
|           | is not zero                                                      |
+-----------+------------------------------------------------------------------+
| ``LOAD``  | pop the address on stack and push the value at that address onto |
|           | stack                                                            |
+-----------+------------------------------------------------------------------+
| ``STORE`` | pop the value and address on stack and store that value at the   |
|           | given address                                                    |
+-----------+------------------------------------------------------------------+

Asm
^^^

A simple assembler (sas).

Features:

- converts text to opcodes.
- label support.
- comments support.

Example::

    make build/main build/sas
    build/sas examples/02_asm_input.txt > build/output.bin
    build/main build/output.bin 2>log

More Examples
-------------

Hello World
^^^^^^^^^^^

`03_hello_world.txt <./examples/03_hello_world.txt>`_

::

    ; HELLO WORLD
      LIT entry
      CALL
      -1
    msg:
      72 69 76 76 79 32 87 79 82 76 68 10
    entry:
      LIT msg  ; msg
    loop:
      DUP      ; msg msg
      LOAD     ; msg c
      DUP      ; msg c c
      WRITE    ; msg c
      SWAP     ; c msg
      LIT -1   ; c msg -1
      SUB      ; c msg-1
      SWAP     ; msg-1 c
      LIT 10   ; msg-1 c 10
      SUB      ; msg-1 c-10
      LIT loop ; msg-1 c-10 loop
      JNZ      ; msg-1
      DROP
      EXIT
