SVM (Simple Virtual Machine)
============================

A simple virtual machine aimed to be a base for educational programming systems
like `popa <https://github.com/AmalIrfan/popa>`_.

I have started taking inspiration from `colorforth <https://colorforth.github.io/inst.htm>`_.

Build & Run
-----------

Windows Build & Run
^^^^^^^^^^^^^^^^^^^

MSVC

::

    .\rebuild.bat cl

gcc

::

    .\rebuild.bat cc

**Running**

Assemble a program

::

    cd build
    .\sas.exe .\build\output ..\examples\06_data_man.txt

Run the program

::

    .\main.exe .\output

Run the program with log

::

    .\main.exe d .\output

Posix Build & Run
^^^^^^^^^^^^^^^^^

::

    ./rebuild.sh

**Running**

Assemble a program

::

    cd build
    ./sas.exe ./build/output ../examples/06_data_man.txt

Run the program

::

    ./main.exe ./output

Run the program with log

::

    ./main.exe d ./output

Spec aim:
---------
- Memory: 65536 (0000 to FFFF)
- Data stack: 0000 to 00FF
- Return stack: 0100 to 01FF
- General: 0200 to FFFF
  - Ports: FFFD to FFFF

Ops
^^^

+---------+--------------------------------------------------------------------+
| ``NOP`` | no operation                                                       |
+---------+--------------------------------------------------------------------+
| ``HLT`` | stop the cpu                                                       |
+---------+--------------------------------------------------------------------+
| ``CAL`` | go to dwordess, push current dwordess to return stack                |
+---------+--------------------------------------------------------------------+
| ``RET`` | go to dwordess in top of return stack, consumes                     |
+---------+--------------------------------------------------------------------+
| ``BNZ`` | branch if top of data stack is not zero, consumes                  |
+---------+--------------------------------------------------------------------+
| ``BNG`` | branch if top of data stack is less than zero, consumes            |
+---------+--------------------------------------------------------------------+
| ``ADD`` | add values at top of the data stack, consumes                      |
+---------+--------------------------------------------------------------------+
| ``SUB`` | sub values at top of the data stack, consumes                      |
+---------+--------------------------------------------------------------------+
| ``LIT B`` | load byte from code                                              |
+---------+--------------------------------------------------------------------+
| ``LAD A`` | load dwordess from code                                           |
+---------+--------------------------------------------------------------------+
| ``FCH`` | fetch data at the dwordess from the top of data stack, consumes     |
+---------+--------------------------------------------------------------------+
| ``PUT`` | put data at the dwordess from the top of data stack, consumes       |
+---------+--------------------------------------------------------------------+
| ``POP`` | pop top of return stack and push it to top of data stack, produces |
+---------+--------------------------------------------------------------------+
| ``PSH`` | pop top of data stack and push it to top of return stack, consumes |
+---------+--------------------------------------------------------------------+
| ``DUP`` | dup value at top of the data stack, produces                       |
+---------+--------------------------------------------------------------------+
| ``DRP`` | drop value at top of the data stack, consumes                      |
+---------+--------------------------------------------------------------------+
| ``OVR`` | dup value next to top of the data stack, produces                  |
+---------+--------------------------------------------------------------------+

Ports
^^^^^

- Input:  FFFD - 0300
- Output: FFFC - 0300
- Data:   FFFF - 0300

On fetch from input port, data port is filled with input.
Similarly, on fetch from output port, data port is written to output.

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
