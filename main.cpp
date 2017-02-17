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
#include <memory>
#include <cstdlib>

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
  ofstream output_file("output.txt");
  ofstream* of = &output_file;
  map<string,ofstream*> outps;
  map<string,shared_ptr<ofstream>> outs;
  shared_ptr<ofstream> ofp;
  map<string,ofstream>::iterator mit;
  vector<string> symbols_processed;
  vector<string> this_batch; // limitied to 512 open files by os
  vector<string> completed_batches;
  bool more_symbols = true; // set to false when a batch processes with no rejects
  // if message must be recorded it will be flushed when delimiter is encountered
  bool message_buffer_mode = false;


  while (more_symbols) {
    ifstream fs(argv[1]);
    if (!fs.is_open()) {
      cerr << "File not found! Bye!" << endl;
      return 1;
    }
    int rejected_symbols = 0;
    while (fs.get(c)) {
      asc = (int)c;
      if (asc == 11 || asc == 13 || asc == 14 || asc == 15) {
        message_mode = true;
        if (message_buffer_mode) {
          message_str.assign(message.begin(),message.end());
          *ofp << message_str;
          message.clear();
        }
        message_buffer_mode = false;
        symbol_discovered = false;
        ignore_message = false;
        message.clear(); // incase not already in case of incomplete message_str
        symbol.clear(); // dido
        //cout << "\nnew_message" << endl;
        //char a;
        //cin >> a;
      } else if (asc == 31) {
        message_mode = false;
        if (message_buffer_mode) {
          message_str.assign(message.begin(),message.end());
          *ofp << message_str;
          message.clear();
        }
        message_buffer_mode = false;
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
            cout << header_str << endl;
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

                // if not in this batch, add it if room, else ignore until
                if (find(this_batch.begin(),this_batch.end(),symbol_str) == this_batch.end()) {
                  // if it's in completed_batches ignore
                  if (find(completed_batches.begin(),completed_batches.end(),symbol_str) != completed_batches.end()) {
                    message.clear();
                    ignore_message = true;
                    continue;
                  } else if (this_batch.size() < 500) {
                    // it hasn't been processed and there's room in this batch
                    this_batch.push_back(symbol_str);
                  } else {
                      // it hasn't been processed and there's no room so next batches will take this
                      rejected_symbols++;
                      message.clear();
                      ignore_message = true;
                      continue;
                  }
                }
                message.push_back('~');
                message.push_back(c);
                message_buffer_mode = true;


                map<string,shared_ptr<ofstream>>::iterator sit = outs.find(symbol_str);
                if (sit == outs.end()) {
                    shared_ptr<ofstream> sptro(new ofstream);
                    sptro->open(argv[2] + symbol_str + ".CAP",ios_base::app);
                    *sptro << header_str;

                    outs.insert(pair<string,shared_ptr<ofstream>>(symbol_str,sptro));
                    symbols_processed.push_back(symbol_str);
                    ofp = sptro;
                    //if (!ofp->is_open()) cout << "NOT OPEN 1" << endl;
                } else {
                    shared_ptr<ofstream> sptro = sit->second;
                    if (find(symbols_processed.begin(),symbols_processed.end(),symbol_str) == symbols_processed.end()) {
                      symbols_processed.push_back(symbol_str);
                      *sptro << header_str;
                    }
                    ofp = sit->second;
                    //if (!ofp->is_open()) cout << "NOT OPEN 2" << endl;
                }

              } else
                symbol.push_back(c);
          } else {
            message.push_back(c);
            //if (!ofp->is_open()) cout << "NOT OPEN 3 " << symbol_str << endl;
          }
      }
    }

    // this_batch vector transfered to copmleted_batches
    for (auto a : this_batch) completed_batches.push_back(a);
    this_batch.clear();
    // outps second
    for (map<string,shared_ptr<ofstream>>::iterator t = outs.begin(); t != outs.end(); ++t)
      (*(t->second)).close();
    outs.clear();
    fs.close();
    if (rejected_symbols == 0) more_symbols = false;
  }
  return 0;
}
