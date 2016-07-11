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

#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "MenuWriter.h"
#include "Category.h"

//WM id numbers
#define mwm 0
#define fvwm 1
#define fluxbox 2
#define openbox 3
#define olvwm 4
#define windowmaker 5
#define icewm 6

MenuWriter::MenuWriter(vector<DesktopFile> files, 
                       string menuName, 
                       int windowmanager, 
                       bool useIcons, 
                       vector<string> exclude, 
                       vector<string> excludeMatching, 
                       vector<string> excludeCategories, 
                       vector<string> include, 
                       vector<string> excludedFilenames, 
                       vector<Category> cats)
{ this->files = files;
  this->cats = cats;
  this->menuName = menuName;
  this->windowmanager = windowmanager;
  this->useIcons = useIcons;
  this->exclude = exclude;
  this->excludeMatching = excludeMatching;
  this->excludeCategories = excludeCategories;
  this->include = include;
  this->excludedFilenames = excludedFilenames;
  printHandler();
}

/* Handles the fetching of the vector indeces for the desktop entries assigned to each category,
 * the exclusion of categories or entries based on command line arguments and then the printing
 * of the menus themselves */
void MenuWriter::printHandler()
{ vector<Category> usedCats;
  vector< vector<int> > usedPositions;
  int longest = getLongestNameLength();

  entryDisplayHandler();

  //Firstly, get the used categories and files array indeces of the corresponding entries
  for (unsigned int x = 0; x < cats.size(); x++)
  { vector<int> positions = getPositionsPerCat(cats[x]);
    if (!positions.empty() && !checkExcludedCategories(cats[x].name)) 
    { usedPositions.push_back(positions);
      usedCats.push_back(cats[x]);
    }
  }

  //Now write the menus
  for (unsigned int x = 0; x < usedCats.size(); x++)
    writeMenu(usedPositions[x], x, longest, usedCats);
  //For WMs which require a menu which sources the individual category menus
  if (windowmanager == mwm && !usedCats.empty()) writeMenu(vector<int>(), 0, longest, usedCats);
  if (windowmanager == fvwm && !usedCats.empty()) writeMenu(vector<int>(), 0, longest, usedCats);
}

/* This function return the indeces in the files vector for the DesktopFile objects
 * belonging to a given category */
vector<int> MenuWriter::getPositionsPerCat(Category category)
{ vector<int> positions;
  positions.reserve(20);

  for (unsigned int x = 0; x < files.size(); x++)
  { if (find(category.incEntries.begin(), category.incEntries.end(), files[x].name) != category.incEntries.end() 
        && files[x].nodisplay != true)
      positions.push_back(x);
  }

  return positions;
}

/* Function to filter out desktop entries specified from the command line
 * based on various criteria. The entries are excluded simply by setting the
 * nodisplay value to true */
