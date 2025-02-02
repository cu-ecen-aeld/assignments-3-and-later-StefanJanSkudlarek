#!/bin/bash

#set up variables
CorrParNum=2
filepath=$1
writestr=$2
backpwd=$( pwd )
#break up information into file name and folder path
folderpath=${filepath%/*}
filename=${filepath##*/}

#check if correct number of input parameters was handed over 
if [ "$#" -ne "$CorrParNum" ]
then
	echo "wrong number of input parameters: $# instead of $CorrParNum, with the first parameter being the path and file to overwrite (or to create anew if required), and the second being the string to write to the file, abort"
	exit 1
#else
#	echo "correct number of input parameters: $#, proceed"
fi

#check how far path and file already exist and are writable

if [ -f "$filepath" ] && [ -w "$filepath" ]
then
	#echo "existing file ok for overwriting"
	echo "$writestr" > "$filepath"
elif [ -f "$filepath" ] && [ ! -w "$filepath" ]
then
	echo "no overwrite permission for existing file, abort"
	exit 1
elif [ -d "$folderpath" ] && [ -w "$folderpath" ]
then
	#echo "write permission existing folder ok, create new file"
	echo "$writestr" > "$filepath"
elif [ -d "$folderpath" ] && [ ! -w "$folderpath" ]
then
	echo "no write permission existing folder, abort creation new file or overwriting existing file"
	exit 1
elif [ ! -d "$folderpath" ]
then
	#echo "folderpath not completely existing yet, create folders level by level, then finally create file"
	#echo "retrieved folder path $folderpath"
	#echo "retrieved file name $filename"
	#echo "file path $folderpath not completely existing yet or partly blocked for writing, check level by level and if possible create missing directories and then create file"
	IFS='/'
	conpath=""
	compathsafe="/"
	for val in $folderpath;
	do
		if [ -n "$val" ]
		then
		 	compath="$compath"/"$val"
		 	#echo "$val"
		 	#echo "$compath"
		 	if [ ! -d "$compath" ]
			then
				#echo "subfolder $val within folder path $compathsafe not existing, try to create new"	
				if [ ! -w "$compathsafe" ]
				then
					echo "no permission to create needed subfolder within folder path, abort"
					exit 1
				fi
				cd "$compathsafe"  
				feedback=$( mkdir "$val" )
				cd "$backpwd"
				if [ ! -d "$compath" ]
				then
					echo "subfolder creation $compath failed despite permission of parent folder, abort"
					exit 1
				fi
			fi
			compathsafe="$compath"	
	 	fi
	done
	#echo "folder path created ok, now create new file"
	echo "$writestr" > "$filepath"
else
	echo "unknown error"
	exit 1
fi

#this section is intended to check whether despite previous checks on file / folder write access, final file modification went wrong
if [ "$?" -ne "0" ]
then	
	echo "error overwriting or creating file despite prior permission check oks, please check system logs"
	exit 1
#else
#	echo "file overwriting or creation completed ok, bye"
#	exit 0
fi




