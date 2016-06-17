// Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.

// This sample illustrates how to use EMG data. EMG streaming is only supported for one Myo at a time.

#include <array>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <time.h>
#include <ctime>
#include <stack>

#include <myo/myo.hpp>
using namespace std;

// Helper function for textual date and time.
// TMSZ must allow extra character for the null terminator.

#define TMFMT "        %H:%M:%S "
#define TMSZ 18
static char *getTm(char *buff) {
	time_t t = time(0);
	strftime(buff, TMSZ, TMFMT, localtime(&t));
	return buff;
}
std::stack<clock_t> tictoc_stack;

class DataCollector : public myo::DeviceListener {
public:
	DataCollector()
		: emgSamples()
	{
		myfile.open("example.txt");//, ios::out | ios::app | ios::ate);
	}

	~DataCollector() {
		myfile.close();
	}

	void tic() {
		tictoc_stack.push(clock());
	}

	void toc() {
		std::cout << "Time elapsed: "
			<< ((double)(clock() - tictoc_stack.top())) / CLOCKS_PER_SEC
			<< std::endl;
		tictoc_stack.pop();
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
		int count = 0;
		char buff[TMSZ];
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
			count++;
        }
		myfile << "Time elapsed: "
			<< ((double)(clock() - tictoc_stack.top())) / CLOCKS_PER_SEC;
		tictoc_stack.pop();
		myfile<<endl;
	
        std::cout << std::flush;
    }

    // The values of this array is set by onEmgData() above.
    std::array<int8_t, 8> emgSamples;
private:
	// We define this function to print the current values that were updated by the on...() functions above.
	ofstream myfile;

};

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

    // Next we construct an instance of our DeviceListener, so that we can register it with the Hub.
    DataCollector collector;
    // Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
    // Hub::run() to send events to all registered device listeners.
    hub.addListener(&collector);


    // Finally we enter our main loop.
    while (1) {
		collector.tic();
        // In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
        // In this case, we wish to update our display 50 times a second, so we run for 1000/20 milliseconds.
        hub.run(1000/100);
        // After processing events, we call the print() member function we defined above to print out the values we've
        // obtained from any events that have occurred.
        collector.print();
		/*ofstream myfile;
		myfile.open("example.txt");
		myfile << "emgSamples";
		myfile.close();*/
    }

    // If a standard exception occurred, we print out its message and exit.
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
        return 1;
    }
}