void MenuWriter::entryDisplayHandler()
{ if (!exclude.empty())
  { for (unsigned int x = 0; x < files.size(); x++)
      if (find(exclude.begin(), exclude.end(), files[x].name) != exclude.end()) files[x].nodisplay = true;
  }
  if (!excludeMatching.empty())
  { for (unsigned int x = 0; x < files.size(); x++)
    { for (unsigned int y = 0; y < excludeMatching.size(); y++)
      { if (files[x].name.find(excludeMatching[y]) != string::npos)
        { files[x].nodisplay = true;
          break;
        }
      }
    }
  }
  if (!excludedFilenames.empty())
  { for (unsigned int x = 0; x < files.size(); x++)
      if (find(excludedFilenames.begin(), excludedFilenames.end(), files[x].filename) != excludedFilenames.end()) files[x].nodisplay = true;
  }
  if (!include.empty())
  { for (unsigned int x = 0; x < files.size(); x++)
      if (find(include.begin(), include.end(), files[x].name) != include.end()) files[x].nodisplay = false;
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

  for (unsigned int x = 0; x < files.size(); x++)
  { /* Most wm menus seem to define an icon path after the entry name so for the longest
     * name we need to add the length of the icon path on. However Fluxbox defines the
     * icon path after the exec so we don't want to add that extra length to the name in
     * this case */
    if (useIcons && windowmanager != fluxbox)
    { if (files[x].name.size() + files[x].icon.size() > longest) longest = files[x].name.size() + files[x].icon.size(); }
    else
    { if (files[x].name.size() > longest) longest = files[x].name.size(); }
  }

  return longest + 10;
}

/* This function is called multiple times. Each time, it prints out the submenu
 * for a given category. Some WMs (MWM and FVWM) require a menu which sources
 * the individual category menus. Such a menu will be printed for those window managers
 * if an empty vector of indeces is provided */
void MenuWriter::writeMenu(vector<int> positions, int catNumber, int longest, vector<Category> usedCats)
{ string category = usedCats[catNumber].name; //Variable for the category name
  string catIcon = usedCats[catNumber].icon; //Variable for the category icon
  int maxCatNumber = usedCats.size() - 1;  //Variable for knowing when the last category has been reached
  string nameFormatted; //Variable for a formatted version of the name, e.g. quotes added
  string execFormatted; //Variable for a formatted version of the exec, e.g. quotes added
  string catFormatted; //Variable for a formatted version of the category name, e.g. quotes added
  string menuFormatted; //Variable for a formatted version of the menu, e.g. quotes added

  switch(windowmanager)
  { case mwm :
      if (!positions.empty())
      { catFormatted = '"' + category + '"';
	cout << "menu " << catFormatted << endl << "{" << endl;
	cout << "\t" << setw(longest) << left << catFormatted << "\t" << "f.title" << endl;
	for (vector<int>::iterator it = positions.begin(); it < positions.end(); it++)
	{ nameFormatted = '"' + files[*it].name + '"';
	  execFormatted = "\"exec " + files[*it].exec + " &" + '"';
	  cout << "\t" << setw(longest) << left << nameFormatted << "\t" << "f.exec " << execFormatted << endl;
	}
	cout << "}" << endl << endl;
      }
      else
      { menuFormatted = '"' + menuName + '"';
	cout << "menu " << menuFormatted << endl << "{" << endl;
	cout << "\t" << setw(longest) << left << menuFormatted << "\t" << "f.title" << endl;
	for (unsigned int x = 0; x < usedCats.size(); x++)
	{ catFormatted = '"' + string(usedCats[x].name) + '"';
	  cout << "\t" << setw(longest) << left << catFormatted << "\t" << "f.menu  " << catFormatted << endl;
	}
	cout << "}" << endl << endl;
      }
      break;
    case fvwm :
      if (!positions.empty())
      { catFormatted = '"' + category + '"';
	cout << "AddToMenu " << setw(15) << left << catFormatted << "\t" << setw(longest) << left << catFormatted << "\tTitle" << endl;
	for (vector<int>::iterator it = positions.begin(); it < positions.end(); it++)
	{ if (useIcons && files[*it].icon != "\0") nameFormatted = '"' + files[*it].name + " %" + files[*it].icon + "%" + '"';
	  else nameFormatted = '"' + files[*it].name + '"';
	  execFormatted = files[*it].exec;
	  cout << "+\t\t\t\t" << setw(longest) << left << nameFormatted << "\t" << "Exec " << execFormatted << endl;
	}
	cout << endl;
      }
      else
      { menuFormatted = '"' + menuName + '"';
	cout << "AddToMenu " << setw(15) << left << menuFormatted << "\t" << setw(longest) << left << menuFormatted << "\tTitle" << endl;
	for (unsigned int x = 0; x < usedCats.size(); x++)
	{ if (useIcons)
	  { catIcon = usedCats[x].icon;
            if (catIcon != "\0") catFormatted = '"' + string(usedCats[x].name) + " %" + catIcon + "%" + '"';
	    else catFormatted = '"' + string(usedCats[x].name) + '"';
	  }
	  else catFormatted = '"' + string(usedCats[x].name) + '"';
	  cout << "+\t\t\t\t" << setw(longest) << left << catFormatted << "\t" << "Popup  " << '"' + usedCats[x].name + '"' << endl;
	}
        cout << endl;
      }
      break;
    case fluxbox :
      if (catNumber == 0) cout << "[submenu] (" << menuName << ')' << endl;
      if (useIcons)
      { if (catIcon != "\0") catFormatted = '(' + category + ") <" + catIcon + '>';
        else catFormatted = '(' + category + ')';
      }
      else catFormatted = '(' + category + ')';
      cout << "\t[submenu] " + catFormatted + " {}" << endl;
      for (vector<int>::iterator it = positions.begin(); it < positions.end(); it++)
      { if (useIcons && files[*it].icon != "\0") execFormatted = '{' + files[*it].exec + "} <" + files[*it].icon + ">";
        else execFormatted = '{' + files[*it].exec + '}';
        nameFormatted = files[*it].name;
        //If a name has brackets, we need to escape the closing bracket or it will be missed out
        if (nameFormatted.find(string(")").c_str()) != string::npos) nameFormatted.insert(static_cast<int>(nameFormatted.find_last_of(')')), string("\\").c_str());
        nameFormatted = '(' + nameFormatted + ')';
        cout << "\t\t[exec] " << setw(longest) << left << nameFormatted << " " << execFormatted << endl;
      }
      cout << "\t[end]" << endl;
      if (catNumber == maxCatNumber) cout << "[end]" << endl;
      break;
    case openbox :
      if (catNumber == 0) cout << "<openbox_pipe_menu xmlns=\"http://openbox.org/3.4/menu\">" << endl << endl;
      catFormatted = '"' + category + '"';
      if (useIcons)
      { if (catIcon != "\0") cout << "<menu id=" << catFormatted << " label=" << catFormatted << " icon=" << '"' + catIcon + '"' << ">" << endl;
        else cout << "<menu id=" << catFormatted << " label=" << catFormatted << ">" << endl;
      }
      else cout << "<menu id=" << catFormatted << " label=" << catFormatted << ">" << endl;
      for (vector<int>::iterator it = positions.begin(); it < positions.end(); it++)
      { if (useIcons && files[*it].icon != "\0") 
          nameFormatted = '"' + files[*it].name + '"' + " icon=\"" + files[*it].icon + "\">";
        else nameFormatted = '"' + files[*it].name + "\">";
        execFormatted = files[*it].exec;
        cout << "\t<item label=" << setw(longest) << left << nameFormatted << endl;
        cout << "\t\t<action name=\"Execute\">" << endl;
        cout << "\t\t\t<execute>" << execFormatted << "</execute>" << endl;
        cout << "\t\t</action>" << endl;
        cout << "\t</item>" << endl;
      }
      cout << "</menu>" << endl << endl;
      if (catNumber == maxCatNumber) cout << "</openbox_pipe_menu>" << endl << endl;
      break;
    case olvwm :
      if (catNumber == 0) cout << setw(longest) << left << '"' + menuName + '"' << "MENU" << endl << endl;
      catFormatted = '"' + category + '"';
      cout << setw(longest) << left << catFormatted << "MENU" << endl;
      for (vector<int>::iterator it = positions.begin(); it < positions.end(); it++)
      { nameFormatted = '"' + files[*it].name + '"';
        execFormatted = files[*it].exec;
        cout << setw(longest) << left << nameFormatted << execFormatted << endl;
      }
      cout << setw(longest) << left << catFormatted << "END PIN" << endl << endl;
      if (catNumber == maxCatNumber) cout << setw(longest) << left << '"' + menuName + '"' << "END PIN" << endl;
      break;   
    case windowmaker :
      if (catNumber == 0) cout << "(\n  " << '"' << menuName << '"' << ',' << endl;
      catFormatted = '"' + category + '"';
      cout << "  (\n    " << catFormatted << ',' << endl;
      for (vector<int>::iterator it = positions.begin(); it < positions.end(); it++)
      { nameFormatted = '"' + files[*it].name + '"';
        execFormatted = '"' + files[*it].exec + '"';
        cout << "    (" << nameFormatted << ", " << "EXEC, " << execFormatted << ")";
        if ((it - positions.begin()) != (positions.end() - positions.begin() - 1)) cout << ',' << endl;
        else cout << endl;
      }
      if (catNumber != maxCatNumber) cout << "  )," << endl;
      else cout << "  )\n)" << endl;
      break;
    case icewm :
      if (useIcons)
      { if (catIcon != "\0") catFormatted = '"' + category + "\" " + catIcon;
        else catFormatted = '"' + category + "\" -";
      }
      else catFormatted = '"' + category + "\" folder";
      cout << "menu " << catFormatted << " {" << endl;
      for (vector<int>::iterator it = positions.begin(); it < positions.end(); it++)
      { if (useIcons && files[*it].icon != "\0") nameFormatted = '"' + files[*it].name + '"' + " " + files[*it].icon;
        else nameFormatted = '"' + files[*it].name + "\" -";
        execFormatted = files[*it].exec;
        cout << "\tprog " + nameFormatted + " " + execFormatted << endl;
      }
      cout << "}\n" << endl;
      break;
  }
}

