#include <iostream>
#include <random>

#define NELEMS(a) (sizeof (a) / sizeof ((a)[0]))

using namespace std;

int code[10];

int main(void)
{
  int i = 0;
  char c;
  random_device rnd;

  cout << "\nCurrent keys for this session is:\n";
  for (unsigned int i = 0; i < NELEMS(code); i++) {
    code[i] = rnd() % 10;
    cout << code[i] << (i == 9 ? "\n\n" : ", ");
  }
  
  while (cin.get(c)) {
    cout.put(c ^ code[i]);
    i = (i+1) % (NELEMS(code));
  }
  
}
