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
#include "Categories.h"

//WM id numbers
#define mwm 0
#define twm 1
#define fvwm 2
#define fluxbox 3
#define openbox 4
#define olvwm 5
#define windowmaker 6
#define icewm 7

MenuWriter::MenuWriter(DesktopFile **files, 
                       int filesLength, 
                       string menuName, 
                       string windowmanager, 
                       bool useIcons, 
                       vector<string> iconpaths, 
                       vector<string> exclude, 
                       vector<string> excludeMatching, 
                       vector<string> excludeCategories, 
                       string iconSize, 
                       vector<string> include, 
                       vector<string> excludedFilenames, 
                       Categories **cats,
                       int customCatNum)
{ this->files = files;
  this->cats = cats;
  this->customCatNum = customCatNum;
  this->filesLength = filesLength;
  this->menuName = menuName;
  this->windowmanager = windowmanager;
  this->useIcons = useIcons;
  this->iconpaths = iconpaths;
  this->exclude = exclude;
  this->excludeMatching = excludeMatching;
  this->excludeCategories = excludeCategories;
  this->iconSize = iconSize;
  this->include = include;
  this->excludedFilenames = excludedFilenames;
  printHandler();
}

/* This function calls the other functions to group the desktop entries by category
 * and then print out each menu. It then calls the function to write the main menu
 * which should contain entries for all the category menus used (but not the ones 
 * not used) */
