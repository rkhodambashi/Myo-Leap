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

using namespace Leap;
using namespace std;

// Helper function for textual date and time.
// TMSZ must allow extra character for the null terminator.

/*
#define TMFMT "        %H:%M:%S "
#define TMSZ 18
static char *getTm(char *buff) {
	time_t t = time(0);
	strftime(buff, TMSZ, TMFMT, localtime(&t));
	return buff;
}
*/
std::stack<clock_t> tictoc_stack;
ofstream myfile;

class DataCollector : public myo::DeviceListener {
	friend class SampleListener;
public:
	DataCollector()
		: emgSamples()
	{
		myfile.open("MyoLeap.txt");//, ios::out | ios::app | ios::ate);
	}

	~DataCollector() {
		myfile.close();
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
			myfile << emgString << ' ';
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
	virtual double mapping(double x, double x1, double x2);
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

double SampleListener::mapping(double x, double x1, double x2)
{
	double y;
	y = (180 / (x2 - x1))*(x - x1);
	return y;
}

int main(int argc, char** argv)
{
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
		const double minMax[10] = { 32, 85, 36, 100, 37, 107, 36, 100, 36, 90 }; //T.I.M.R.P

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
			// In this case, we wish to update our display 50 times a second, so we run for 1000/20 milliseconds.
			hub.run(1000 / 100);
			// After processing events, we call the print() member function we defined above to print out the values we've
			// obtained from any events that have occurred.
			collector.print();
			int i = 0;
			int j = 1;
			const Frame frame = controller.frame();
			HandList hands = frame.hands();
			for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
				// Get the first hand
				const Hand hand = *hl;
				// Get fingers
				const FingerList fingers = hand.fingers();
				
				for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
					const Finger finger = *fl;

					myfile << std::string(4, ' ') << fingerNames[finger.type()]
						<< ": " << hand.palmPosition().distanceTo(finger.tipPosition());
					myfile << std::string(4, ' ') << fingerNames[finger.type()]
						<< ": " << listener.mapping(hand.palmPosition().distanceTo(finger.tipPosition()), minMax[i + i], minMax[i + j]);
					i++;
					j++;
						if (i == 5 && j == 6)
						{
							i = 0;
							j = 1;
						}
				}
			}
			timeElasped = timeElasped + ((double)(clock() - tictoc_stack.top())) / CLOCKS_PER_SEC;
			/*myfile << " Time elapsed: "
				<< ((double)(clock() - tictoc_stack.top())) / CLOCKS_PER_SEC;*/
			tictoc_stack.pop();
			myfile << " Time elasped: " << timeElasped << endl;

		}

		// If a standard exception occurred, we print out its message and exit.
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::cerr << "Press enter to continue.";
		std::cin.ignore();
		return 1;
	}
}