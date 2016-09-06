// Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.

// This sample illustrates how to use EMG data. EMG streaming is only supported for one Myo at a time.

/******************************************************************************\
* Copyright (C) 2012-2014 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#include <array>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <time.h>
#include <ctime>
#include <stack>
#include <math.h>
#include "Leap.h"
#include <myo/myo.hpp>
#include <winsock2.h>
#include <stdio.h>
#include <strsafe.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <conio.h>
#include "dynamixel.h"
//#include "FingerLib.h"
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "dynamixel.lib")


using namespace Leap;
using namespace std;

// UDP set up========================
#define SERVER "127.0.0.1" //ip address of udp server
//#define SERVER "128.61.126.213"  //ip address of udp server

#define BUFLEN 512  //Max length of buffer
#define PORT 8888  //The port on which to listen for incoming data
void PrintCommStatus(int CommStatus);
void PrintErrorCode();
struct sockaddr_in si_other;
int s, slen = sizeof(si_other);
char buf[BUFLEN];
char message[BUFLEN];
WSADATA wsa;
//===================================

// Dynamixel=========================
//Control table address
#define P_GOAL_POSITION_L		30
#define P_VELOCITY_L			32
#define P_GOAL_POSITION_H		31
#define P_PRESENT_POSITION_L	36
#define P_PRESENT_POSITION_H	37
#define P_MOVING				46

int P_GOAL_POSITION_Thumb;
int P_GOAL_POSITION_Index;
int P_GOAL_POSITION_Middle;
int P_GOAL_POSITION_Ring;
int P_GOAL_POSITION_Pinky;
// Defulat setting
#define DEFAULT_PORTNUM		6 // COM6
#define DEFAULT_BAUDNUM		7 // 250000Kbps
#define THUMB_MOTOR_ID			5
#define INDEX_MOTOR_ID			4
#define MIDDLE_MOTOR_ID			3
#define RING_MOTOR_ID			2
#define PINKY_MOTOR_ID			1
//===================================





///// <summary>
///// Entry point for the application
///// </summary>
///// <param name="hInstance">handle to the application instance</param>
///// <param name="hPrevInstance">always 0</param>
///// <param name="lpCmdLine">command line arguments</param>
///// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
///// <returns>status</returns>
//int APIENTRY wWinMain(
//	_In_ HINSTANCE hInstance,
//	_In_opt_ HINSTANCE hPrevInstance,
//	_In_ LPWSTR lpCmdLine,
//	_In_ int nShowCmd
//)
//{
	////Initialise winsock
	//printf("\nInitialising Winsock...");
	//if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	//{
	//	printf("Failed. Error Code : %d", WSAGetLastError());
	//	exit(EXIT_FAILURE);
	//}
	//printf("Initialised.\n");

	////create socket
	////if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	//if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	//{
	//	printf("socket() failed with error code : %d", WSAGetLastError());
	//	exit(EXIT_FAILURE);
	//}

	////setup address structure
	//memset((char *)&si_other, 0, sizeof(si_other));
	//si_other.sin_family = AF_INET;
	//si_other.sin_port = htons(PORT);
	////si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);
	//si_other.sin_addr.S_un.S_addr = inet_pton(AF_INET, SERVER, &(si_other.sin_addr));

	//closesocket(s);
	//WSACleanup();
//}
/*
// Helper function for textual date and time.
// TMSZ must allow extra character for the null terminator.

#define TMFMT "        %H:%M:%S "
#define TMSZ 18
static char *getTm(char *buff) {
	time_t t = time(0);
	strftime(buff, TMSZ, TMFMT, localtime(&t));
	return buff;
} 
*/

std::stack<clock_t> tictoc_stack;   // adding clock_t to the stack
ofstream myfile;
ofstream myfile2;

class DataCollector : public myo::DeviceListener {
	friend class SampleListener;
public:
	DataCollector()
		: emgSamples()
	{
		myfile.open("LeapData.txt");//, ios::out | ios::app | ios::ate);
		myfile2.open("MyoData.txt");
	}

