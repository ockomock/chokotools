#include <iostream>
#include <string>
#include <random>
#include <map>

using namespace std;

map<int, int> hashmonth = {
	{1, 31},
	{2, 28},
	{3, 31},
	{4, 30},
	{5, 31},
	{6, 30},
	{7, 31},
	{8, 31},
	{9, 30},
	{10, 31},
	{11, 30},
	{12, 31}
};

bool luhn_valid(string s)
{
	int sum = 0, last = ((int)s.back() - '0');
 
	for(int i = 0; i < s.size() - 1; ++i) {
		int t = ((int)s.at(i) - '0') * (i % 2 == 0 ? 2 : 1);
		sum += t <= 9 ? t : t - 9;
	}

	return 10 - (sum % 10) == last || 10 - (sum % 10) == 10 ? true : false;
}

void gen_digits(string &s)
{
	s = "";
	int year, month, days, d;
	random_device rnd;
	map<int, int>::iterator it;
	
	year = rnd() % (99-80) + 80;
	s += to_string(year);

	month = rnd() % (13-1) + 1;
	if (month < 10)
		s += '0' + to_string(month);
	else
		s += to_string(month);


	it = hashmonth.find(month);
	days = rnd() % (it->second-1) + 1;
	if (days < 10)
		s += '0' + to_string(days);
	else
		s += to_string(days);

	d = rnd() % (9999-1000) + 1000;
	s += '-' + to_string(d);
}

int main(void)
{
	string s;
	
	do 
		gen_digits(s);
   	while (!luhn_valid(s));
	
	cout << s << endl;
}
