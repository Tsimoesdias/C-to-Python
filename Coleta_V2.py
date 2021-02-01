# -*- coding: utf-8 -*-

"""
Thiago S.  01/02/21
Adaptado do código do Daniel C.: <https://github.com/camposdp/emg-monitor>
Objetivo: receber os dados coletados com o esp32 (freq. de 1000 Hz) 
Pacote com buffer (100 linhas e 4 colunas). Cada coluna representa um sensor 
*** Código apresentando msg de erro do tipo: "Execting','" e "Extra data"
"""

import socket
import json

######################################
#Initial parameters

#HOST = socket.gethostbyname(socket.gethostname())
HOST = '192.168.4.1'  # Manual host enter
PORT = 80  # Port

#Main function
def connected(c):
    
    global msg

    while True:      

        msg = c.recv(4096) #receive data...

        #if not msg: break
        if msg is not None:
            
            data_msg = json.loads(msg)                
            print(data_msg) 
            print("######################")               

######################################
# Main Code
######################################
                
#Create Socket                
sock = socket.socket()
sock.connect((HOST, PORT))
#tcp.bind(orig)
#tcp.listen(1)

while True:
    try:
        connected(sock)
    except Exception as e: print(e)


sock.close()