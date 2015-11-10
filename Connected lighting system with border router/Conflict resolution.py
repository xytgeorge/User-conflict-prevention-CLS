#! /usr/bin/env python
import sys
from math import exp,sqrt,fabs,log
from socket import *
from socket import error
#------------------------------------------------------------#
# Initialization
#------------------------------------------------------------#
PORT = 5678
BUFSIZE = 1024

HOST, PORT = '127.0.0.1', 60001

TOTAL_SENDER = 18
DEFAULT_OCCUPANCY = 0
DEFAULT_ILLUMINANCE = 250
HEIGHT = 2.5

file_occupancy_info = open("occupancy_info.txt", 'r')
file_illuminance_info = open("illuminance_info.txt", 'r')

occupancy_info = ''
illuminance_info = ''

Ill_ID = 0

T_desk = [25,23,21,25,23,21,25,23,21,25,23,21,25,23,21,25,23,21]
C_desk = [3000,3000,3000,3000,3000,3000,4000,4000,4000,4000,4000,4000,5000,5000,5000,5000,5000,5000]
I_desk = [DEFAULT_ILLUMINANCE] * TOTAL_SENDER
L_desk = [1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0]
axis_x = [0,2.4,4.8,0,2.4,4.8,0,2.4,4.8,0,2.4,4.8,0,2.4,4.8,0,2.4,4.8]
axis_y = [0,0,0,1.8,1.8,1.8,5.4,5.4,5.4,7.2,7.2,7.2,10.8,10.8,10.8,12.6,12.6,12.6]

lagrange = [0] * TOTAL_SENDER
max_lagrange = 0.0
optimal_desk_number = 0

delta_T = 0.0
delta_C = 0.0
delta_I = 0.0
delta_L = 0.0

SITA_T = 3.2683
SITA_C = 1429.88
SITA_I = 0.1823

ID = '000'
T = 19
C = 3000
I = 320
L = 0
AT = 0.01
AC = 0.01
AI = 0.01
AL = 0.01

#------------------------------------------------------------#
# Start a client or server application for testing
#------------------------------------------------------------#
def main():
	if len(sys.argv) < 2:
		usage()
	if sys.argv[1] == '-s':
		server()
	elif sys.argv[1] == '-c':
		client()
	else:
		usage()
#------------------------------------------------------------#
# Prints the instructions
#------------------------------------------------------------#
def usage():
	sys.stdout = sys.stderr
	print 'Usage: udpecho -s [port] (server)'
	print 'or: udpecho -c host [port] <file (client)'
	sys.exit(2)
#------------------------------------------------------------#
# Creates a server, echoes the message back to the client
#------------------------------------------------------------#
def server():
	if len(sys.argv) > 2:
		port = eval(sys.argv[2])
	else:
		port = PORT
	try:
		s = socket(AF_INET6, SOCK_DGRAM)
		s.bind(('aaaa::1', port))
	except Exception:
		print "ERROR: Server Port Binding Failed"
		return
	print 'udp echo server ready: %s' % port
	while 1:
		data, addr = s.recvfrom(BUFSIZE)
		print 'server received', `data`, 'from', `addr`
		s.sendto(control(data), addr)
#------------------------------------------------------------#
# Creates a client that sends an UDP message to a server
#------------------------------------------------------------#
def client():
	if len(sys.argv) < 3:
		usage()
		host = sys.argv[2]
	if len(sys.argv) > 3:
		port = eval(sys.argv[3])
	else:
		port = PORT
	addr = host, port
	s = socket(AF_INET6, SOCK_DGRAM)
	s.bind(('', 0))
	print 'udp echo client ready, reading stdin'
	try:
		s.sendto("hello", addr)
	except error as msg:
		print msg
	data, fromaddr = s.recvfrom(BUFSIZE)
	print 'client received', `data`, 'from', `fromaddr`
