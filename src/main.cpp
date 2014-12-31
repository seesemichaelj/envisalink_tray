#include <iostream>
#include <stdlib.h>
#include <boost/array.hpp>
#include <boost/asio.hpp>

using namespace std;
using boost::asio::ip::tcp;

#define IP_ADDRESS "127.0.0.1"
#define PORT "1337"
#define PASSWORD "user"

void read_command(tcp::socket &socket, boost::array<char, 128> &buf, size_t &len) {
	boost::system::error_code error;

	len = socket.read_some(boost::asio::buffer(buf), error);
	if (error == boost::asio::error::eof) {
		cout << "Error [read_command]" << endl << "Error: Connection closed by peer" << endl;
		buf.empty();
		return;
	}
	else if (error) {
		//throw boost::system::system_error(error); // Some other error.
		cout << "Error [read_command]" << endl << "Error: " << error.value() << endl;;
		buf.empty();
		len = 0;
		return;
	}

	if(buf[len-2] != '\r' || buf[len-1] != '\n') {
		cout << "Error [read_command]" << endl << "Error: No CR and LF" << endl;
		cout << buf[len-2] << buf[len-1] << endl;
		buf.empty();
		len = 0;
		return;
	}
}

string calculate_checksum(string in) {
	unsigned int sum = 0;
	char buf[2];

	for(int i = 0; i < in.length(); i++) {
		sum += in.at(i);
	}

	itoa(sum & 0xFF, buf, 16);
	string out(buf);
	return out;
}

int main(int argc, char* argv[]) {
	cout << "Initializing..." << endl;

	boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(IP_ADDRESS, PORT);
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::resolver::iterator end;
	tcp::socket socket(io_service);
	boost::system::error_code error = boost::asio::error::host_not_found;

	cout << "Finished Initializing." << endl;

	cout << "Connecting..." << endl;
	char key;
	while(error && endpoint_iterator != end) {
		socket.close();
		socket.connect(*endpoint_iterator++, error);
	}
	if(error) {
		//throw boost::system::system_error(error);
		cout << "Error 1" << endl << "Error: " << error.value();
		cin >> key;
		return -1;
	}

	cout << "Connected." << endl << "Reading data from buffer..." << endl;

	boost::array<char, 128> buf;
	size_t len;

	read_command(socket, buf, len);
	cout << "RX:" << endl;
	std::cout.write(buf.data(), len);

	boost::system::error_code ignored_error;
	string message;
	string code = "005";
	string data = PASSWORD;
	message.append(code);
	message.append(data);
	message.append(calculate_checksum(message));
	cout << "Message prepared to write out: " << message.c_str() << endl;
	message.append("\r\n");
	boost::asio::write(socket, boost::asio::buffer(message), boost::asio::transfer_all(), ignored_error);


	read_command(socket, buf, len);
	cout << "RX:" << endl;
	std::cout.write(buf.data(), len);
	
	system("pause");

	return 0;
}
