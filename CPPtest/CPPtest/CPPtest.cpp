#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <winsock.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <math.h>
#include <stddef.h>
#include "strdef.h"

#include "char_defs.h"
#include "char_defs.c"

#include <thread> // Required for std::this_thread::sleep_for
#include <chrono> // Required for time units like std::chrono::seconds
#include <fstream>   // For file input/output (std::ifstream)
#include <sstream>   // For string stream operations (std::istringstream)
#include <string>    // For string manipulation
#include <vector>    // To store parsed data (e.g., std::vector<std::vector<std::string>>)

#define NO_FLAGS_SET 0
#define MAXBUFLEN 512

//units are mm for all below
#define MAX_STEP_SIZE 1
#define CANVAS_X 100
#define CANVAS_Y 100

#define FILE_PATH_SCALE_X 2
#define FILE_PATH_SCALE_Y 2
#define FILE_PATH_SCALE_Z 10

#define GLOBAL_PATH_SCALE_X 1
#define GLOBAL_PATH_SCALE_Y 1
#define GLOBAL_PATH_SCALE_Z 1

//milliseconds
#define PATH_STEP_DELAY 150

POSE tgtPose;

typedef std::vector<POSE> path;
using namespace std;

path loadPathFromFile(string);
path interpolate(POSE, POSE, float);
path generateSpiral(int);
path eraseBoard(POSE, POSE);
path combinePaths(path, path);
path createTextPath(string);
path createCharPath(char, float);
void printPath(path);

