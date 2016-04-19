#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

using namespace std;

#define NELEMS(a) (sizeof (a) / (sizeof (a[0])))

static const struct languages {
	const string lang;
	const string ending;	
} table[] = {
	{ "C", "c" },
	{ "C%2B%2B", "cc" },
	{ "C%2B%2B", "cpp" },
	{ "D", "d" },
	{ "Haskell", "hs" },
	{ "Haskell", "lhs" },
	{ "Lua", "lua" },
	{ "OCaml", "ml" },
	{ "PHP", "php" },
	{ "Perl", "pl" },
	{ "Plain+Text", "txt" },
	{ "Python", "py" },
	{ "Ruby", "rb" },
	{ "Scheme", "scm" },
	{ "Tcl", "tcl" } 
};

class Codeinepad {
 public:
 
	~Codeinepad();

	void to_lower(string &);
	string to_hex(char);

	void set_flags(const char *);
	void get_data(const char *);
	void determine_lang(const string &);
	void get_content(const string &);
	
	void set_codeinepad_lang(const string &);

	int concodeinepad();
	int postdata();
	string get_link();

 private:
	int fd;
	int conlen;
	string lang; 
	string data;
	string priv = "False";
	string run  = "False";
};

Codeinepad::~Codeinepad()
{
	close(fd);
}

void Codeinepad::to_lower(string &s)
{
	for (size_t i = 0; i < s.size(); i++)
		s.at(i) = tolower(s.at(i));
}

string Codeinepad::to_hex(char c)
{
	stringstream ss;
	ss << '%' << hex << int(c);
	return ss.str();
}

void Codeinepad::set_flags(const char *s)
{
	string str = s;
	if (str.at(0) != '-' || str.size() > 3) {
		cerr << "invalid flags\n";
		return;
	}
	for (size_t i = 1; i < str.size(); i++) {
		if (str.at(i) == 'p')
			priv = "True";
		if (str.at(i) == 'r')
			run = "True";
	}	
}

void Codeinepad::get_data(const char *s)
{
	string fn = s;

	determine_lang(fn);
	get_content(fn);	
}

void Codeinepad::determine_lang(const string &s)
{
	string d;
	for (size_t i = s.size()-1; i > 0; i--)
		if (s.at(i) == '.')
			break;
		else
			d = s.at(i) + d;

	Codeinepad::to_lower(d);
	set_codeinepad_lang(d);
}

void Codeinepad::get_content(const string &file)
{
	char c;
	ifstream f(file);

	while (f.get(c))
		if (isspace(c))
			data += c;
		else
			data += to_hex(c);


	f.close();
}

void Codeinepad::set_codeinepad_lang(const string &s)
{
	size_t i = 0;
	const struct languages *tp = table;
	
	while (tp->ending != s) {
		if (++i == NELEMS(table))
			break;

		tp++;
	}	

	lang = (i == NELEMS(table) ? "Plain+Text" : tp->lang);
}

int Codeinepad::concodeinepad()
{
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo("codepad.org", "80", &hints, &res) != 0)
		return -1;
	if ((fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
		return -1;
	if (connect(fd, res->ai_addr, res->ai_addrlen) == -1)
		return -1;

	freeaddrinfo(res);
	return 0;
}

int Codeinepad::postdata()
{
	string flip = "lang=" + lang + "&code=" + data + "&private=" + priv + "&run=" + run + "&submit=Submit";

	conlen = flip.size();
	string request = "POST / HTTP/1.1\r\nHost: codepad.org\r\nConnection: close\r\nContent-Type: application/x-www-form-urlencoded\r\n" 
			 "Content-Length: " + to_string(conlen) + "\r\n\r\n" + flip;

	if (send(fd, request.c_str(), request.size(), 0) == -1)
		return -1;


	return 0;
}

string Codeinepad::get_link()
{
	int n;
	char buf[4096];
	while ((n = recv(fd, buf, sizeof buf -1, 0)) > 0) 
		buf[n] = '\0';

	size_t i;
	string link, s = buf, search = "Location: ";

	i = s.find(search);
	for (i += search.size(); s.at(i) != '\r'; i++)
		link += s.at(i);
	
	return link;
}

int main(int argc, char *argv[])
{
	if (argc < 2 ) {
		cerr << "USAGE: " << argv[0] << " file [-pr] (-p: private, -r: run)\n";
		return EXIT_FAILURE;
	}


	Codeinepad c;
	c.get_data(argv[1]);
	if (argc == 3)
		c.set_flags(argv[2]);
	if (c.concodeinepad() == -1) {
		cerr << "Unable to connect to codepad.org\n";
		return EXIT_FAILURE;
	}
	if (c.postdata() != 0) {
		cerr << "Unable to post content\n";
		return EXIT_FAILURE;
	}

	cout << c.get_link() << endl;
}