	~DataCollector() {
		myfile.close();
		myfile2.close();
	}

	void tic() {
		tictoc_stack.push(clock());
	}

	// onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user.
	void onUnpair(myo::Myo* myo, uint64_t timestamp)
	{
		// We've lost a Myo.
		// Let's clean up some leftover state.
		emgSamples.fill(0);
	}

	// onEmgData() is called whenever a paired Myo has provided new EMG data, and EMG streaming is enabled.
	void onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg)
	{
		for (int i = 0; i < 8; i++) {
			emgSamples[i] = emg[i];
		}
	}

	// There are other virtual functions in DeviceListener that we could override here, like onAccelerometerData().
	// For this example, the functions overridden above are sufficient.


	void print()
	{
		//int count = 0;
		//char buff[TMSZ];
		// Clear the current line
		std::cout << '\r';

		// Print out the EMG data.
		for (size_t i = 0; i < emgSamples.size(); i++) {
			std::ostringstream oss;
			oss << static_cast<int>(emgSamples[i]);
			std::string emgString = oss.str();


			std::cout << '[' << emgString << std::string(4 - emgString.size(), ' ') << ']';

			//myfile<<endl;
			//myfile << '[' << emgString << std::string(4 - emgString.size(), ' ') << ']'; 
			myfile2 << emgString << ' ';
			//count++;
		}
		/*myfile << "Time elapsed: "
			<< ((double)(clock() - tictoc_stack.top())) / CLOCKS_PER_SEC;
		tictoc_stack.pop();
		myfile << endl;*/

		std::cout << std::flush;
	}

	// The values of this array is set by onEmgData() above.
	std::array<int8_t, 8> emgSamples;
private:
	// We define this function to print the current values that were updated by the on...() functions above.
};

class SampleListener : public Listener {
public:
	virtual void onInit(const Controller&);
	virtual void onConnect(const Controller&);
	virtual void onDisconnect(const Controller&);
	virtual void onExit(const Controller&);
	virtual void onFrame(const Controller&);
	virtual void onFocusGained(const Controller&);
	virtual void onFocusLost(const Controller&);
	virtual void onDeviceChange(const Controller&);
	virtual void onServiceConnect(const Controller&);
	virtual void onServiceDisconnect(const Controller&);
	virtual double mapping(double x, double x1, double x2, double y1, double y2);
	/*SampleListener()
	{
		myfile.open("LeapData.txt");
	}
	~SampleListener() {
		myfile.close();
	}*/

private:
};

const std::string fingerNames[] = { "T", "I", "M", "R", "P" };
const std::string boneNames[] = { "Metacarpal", "Proximal", "Middle", "Distal" };
const std::string stateNames[] = { "STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END" };

void SampleListener::onInit(const Controller& controller) {
	std::cout << "Initialized" << std::endl;
}

void SampleListener::onConnect(const Controller& controller) {
	std::cout << "Connected" << std::endl;
	controller.enableGesture(Gesture::TYPE_CIRCLE);
	controller.enableGesture(Gesture::TYPE_KEY_TAP);
	controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
	controller.enableGesture(Gesture::TYPE_SWIPE);
}

void SampleListener::onDisconnect(const Controller& controller) {
	// Note: not dispatched when running in a debugger.
	std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
	std::cout << "Exited" << std::endl;
}

