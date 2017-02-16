/*
 * argv[1] path file to a cap file
 * argv[2] path to output directory
 * breaks up the cap file by symbol
 * eliminates packet date except at begining of file
 * eliminates message symbols and places a ~ in place where symbol should be
 */
#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>



using namespace std;

int main(int argc, char* argv[])
{
  if (argc != 3) {
    cerr << "please enter CAP file path and output directory" << endl;
    return 1;
  } else {
    cout << "File path: " << argv[1] << endl;
    cout << "Output path: " << argv[2] << endl;
    cout << "Countinue (y/n): ";
    char a;
    cin >> a;
    if (a != 'y') {
      cout << "Bye!" << endl;
      return 0;
    }
  }
  ifstream fs(argv[1]);
  if (!fs.is_open()) {
    cerr << "File not found! Bye!" << endl;
    return 1;
  }

  char c;
    int asc;
  bool message_mode = false;
  vector<char> header;
  string header_str;
  bool write_packet = true;
  bool symbol_discovered = false;
  vector<char> message;
  string message_str;
  vector<char> symbol;
  string symbol_str;
  bool ignore_message = false;
  // ofstream& of;
  map<string,ofstream> outs;
  map<string,ofstream>::iterator mit;
  vector<string> symbols_processed;



  while (fs.get(c)) {
    asc = (int)c;
    if (asc == 11 || asc == 13 || asc == 14 || asc == 15) {
      message_mode = true;
      symbol_discovered = false;
      ignore_message = false;
      //cout << "\nnew_message" << endl;
      //char a;
      //cin >> a;
    } else if (asc == 31) {
      message_mode = false;
      symbol_discovered = false;
      ignore_message = false;
      write_packet = true;
      symbols_processed.clear();
    }
    if (!message_mode) {
        // collecting a header in header vector<char>
        if (write_packet) header.push_back(c);
        asc = (int)c;
        if (asc == 32) {
          write_packet = false;
          header_str.assign(header.begin(),header.end());
          header.clear();
        }
    } else {
        // cout << c;
        if (ignore_message) continue;
        if (!symbol_discovered) {
            if (message.size() == 0) {
              message.push_back(c);
              continue;
            }

            // delimiter taken care of
            // get first letter of symbol or exchange code
            if (message.size() == 1 && symbol.size() == 0) {
              asc = (int)c;
              if (asc >=97 && asc <=122)
                message.push_back(c);
              else if (c == '.')
                ignore_message = true;
              else
                symbol.push_back(c);
              continue;
            }

            // if symbol still 0 there was message code(s)
            if (symbol.size() == 0) {
              if (c == '.')
                ignore_message = true;
              else
                symbol.push_back(c);
              continue;
            }

            // there was an exchange prefix
            if (symbol.size() == 1 && c == ':') {
              message.push_back(symbol[0]);
              message.push_back(c);
              symbol.clear();
              continue;
            }

            // at this point symbol is at least 1 and it's not a .TRACER
            asc = (int)c;
            if ((asc >=97 && asc <=122) || c == '*' || c == '+') {
              symbol_str.assign(symbol.begin(),symbol.end());
              symbol.clear();
              symbol_discovered = true;
              message.push_back('~');
              message.push_back(c);
              message_str.assign(message.begin(),message.end());
              message.clear();
              cout << endl << "--------------------------------------";
              cout << endl << symbol_str;
              cout << endl << header_str;
              cout << endl << message_str;



            } else
              symbol.push_back(c);
        } else {
          // of << c;
        }

    }
  }

  fs.close();
  return 0;
}
