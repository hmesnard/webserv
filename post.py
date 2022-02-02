import socket

ip = socket.gethostbyname(socket.gethostname())
port = 8080

mysock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
mysock.connect((ip, port))
fd = open("./website1/404.jpg", "rb")

# data_to_send = 'fkdafjdsa;fdks;fkjasdfdsjkfklsdfksdjkflsjldkflkjdslkflskdfjksdjlkfsjdfjklsdklfjs'.encode()
data_to_send = fd.read()
print(str(len(data_to_send)))
cmd = 'POST /food/cgi_example.php HTTP/1.1\r\nContent-Length: ' + str(len(data_to_send)) + '\r\n\r\n'
mysock.send(cmd.encode())
cmd = data_to_send
mysock.send(cmd)
