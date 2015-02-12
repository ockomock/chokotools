/* Copyright (C) Lab group A-8, TDTS04 - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * DISCLAIMER:
 * Infringement against this document will lead to court of law,
 * fine, jail and other various things. Such as, we (Lab group A-8), might
 * send Petty-Poop after you. Who are most likely to sneak up on you from 
 * behind, and pop a cap in yo ass. ("FÖR BÖÖVELENS!!") Watch out!! 
 * As we will not accept your irresponsible and childish manner.
 * 
 * Written by Oscar Fredriksson <oscfr819@student.liu.se> 
 *            Victor Hellström <viche873@student.liu.se>, February 2015
 */

#include <iostream>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>


class Proxy {
public:
	~Proxy();


	/*----- proxy server part -----*/
	
	int servinit(const char *);
	int servrun(void);
	int servhandle(int);
	
	/*-----------------------------*/

	/*----- proxy client part -----*/

	bool clientcon(int &, const std::string &);
	int clientsendrecv(int, const std::string &);

	std::string creategetrequest(const std::string &);

	/*----------------------------*/

	void reterror(const std::string &);
	bool isforbidden(const std::string &);
	std::string getheaderinfo(const std::string &, const std::string &);
	
private:
	int fd;
	int listener;

	/*------------ configurations ------------*/

	const std::string filter[4] = {
		"Norrköping",
		"Britney Spears",
		"Paris Hilton",
		"SpongeBob"
	};

	const std::string error1 = "GET /~TDTS04/labs/2011/ass2/error1.html HTTP/1.1\r\nHost: ida.liu.se\r\nConnention: close\r\n\r\n";	

	const std::string error2 = "GET /~TDTS04/labs/2011/ass2/error2.html HTTP/1.1\r\nHost: ida.liu.se\r\nConnection: close\r\n\r\n";

	/*----------------------------------------*/
};

Proxy::~Proxy()
{
	close(fd);
	close(listener);
}

/* servinit: set up a socket to listen to a port */
int Proxy::servinit(const char *service)
{
	int opt = 1;
	struct addrinfo hints, *res, *rp;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;


	if (getaddrinfo(NULL, service, &hints, &res) != 0)
		return -1;	
	for (rp = res;; rp = rp->ai_next) {
		if (rp == NULL) {
			std::cerr << "unable to host proxy at port: " << service << std::endl;
			freeaddrinfo(res);
			return EXIT_FAILURE;
		}

		if ((listener = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1)
			continue;
		if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt) == -1) {
			close(listener);
			continue;
		}
		if (!bind(listener, rp->ai_addr, rp->ai_addrlen) && !listen(listener, 5))
			break;
		close(listener);
	}
	
	freeaddrinfo(res);

	return 0;
}

/* servrun: listen for incoming connections */
int Proxy::servrun(void)
{
	std::cout << "** Listening...\n";
	while ((fd = accept(listener, 0, 0))) {
		if (fd == -1)
			continue;
		
		switch (fork()) {
		case 0:
			close(listener);
			servhandle(fd);
			close(fd);
			_exit(0); // exit the child process
		case -1:
			std::cerr << "fork() error\n";
			// fall through
		default:
			close(fd);
			continue;
		}
	}

	return 0;
}

/* servhandle: recieve request from URL and check for forbidden content */
int Proxy::servhandle(int fd)
{
	std::string request;
	char buf[8192];
	int n;

	while ((n = recv(fd, buf, sizeof buf-1, 0))) {
		buf[n] = '\0';
		request += buf;
		if (request.find("\r\n\r\n") != std::string::npos)
			break;
	}

	if (n == 0) {
		std::cout << "** Closed connection\n";
		return 1;
	} else if (n == -1) {
		std::cerr << "recv() error\n";
		return 2;
	}
	
	
	if (!isforbidden(request))
		clientsendrecv(fd, request);
	else 
		reterror(error1);
	

	return 0;
}

