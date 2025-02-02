#!/bin/bash

#set up variables for input parameters
CorrParNum=2
filesdir=$1
searchstr=$2
#set up variables for output values
#number of files containing matches and total number of lines containing one or more matches across all files 
filenum=0
clinenum=0

#check if correct number of input parameters was handed over 
if [ "$#" -ne "$CorrParNum" ]
then
	echo "wrong number of input parameters: $# instead of $CorrParNum, with the first parameter being the folder to search, and the second being the search string, abort"
	exit 1
#else
#	echo "correct number of input parameters: $#, proceed"
fi

#check if valid directory information was handed over
if [ ! -d "$filesdir" ]
then
	echo "directory $filesdir not found, abort"
	exit 1	
	echo "validity directory $filesdir confirmed, proceed"
#else
#	echo "validity directory $filesdir confirmed, proceed"
fi

#search for string in all files in all subdirectories of target directory and save output in variables

#get list of files containing the string with directory and subdirectories
output=$(grep -rl  "$searchstr" "$filesdir")
#echo "List of files that contain search string:" 
#echo "$output"

#loop through matching files and sum up matching lines over all files.
if [ -n "$output" ]
then
	for val in $output;
	do
 		((filenum=filenum+1))
 		delta=$(grep -c "$searchstr" "$val")
 		((clinenum=clinenum+delta))
 		#echo "$val file number $filenum with $delta lines containing the string"
	done
fi

#output result and exit 0
echo "The number of files are $filenum and the number of matching lines are $clinenum"
#exit 0




