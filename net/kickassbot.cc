#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#define CHANNEL "##whatever"
#define NICK "lilbthebasedg0d"
#define MSG "THEY TRIED TO HACK MY PROFILE! FUCK THE HACKERS! ALL MY HACKER BROTHERS ONLINE PROTECT ME! AND MY PROFILES!! SHOUT OUT 4CHAN BROS - Lil B"

#define JOIN "JOIN "  CHANNEL  "\r\n"
#define REGNICK "NICK " NICK "\r\n"
#define USER "USER a b c d \r\n"
#define PRIVMSG "PRIVMSG "  CHANNEL  " :" MSG "\r\n"

class Netstream {
public:
	~Netstream();
	
	int dial(const char *, const char *);
	int netsend(const char *);

private:
	int fd;
};

Netstream::~Netstream()
{
	close(fd);
}

int Netstream::dial(const char *node, const char *service)
{
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(node, service, &hints, &res) != 0)
		return -1;

	if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
		return -1;

	if (connect(fd, res->ai_addr, res->ai_addrlen) == -1)
		return -1;

	freeaddrinfo(res);

	int n, count = 0;
	char buf[256];
	
	while (true) {
		
		count++;
		switch (count) {
		case 3:
			netsend(REGNICK);
			netsend(USER);
			break;
		case 4:
			netsend(JOIN);
			break;
		default:
			break;
		}


		n = recv(fd, buf, 255, 0);
		buf[n] = '\0';
		std::string buf2 = buf;
			
		if (n == 0) {
			std::cerr << "closed connection - ";
			return -1;
		} else if (buf2.find("/NAMES") != std::string::npos)
			break;
	}
	
	return 0;
	
}

int Netstream::netsend(const char *msg)
{
	return send(fd, msg, strlen(msg), 0);
}

int main(void)
{
	Netstream bot;

	if (bot.dial("irc.freenode.net", "6667") == -1) {
		std::cerr << "unable to connect\n";
		return EXIT_FAILURE;
	}

	if (bot.netsend(PRIVMSG) == -1) {
		std::cerr << "unable to send\n";
		return EXIT_FAILURE;
	}
	
	return 0;
}
