#!/bin/bash


OPAT=$(pwd)
echo "path from which script is started is ${OPAT}"
cd "$(dirname $0)"
SCPAT=$(pwd)
echo "path where socket server demon start script is located is ${SCPAT}"
cd ..
cd ..
cd usr
cd bin
EPAT=$(pwd)
echo "path where socket server demon executable is located is ${EPAT}"
cd "${OPAT}"

case "$1" in
	start)
		echo "starting socket server dameon"
		#native environment
		#start-stop-daemon -S -n aesdsocket --exec "${EPAT}/aesdsocket" -- "-d"
		#embedded environment
		start-stop-daemon -S -n aesdsocket --exec "/usr/bin/aesdsocket" -- "-d" 
		;;
	stop)  
		echo "stop socket server dameon"
		start-stop-daemon -K -n aesdsocket
		;;
	*)
		echo "Usage: $0 {start|stop}"
		exit 1
esac
exit 0
