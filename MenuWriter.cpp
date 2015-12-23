/*
 * mwmmenu - a program to produce application menus for MWM and other window 
 * managers, based on freedesktop.org desktop entries.
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
#include "MenuWriter.h"

#define mwm 0
#define twm 1
#define fvwm 2
#define fluxbox 3

MenuWriter::MenuWriter(DesktopFile **files, int filesLength, string menuName, string windowmanager, bool useIcons, vector<string> iconpaths, string exclude, string excludeMatching)
{ this->files = files;
  this->filesLength = filesLength;
  this->menuName = menuName;
  this->windowmanager = windowmanager;
  this->useIcons = useIcons;
  this->iconpaths = iconpaths;
  this->exclude = exclude;
  this->excludeMatching = excludeMatching;
  exclusionHandler();
  printHandler();
}

/* This function calls the other functions to group the desktop entries by category
 * and then print out each menu. It then calls the function to write the main menu
 * which should contain entries for all the category menus used (but not the ones 
 * not used) */
void MenuWriter::printHandler()
{ const char *validCatsArr[] = {"Accessories", "Development", "Education", "Game", "Graphics", "Multimedia", "Internet",
                                "Office", "Other", "Science", "Settings", "System"};
  const char *usedCats[sizeof(validCatsArr) / sizeof(validCatsArr[0])] = {"\0"};
  int usedCounter = 0;
  int wmID = getWmID(windowmanager);

  for (unsigned int x = 0; x < sizeof(validCatsArr) / sizeof(validCatsArr[0]); x++)
  { vector< pair<int,string> > positions = getPositionsPerCat(validCatsArr[x]);
    if (!positions.empty()) //Ignore any categories that do not have any entries associated
    { writeCategoryMenu(positions, validCatsArr[x], wmID, usedCounter, ((sizeof(validCatsArr) / sizeof(validCatsArr[0])) - 1));
      usedCats[usedCounter] = validCatsArr[x];
      usedCounter++;
    }
  }
  writeMainMenu(menuName, usedCats, usedCounter, wmID);
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
vector< pair<int,string> > MenuWriter::getPositionsPerCat(string category)
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

/* Function to filter out desktop entries specified from the command line
 * as entries that should be excluded. We do this by checking if an entry
 * name fully or partially matches the names specified on the command line
 * as appropriate and then setting the nodisplay value for that DesktopFile
 * to true if so */
void MenuWriter::exclusionHandler()
{ vector<string> excludeStrings;
  vector<string> excludeMatchingStrings;
  char buffer[exclude.size() + excludeMatching.size() + 1] = {'\0'};
  int selector = 0;

  if (exclude != "\0")
  { for (unsigned int x = 0; x < exclude.size(); x++)
    { if (exclude[x] == ',') 
      { excludeStrings.push_back(buffer);
        selector = 0;
        fill(buffer, buffer + sizeof(buffer) / sizeof(buffer[0]), '\0');
        continue;
      }
      buffer[selector] = exclude[x];
      selector += 1;
    }
    if (selector != 0) excludeStrings.push_back(buffer);
    for (int x = 0; x < filesLength; x++)
      if (find(excludeStrings.begin(), excludeStrings.end(), files[x]->name) != excludeStrings.end()) files[x]->nodisplay = true;
  }

  if (excludeMatching != "\0")
  { selector = 0;
    fill(buffer, buffer + sizeof(buffer) / sizeof(buffer[0]), '\0');
    for (unsigned int x = 0; x < excludeMatching.size(); x++)
    { if (excludeMatching[x] == ',') 
      { excludeMatchingStrings.push_back(buffer);
        selector = 0;
        fill(buffer, buffer + sizeof(buffer) / sizeof(buffer[0]), '\0');
        continue;
      }
      buffer[selector] = excludeMatching[x];
      selector += 1;
    }
    if (selector != 0) excludeMatchingStrings.push_back(buffer);
    for (int x = 0; x < filesLength; x++)
    { for (unsigned int y = 0; y < excludeMatchingStrings.size(); y++)
      { if (files[x]->name.find(excludeMatchingStrings[y]) != string::npos)
        { files[x]->nodisplay = true;
          break;
        }
      }
    }
  }
}

/* This function is used to get the length of the longest entry name so that we can
 * neatly format the menu output */
int MenuWriter::getLongestNameLength()
{ unsigned int longest = 0;

  for (int x = 0; x < filesLength; x++)
  { if (useIcons && windowmanager != "Fluxbox")
    { if (this->files[x]->name.size() + this->files[x]->icon.size() > longest) longest = this->files[x]->name.size() + this->files[x]->icon.size() + 10; }
    else
    { if (this->files[x]->name.size() > longest) longest = this->files[x]->name.size() + 10; }
  }

  return longest;
}

//Return an integer identifying which window manager, menus should be produced for
int MenuWriter::getWmID(string windowmanager)
{ if (windowmanager == "TWM") return twm;
  if (windowmanager == "FVWM") return fvwm;
  if (windowmanager == "Fluxbox") return fluxbox;
  else return mwm;
}

/* Cycle through list of icon paths and attempt to match the category
 * name against a category icon of the appropriate size. Return null
 * character if we can't find one */
string MenuWriter::getCategoryIcon(string catName)
{ catName.at(0) = tolower(catName.at(0));
  for (unsigned int x = 0; x < iconpaths.size(); x++)
    if (iconpaths[x].find("16x16") != string::npos
        && iconpaths[x].find("categories") != string::npos
        && iconpaths[x].find(catName) != string::npos)
      return iconpaths[x];
  return "\0";
}

//Write category menu
void MenuWriter::writeCategoryMenu(vector< pair<int,string> > positions, string category, int wmID, int catNumber, int maxCatNumber)
{ int longest = getLongestNameLength();
  string entryName;
  string entryExec;
  string catName;

  switch(wmID)
  { case mwm :
      cout << "Menu " << category << endl << "{" << endl;
      cout << "\t" << setw(longest) << left << category << "\t" << "f.title" << endl;
      for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
      { entryName = '"' + files[it->first]->name + '"';
        entryExec = '"' + files[it->first]->exec + '"';
        cout << "\t" << setw(longest) << left << entryName << "\t" << "f.exec " << entryExec << endl;
      }
      cout << "}" << endl << endl;
      break;
    case twm :
      catName = '"' + category + '"';
      cout << "menu " << catName << endl << "{" << endl;
      cout << setw(longest) << left << catName << "\t" << "f.title" << endl;
      for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
      { entryName = '"' + files[it->first]->name + '"';
        entryExec = "\"exec " + files[it->first]->exec + " &" + '"';
        cout << setw(longest) << left << entryName << "\t" << "f.exec " << entryExec << endl;
      }
      cout << "}" << endl << endl;
      break;
    case fvwm :
      cout << "AddToMenu " << category << "\t\t" << category << " Title" << endl;
      for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
      { if (useIcons && files[it->first]->icon != "\0") entryName = '"' + files[it->first]->name + " %" + files[it->first]->icon + "%" + '"';
        else entryName = '"' + files[it->first]->name + '"';
        entryExec = files[it->first]->exec;
        cout << "+\t\t" << setw(longest) << left << entryName << "\t" << "Exec " << entryExec << endl;
      }
      cout << endl;
      break;
    case fluxbox :
      if (catNumber == 0) cout << "[submenu] (" << menuName << ')' << endl;
      if (useIcons)
      { string catIcon = getCategoryIcon(category);
        if (catIcon != "\0") catName = '(' + category + ") <" + catIcon + '>';
        else catName = '(' + category + ')';
      }
      else catName = '(' + category + ')';
      cout << "\t[submenu] " + catName + " {}" << endl;
      for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
      { if (useIcons && files[it->first]->icon != "\0") entryExec = '{' + files[it->first]->exec + "} <" + files[it->first]->icon + ">";
        else entryExec = '{' + files[it->first]->exec + '}';
        entryName = '(' + files[it->first]->name + ')';
        cout << "\t\t[exec] " << setw(longest) << left << entryName << " " << entryExec << endl;
      }
      cout << "\t[end]" << endl;
      if (catNumber == maxCatNumber) cout << "[end]" << endl;
      break;
  }
}

//Write main menu
void MenuWriter::writeMainMenu(string menuName, const char *usedCats[], int catNumber, int wmID)
{ if (catNumber > 0)
  { int longest = getLongestNameLength();
    string catName;
    string menuNameWithQuotes;
    string catMenuWithQuotes;

    switch(wmID)
    { case mwm :
        cout << "Menu " << menuName << endl << "{" << endl;
        cout << "\t" << setw(longest) << left << menuName << "\t" << "f.title" << endl;
        for (int x = 0; x < catNumber; x++)
          cout << "\t" << setw(longest) << left << usedCats[x] << "\t" << "f.menu  " << usedCats[x] << endl;
        cout << "}" << endl << endl;
        break;
      case twm :
        menuNameWithQuotes = '"' + menuName + '"';
        cout << "menu " << menuNameWithQuotes << endl << "{" << endl;
        cout << setw(longest) << left << menuNameWithQuotes << "\t" << "f.title" << endl;
        for (int x = 0; x < catNumber; x++)
        { catMenuWithQuotes = '"' + string(usedCats[x]) + '"';
          cout << setw(longest) << left << catMenuWithQuotes << "\t" << "f.menu  " << catMenuWithQuotes << endl;
        }
        cout << "}" << endl << endl;
        break;
      case fvwm :
        cout << "AddToMenu " << menuName << "\t\t" << menuName << " Title" << endl;
        for (int x = 0; x < catNumber; x++)
        { if (useIcons)
          { string catIcon = getCategoryIcon(string(usedCats[x]));
            if (catIcon != "\0") catName = '"' + string(usedCats[x]) + " %" + catIcon + "%" + '"';
            else catName = '"' + string(usedCats[x]) + '"';
          }
          else catName = '"' + string(usedCats[x]) + '"';
          cout << "+\t\t" << setw(longest) << left << catName << "\t" << "Popup  " << usedCats[x] << endl;
        }
        cout << endl;
        break;
      case fluxbox :
        //Do nothing here, main menu needs to be handled in the writeCategoryMenu function
        break;
    }
  }
  else cout << "We couldn't find any desktop entries. Sorry." << endl;
}
