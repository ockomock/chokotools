#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
    return (&(((struct sockaddr_in*)sa)->sin_addr));
  
  return (&(((struct sockaddr_in6*)sa)->sin6_addr));
}

int main(int argc, char *argv[])
{
	int listener, fd;
	char buf[256];
	struct addrinfo hints, *res, *rp;
	struct sockaddr_storage their_addr;
	socklen_t sin_size = sizeof their_addr;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (argc != 2) {
		std::cerr << "USAGE: " << argv[0] << " port\n";
		return EXIT_FAILURE;
	}

	if (getaddrinfo(NULL, argv[1], &hints, &res) != 0) {
		std::cerr << "getaddrinfo error\n";
		return EXIT_FAILURE;
	}
	for (rp = res; ; rp = rp->ai_next) {
		if (rp == NULL) {
			freeaddrinfo(res);
			return EXIT_FAILURE;
		}
		
		if ((listener = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1)
			continue;
		if (!bind(listener, rp->ai_addr, rp->ai_addrlen) && !listen(listener, 10))
			break;
		close(listener);
	}

	freeaddrinfo(res);
	
	while ((fd = accept(listener, (struct sockaddr *)&their_addr, &sin_size)) < 0)
		;

	std::cout << inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), buf, sizeof buf) << std::endl;
	
	close(fd);
	close(listener);
}