/* clientcon: client part connects to host given in http headers */
bool Proxy::clientcon(int &ps, const std::string &req)
{
	struct addrinfo hints, *res;
	std::string host = getheaderinfo(req, "Host: ");

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(host.c_str(), "80", &hints, &res) != 0)
		return false;
	if ((ps = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
		return false;
	if (connect(ps, res->ai_addr, res->ai_addrlen) == -1)
		return false;
	
	freeaddrinfo(res);

	return true;
}

/* clientsendrecv: sends a get request to web server and recieves data,
                   sends back the data to the user */
int Proxy::clientsendrecv(int fd, const std::string &req)
{	
	bool check = false;
	int proxy_fd, n = 0;
	char buf[123456];
	std::string result, greq = creategetrequest(req);

	if (!clientcon(proxy_fd, req)) {
		close(proxy_fd);
		return 1;
	}

	std::cout << greq << std::endl;
	send(proxy_fd, greq.c_str(), greq.size(), 0);
	if ((n = recv(proxy_fd, buf, sizeof buf-1, 0)) <= 0) {
		close(proxy_fd);
		return 2;
	}

	result += std::string(buf, buf+n);
	if (result.find("Content-Type: text") != std::string::npos)
		check = true;

	while ((n = recv(proxy_fd, buf, sizeof buf-1, 0)) > 0) {
		result += std::string(buf, buf+n);
		if (check)
			if (isforbidden(result)) {
				close(proxy_fd);
				reterror(error2);
				return -1;
			}	
	}
	
	result += '\0';
	std::cout << result << std::endl;
	if (send(fd, result.c_str(), result.size(), 0) == -1) {
		std::cerr << "send() error\n";
		close(proxy_fd);
		return -1;
	}

	close(proxy_fd);

	return 0;
}

/* creategetrequest: remove hostname from GET header, changes Conection: keep-alive to close */
std::string Proxy::creategetrequest(const std::string &req)
{
	std::stringstream ss(req);
	std::string g, s, fnv, host;

	host = getheaderinfo(req, "Host: ");
	if ((fnv = getheaderinfo(req, host)) == "ghi-error")
		fnv = getheaderinfo(req, "GET ");
	
	g = "GET " + fnv + "\r\n";

	getline(ss, s); // skip GET
	while (getline(ss, s)) {
		if (s.find("Connection: keep-alive") != std::string::npos || s.find("Connection: Keep-Alive") != std::string::npos) {
			g += "Connection: close\r\n";
			continue;
		} else if (s.find("Accept-Encoding: ") != std::string::npos)
			continue; // avoid content encoding error

		g += s + "\n";
	}


	return g;
}

/* reterror: sends a HTTP redirection to provide the web browser with an error page */
void Proxy::reterror(const std::string &error)
{
	int ps, n;
	char buf[8192];
	
	clientcon(ps, "Host: ida.liu.se\r");
	
	send(ps, error.c_str(), error.size(), 0);
	
	if ((n = recv(ps, buf, sizeof buf-1, 0)) <= 0) {
		close(ps);
		return;
	}

	buf[n] = '\0';
	send(fd, buf, strlen(buf), 0);

	close(ps);
}

/* filtertext: check if s contains any forbidden words */
bool Proxy::isforbidden(const std::string &s)
{
	for (size_t i = 0; i < sizeof filter / sizeof filter[0]; i++)
		if (s.find(filter[i]) != std::string::npos)
			return true;

	return false;
}

/* getheaderinfo: extracts header information */
std::string Proxy::getheaderinfo(const std::string &buf, const std::string &f)
{
	size_t i;
	std::string info;
	
	if ((i = buf.find(f)) == std::string::npos)
		return "ghi-error";
 
	for (i += f.size(); buf.at(i) != '\r'; i++)
		info += buf.at(i);
 
	return info;
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		std::cerr << "USAGE:\n" << argv[0] << " proxy-port\n";
		return EXIT_FAILURE;
	}
	
	Proxy p;
	
	if (p.servinit(argv[1]) != 0) {
		std::cerr << "Unable to run setup proxy server\n";
		return EXIT_FAILURE;
	}

	p.servrun();
}
