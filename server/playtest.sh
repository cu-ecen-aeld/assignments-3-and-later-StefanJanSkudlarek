# !/bin/bash
# Tester script for sockets using Netcat

#cd to the location of the script and save to stacl
pushd `dirname $0`
target=localhost
port=9000

#function printing general explanation on script parameters and their defaults 
function printusage
{
	echo "Usage: $0 [-t target_ip] [-p port]"
	echo "	Runs a socket test on the aesdsocket application at"
	echo " 	target_ip and port specified by port"
	echo "	target_ip defaults to ${target}" 
	echo "	port defaults to ${port}" 
}

#parsing received options to the script and assigning them to the variables target and port, otherwise explaining via printusage 
while getopts "t:p:" opt; do
	case ${opt} in
		t )
			target=$OPTARG
			;;
		p )
			port=$OPTARG
			;;
		\? )
			echo "Invalid option $OPTARG" 1>&2
			printusage
			exit 1
			;;
		: )
			echo "Invalid option $OPTARG requires an argument" 1>&2
			printusage
			exit 1
			;;
	esac
done

echo "Testing target ${target} on port ${port}"

# Tests to ensure socket send/receive is working properly on an aesdsocket utility
# running on the system
# @param1 : The string to send
# @param2 : The previous compare file
# Returns if the test passes, exits with error if the test fails.
function test_send_socket_string
{
	string=$1
	prev_file=$2
	new_file=$(mktemp)
	expected_file=$(mktemp)

	echo "sending string ${string} to ${target} on port ${port}"
	#string pipelined to nc command which writes it to given destination AND wits for more or a response for 1 second, writing the response to file "new_file", after this connection is closed
	echo ${string} | nc ${target} ${port} -w 1 > ${new_file}
	#string sent is copied to file "expected"
	cp ${prev_file} ${expected_file}
	echo ${string} >> ${expected_file}
	#compare received response to sent string, iff different abort script
	diff ${expected_file} ${new_file} > /dev/null
	if [ $? -ne 0 ]; then
		echo "Differences found after sending ${string} to ${target} on port ${port}"
		echo "Expected contents to match:"
		cat ${expected_file}
		echo "But found contents:"
		cat ${new_file}
		echo "With differences"
		diff -u ${expected_file} ${new_file}
		echo "Test complete with failure"
		exit 1
	else
		cp ${expected_file} ${prev_file}
		rm ${new_file}
		rm ${expected_file}
		echo "test ok"
	fi
}

# Tests to ensure first server setup is ok
# open connection to default server and port and confirm that a certain string is received
function test_rec_socket_string
{
	estring="hello world"
	echo "open connection to ${target} on port ${port} and listen for 20 seconds, expecting to receive string ${estring}"
	new_file=$(mktemp)
	#string pipelined to nc command which writes it to given destination AND wits for more or a response for 1 second, writing the response to file "new_file", after this connection is closed
	nc ${target} ${port} -w 15 > ${new_file}
	#string sent is copied to file "expected"
	echo $(cat "${new_file}")
	rstring="$(cat ${new_file})"
	echo "received string: ${rstring}"
	echo "expected string: ${estring}"
	rm ${new_file}
	if [[ "$rstring" != "$estring" ]]; then
		echo "Differences found"
	else
		echo "expected string received"
	fi
}


#test_rec_socket_string
comparefile=$(mktemp)
test_send_socket_string "abcdefg" ${comparefile}
test_send_socket_string "hijklmnop" ${comparefile}
test_send_socket_string "1234567890" ${comparefile}
test_send_socket_string "9876543210" ${comparefile}
