#include <iostream>
#include <fstream>
#include <cstdlib>

int main(int argc, char *argv[])
{
  using namespace std;

  int r = EXIT_SUCCESS;

  if ( argc == 1) {
    
    char c;
    while (cin.get(c))
      cout.put(c);

  } else {

    for (int i = 1; i < argc; i++) {

      ifstream f(argv[i]);
      if (!f) {
	cerr << "cat: " << argv[i] << ": unable to open file\n";
        r = EXIT_FAILURE;
	continue;
      }

      char c;
      while (f.get(c))
	cout.put(c);

      f.close();
    }

  }

  return r;
}
