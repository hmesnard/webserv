import socket
import signal
import os
import time
import sys
import requests

def child():
	signal.sigwait([signal.SIGUSR1])


	ip = "127.0.0.1"
	fd = open("stress_test.txt", "a")
	port = 8080
	url = "http://127.0.0.1:8080/"

	# response = requests.get(url = url)
	# data = response.text()
	# print(data)
	# fd.write(data)
	# fd.close()	

	mysock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	mysock.connect((ip, port))
	cmd = 'GET /upload/lol/ HTTP/1.1\r\n\r\n'.encode()
	mysock.send(cmd)
	data = mysock.recv(1024)
	data = data.decode('ASCII')

	fd.write(data)
	fd.write("\n\n\n\n\n______________________\n\n\n\n\n")
	mysock.close()

	
	
	print("Process number " + str(os.getpid()) + " is exiting now")
	sys.exit()


pids = []
pid_number = 500

for i in range(pid_number):
	try :
		pid = os.fork()
	except OSError as error:
		print(error)
	if (pid == 0):
		child()
	else:
		pids.append(pid)

time.sleep(4)

for i in range(pid_number):
	os.kill(pids[i], signal.SIGUSR1)
	time.sleep(0.01)
sys.exit()