void SampleListener::onFrame(const Controller& controller) {
	//tictoc_stack.push(clock());
	// Get the most recent frame and report some basic information
	const Frame frame = controller.frame();

	HandList hands = frame.hands();
	for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
		// Get the first hand
		const Hand hand = *hl;
		std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
		
		const Vector normal = hand.palmNormal();
		const Vector direction = hand.direction();

		// Get the Arm bone
		Arm arm = hand.arm();

		// Get fingers
		const FingerList fingers = hand.fingers();
		for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
			const Finger finger = *fl;
	
			myfile << std::string(4, ' ') << fingerNames[finger.type()]
				<< ": " << hand.palmPosition().distanceTo(finger.tipPosition());

			// Get finger bones
			for (int b = 0; b < 4; ++b) {
				Bone::Type boneType = static_cast<Bone::Type>(b);
				Bone bone = finger.bone(boneType);
			}
		}
	}

	// Get tools
	const ToolList tools = frame.tools();
	for (ToolList::const_iterator tl = tools.begin(); tl != tools.end(); ++tl) {
		const Tool tool = *tl;
	}

	// Get gestures
	const GestureList gestures = frame.gestures();
	for (int g = 0; g < gestures.count(); ++g) {
		Gesture gesture = gestures[g];

		switch (gesture.type()) {
		case Gesture::TYPE_CIRCLE:
		{
			CircleGesture circle = gesture;
			std::string clockwiseness;

			if (circle.pointable().direction().angleTo(circle.normal()) <= PI / 2) {
				clockwiseness = "clockwise";
			}
			else {
				clockwiseness = "counterclockwise";
			}

			// Calculate angle swept since last frame
			float sweptAngle = 0;
			if (circle.state() != Gesture::STATE_START) {
				CircleGesture previousUpdate = CircleGesture(controller.frame(1).gesture(circle.id()));
				sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * PI;
			}
			break;
		}
		case Gesture::TYPE_SWIPE:
		{
			SwipeGesture swipe = gesture;
			break;
		}
		case Gesture::TYPE_KEY_TAP:
		{
			KeyTapGesture tap = gesture;
			break;
		}
		case Gesture::TYPE_SCREEN_TAP:
		{
			ScreenTapGesture screentap = gesture;
			break;
		}
		default:
			break;
		}
	}

	if (!frame.hands().isEmpty() || !gestures.isEmpty()) {
		std::cout << std::endl;
	}
	myfile << " Time elapsed: "
		<< ((double)(clock() - tictoc_stack.top())) / CLOCKS_PER_SEC;
	tictoc_stack.pop();
	myfile << endl;

}

void SampleListener::onFocusGained(const Controller& controller) {
	std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Controller& controller) {
	std::cout << "Focus Lost" << std::endl;
}

void SampleListener::onDeviceChange(const Controller& controller) {
	std::cout << "Device Changed" << std::endl;
	const DeviceList devices = controller.devices();

	for (int i = 0; i < devices.count(); ++i) {
		std::cout << "id: " << devices[i].toString() << std::endl;
		std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
	}
}

void SampleListener::onServiceConnect(const Controller& controller) {
	std::cout << "Service Connected" << std::endl;
}

void SampleListener::onServiceDisconnect(const Controller& controller) {
	std::cout << "Service Disconnected" << std::endl;
}

double SampleListener::mapping(double x, double x1, double x2, double y1, double y2)
{
	int y;
	y = ((y2 - y1 )/ (x2 - x1))*(x - x1) + y1;
	return y;
}

