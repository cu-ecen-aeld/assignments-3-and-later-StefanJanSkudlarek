#!/bin/sh
# Tester script for assignment 1 and assignment 2
# Author: Siddhant Jajoo

set -e
set -u

NUMFILES=10
WRITESTR=AELD_IS_FUN
# changes for ass 4 - 2 
# WRITEDIR=/tmp/aeld-data
# username=$(cat conf/username.txt)
#backup current directory from which script was invoked and get directory where script is located (should be /usr/bin/ within buildroot target directory, root path)
CALLP=$(pwd)
echo "path from which finder-test.sh is called is ${CALLP}"
cd "$(dirname $0)"
SCPAT=$(pwd)
echo "path where finder-test.sh is located is ${SCPAT}"
#cd to root of buildroot folder or QEMU target (move outwards from usr/bin)
cd ..
cd ..
RPAT=$(pwd)
echo "root path is ${RPAT}"
# retrieve username from config file which is located in subfolder etc/finder-app/conf/ of root folder
username=$(cat "${RPAT}/etc/finder-app/conf/username.txt")
#username=$(cat "${RPAT}/conf/username.txt")
echo "retrieved username is ${username}"
#create output file path demanded by assignment 4 - 2 
# "c. Modify your finder-test.sh script to write a file with output of the finder command to /tmp/assignment4-result.txt" 
ASS42_WRITEF="${RPAT}/tmp/assignment4-result.txt"
echo "new output text file ${ASS42_WRITEF}"
#return to call path
WRITEDIR="${RPAT}/tmp/aeld-data/"
cd "${CALLP}"

if [ $# -lt 3 ]
then
	echo "Using default value ${WRITESTR} for string to write"
	if [ $# -lt 1 ]
	then
		echo "Using default value ${NUMFILES} for number of files to write"
	else
		NUMFILES=$1
	fi	
else
	NUMFILES=$1
	WRITESTR=$2
	WRITEDIR="${RPAT}/tmp/aeld-data/${3}"
fi

MATCHSTR="The number of files are ${NUMFILES} and the number of matching lines are ${NUMFILES}"

echo "Writing ${NUMFILES} files containing string ${WRITESTR} to ${WRITEDIR}"

rm -rf "${WRITEDIR}"

# create $WRITEDIR if not assignment1
# change for ass 4 - 2
#assignment=`cat ../conf/assignment.txt`
assignment=$(cat "${RPAT}/etc/finder-app/conf/assignment.txt")
#assignment=$(cat "${RPAT}/conf/assignment.txt")
echo "assignment is ${assignment}"
if [ $assignment != 'assignment1' ]
then
	mkdir -p "$WRITEDIR"

	#The WRITEDIR is in quotes because if the directory path consists of spaces, then variable substitution will consider it as multiple argument.
	#The quotes signify that the entire string in WRITEDIR is a single string.
	#This issue can also be resolved by using double square brackets i.e [[ ]] instead of using quotes.
	if [ -d "$WRITEDIR" ]
	then
		echo "$WRITEDIR created"
	else
		exit 1
	fi
fi
#echo "Removing the old writer utility and compiling as a native application"
#make clean
#make
cd "${SCPAT}"

for i in $( seq 1 $NUMFILES)
do
	echo $(./writer "${WRITEDIR}/${username}$i.txt" "${WRITESTR}")
done
OUTPUTSTRING=$(./finder.sh "${WRITEDIR}" "${WRITESTR}")
echo "${OUTPUTSTRING}" | cat > "${ASS42_WRITEF}"

# remove temporary directories
rm -rf "${RPAT}/tmp/aeld-data"

cd "${CALLP}"
set +e
echo ${OUTPUTSTRING} | grep "${MATCHSTR}"
if [ $? -eq 0 ]; then
	echo "success"
	exit 0
else
	echo "failed: expected  ${MATCHSTR} in ${OUTPUTSTRING} but instead found"
	exit 1
fi
