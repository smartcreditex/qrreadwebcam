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

using namespace cv;
using namespace std;

std::string hostname = "http://localhost:41337";
int cam_id = 0;
/*
* 0 testing
* 1 
* 2 deliver
*/
int procedure = 0;


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

void doDelivered(){
	std::string url = hostname + "/delivered";
	
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

void parseData(std::string data){
	std::cout << data << std::endl;
	
	switch(procedure){
	case 0:{
		stringstream ss(data);
		vector<string> tokens; // Create vector to hold our words
		string buf;
		while (ss >> buf)
			tokens.push_back(buf);

		if(tokens.size() >= 2){
			std::string from = tokens[0];
			std::string to = tokens[1];
			postTransaction(from, to);
		}
	}break;
	case 1:{
		doArrived();
	}break;
	case 2:
		doDelivered();
	break;
	default:
	break;
	};

	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}
			

// source: http://blog.ayoungprogrammer.com/2013/07/tutorial-scanning-barcodes-qr-codes.html/
void printQrCode(cv::Mat image){
	zbar::ImageScanner scanner;
	scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);

	zbar::Image img(image.cols, image.rows, "Y800", (uchar*)image.data, image.cols*image.rows);
	int n = scanner.scan(img);
	for(zbar::Image::SymbolIterator symbol = img.symbol_begin();  
	symbol != img.symbol_end();  
	++symbol) {  
			   vector<Point> vp;  
	// do something useful with results  
		std::string data = symbol->get_data();
		std::cout << data << std::endl;
		parseData(data);
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

    VideoCapture capture(cam_id);

	//Mat image = imread(argv[1]);
	Mat image;

	if(!capture.isOpened()) { cerr << " ERR: Unable find input Video source." << endl;
		return -1;
	}

	//Step	: Capture a frame from Image Input for creating and initializing manipulation variables
	//Info	: Inbuilt functions from OpenCV
	//Note	: 
	
 	capture >> image;
	if(image.empty()){ cerr << "ERR: Unable to query image from capture device.\n" << endl;
		return -1;
	}
	
	int key = 0;
	while(key != 'q')				// While loop to query for Image Input frame
	{	
		
		capture >> image;
		cv::Mat gray;

		cvtColor(image,gray,CV_RGB2GRAY);

        printQrCode(gray);
		key = waitKey(1);

	}	// End of 'while' loop

	return 0;
}
