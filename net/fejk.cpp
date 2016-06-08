#include <iostream>
#include <cstring>
#include <cstdio>

using namespace std;

static const struct Search {
	const char *search;
	int times;
} table[] = {
	{ "Namn:", 2 },
	{ "Gata:", 2 },
	{ "Postort:", 2 },
	{ "Telefon:", 2 },
	{ "Mail:", 3 },
	{ "Ålder:", 2 },
	{ "Personnummer", 3 },
	{ "IP-adress:", 3 },
	{ "PIN-kod:", 2 },
	{ "Lösenord:", 2 },
	{ "Skostorlek:", 2 },
	{ NULL, 0 }
};

class Fejk {
 public:

	~Fejk();

	void generate(void);
	int fejkContent(void);
	void parseInfo(void);
	string extractInfo(const char *, int);

 private:
	FILE *fin;
	string fContent;
};

Fejk::~Fejk()
{
	pclose(fin);
}

void Fejk::generate(void)
{
	if (fejkContent() != 0) {
		cerr << "Kunde inte generera uppgifter\n";
		return;
	}	

	parseInfo();

}

int Fejk::fejkContent()
{
	char buf[4096];
		
	fin = popen("curl fejk.se -L", "r");
	while (fgets(buf, sizeof buf, fin) != NULL)
		fContent += buf;


	return 0;
}

string Fejk::extractInfo(const char *search, int times)
{
	int n = 0;
	size_t si;
	string s, se = search;

	if ((si = fContent.find(se)) == string::npos)
		return "error";

	si += se.size();
	while (1) {
		if (fContent.at(si) == '>') {
			n++;
			si++;
			continue;
		} else if (fContent.at(si) == '<' && n >= times)
			break;
		if (n == times)	
			s += fContent.at(si);

		si++;
	}


	return s;
}

void Fejk::parseInfo(void)
{
	const struct Search *p;

	cout << endl;
	for (p = table; p->search != NULL; p++) 
		cout << p->search << " " 
		     << extractInfo(p->search, p->times) << endl;	
	
}

int main(void)
{
	Fejk f;
	
	f.generate();
}
