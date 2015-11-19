#ifndef _DESKTOP_FILE_H_
#define _DESKTOP_FILE_H_

#include <string>
#include <fstream>
#include <vector>

using namespace std;

class DesktopFile
{
  public:
    DesktopFile();
    DesktopFile(const char *filename);

    string name;
    string exec;
    vector<string> categories;
    bool nodisplay;

  private:
    ifstream dfile;

    void close();

    void populate();

    string getID(string line);
    string getSingleValue(string line);
    vector<string> getMultiValue(string line);

    void processCategories(vector<string> &categories);
};

#endif
