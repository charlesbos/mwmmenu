/*
 * mwmmenu - a program to produce menus for the Motif Window Manager, based on
 * freedesktop.org desktop entries.
 *
 * Copyright (C) 2015  Charles Bos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

/* This function calls the other functions to group the desktop entries by category
 * and then print out each menu. It then calls the function to write the main menu
 * which should contain entries for all the category menus used (but not the ones 
 * not used) */
void MwmMenuWriter::printHandler()
{ const char *validCatsArr[] = {"Development", "Education", "Game", "Graphics", "Multimedia", "Network",
                                "Office", "Other", "Science", "Settings", "System", "Utility"};
  const char *usedCats[sizeof(validCatsArr) / sizeof(validCatsArr[0])] = {"\0"};
  int usedCounter = 0;

  for (unsigned int x = 0; x < sizeof(validCatsArr) / sizeof(validCatsArr[0]); x++)
  { vector< pair<int,string> > positions = getPositionsPerCat(validCatsArr[x]);
    if (!positions.empty()) //Ignore any categories that do not have any entries associated
    { writeMwmCategoryMenu(positions, validCatsArr[x]);
      usedCats[usedCounter] = validCatsArr[x];
      usedCounter++;
    }
  }

  writeMwmMainMenu(menuName, usedCats, usedCounter);
}

/* This function is used by the sort function to sort the menu entries for each category
 * by name (which is the second half of the pair. All chars are converted to upper case
 * to avoid lower case being treated as greater */
bool sortPairs(pair<int,string> p1, pair<int,string> p2)
{ for (unsigned int x = 0; x < p1.second.size(); x++) p1.second[x] = toupper(p1.second[x]);
  for (unsigned int x = 0; x < p2.second.size(); x++) p2.second[x] = toupper(p2.second[x]);
  if (p1.second < p2.second) return true;
  else return false;
}

/* This function is used to determine which entries should be printed for a given category.
 * It returns a vector of pairs where each pair contains the index of the DesktopFile
 * object in the DesktopFi;e array and the name of the entry. The name is collected only so
 * that we can alphabetically sort the menu entries */
vector< pair<int,string> > MwmMenuWriter::getPositionsPerCat(string category)
{ vector< pair<int,string> > positions;

  for (int x = 0; x < filesLength; x++)
  { //If the entry matches the category, add it but if NoDisplay is true, then don't
    if (find(this->files[x]->categories.begin(), this->files[x]->categories.end(), category) != this->files[x]->categories.end()
      && this->files[x]->nodisplay != true && this->files[x]->onlyShowIn != true)
    { pair<int,string> p(x, this->files[x]->name);
      positions.push_back(p);
    }
  }

  sort(positions.begin(), positions.end(), sortPairs);
  return positions;
}

/* This function is used to get the length of the longest entry name so that we can
 * neatly format the menu output */
int MwmMenuWriter::getLongestNameLength()
{ unsigned int longest = 0;

  for (int x = 0; x < filesLength; x++)
    if (this->files[x]->name.size() > longest) longest = this->files[x]->name.size();

  return longest;
}

//This function writes the menu for a given category to the console
void MwmMenuWriter::writeMwmCategoryMenu(vector< pair<int,string> > positions, string category)
{ int longest = getLongestNameLength();

  cout << "Menu " << category << endl << "{" << endl;
  cout << "\t" << setw(longest) << left << category << "\t" << "f.title" << endl;
  for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
    cout << "\t" << setw(longest) << left << files[it->first]->name << "\t" << "f.exec " << files[it->first]->exec << endl;
  cout << "}" << endl << endl;
}

/* This function writes the main menu to the console. The main menu contains only
 * only references to the used category menus */
void MwmMenuWriter::writeMwmMainMenu(string menuName, const char *usedCats[], int catNumber)
{ if (catNumber > 0)
  { int longest = getLongestNameLength();

    cout << "Menu " << menuName << endl << "{" << endl;
    cout << "\t" << setw(longest) << left << menuName << "\t" << "f.title" << endl;
    for (int x = 0; x < catNumber; x++)
      cout << "\t" << setw(longest) << left << usedCats[x] << "\t" << "f.menu  " << usedCats[x] << endl;
    cout << "}" << endl << endl;
  }
  else cout << "We couldn't find any desktop entries. Sorry." << endl;
}