INT main(VOID)
{

	cout << "starting program..." << endl;

	WSADATA Data;
	SOCKADDR_IN destSockAddr;
	SOCKET destSocket;
	unsigned long destAddr;
	int status;
	int numsnt;
	int numrcv;
	char sendText[MAXBUFLEN];
	char recvText[MAXBUFLEN];
	char dst_ip_address[MAXBUFLEN];
	unsigned short port;
	char msg[MAXBUFLEN];
	char buf[MAXBUFLEN];
	char type, type_mon[4];
	unsigned short IOSendType = 0; // Send input/output signal data designation
	unsigned short IORecvType = 0; // Reply input/output signal data designation
	unsigned short IOBitTop = 0;
	unsigned short IOBitMask = 0xffff;
	unsigned short IOBitData = 0;

	float next_z = 0.0;

	int count = 0;

	//attempt to load file to draw:

	path currentPath = createTextPath("!");//generateSpiral(1000);//loadPathFromFile("C:\\Users\\Owner\\Desktop\\CPPtest\\CPPtest\\raylist.csv");
	printPath(currentPath);


	port = 10000;
	strcpy(dst_ip_address, "192.168.0.20");
	
	sprintf(msg, "IP=%s / PORT=%d", dst_ip_address, port);
	std::cout << msg << endl;
	std::cout << "[Enter]= End / [d]= Monitor data display";
	std::cout << "[z/x]= Increment/decrement first command data transmitted by the delta amount. ";
	std::cout << " Is it all right? [Enter] / [Ctrl+C] ";
	cin.getline(msg, MAXBUFLEN);
	// Windows Socket DLL initialization
	status = WSAStartup(MAKEWORD(1, 1), &Data);
	if (status != 0)
		cerr << "ERROR: WSAStartup unsuccessful" << endl;
	// IP address, port, etc., setting
	memset(&destSockAddr, 0, sizeof(destSockAddr));
	destAddr = inet_addr(dst_ip_address);
	memcpy(&destSockAddr.sin_addr, &destAddr, sizeof(destAddr));
	destSockAddr.sin_port = htons(port);
	destSockAddr.sin_family = AF_INET;
	// Socket creation
	destSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (destSocket == INVALID_SOCKET) {
		cerr << "ERROR: socket unsuccessful" << endl;
		status = WSACleanup();
		if (status == SOCKET_ERROR)
			cerr << "ERROR: WSACleanup unsuccessful" << endl;
		return(1);
	}

	MXTCMD MXTsend;
	MXTCMD MXTrecv;
	JOINT jnt_now;
	POSE pos_now;
	PULSE pls_now;

	tgtPose = { 0 };
	int pathIdx = 0;

	unsigned long counter = 0;
	int loop = 1;
	int disp = 0;
	int disp_data = 0;
	int ch;
	float delta = (float)0.0;
	int retry;
	fd_set SockSet; // Socket group used with select
	timeval sTimeOut; // For timeout setting
	memset(&MXTsend, 0, sizeof(MXTsend));
	memset(&jnt_now, 0, sizeof(JOINT));
	memset(&pos_now, 0, sizeof(POSE));
	memset(&pls_now, 0, sizeof(PULSE));
	while (loop) {
		memset(&MXTsend, 0, sizeof(MXTsend));
		memset(&MXTrecv, 0, sizeof(MXTrecv));
		// Transmission data creation
		if (loop == 1) { // Only first time
			MXTsend.Command = MXT_CMD_NULL;
			MXTsend.SendType = MXT_TYP_NULL;
			MXTsend.RecvType = 1;
			MXTsend.SendIOType = MXT_IO_NULL;
			MXTsend.RecvIOType = IOSendType;
			MXTsend.CCount = counter = 0;

			tgtPose = MXTrecv.dat.pos;
			next_z = MXTrecv.dat.pos.w.z;

		}
		else { // Second and following times
			MXTsend.Command = MXT_CMD_MOVE;
			MXTsend.SendType = 1;
			MXTsend.RecvType = 1;
			MXTsend.RecvType1 = 1;
			MXTsend.RecvType2 = 1;
			MXTsend.RecvType3 = 1;
			
			memcpy(&MXTsend.dat.pos, &pos_now, sizeof(POSE));

			MXTsend.dat.pos.w.x += (float)(tgtPose.w.x);
			MXTsend.dat.pos.w.y += (float)(tgtPose.w.y);
			MXTsend.dat.pos.w.z += (float)(tgtPose.w.z);
			/*MXTsend.dat.pos.w.a += (float)(tgtPose.w.a * ratio);
			MXTsend.dat.pos.w.b += (float)(tgtPose.w.b * ratio);*/
			//MXTsend.dat.pos.w.c += (float)(tgtPose.w.c * ratio);

			MXTsend.SendIOType = IOSendType;
			MXTsend.RecvIOType = IORecvType;
			MXTsend.BitTop = IOBitTop;
			MXTsend.BitMask = IOBitMask;
			MXTsend.IoData = IOBitData;
			MXTsend.CCount = counter;
		}

		tgtPose.w.x = currentPath[pathIdx].w.x * GLOBAL_PATH_SCALE_X;
		tgtPose.w.y = currentPath[pathIdx].w.y * GLOBAL_PATH_SCALE_Y;
		tgtPose.w.z = currentPath[pathIdx].w.z * GLOBAL_PATH_SCALE_Z;

		std::cout << "tgtPose.x is currently " << tgtPose.w.x << endl;
		std::cout << "tgtPose.y is currently " << tgtPose.w.y << endl;
		std::cout << "tgtPose.z is currently " << tgtPose.w.z << endl;
		std::cout << "path index is currently " << pathIdx << "\n" << endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(PATH_STEP_DELAY));

		pathIdx++;
		if (pathIdx > 6456) {
				return 0;
		}

		// Keyboard input
		// [Enter]=End / [d]= Display the monitor data, or none / [0/1/2/3]= Change of monitor data display
		// [z/x]=Increment/decrement first command data transmitted by the delta amount
		while (_kbhit() != 0) {
			ch = _getch();
			switch (ch) {
			case 0x0d:
				MXTsend.Command = MXT_CMD_END;
				loop = 0;
				break;
			case 'p':
				disp = ~disp;
				break;

			case 'w':
				tgtPose.w.x += 0.1;
				break;

			case 'a':
				tgtPose.w.y += 0.1;
				break;

			case 's':
				tgtPose.w.x -= 0.1;
				break;

			case 'd':
				tgtPose.w.y -= 0.1;
				break;
			}
		}

		memset(sendText, 0, MAXBUFLEN);
		memcpy(sendText, &MXTsend, sizeof(MXTsend));
		if (disp) {
			sprintf(buf, "Send (%ld):", counter);
			std::cout << buf << endl;
		}
		numsnt = sendto(destSocket, sendText, sizeof(MXTCMD), NO_FLAGS_SET, (LPSOCKADDR)&destSockAddr,
			sizeof(destSockAddr));
		if (numsnt != sizeof(MXTCMD)) {
			cerr << "ERROR: sendto unsuccessful" << endl;
			status = closesocket(destSocket);
			if (status == SOCKET_ERROR)
				cerr << "ERROR: closesocket unsuccessful" << endl;
			status = WSACleanup();
			if (status == SOCKET_ERROR)
				cerr << "ERROR: WSACleanup unsuccessful" << endl;
			return(1);
		}
		memset(recvText, 0, MAXBUFLEN);
		retry = 1; // No. of reception retries
		while (retry) {
			FD_ZERO(&SockSet); // SockSet initialization
			FD_SET(destSocket, &SockSet); // Socket registration
			sTimeOut.tv_sec = 1; // Transmission timeout setting (sec)
			sTimeOut.tv_usec = 0; // (micro sec)
			status = select(0, &SockSet, (fd_set*)NULL, (fd_set*)NULL, &sTimeOut);
			if (status == SOCKET_ERROR) {
				return(1);
			}
			if ((status > 0) && (FD_ISSET(destSocket, &SockSet) != 0)) { // If it receives by the time-out
				numrcv = recvfrom(destSocket, recvText, MAXBUFLEN, NO_FLAGS_SET, NULL, NULL);
				if (numrcv == SOCKET_ERROR) {
					cerr << "ERROR: recvfrom unsuccessful" << endl;
					status = closesocket(destSocket);
					if (status == SOCKET_ERROR)
						cerr << "ERROR: closesocket unsuccessful" << endl;
					status = WSACleanup();
					if (status == SOCKET_ERROR)
						cerr << "ERROR: WSACleanup unsuccessful" << endl;
					return(1);
				}
				memcpy(&MXTrecv, recvText, sizeof(MXTrecv));
				char str[10];
				if (MXTrecv.SendIOType == MXT_IO_IN) sprintf(str, "IN%04x", MXTrecv.IoData);
				else if (MXTrecv.SendIOType == MXT_IO_OUT) sprintf(str, "OT%04x", MXTrecv.IoData);
				else sprintf(str, "------");
				int DispType;
				void* DispData;
				switch (disp_data) {
				case 0:
					DispType = MXTrecv.RecvType;
					DispData = &MXTrecv.dat;
					break;
				case 1:
					DispType = MXTrecv.RecvType1;
					DispData = &MXTrecv.dat1;
					break;
				case 2:
					DispType = MXTrecv.RecvType2;
					DispData = &MXTrecv.dat2;
					break;
				case 3:
					DispType = MXTrecv.RecvType3;
					DispData = &MXTrecv.dat3;
					break;
				default:
					break;
				}
				switch (DispType) {
				
				case MXT_TYP_POSE:
				case MXT_TYP_FPOSE:
				case MXT_TYP_FB_POSE:
					if (loop == 1) {
						memcpy(&pos_now, &MXTrecv.dat.pos, sizeof(POSE));
						loop = 2;
					}
					if (disp) {
						POSE* p = (POSE*)DispData;
						sprintf(buf, "Receive (%ld): TCount=%dType(POSE) = % d¥n % 7.2f, % 7.2f, % 7.2f, % 7.2f, % 7.2f, % 7.2f, % 04x, % 04x(% s)"
							, MXTrecv.CCount, MXTrecv.TCount, DispType
							, p->w.x, p->w.y, p->w.z, p->w.a, p->w.b, p->w.c, p->sflg1, p->sflg2, str);
						std::cout << buf << endl;
					}
					break;
				
					break;
				case MXT_TYP_NULL:
					if (loop == 1) {
						loop = 2;
					}
					if (disp) {
						sprintf(buf, "Receive (%ld): TCount=%d Type(NULL)=%d¥n (%s)"
							, MXTrecv.CCount, MXTrecv.TCount, DispType, str);
						std::cout << buf << endl;
					}
					break;
				default:
					std::cout << "Bad data type.¥n" << endl;
					break;
				}
				counter++; // Count up only when communication is successful
				retry = 0; // Leave reception loop
			}
			else { // Reception timeout
				std::cout << "... Receive Timeout! <Push [Enter] to stop the program>" << endl;
				retry--; // No. of retries subtraction
				if (retry == 0) loop = 0; // End program if No. of retries is 0
			}
		} /* while(retry) */
} /* while(loop) */
// End
std::cout << "program ended" << endl;
sprintf(buf, "counter = %ld", counter);
std::cout << buf << endl;
// Close socket
status = closesocket(destSocket);
if (status == SOCKET_ERROR)
cerr << "ERROR: closesocket unsuccessful" << endl;
status = WSACleanup();
if (status == SOCKET_ERROR)
cerr << "ERROR: WSACleanup unsuccessful" << endl;
return 0;
}

