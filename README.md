binder
======

A simple tool that can be used to compare binary files.

The following operators are supported:

| Operator | Description                                                |
| -------- | ---------------------------------------------------------- |
| xor      | Returns a list of all bytes that differ between two files. |
| and      | Takes two diff lists and finds the intersect.              |
| or       | Takes two diff lists and finds the union.                  |

For example, the following command can be used to find the bytes that differ between three different binary files:

	$ ./bin/binder { examples/0.txt ^ examples/1.txt } + { examples/1.txt ^ examples/2.txt }
	         examples/0.txt examples/1.txt examples/2.txt
	00000004             65             65             45 
	0000000d             6e             4e             6e 
	0000000e             6f             4f             6f 
	0000000f             70             50             50 
	00000011             72             72             52 
	00000018             79             79             7a 
	00000019             7a             7a             79 

This command can be used to find only the bytes that differ both between the first and second, and the second and third:

	$ ./bin/binder { examples/0.txt ^ examples/1.txt } . { examples/1.txt ^ examples/2.txt }
	         examples/0.txt examples/1.txt examples/2.txt
	0000000d             6e             4e             6e 
	0000000e             6f             4f             6f 

Operator precedance is currently not supported and brackets around compound expressions are required.
