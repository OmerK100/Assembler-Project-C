Final project in 20465 course of the Open University in Israel.
A simulator that transaltes instructions of a pre-made assembly language into a pre-made machine code.
The process consists of several stages:
1.) Pre-assembly of the instructions file, which saves the labels of the pre-made Assembly language 
into a list data structure and spreads the macros into a pre-assembled file.
2.) First translation run which runs over the pre-assembled file and translates all possible instructions into machine code.
3.) Second run which transaltes all the remaining lines (such as lines that include lables). All the machine code is stored within a desiganted data structre.
4.) Finally after the transaltion, we count the number of instructions from each type (.data or .text codes), and write the saved machine code into a file.