path interpolate(POSE start, POSE end, float stepSize) {

	float length = sqrt(pow((end.w.x - start.w.y), 2) + pow((end.w.y - start.w.y), 2) + pow((end.w.z - start.w.z), 2));
	int stepCount = ceil(length / stepSize);

	path pathToReturn;
	POSE tempPose = { 0 };

	for (int i = 0; i < stepCount; i++) {
		tempPose.w.x = start.w.x + ((float)(i / stepCount) * end.w.x);
		tempPose.w.y = start.w.y + ((float)(i / stepCount) * end.w.y);
		tempPose.w.z = start.w.z + ((float)(i / stepCount) * end.w.z);
		tempPose.w.a = start.w.a + ((float)(i / stepCount) * end.w.a);
		tempPose.w.b = start.w.b + ((float)(i / stepCount) * end.w.b);
		tempPose.w.c = start.w.c + ((float)(i / stepCount) * end.w.c);
	}
	return pathToReturn;
}

path loadPathFromFile(string filename) {

	//attempt to load the file
	ifstream inputFile(filename);
	if (!inputFile.is_open()) {
		std::cerr << "Error opening file!" << std::endl;
	}
	else {
		std::cout << "file: " << filename << " opened successfully" << endl;
	}

	std::string line;
	std::vector<std::vector<float>> data;

	while (std::getline(inputFile, line)) {
		std::stringstream ss(line);
		std::string cell;
		std::vector<float> rowData;

		while (std::getline(ss, cell, ',')) { // Use ',' as the delimiter
			try {
				rowData.push_back(std::stof(cell)); // Convert string to float
			}
			catch (const std::invalid_argument& e) {
				std::cerr << "Invalid argument: " << cell << " - " << e.what() << std::endl;
			}
			catch (const std::out_of_range& e) {
				std::cerr << "Out of range: " << cell << " - " << e.what() << std::endl;
				// Handle error (hopefully never happens)
			}
		}
		data.push_back(rowData);
	}
	inputFile.close();

	//convert the data loaded from the file into a path type
	path pathToReturn;
	POSE tempPose = { 0 };
	int pathSteps = data.size();
	pathSteps = 403;


	int counter = 0;
	while (counter < pathSteps) {
		tempPose.w.x = data[0][counter] * FILE_PATH_SCALE_X;
		tempPose.w.y = data[1][counter] * FILE_PATH_SCALE_Y;
		tempPose.w.z = data[2][counter] * FILE_PATH_SCALE_Z;

		pathToReturn.push_back(tempPose);
		counter++;
	}

	cout << "path with " << pathSteps << " points generated successfully" << endl;

	//do the thing jim
	return pathToReturn;
}

