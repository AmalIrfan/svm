SVM (Simple Virtual Machine)
============================

A simple virtual machine aimed to be a base for educational programming systems
like [popa](https://github.com/AmalIrfan/popa).

## Spec aim:

- Memory: 65536 (0000 to FFFF)
- Data stack: 0000 to 2FFF
- Return stack: 3000 to 4FFF
- General: 5000 to FFFF

### Ops

+---------+--------------------------------------------------------------------+
| `CALL ` | store current address on return stack, pop the address on stack    |
|         | and jump to it                                                     |
+---------+--------------------------------------------------------------------+
| `EXIT ` | pop the return address from return stack and jump to it            |
+---------+--------------------------------------------------------------------+
| `LIT X` | push `X` to stack                                                  |
+---------+--------------------------------------------------------------------+
