#include <algorithm>
#include <iostream>
#include <iomanip>
#include "MwmMenuWriter.h"

MwmMenuWriter::MwmMenuWriter(DesktopFile **files, int filesLength, string menuName)
{ this->files = files;
  this->filesLength = filesLength;
  this->menuName = menuName;
  printHandler();
}

void MwmMenuWriter::printHandler()
{ const char *validCatsArr[] = {"Development", "Education", "Game", "Graphics", "Multimedia", "Network",
                                "Office", "Other", "Science", "Settings", "System", "Utility"};
  const char *usedCats[sizeof(validCatsArr) / sizeof(validCatsArr[0])] = {"\0"};
  int usedCounter = 0;

  for (unsigned int x = 0; x < sizeof(validCatsArr) / sizeof(validCatsArr[0]); x++)
  { vector< pair<int,string> > positions = getPositionsPerCat(validCatsArr[x]);
    if (!positions.empty()) 
    { writeMwmCategoryMenu(positions, validCatsArr[x]);
      usedCats[usedCounter] = validCatsArr[x];
      usedCounter++;
    }
  }

  writeMwmMainMenu(menuName, usedCats, usedCounter);
}

bool sortPairs(pair<int,string> p1, pair<int,string> p2)
{ for (unsigned int x = 0; x < p1.second.size(); x++) p1.second[x] = toupper(p1.second[x]);
  for (unsigned int x = 0; x < p2.second.size(); x++) p2.second[x] = toupper(p2.second[x]);
  if (p1.second < p2.second) return true;
  else return false;
}

vector< pair<int,string> > MwmMenuWriter::getPositionsPerCat(string category)
{ vector< pair<int,string> > positions;

  for (int x = 0; x < filesLength; x++)
  { if (find(this->files[x]->categories.begin(), this->files[x]->categories.end(), category) != this->files[x]->categories.end()
      && this->files[x]->nodisplay != true)
    { pair<int,string> p(x, this->files[x]->name);
      positions.push_back(p);
    }
  }

  sort(positions.begin(), positions.end(), sortPairs);
  return positions;
}

int MwmMenuWriter::getLongestNameLength()
{ unsigned int longest = 0;

  for (int x = 0; x < filesLength; x++)
    if (this->files[x]->name.size() > longest) longest = this->files[x]->name.size();

  return longest;
}

void MwmMenuWriter::writeMwmCategoryMenu(vector< pair<int,string> > positions, string category)
{ int longest = getLongestNameLength();

  cout << "Menu " << category << endl << "{" << endl;
  cout << "\t" << setw(longest) << left << category << "\t\t" << "f.title" << endl;
  for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
    cout << "\t" << setw(longest) << left << files[it->first]->name << "\t\t" << "f.exec " << files[it->first]->exec << endl;
  cout << "}" << endl << endl;
}

void MwmMenuWriter::writeMwmMainMenu(string menuName, const char *usedCats[], int catNumber)
{ int longest = getLongestNameLength();

  cout << "Menu " << menuName << endl << "{" << endl;
  cout << "\t" << setw(longest) << left << menuName << "\t\t" << "f.title" << endl;
  for (int x = 0; x < catNumber; x++)
    cout << "\t" << setw(longest) << left << usedCats[x] << "\t\t" << "f.menu  " << usedCats[x] << endl;
  cout << "}" << endl << endl;
}