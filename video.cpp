//______________________________________________________________________________________
// Program : OpenCV based QR code Detection and Retrieval
// Author  : Bharath Prabhuswamy
//______________________________________________________________________________________

#include <iostream>
#include <cmath>
#include <string>
#include <chrono>
#include <thread>

#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat, Scalar)
#include <opencv2/imgproc/imgproc.hpp>  // Gaussian Blur
#include <opencv2/highgui/highgui.hpp>  // OpenCV window I/O
#include <zbar.h>
#include <Poco/URI.h>
#include <Poco/Net/DNS.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/StreamCopier.h>

#ifdef WITH_GROVEPI
#include <grovepi.h>
#include <grove_rgb_lcd.h>
using namespace GrovePi;
LCD lcd;
#endif

using namespace cv;
using namespace std;

std::string hostname = "http://localhost:41337";
int cam_id = 0;
/*
* 0 both
* 1 arrive
* 2 deliver
*/
int procedure = 0;

std::string stdLcdText = "SmartCredit";

VideoCapture capture[2];
Mat image[2];

#ifdef WITH_GROVEPI
int buzzer_pin = 8;
int green_led_pin = 3;
int red_led_pin = 2;
#endif


void makeSound(){
	#ifdef WITH_GROVEPI
	digitalWrite(buzzer_pin, HIGH);
	delay(50);
	digitalWrite(buzzer_pin, LOW);
	#endif
}

void signalArrived(){
	#ifdef WITH_GROVEPI
	lcd.setRGB(255, 0, 0);
	lcd.setText("Goods Received!");
	digitalWrite(green_led_pin, HIGH);
	delay(500);
	digitalWrite(green_led_pin, LOW);
	delay(500);
	digitalWrite(green_led_pin, HIGH);
	delay(500);
	digitalWrite(green_led_pin, LOW);
	delay(500);
	digitalWrite(green_led_pin, HIGH);
	delay(500);
	digitalWrite(green_led_pin, LOW);
	lcd.setRGB(200, 200, 200);
	lcd.setText(stdLcdText.c_str());
	#endif
}

void signalDelivered(){
	#ifdef WITH_GROVEPI
	lcd.setRGB(0, 0, 255);
	lcd.setText("Goods Issued!");
	digitalWrite(red_led_pin, HIGH);
	delay(500);
	digitalWrite(red_led_pin, LOW);
	delay(500);
	digitalWrite(red_led_pin, HIGH);
	delay(500);
	digitalWrite(red_led_pin, LOW);
	delay(500);
	digitalWrite(red_led_pin, HIGH);
	delay(500);
	digitalWrite(red_led_pin, LOW);
	lcd.setRGB(200, 200, 200);
	lcd.setText(stdLcdText.c_str());
	#endif
}


void postTransaction(std::string from, std::string to){


	//cout << from << " " << to << endl;
	std::string url = hostname + "/transaction/"+from+"/"+to;
	
	Poco::URI uri(url);
    Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET,
                               uri.getPathAndQuery());
    session.sendRequest(req);
    Poco::Net::HTTPResponse res;
	std::istream &iStr = session.receiveResponse(res);
	std::string outStr;
	Poco::StreamCopier::copyToString(iStr, outStr);
	std::cout << outStr << std::endl;

}

void doArrived(){
	std::string url = hostname + "/arrived";
	try{
	Poco::URI uri(url);
    Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET,
                               uri.getPathAndQuery());
    session.sendRequest(req);
    Poco::Net::HTTPResponse res;
	std::istream &iStr = session.receiveResponse(res);
	std::string outStr;
	Poco::StreamCopier::copyToString(iStr, outStr);
	std::cout << outStr << std::endl;
	}catch(std::exception &e){
	
	}
	signalArrived();
}

