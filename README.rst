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
| ``CALL``  | store current address on return stack, pop the address on stack  |
|           | and jump to it                                                   |
+-----------+------------------------------------------------------------------+
| ``EXIT``  | pop the return address from return stack and jump to it          |
+-----------+------------------------------------------------------------------+
| ``LIT X`` | push `X` to stack                                                |
+-----------+------------------------------------------------------------------+
| ``DUP``   | duplicate stack top                                              |
+-----------+------------------------------------------------------------------+
| ``DROP``  | pop stack top                                                    |
+-----------+------------------------------------------------------------------+
| ``READ``  | push user input to stack                                         |
+-----------+------------------------------------------------------------------+
| ``WRITE`` | pop the value (character) on the stack and write it to output    |
+-----------+------------------------------------------------------------------+
| ``SUB``   | pop two values from stack and push the difference onto stack     |
+-----------+------------------------------------------------------------------+
| ``JNZ``   | pop two values from stack, conditional and address, then jump to |
|           | address if conditional value is not zero                         |
+-----------+------------------------------------------------------------------+

Asm
^^^

A simple assembler.

Features:
- converts text to opcodes.
- label support.