#------------------------------------------------------------#
# Tell from room controller or desk controllers
#------------------------------------------------------------#
def control(content):
	global occupancy_info, illuminance_info, max_lagrange, optimal_desk_number
	#open and read files from local	
	file_occupancy_info = open("occupancy_info.txt", 'r')
	file_illuminance_info = open("illuminance_info.txt", 'r')
	occupancy_info = file_occupancy_info.read()
	illuminance_info = file_illuminance_info.read()
	print occupancy_info
	data = '\n'
	if content[content.find("{") + 1] == '"':
		extractAttribute(content)
		print ID,T,C,I,L,AT,AC,AI,AL
		if check() == 1:
			max_lagrange = 0.0
			optimal_desk_number = 0
			lagrangeOptimization()
			if optimal_desk_number < 10:
				data = 'ID:' + ID + ', Result:0' + str(optimal_desk_number) + ', UserSatifaction:' + str(max_lagrange)
			else:
				data = 'ID:' + ID + ', Result:' + str(optimal_desk_number) + ', UserSatifaction:' + str(max_lagrange)
		elif check() == 2:
			data = "Temperature weight is not in the correct range!"
		elif check() == 3:
			data = "Color Temperature weight is not in the correct range!"
		elif check() == 4:
			data = "Illuminance weight is not in the correct range!"
		elif check() == 5:
			data = "Location weight is not in the correct range!"
		elif check() == 6:
			data = "Weights of this profile are not summed as 1 !"
		elif check() == 7:
			data = "All desks are occupied!"
		else:
			data = "Error!"
		print data
	elif content[content.find("#") + 1] == '2':
		if updateInfo() == 1:
			print "Ill_ID: " + str(Ill_ID) + ', Occupancy info:' + occupancy_info[2 * Ill_ID] + ', Illiminance info:' + illuminance_info[4 * Ill_ID] + illuminance_info[4 * Ill_ID + 1] + illuminance_info[4 * Ill_ID + 2]
		elif updateInfo() == 2:
			print "Wrong value of occupancy info"
		elif updateInfo() == 3:
			print "Wrong value of desk illuminance"
		else:
			print "Error!"
	else:
		print "Wrong content from node #1."
	
	file_occupancy_info = open("occupancy_info.txt", 'w')
	file_illuminance_info = open("illuminance_info.txt", 'w')
	file_occupancy_info.write(occupancy_info)
	file_illuminance_info.write(illuminance_info)
	file_occupancy_info.close()
	file_illuminance_info.close()

	return data
#------------------------------------------------------------#
# Parse JSON file, extract attributes and weights values
#------------------------------------------------------------#
def extractAttribute(content):
	global ID,T,C,I,L,AT,AC,AI,AL
	ID = content[content.find("id") + 6 : content.find("id") + 9]
	T = int(content[content.find("T\"") + 5 : content.find("T\"") + 7])
	C = int(content[content.find("C\"") + 5 : content.find("C\"") + 9])
	I = int(content[content.find("I\"") + 5 : content.find("I\"") + 8])
	L = int(content[content.find("L\"") + 5 : content.find("L\"") + 6])
	AT = float(content[content.find("AT\"") + 6 : content.find("AT\"") + 10])
	AC = float(content[content.find("AC\"") + 6 : content.find("AC\"") + 10])
	AI = float(content[content.find("AI\"") + 6 : content.find("AI\"") + 10])
	AL = float(content[content.find("AL\"") + 6 : content.find("AL\"") + 10])
#------------------------------------------------------------#
# Check attributes' availiblity
#------------------------------------------------------------#
def check():
	global ID,T,C,I,L,AT,AC,AI,AL,occupancy_info
	if T < 19:  
		T = 19
	if T > 27:
		T = 27
	if C < 3000:
		C = 3000
	if C > 6500:
		C = 6500
	if I < 320:
		I = 320
	if I > 500:
		I = 500
	if L < 0:
		L = 0
	if L > 1:
		L = 1
	if not(0 <= AT <= 1):
		return 2
	if not(0 <= AC <= 1):
		return 3
	if not(0 <= AI <= 1):
		return 4
	if not(0 <= AL <= 1):
		return 5
	if AT + AC + AI + AL != 1:
		return 6
	sum_temp = 0
	for i in range(18):	
		sum_temp = sum_temp + int(occupancy_info[2*i])
	if sum_temp == 18:
		return 7
	return 1