path eraseBoard(POSE startPoint, POSE stopPoint){

	path erasePath;
	POSE tempPose = { 0 };



	/*for (int y = 0; y < boar; y++) {
		for (int x = 0; x < boardSizeX; x++) {
			erasePath.push_back(tempPose);
		}
	}*/

	return erasePath;
}

path generateSpiral(int steps){

	POSE tempPose = { 0 };
	path pathToReturn;

	for (int t = 0; t < steps; t++) {
		tempPose.w.x = 0.5 * t * cos(t);
		tempPose.w.y = 0;
		tempPose.w.z = 0.5 * t * sin(t);
		pathToReturn.push_back(tempPose);
	}
	return pathToReturn;
}

path createTextPath(string input) {
	path textPath;
	path currentChar;

	int count = 0;
	while (count < input.length()) {
		currentChar = createCharPath(input[count], 1);
		textPath = combinePaths(textPath, currentChar);
		count++;
	}
	return textPath;
}

path createCharPath(char input, float scale){
	//stupid prevention
	cout << "generating charPath for character " << (char)input << endl;
	if (input > 126 && input < 32) {
		input = 32;
	}
	//shift down to array index
	input = input - 32;

	int numPoints = simplexchars[input][0];

	path charPath;
	POSE tempPose;
	POSE lastPose;

	int isPenUp = 1;

	//the first two characters are the number of points and spacing of the letter respectively
	for (int i = 2; i < (numPoints * 2) + 2; i+=2) {

		if (simplexchars[input][i] == -1 && simplexchars[input][i+1] == -1) {
			////goto position with pen down
			//tempPose = lastPose;
			//tempPose.w.y = 0;
			//charPath.push_back(tempPose);
			//lift pen
			tempPose = lastPose;
			tempPose.w.y = 1;
			charPath.push_back(tempPose);

			isPenUp = 1;

		}
		else {
			if (isPenUp) {
				//goto position with pen up
				tempPose.w.x = simplexchars[input][i];
				tempPose.w.z = simplexchars[input][i + 1];
				tempPose.w.y = 1;
				charPath.push_back(tempPose);
			}

			//then drop pen
			tempPose.w.x = simplexchars[input][i];
			tempPose.w.z = simplexchars[input][i + 1];
			tempPose.w.y = 0;
			charPath.push_back(tempPose);

			isPenUp = 0;
			lastPose = tempPose;
		}
	}
	return charPath;
}

path combinePaths(path first, path second) {
	path combined;

	for (int i = 0; i < size(first); i++) {
		combined.push_back(first[i]);
	}
	for (int i = 0; i < size(second); i++) {
		combined.push_back(second[i]);
	}

	return combined;
}

void printPath(path input) {
	int count = 0;
	cout << "printing path of size " << input.size() << endl;
	while (count < input.size()){
		
		cout << "path[" << count << "].w.x = " << input[count].w.x << endl;
		cout << "path[" << count << "].w.y = " << input[count].w.y << endl;
		cout << "path[" << count << "].w.z = " << input[count].w.z << endl;
		cout << endl;
		count++;
	}
}