int main(int argc, char** argv)
{
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other);
	char buf[BUFLEN];
	char message[BUFLEN];
	WSADATA wsa;

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//create socket
	//if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	char str[INET_ADDRSTRLEN];
	//setup address structure
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	//si_other.sin_addr.S_un.S_addr = inet_pton(AF_INET, SERVER, &(si_other.sin_addr));
	inet_pton(AF_INET, SERVER, &(si_other.sin_addr.S_un.S_addr));
	/*if (inet_pton(AF_INET, SERVER, &(si_other.sin_addr.S_un.S_addr)))
	{
		inet_ntop(AF_INET, &(si_other.sin_addr.S_un.S_addr), str, INET_ADDRSTRLEN);
		std::cout << ("%s\n", str);
	}*/

	int GoalPos[2] = { 0, 1023 };
	//int GoalPos[2] = {0, 4095}; // for EX serise
	int index = 0;
	int Moving, PresentPos;
	int CommStatus;
	//===============================================
	// Open device MOTORS. Use these lines if directly controlling the dynamixels through serial port and usbtodunamixel. If sending data with UDP, leave these lines commented
	//if (dxl_initialize(DEFAULT_PORTNUM, DEFAULT_BAUDNUM) == 0)
	//{
	//	printf("Failed to open USB2Dynamixel!\n");
	//	printf("Press any key to terminate...\n");
	//	//getch();
	//	return 0;
	//}
	//else
	//	printf("Succeed to open USB2Dynamixel!\n");
	//================================================


	// We catch any exceptions that might occur below -- see the catch statement for more details.
	try {

		// First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
		// publishing your application. The Hub provides access to one or more Myos.
		myo::Hub hub("om.example.emg-data-sample");

		std::cout << "Attempting to find a Myo..." << std::endl;

		// Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
		// immediately.
		// waitForMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
		// if that fails, the function will return a null pointer.
		myo::Myo* myo = hub.waitForMyo(10000);

		// If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
		if (!myo) {
			throw std::runtime_error("Unable to find a Myo!");
		}

		// We've found a Myo.
		std::cout << "Connected to a Myo armband!" << std::endl << std::endl;

		// Next we enable EMG streaming on the found Myo.
		myo->setStreamEmg(myo::Myo::streamEmgEnabled);

		// Create a sample listener and controller for Leap Motion
		SampleListener listener;
		Controller controller;

		// Next we construct an instance of our DeviceListener, so that we can register it with the Hub.
		DataCollector collector;
		double timeElasped = 0.000;
		const double minMax[10] = { 50, 95, 60, 112, 60, 100, 70, 105, 55, 100 }; // Min and Max values for each finger T.I.M.R.P 
		
		//This is for the Dynamixels
		const int minMaxMotors[10] = { 1100, 1400, 500, 1100, 1100, 700, 1100, 350, 700, 1200 };
		
		//This is for the OpenBionics Hand
		//const int minMaxMotors[10] = { 110, 975, 110, 975, 110, 975, 110, 975, 110, 975 };
		
		// Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
		// Hub::run() to send events to all registered device listeners.
		hub.addListener(&collector);

		//controller.addListener(listener);

		if (argc > 1 && strcmp(argv[1], "--bg") == 0)
			controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);
		
		
		// Finally we enter our main loop.
		while (1) {
			collector.tic();
			// In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
			// In this case, we wish to update our display 20 times a second, so we run for 1000/20 milliseconds.
			hub.run(1000 / 60);
			// After processing events, we call the print() member function we defined above to print out the values we've
			// obtained from any events that have occurred.
			collector.print();
			int i = 0;
			int j = 1;
			int h = 0;
			double fingDis[5];
			const Frame frame = controller.frame();
			HandList hands = frame.hands();
			for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
				// Get the first hand
				const Hand hand = *hl;
				// Get fingers
				const FingerList fingers = hand.fingers();
				
				for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
					const Finger finger = *fl;

					myfile << hand.palmPosition().distanceTo(finger.tipPosition()) << " ";
					/*myfile << std::string(4, ' ') << fingerNames[finger.type()]
						<< ": " << listener.mapping(hand.palmPosition().distanceTo(finger.tipPosition()), minMax[i + i], minMax[i + j]);*/
					
					//print mapped values
					//fingDis[h] = listener.mapping(hand.palmPosition().distanceTo(finger.tipPosition()), minMax[i + i], minMax[i + j], minMaxMotors[i + i], minMaxMotors[i + j]);
					
					//Print raw values
					fingDis[h] = hand.palmPosition().distanceTo(finger.tipPosition());
					
					i++;
					j++;
					h++;
						if (i == 5 && j == 6 && h == 5)
						{
							
							/*dxl_write_word(THUMB_MOTOR_ID, P_GOAL_POSITION_L, fingDis[0]);
							dxl_write_word(THUMB_MOTOR_ID, P_VELOCITY_L, 100);
							dxl_write_word(INDEX_MOTOR_ID, P_GOAL_POSITION_L, fingDis[1]);
							dxl_write_word(INDEX_MOTOR_ID, P_VELOCITY_L, 100);
							dxl_write_word(MIDDLE_MOTOR_ID, P_GOAL_POSITION_L, fingDis[2]);
							dxl_write_word(MIDDLE_MOTOR_ID, P_VELOCITY_L, 100);
							dxl_write_word(RING_MOTOR_ID, P_GOAL_POSITION_L, fingDis[3]);
							dxl_write_word(RING_MOTOR_ID, P_VELOCITY_L, 100);
							dxl_write_word(PINKY_MOTOR_ID, P_GOAL_POSITION_L, fingDis[4]);
							dxl_write_word(PINKY_MOTOR_ID, P_VELOCITY_L, 100);				*/
							

							//send over UDP
							string tmp = to_string(fingDis[0]) + " "+ to_string(fingDis[1]) + " " + to_string(fingDis[2]) + " " + to_string(fingDis[3]) + " " + to_string(fingDis[4]);
							strcpy_s(message, tmp.c_str());
							//send message
							if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
							{
								printf("sendto() failed with error code : %d", WSAGetLastError());
								exit(EXIT_FAILURE);
							}
							std::cout << "Data Sent";
							i = 0;
							j = 1;
							h = 0;
						}
				}
			}

			timeElasped = timeElasped + ((double)(clock() - tictoc_stack.top())) / CLOCKS_PER_SEC;
			/*myfile << " Time elapsed: "
				<< ((double)(clock() - tictoc_stack.top())) / CLOCKS_PER_SEC;*/
			tictoc_stack.pop();
			myfile << timeElasped << endl;
			myfile2 << timeElasped << endl;

		}
		
		// If a standard exception occurred, we print out its message and exit.
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::cerr << "Press enter to continue.";
		std::cin.ignore();
		return 1;
	}
	closesocket(s);
	WSACleanup();

	// Close device
	dxl_terminate();
	printf("Press any key to terminate...\n");
	//getch();
	return 0;
}