#------------------------------------------------------------#
# The Lagrange Optimization Calculation for optimal seat
#------------------------------------------------------------#
def lagrangeOptimization():
		global occupancy_info
		for i in range(18):
			if occupancy_info[2 * i]  == '0':
				delta_T = AT * exp(-(T - T_desk[i]) * (T - T_desk[i]) / (2 * SITA_T * SITA_T))
				delta_C = AC * exp(-(C - C_desk[i]) * (C - C_desk[i]) / (2 * SITA_C * SITA_C))
				delta_I = AI * exp(-(log(I) - log(I_desk[i])) * (log(I) - log(I_desk[i])) / (2 * SITA_I * SITA_I))
				delta_L = AL * (1 - fabs(L - L_desk[i]))
				lagrange[i] = delta_T + delta_C + delta_I + delta_L
				print lagrange[i]
				global max_lagrange,optimal_desk_number
				if lagrange[i] > max_lagrange:
					max_lagrange = lagrange[i]
					optimal_desk_number = i
		occupancy_info = occupancy_info[:(2*optimal_desk_number)] + '1' + occupancy_info[(2*optimal_desk_number+1):35]	
		print occupancy_info
		print "Optimal desk number: " + str(optimal_desk_number)
		print "User Satisfaction: " + str(max_lagrange)
#------------------------------------------------------------#
# The distance of a luminaire and a desk
#------------------------------------------------------------#
def distance_square(k,j):
	s = sqrt((axis_x[j] - axis_x[k]) * (axis_x[j] - axis_x[k]) + (axis_y[j] - axis_y[k]) * (axis_y[j] - axis_y[k]))
	d = s * s + HEIGHT * HEIGHT
	return d
#------------------------------------------------------------#
# The cosine degree of a luminaire and a desk
#------------------------------------------------------------#
def cosine(k,j):
	return HEIGHT / sqrt(distance_square(k,j))/4
#------------------------------------------------------------#
# Update occupancy and illuminance info from desks
#------------------------------------------------------------#
def updateInfo():
	global Ill_ID,occupancy_info,illuminance_info
	Ill_ID = int(content[content.find("Ill_ID:") + 7 : content.find("Ill_ID:") + 9])
	if int(content[content.find("O:") + 2 : content.find("O:") + 3]) == 1 or int(content[content.find("O:") + 2 : content.find("O:") + 3]) == 0:
		occupancy_info = occupancy_info[:(2 * Ill_ID)] + content[content.find("O:") + 2 : content.find("O:") + 3] + occupancy_info[(2*Ill_ID+1):35]
	else:
		return 2
	if 100 <= int(content[content.find("I:") + 2 : content.find("I:") + 5]) <= 300:
		illuminance_info = illuminance_info[: 4 * Ill_ID] + content[content.find("I:") + 2 : content.find("I:") + 5] + illuminance_info[(4 * Ill_ID + 3):]
		for i in range(18):
			I_desk[i] =  int(illuminance_info[4 * i]) * 100 + int(illuminance_info[4 * i + 1]) * 10 + int(illuminance_info[4 * i + 2])
			print I_desk[i]
			for j in range(18):	
				I_desk[i] = I_desk[i] + (int(illuminance_info[4 * j]) * 100 + int(illuminance_info[4 * j + 1]) * 10 + int(illuminance_info[4 * j + 2]) ) * cosine(i,j) / distance_square(i,j)
	else:
		return 3
	return 1
#------------------------------------------------------------#
# MAIN APP
#------------------------------------------------------------#
main()

