#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

/*-------------------- configuration --------------------*/

#define CHANNEL "#example"
#define NICK "example-nick"
#define USER "a b c d"
#define MSG "example"

static const char *setup = {

	"NICK " NICK " \r\n"
	"USER " USER " \r\n"
	"JOIN " CHANNEL " \r\n" 
	"PRIVMSG " CHANNEL " :" MSG " \r\n"
};

/*-------------------------------------------------------*/

class TB {
public:

	~TB();

	int con(const char *, const char *);
	void run(void);
	void handle(const std::string &);
	void pong(const std::string &s);
	int netsend(const char *);

private:
	int fd;
};

TB::~TB()
{
	close(fd);
}

int TB::con(const char *node, const char *service)
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

	return 0;
}

void TB::run(void)
{
	int n;
	char buf[2048];
	std::string str;
	
	send(fd, setup, strlen(setup), 0);

	while ((n = recv(fd, buf, sizeof buf -1, 0)) > 0) {
		
		buf[n] = '\0';
		str = buf;

		if (str.find("PING") != std::string::npos)
			pong(str);
		else if (str.find("366") != std::string::npos)
			break;
	}
}


void TB::pong(const std::string &s)
{
	if (s.at(0) != 'P' || s.at(1) != 'I' || s.at(2) != 'N' || s.at(3) != 'G' || s.at(4) != ' ') 
		return;
			
	std::string pong = s.substr(5, s.size()-1);
	netsend(("PONG :" + pong).c_str());
}

int TB::netsend(const char *msg)
{
	return send(fd, msg, strlen(msg), 0);
}

int main(void)
{
	TB b;
	
	if (b.con("irc.freenode.net", "6667") == -1) {
		std::cerr << "unable to connect\n";
		return EXIT_FAILURE;
	}
	
	b.run();


	return 0;
}