void MenuWriter::printHandler()
{ vector<string> validCatsArr = {"Accessories", "Development", "Education", "Game", "Graphics", "Multimedia", "Internet",
                                 "Office", "Other", "Science", "Settings", "System"};
  for (int x = 0; x < customCatNum; x++) validCatsArr.push_back(cats[x]->name);
  sort(validCatsArr.begin(), validCatsArr.end());
  int validCatsLength = validCatsArr.size();
  vector<string> usedCats;
  int usedCounter = 0;
  int wmID = getWmID();
  int longest = getLongestNameLength();

  entryDisplayHandler();

  for (int x = 0; x < validCatsLength; x++)
  { vector< pair<int,string> > positions = getPositionsPerCat(validCatsArr[x]);
    /* Ignore categories that do not have entries associated and also ignore categories
     * that have been specified on the command line as categories that should be ignored */
    if (!positions.empty() && !checkExcludedCategories(validCatsArr[x]))
    { writeCategoryMenu(positions, validCatsArr[x], wmID, usedCounter, usedCats.size(), longest);
      usedCats.push_back(validCatsArr[x]);
      usedCounter++;
    }
  }
  writeMainMenu(usedCats, usedCounter, wmID, longest);
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
  positions.reserve(20);

  for (int x = 0; x < filesLength; x++)
  { //If the entry matches the category, add it but if NoDisplay is true, then don't
    if (find(files[x]->categories.begin(), files[x]->categories.end(), category) != files[x]->categories.end()
      && files[x]->nodisplay != true && files[x]->onlyShowIn != true)
    { pair<int,string> p(x, files[x]->name);
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
 * to true if so 
 *
 * Now it also forces the inclusion of any specified desktop entries that
 * hav a no display value of true */
void MenuWriter::entryDisplayHandler()
{ if (!exclude.empty())
  { for (int x = 0; x < filesLength; x++)
      if (find(exclude.begin(), exclude.end(), files[x]->name) != exclude.end()) files[x]->nodisplay = true;
  }
  if (!excludeMatching.empty())
  { for (int x = 0; x < filesLength; x++)
    { for (unsigned int y = 0; y < excludeMatching.size(); y++)
      { if (files[x]->name.find(excludeMatching[y]) != string::npos)
        { files[x]->nodisplay = true;
          break;
        }
      }
    }
  }
  if (!excludedFilenames.empty())
  { for (int x = 0; x < filesLength; x++)
      if (find(excludedFilenames.begin(), excludedFilenames.end(), files[x]->filename) != excludedFilenames.end()) files[x]->nodisplay = true;
  }
  if (!include.empty())
  { for (int x = 0; x < filesLength; x++)
      if (find(include.begin(), include.end(), files[x]->name) != include.end()) files[x]->nodisplay = false;
  }
}

/* A function to check whether a category is present in the list of excluded categories.
 * If it is, return true, otherwise return false */
bool MenuWriter::checkExcludedCategories(string category)
{ if (find(excludeCategories.begin(), excludeCategories.end(), category) != excludeCategories.end())
    return true;
  else
    return false;
}

/* This function will return either the longest name length (or the longest length name + icon
 * path string) plus an offset of 10 to account for any tab chars */
int MenuWriter::getLongestNameLength()
{ unsigned int longest = 0;

  for (int x = 0; x < filesLength; x++)
  { /* Most wm menus seem to define an icon path after the entry name so for the longest
     * name we need to add the length of the icon path on. However Fluxbox defines the
     * icon path after the exec so we don't want to add that extra length to the name in
     * this case */
    if (useIcons && windowmanager != "Fluxbox")
    { if (files[x]->name.size() + files[x]->icon.size() > longest) longest = files[x]->name.size() + files[x]->icon.size(); }
    else
    { if (files[x]->name.size() > longest) longest = files[x]->name.size(); }
  }

  return longest + 10;
}

//Return an integer identifying which window manager, menus should be produced for
int MenuWriter::getWmID()
{ if (windowmanager == "TWM") return twm;
  if (windowmanager == "FVWM") return fvwm;
  if (windowmanager == "Fluxbox") return fluxbox;
  if (windowmanager == "Openbox") return openbox;
  if (windowmanager == "Blackbox") return fluxbox; //Fluxbox menus work in Blackbox
  if (windowmanager == "Olvwm") return olvwm;
  if (windowmanager == "Windowmaker") return windowmaker;
  if (windowmanager == "IceWM") return icewm;
  else return mwm;
}

/* Cycle through list of icon paths and attempt to match the category
 * name against a category icon of the appropriate size. Return null
 * character if we can't find one */
string MenuWriter::getCategoryIcon(string catName)
{ string nameGuard = "categories";

  //There is no icon for education so use the science one instead
  if (catName == "Education") catName = "Science";

  //If it's a custom category, use its icon definition instead
  for (unsigned int x = 0; x < sizeof(cats) / sizeof(cats[0]); x++)
  { if (catName == cats[x]->name)
    { catName = cats[x]->icon;
      nameGuard = "/"; //If its custom, we know the icondef is valid so remove the guard
      break;
    }
  }

  catName.at(0) = tolower(catName.at(0));
  for (unsigned int x = 0; x < iconpaths.size(); x++)
    if (iconpaths[x].find(iconSize) != string::npos
        && iconpaths[x].find(nameGuard) != string::npos
        && iconpaths[x].find(catName) != string::npos)
      return iconpaths[x];
  return "\0";
}

/* This function is called multiple times. Each time, it prints out the submenu
 * for a given category. It might also print out the 'main' menu which in which
 * the category menus are nested. This is done only if the window manager menu syntax
 * requires the category submenus to be literally nested inside the main one */
void MenuWriter::writeCategoryMenu(vector< pair<int,string> > positions, string category, int wmID, int catNumber, int maxCatNumber, int longest)
{ string entryName;
  string entryExec;
  string catName;

  switch(wmID)
  { case mwm :
      catName = '"' + category + '"';
      cout << "Menu " << catName << endl << "{" << endl;
      cout << "\t" << setw(longest) << left << catName << "\t" << "f.title" << endl;
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
      catName = '"' + category + '"';
      cout << "AddToMenu " << setw(10) << left << catName << "\t" << setw(longest) << left << catName << "\tTitle" << endl;
      for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
      { if (useIcons && files[it->first]->icon != "\0") entryName = '"' + files[it->first]->name + " %" + files[it->first]->icon + "%" + '"';
        else entryName = '"' + files[it->first]->name + '"';
        entryExec = files[it->first]->exec;
        cout << "+\t\t\t" << setw(longest) << left << entryName << "\t" << "Exec " << entryExec << endl;
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
    case openbox :
      catName = '"' + category + '"';
      cout << "<menu id=" << catName << " label=" << catName << ">" << endl;
      for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
      { if (useIcons && files[it->first]->icon != "\0") 
          entryName = '"' + files[it->first]->name + '"' + " icon=\"" + files[it->first]->icon + "\">";
        else entryName = '"' + files[it->first]->name + "\">";
        entryExec = files[it->first]->exec;
        cout << "\t<item label=" << setw(longest) << left << entryName << endl;
        cout << "\t\t<action name=\"Execute\">" << endl;
        cout << "\t\t\t<execute>" << entryExec << "</execute>" << endl;
        cout << "\t\t</action>" << endl;
        cout << "\t</item>" << endl;
      }
      cout << "</menu>" << endl << endl;
      break;
    case olvwm :
      if (catNumber == 0) cout << setw(longest) << left << '"' + menuName + '"' << "MENU" << endl << endl;
      catName = '"' + category + '"';
      cout << setw(longest) << left << catName << "MENU" << endl;
      for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
      { entryName = '"' + files[it->first]->name + '"';
        entryExec = files[it->first]->exec;
        cout << setw(longest) << left << entryName << entryExec << endl;
      }
      cout << setw(longest) << left << catName << "END PIN" << endl << endl;
      if (catNumber == maxCatNumber) cout << setw(longest) << left << '"' + menuName + '"' << "END PIN" << endl;
      break;   
    case windowmaker :
      if (catNumber == 0) cout << "(\n  " << '"' << menuName << '"' << ',' << endl;
      catName = '"' + category + '"';
      cout << "  (\n    " << catName << ',' << endl;
      for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
      { entryName = '"' + files[it->first]->name + '"';
        entryExec = '"' + files[it->first]->exec + '"';
        cout << "    (" << entryName << ", " << "EXEC, " << entryExec << ")";
        if ((it - positions.begin()) != (positions.end() - positions.begin() - 1)) cout << ',' << endl;
        else cout << endl;
      }
      if (catNumber != maxCatNumber) cout << "  )," << endl;
      else cout << "  )\n)" << endl;
      break;
    case icewm :
      if (useIcons)
      { string catIcon = getCategoryIcon(category);
        if (catIcon != "\0") catName = '"' + category + "\" " + catIcon;
        else catName = '"' + category + "\" -";
      }
      else catName = '"' + category + "\" folder";
      cout << "menu " << catName << " {" << endl;
      for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
      { if (useIcons && files[it->first]->icon != "\0") entryName = '"' + files[it->first]->name + '"' + " " + files[it->first]->icon;
        else entryName = '"' + files[it->first]->name + "\" -";
        entryExec = files[it->first]->exec;
        cout << "\tprog " + entryName + " " + entryExec << endl;
      }
      cout << "}\n" << endl;
      break;
  }
}

/* This function is called once. It will print out a menu referring to the category
 * submenus. Note that some window managers require the submenus to be literally
 * nested inside the 'main' menu. In this case, the writeCategoryMenu function will also
 * handle the main menu and this function will do nothing. In cases where there
 * are no printable menu entries, this function will print a warning message instead */
void MenuWriter::writeMainMenu(vector<string> usedCats, int catNumber, int wmID, int longest)
{ if (catNumber > 0)
  { string menuNameWithQuotes;
    string catNameWithQuotes;

    switch(wmID)
    { case mwm :
        cout << "Menu " << menuName << endl << "{" << endl;
        cout << "\t" << setw(longest) << left << menuName << "\t" << "f.title" << endl;
        for (int x = 0; x < catNumber; x++)
        { catNameWithQuotes = '"' + string(usedCats[x]) + '"';
          cout << "\t" << setw(longest) << left << catNameWithQuotes << "\t" << "f.menu  " << catNameWithQuotes << endl;
        }
        cout << "}" << endl << endl;
        break;
      case twm :
        menuNameWithQuotes = '"' + menuName + '"';
        cout << "menu " << menuNameWithQuotes << endl << "{" << endl;
        cout << setw(longest) << left << menuNameWithQuotes << "\t" << "f.title" << endl;
        for (int x = 0; x < catNumber; x++)
        { catNameWithQuotes = '"' + string(usedCats[x]) + '"';
          cout << setw(longest) << left << catNameWithQuotes << "\t" << "f.menu  " << catNameWithQuotes << endl;
        }
        cout << "}" << endl << endl;
        break;
      case fvwm :
        cout << "AddToMenu " << setw(10) << left << menuName << "\t" << setw(longest) << left << menuName << "\tTitle" << endl;
        for (int x = 0; x < catNumber; x++)
        { if (useIcons)
          { string catIcon = getCategoryIcon(string(usedCats[x]));
            if (catIcon != "\0") catNameWithQuotes = '"' + string(usedCats[x]) + " %" + catIcon + "%" + '"';
            else catNameWithQuotes = '"' + string(usedCats[x]) + '"';
          }
          else catNameWithQuotes = '"' + string(usedCats[x]) + '"';
          cout << "+\t\t\t" << setw(longest) << left << catNameWithQuotes << "\t" << "Popup  " << '"' + usedCats[x] + '"' << endl;
        }
        cout << endl;
        break;
      case fluxbox :
        //Do nothing here, main menu needs to be handled in the writeCategoryMenu function
        break;
      case openbox :
        menuNameWithQuotes = '"' + menuName + '"';
        cout << "<menu id=" << menuNameWithQuotes << " label=" << menuNameWithQuotes << ">" << endl;
        for (int x = 0; x < catNumber; x++)
        { if (useIcons)
          { string catIcon = getCategoryIcon(string(usedCats[x]));
            if (catIcon != "\0") catNameWithQuotes = '"' + string(usedCats[x]) + '"' + " icon=\"" + catIcon + "\"/>";
            else catNameWithQuotes = '"' + string(usedCats[x]) + "\"/>";
          }
          else catNameWithQuotes = '"' + string(usedCats[x]) + "\"/>";
          cout << "\t<menu id=" << setw(longest) << left << catNameWithQuotes << endl;
        }
        cout << "</menu>" << endl << endl;
        break;
      case olvwm :
        //Do nothing here, main menu needs to be handled in the writeCategoryMenu function
        break;
      case windowmaker :
        //Do nothing here, main menu needs to be handled in the writeCategoryMenu function
        break;
      case icewm :
        //Do nothing here, main menu is defined in ~/.icewm/menu
        break;
    }
  }
  else cout << "We couldn't find any desktop entries. Sorry." << endl;
}
