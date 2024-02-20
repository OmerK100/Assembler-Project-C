Final project in 20465 course of the Open University in Israel.
A simulator that transaltes instructions of a pre-made assembly language into a pre-made machine code.
The process consists of several stages:
1.) Pre-assembly of the instructions file, which saves the labels of the pre-made Assembly language 
into a list data structure and spreads the macros into a pre-assembled file.
2.) First translation run which runs over the pre-assembled file and translates all possible instructions into machine code.
3.) Second run which transaltes all the remaining lines (such as lines that include lables). All the machine code is stored within a desiganted data structre.
4.) Finally after the transaltion, we count the number of instructions from each type (.data or .text codes), and write the saved machine code into a file.


## Visual exmamples:

Showcase of various errors found during the assembly:
![filesTogether19](https://github.com/OmerK100/AssemblerProjectC/assets/139342166/bbb6baf8-f111-4e25-a3fb-05558bdad6a0)

Assembly of several files with errors located:
![filesTogether18](https://github.com/OmerK100/AssemblerProjectC/assets/139342166/8db20760-83f8-4b86-90f4-3135e8bd8dac)

Empty file to assemble error:
![empty23](https://github.com/OmerK100/AssemblerProjectC/assets/139342166/55089341-d90a-4177-9d53-25d721abcbd7)

Successful assembly!
![test5good8](https://github.com/OmerK100/AssemblerProjectC/assets/139342166/5d76d759-29d8-4b63-8d30-38911bb89bc9)

Output of assembly:
![test5good9](https://github.com/OmerK100/AssemblerProjectC/assets/139342166/35b74bf4-d556-4914-a9a5-0e0b9655f2fb)

More sorts of errors found examples:
![test4bad7](https://github.com/OmerK100/AssemblerProjectC/assets/139342166/8efee93d-fe38-4f57-bd15-89ae14ea602a)

![test4bad6](https://github.com/OmerK100/AssemblerProjectC/assets/139342166/40beb5d2-9563-4142-9ea4-7f28c929b414)

![filesTogether20](https://github.com/OmerK100/AssemblerProjectC/assets/139342166/f306e943-ab21-4351-8c6a-1a34093c221b)

