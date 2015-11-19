#ifndef _MWM_MENU_WRITER_H_
#define _MWM_MENU_WRITER_H_

#include <fstream>
#include "DesktopFile.h"

class MwmMenuWriter
{ public:
    MwmMenuWriter(DesktopFile **files, int filesLength, string menuName);

  private:
    DesktopFile **files;
    int filesLength;
    string menuName;

    void printHandler();

    vector< pair<int,string> > getPositionsPerCat(string category);
    int getLongestNameLength();

    void writeMwmCategoryMenu(vector< pair<int,string> > positions, string category);
    void writeMwmMainMenu(string menuName, const char *usedCats[], int catNumber);
};

#endif