// Print communication result
void PrintCommStatus(int CommStatus)
{
	switch (CommStatus)
	{
	case COMM_TXFAIL:
		printf("COMM_TXFAIL: Failed transmit instruction packet!\n");
		break;

	case COMM_TXERROR:
		printf("COMM_TXERROR: Incorrect instruction packet!\n");
		break;

	case COMM_RXFAIL:
		printf("COMM_RXFAIL: Failed get status packet from device!\n");
		break;

	case COMM_RXWAITING:
		printf("COMM_RXWAITING: Now recieving status packet!\n");
		break;

	case COMM_RXTIMEOUT:
		printf("COMM_RXTIMEOUT: There is no status packet!\n");
		break;

	case COMM_RXCORRUPT:
		printf("COMM_RXCORRUPT: Incorrect status packet!\n");
		break;

	default:
		printf("This is unknown error code!\n");
		break;
	}
}

// Print error bit of status packet
void PrintErrorCode()
{
	if (dxl_get_rxpacket_error(ERRBIT_VOLTAGE) == 1)
		printf("Input voltage error!\n");

	if (dxl_get_rxpacket_error(ERRBIT_ANGLE) == 1)
		printf("Angle limit error!\n");

	if (dxl_get_rxpacket_error(ERRBIT_OVERHEAT) == 1)
		printf("Overheat error!\n");

	if (dxl_get_rxpacket_error(ERRBIT_RANGE) == 1)
		printf("Out of range error!\n");

	if (dxl_get_rxpacket_error(ERRBIT_CHECKSUM) == 1)
		printf("Checksum error!\n");

	if (dxl_get_rxpacket_error(ERRBIT_OVERLOAD) == 1)
		printf("Overload error!\n");

	if (dxl_get_rxpacket_error(ERRBIT_INSTRUCTION) == 1)
		printf("Instruction code error!\n");
}
