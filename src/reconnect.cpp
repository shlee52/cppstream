#include <iostream>
#include <string>
#include <functional>
#include <cstdlib> //atoi

#include <boost/thread/thread.hpp>

#include "streamclient/streamclient.h"

static void streamMetaInformationCb(hbm::streaming::StreamClient& stream, const std::string& method, const Json::Value& params)
{
	if (method==hbm::streaming::META_METHOD_AVAILABLE) {
		// simply subscibe all signals that become available.
		hbm::streaming::signalReferences_t signalReferences;
		for (Json::ValueConstIterator iter = params.begin(); iter!= params.end(); ++iter) {
			const Json::Value& element = *iter;
			signalReferences.push_back(element.asString());
		}

		try {
			stream.subscribe(signalReferences);
			std::cout << __FUNCTION__ << "the following signal(s) were subscribed: ";
		} catch(const std::runtime_error& e) {
			std::cerr << __FUNCTION__ << "error '" << e.what() << "' subscribing the following signal(s): ";
		}

		for(hbm::streaming::signalReferences_t::const_iterator iter=signalReferences.begin(); iter!=signalReferences.end(); ++iter) {
			std::cout << "'" << *iter << "' ";
		}
		std::cout << std::endl;
	}
}


int main(int argc, char* argv[])
{
	boost::chrono::milliseconds cycleTime(3000);

	if((argc<2) || (std::string(argv[1])=="-h") ) {
		std::cout << "periodically connects to a daq stream, subscribes all signals and disconnects after a specified time" << std::endl;
		std::cout << "syntax: " << argv[0] << " <stream server address> <control port (default is \"http\")> <cycle time in ms (default is " << cycleTime.count() << ")>" << std::endl;
		return EXIT_SUCCESS;
	}


	static const std::string address = argv[1];

	// the control port might differ when communication runs via a router (CX27)
	std::string controlPort = "http";
	if(argc>2) {
		controlPort = argv[2];
	}



	if(argc>3) {
		cycleTime = boost::chrono::milliseconds(atoi(argv[3]));
	}

	hbm::streaming::StreamClient stream;
	stream.setStreamMetaCb(streamMetaInformationCb);
	do {
		boost::thread streamer = boost::thread(std::bind(&hbm::streaming::StreamClient::start, &stream, address, hbm::streaming::DAQSTREAM_PORT, controlPort));
		std::cout << "Started" << std::endl;
		boost::this_thread::sleep_for(cycleTime);
		stream.stop();
		streamer.join();
	} while(true);
}
