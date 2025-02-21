#!/bin/bash

case "$1" in
	start)
		echo "starting socket server dameon"
		#native environment
		#start-stop-daemon -S -n aesdsocket --exec "/home/skudlarek/Documents/server/aesdsocket" -- "-d"
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
