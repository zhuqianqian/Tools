WordChain Dict Tools Collection
=====

# Summary 
The collection contains (raw) dictionary data and the scripts/tools built to convert the data to game [Word Chain] and [Word Chain (Chinese Version)].

# File List
A.txt~F.txt: The raw English dictionary file with Chinese definition.

en_US.dic: The English word list (United States) file. It is downloaded from [here].

dictdb.cpp: C source file to convert and merge the words in A.txt~F.txt and en_US.dic into a sqlite3 database.

craw.py: The python script to craw the Chinese definition for the words in sqlite3 database from http://dict.cn

createdir.py: Create folders named from 'a' to 'z', which are used by craw.py

createtable.py: Simply create a sqlite3 database.

bin/*: Binary files (if there are any), including executables, databases

dicttrans.cpp: Parse the files crawed by craw.py and put the Chinese definition to sqlite3 database.