void doDelivered(){
	std::string url = hostname + "/delivered";
	try{
		Poco::URI uri(url);
		Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
		Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET,
								uri.getPathAndQuery());
		session.sendRequest(req);
		Poco::Net::HTTPResponse res;
		std::istream &iStr = session.receiveResponse(res);
		std::string outStr;
		Poco::StreamCopier::copyToString(iStr, outStr);
		std::cout << outStr << std::endl;
	}catch(std::exception &e){

	}
	signalDelivered();
}

void postCount(){
	std::string url = hostname + "/count";
	
	Poco::URI uri(url);
    Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET,
                               uri.getPathAndQuery());
    session.sendRequest(req);
    Poco::Net::HTTPResponse res;
	std::istream &iStr = session.receiveResponse(res);
	std::string outStr;
	Poco::StreamCopier::copyToString(iStr, outStr);
	std::cout << outStr << std::endl;
}

void parseData(std::string data, int cam){

	makeSound();
	if(data.compare("IN") == 0){
		doArrived();
	}else if(data.compare("OUT") == 0){
		doDelivered();
	}else{
		switch(cam){
			case 0:{
				doArrived();
			}break;
			case 1:
				doDelivered();
			break;
			default:
			break;
		};
	}
	
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}
			

// source: http://blog.ayoungprogrammer.com/2013/07/tutorial-scanning-barcodes-qr-codes.html/
void printQrCode(cv::Mat image, int cam){
	zbar::ImageScanner scanner;
	scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);

	zbar::Image img(image.cols, image.rows, "Y800", (uchar*)image.data, image.cols*image.rows);
	int n = scanner.scan(img);
	for(zbar::Image::SymbolIterator symbol = img.symbol_begin();  
	symbol != img.symbol_end();  
	++symbol) { 
		std::string data = symbol->get_data();
		std::cout << data << std::endl;
		parseData(data, cam);
	}
}

void doCapture(int cam){
	while(true){
		capture[cam] >> image[cam];
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void doScanCode(int cam){
	while(true){
		cv::Mat gray;
		cvtColor(image[cam], gray, CV_RGB2GRAY);
		printQrCode(gray, cam);
	}
}

// Start of Main Loop
//------------------------------------------------------------------------------------------------------------------------
int main ( int argc, char **argv )
{

	if(argc > 1){
		hostname = std::string(argv[1]);
		if(argc > 2){
			cam_id = std::stoi(argv[2]);
			if(argc > 3){
				procedure = std::stoi(argv[3]);
			}
		}

	} else {
		cout << "usage: program hostname cameraid procedureid\n eg: ./program http://localhost:41337 0 0" << endl;
	}

	cout << "hostname: " << hostname << endl;
	cout << "cameraid: " << cam_id << endl;
	cout << "procedure: " << procedure << endl;

	#ifdef WITH_GROVEPI
	initGrovePi(); // initialize communication with the GrovePi
	pinMode(buzzer_pin, OUTPUT);
	lcd.connect();
	lcd.setText(stdLcdText.c_str());
	lcd.setRGB(200, 200, 200);
	#endif

	if(procedure == 1){
		capture[0].open(0);
	}else if(procedure == 2){
		capture[1].open(1);
	}else{
		capture[0].open(1);
		capture[1].open(0);
	}

	if(!capture[0].isOpened() && !capture[1].isOpened()) { cerr << " ERR: Unable find input Video source." << endl;
	    return -1;
	}
	
	if(procedure == 1){
		std::thread t1(doCapture,0);
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));
		std::thread t2(doScanCode,0);
		t1.join(); 
		t2.join(); 
	} else if(procedure == 1){
		std::thread t1(doCapture,1);
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));
		std::thread t2(doScanCode,1);
		t1.join(); 
		t2.join(); 
	}else{
		std::thread t1(doCapture,0);
		std::thread t2(doCapture,1);
	
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));
		std::thread t3(doScanCode,0);
		std::thread t4(doScanCode,1);
	
		t1.join(); 
		t2.join();
		t3.join(); 
		t4.join();

	}

	return 0;
}
