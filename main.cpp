/*
 * argv[1] path file to a cap file
 * argv[2] path to output directory
 * breaks up the cap file by symbol
 * eliminates packet date except at begining of file
 * eliminates message symbols and places a ~ in place where symbol should be
 */
#include <fstream>
#include <iostream>
#include <vector>



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

  while (fs.get(c)) {
    if (!message_mode) cout << ((int)c) << endl;
    asc = (int)c;
    if (asc == 11 || asc == 13 || asc == 14 || asc == 15) {
      message_mode = true;

      //cout << "\nnew_message" << endl;
      //char a;
      //cin >> a;
    } else if (asc == 31) {
      message_mode = false;
      header.clear();
      write_packet = true;
      cout << "\nnew_packet" << endl;
      char a;
      cin >> a;
    }
    if (!message_mode) {
        // collecting a header in header vector<char>
        if (write_packet) header.push_back(c);
        asc = (int)c;
        if (asc == 32) {
          write_packet = false;
          header_str.assign(header.begin(),header.end());
          cout << header_str << endl;
        }
    } else {
        // cout << c;
    }
  }

  fs.close();
  return 0;
}
