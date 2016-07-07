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
                       string windowmanager, 
                       bool useIcons, 
                       vector<string> exclude, 
                       vector<string> excludeMatching, 
                       vector<string> excludeCategories, 
                       vector<string> include, 
                       vector<string> excludedFilenames, 
                       vector<Category> cats)
{ this->files = files;
  this->cats = cats;
  filesLength = files.size();
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

/* Handles the fetching of the vector indexes the desktop entries assigned to each category,
 * the exclusion of categories or entries based on command line arguments and then the printing
 * of the menus themselves */
void MenuWriter::printHandler()
{ int validCatsLength = cats.size();
  vector<Category> usedCats;
  vector< vector< pair<int,string> > > usedPositions;
  int usedCounter = 0;
  int maxCatNum = 0;
  int wmID = getWmID();
  int longest = getLongestNameLength();

  entryDisplayHandler();

  //First, get the usedCategories and files array indexes of the corresponding entries
  for (int x = 0; x < validCatsLength; x++)
  { vector< pair<int,string> > positions = getPositionsPerCat(cats[x]);
    if (!positions.empty() && !checkExcludedCategories(cats[x].name)) 
    { usedPositions.push_back(positions);
      usedCats.push_back(cats[x]);
      maxCatNum++;
    }
  }

  //Now write the menus
  for (int x = 0; x < maxCatNum; x++)
  { writeMenu(usedPositions[x], usedCats[x], wmID, usedCounter, maxCatNum - 1, longest, usedCats);
    usedCounter++;
  }
  //For WMs which require a menu which sources the individual category menus
  if (wmID == mwm && !usedCats.empty()) writeMenu(vector< pair<int,string> >(), Category(), wmID, usedCounter, maxCatNum - 1, longest, usedCats);
  if (wmID == fvwm && !usedCats.empty()) writeMenu(vector< pair<int,string> >(), Category(), wmID, usedCounter, maxCatNum - 1, longest, usedCats);
}

/* This function is used to determine which entries should be printed for a given category.
 * It returns a vector of pairs where each pair contains the index of the DesktopFile
 * object in the DesktopFile array and the name of the entry. The name is collected only so
 * that we can alphabetically sort the menu entries */
vector< pair<int,string> > MenuWriter::getPositionsPerCat(Category category)
{ vector< pair<int,string> > positions;
  positions.reserve(20);

  for (int x = 0; x < filesLength; x++)
  { if (find(category.incEntries.begin(), category.incEntries.end(), files[x].name) != category.incEntries.end() 
        && files[x].nodisplay != true)
    { pair<int,string> p(x, files[x].name);
      positions.push_back(p);
    }
  }

  return positions;
}

/* Function to filter out desktop entries specified from the command line
 * based on various criteria. The entries are excluded simply be setting the
 * nodisplay value to true */
void MenuWriter::entryDisplayHandler()
{ if (!exclude.empty())
  { for (int x = 0; x < filesLength; x++)
      if (find(exclude.begin(), exclude.end(), files[x].name) != exclude.end()) files[x].nodisplay = true;
  }
  if (!excludeMatching.empty())
  { for (int x = 0; x < filesLength; x++)
    { for (unsigned int y = 0; y < excludeMatching.size(); y++)
      { if (files[x].name.find(excludeMatching[y]) != string::npos)
        { files[x].nodisplay = true;
          break;
        }
      }
    }
  }
  if (!excludedFilenames.empty())
  { for (int x = 0; x < filesLength; x++)
      if (find(excludedFilenames.begin(), excludedFilenames.end(), files[x].filename) != excludedFilenames.end()) files[x].nodisplay = true;
  }
  if (!include.empty())
  { for (int x = 0; x < filesLength; x++)
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

  for (int x = 0; x < filesLength; x++)
  { /* Most wm menus seem to define an icon path after the entry name so for the longest
     * name we need to add the length of the icon path on. However Fluxbox defines the
     * icon path after the exec so we don't want to add that extra length to the name in
     * this case */
    if (useIcons && windowmanager != "Fluxbox")
    { if (files[x].name.size() + files[x].icon.size() > longest) longest = files[x].name.size() + files[x].icon.size(); }
    else
    { if (files[x].name.size() > longest) longest = files[x].name.size(); }
  }

  return longest + 10;
}

//Return an integer identifying which window manager, menus should be produced for
int MenuWriter::getWmID()
{ if (windowmanager == "FVWM") return fvwm;
  if (windowmanager == "Fluxbox") return fluxbox;
  if (windowmanager == "Openbox") return openbox;
  if (windowmanager == "Olvwm") return olvwm;
  if (windowmanager == "Windowmaker") return windowmaker;
  if (windowmanager == "IceWM") return icewm;
  else return mwm;
}

/* This function is called multiple times. Each time, it prints out the submenu
 * for a given category. It might also print out the 'main' menu if the wm requires
 * it. Currently, only MWM and FVWM use this. The main menu code is called if the
 * category string is "\0" */
void MenuWriter::writeMenu(vector< pair<int,string> > positions, Category cat, int wmID, int catNumber, int maxCatNumber, int longest, vector<Category> usedCats)
{ string category = cat.name;
  string catIcon = cat.icon;
  string entryName;
  string entryExec;
  string catName;
  string menuNameWithQuotes;
  string catNameWithQuotes;

  switch(wmID)
  { case mwm :
      /* FIXME: code for the category menus and main menus should be integrated to
       * avoid duplication */
      if (category != "\0")
      { catName = '"' + category + '"';
	cout << "menu " << catName << endl << "{" << endl;
	cout << "\t" << setw(longest) << left << catName << "\t" << "f.title" << endl;
	for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
	{ entryName = '"' + files[it->first].name + '"';
	  entryExec = "\"exec " + files[it->first].exec + " &" + '"';
	  cout << "\t" << setw(longest) << left << entryName << "\t" << "f.exec " << entryExec << endl;
	}
	cout << "}" << endl << endl;
      }
      else
      { menuNameWithQuotes = '"' + menuName + '"';
	cout << "menu " << menuNameWithQuotes << endl << "{" << endl;
	cout << "\t" << setw(longest) << left << menuNameWithQuotes << "\t" << "f.title" << endl;
	for (int x = 0; x < catNumber; x++)
	{ catNameWithQuotes = '"' + string(usedCats[x].name) + '"';
	  cout << "\t" << setw(longest) << left << catNameWithQuotes << "\t" << "f.menu  " << catNameWithQuotes << endl;
	}
	cout << "}" << endl << endl;
      }
      break;
    case fvwm :
      /* FIXME: code for the category menus and main menus should be integrated to
       * avoid duplication */
      if (category != "\0")
      { catName = '"' + category + '"';
	cout << "AddToMenu " << setw(15) << left << catName << "\t" << setw(longest) << left << catName << "\tTitle" << endl;
	for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
	{ if (useIcons && files[it->first].icon != "\0") entryName = '"' + files[it->first].name + " %" + files[it->first].icon + "%" + '"';
	  else entryName = '"' + files[it->first].name + '"';
	  entryExec = files[it->first].exec;
	  cout << "+\t\t\t\t" << setw(longest) << left << entryName << "\t" << "Exec " << entryExec << endl;
	}
	cout << endl;
      }
      else
      { menuNameWithQuotes = '"' + menuName + '"';
	cout << "AddToMenu " << setw(15) << left << menuNameWithQuotes << "\t" << setw(longest) << left << menuNameWithQuotes << "\tTitle" << endl;
	for (int x = 0; x < catNumber; x++)
	{ if (useIcons)
	  { catIcon = usedCats[x].icon;
            if (catIcon != "\0") catNameWithQuotes = '"' + string(usedCats[x].name) + " %" + catIcon + "%" + '"';
	    else catNameWithQuotes = '"' + string(usedCats[x].name) + '"';
	  }
	  else catNameWithQuotes = '"' + string(usedCats[x].name) + '"';
	  cout << "+\t\t\t\t" << setw(longest) << left << catNameWithQuotes << "\t" << "Popup  " << '"' + usedCats[x].name + '"' << endl;
	}
        cout << endl;
      }
      break;
    case fluxbox :
      if (catNumber == 0) cout << "[submenu] (" << menuName << ')' << endl;
      if (useIcons)
      { if (catIcon != "\0") catName = '(' + category + ") <" + catIcon + '>';
        else catName = '(' + category + ')';
      }
      else catName = '(' + category + ')';
      cout << "\t[submenu] " + catName + " {}" << endl;
      for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
      { if (useIcons && files[it->first].icon != "\0") entryExec = '{' + files[it->first].exec + "} <" + files[it->first].icon + ">";
        else entryExec = '{' + files[it->first].exec + '}';
        entryName = files[it->first].name;
        //If a name has brackets, we need to escape the closing bracket or it will be missed out
        if (entryName.find(string(")").c_str()) != string::npos) entryName.insert(static_cast<int>(entryName.find_last_of(')')), string("\\").c_str());
        entryName = '(' + entryName + ')';
        cout << "\t\t[exec] " << setw(longest) << left << entryName << " " << entryExec << endl;
      }
      cout << "\t[end]" << endl;
      if (catNumber == maxCatNumber) cout << "[end]" << endl;
      break;
    case openbox :
      if (catNumber == 0) cout << "<openbox_pipe_menu xmlns=\"http://openbox.org/3.4/menu\">" << endl << endl;
      catName = '"' + category + '"';
      if (useIcons)
      { if (catIcon != "\0") cout << "<menu id=" << catName << " label=" << catName << " icon=" << '"' + catIcon + '"' << ">" << endl;
        else cout << "<menu id=" << catName << " label=" << catName << ">" << endl;
      }
      else cout << "<menu id=" << catName << " label=" << catName << ">" << endl;
      for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
      { if (useIcons && files[it->first].icon != "\0") 
          entryName = '"' + files[it->first].name + '"' + " icon=\"" + files[it->first].icon + "\">";
        else entryName = '"' + files[it->first].name + "\">";
        entryExec = files[it->first].exec;
        cout << "\t<item label=" << setw(longest) << left << entryName << endl;
        cout << "\t\t<action name=\"Execute\">" << endl;
        cout << "\t\t\t<execute>" << entryExec << "</execute>" << endl;
        cout << "\t\t</action>" << endl;
        cout << "\t</item>" << endl;
      }
      cout << "</menu>" << endl << endl;
      if (catNumber == maxCatNumber) cout << "</openbox_pipe_menu>" << endl << endl;
      break;
    case olvwm :
      if (catNumber == 0) cout << setw(longest) << left << '"' + menuName + '"' << "MENU" << endl << endl;
      catName = '"' + category + '"';
      cout << setw(longest) << left << catName << "MENU" << endl;
      for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
      { entryName = '"' + files[it->first].name + '"';
        entryExec = files[it->first].exec;
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
      { entryName = '"' + files[it->first].name + '"';
        entryExec = '"' + files[it->first].exec + '"';
        cout << "    (" << entryName << ", " << "EXEC, " << entryExec << ")";
        if ((it - positions.begin()) != (positions.end() - positions.begin() - 1)) cout << ',' << endl;
        else cout << endl;
      }
      if (catNumber != maxCatNumber) cout << "  )," << endl;
      else cout << "  )\n)" << endl;
      break;
    case icewm :
      if (useIcons)
      { if (catIcon != "\0") catName = '"' + category + "\" " + catIcon;
        else catName = '"' + category + "\" -";
      }
      else catName = '"' + category + "\" folder";
      cout << "menu " << catName << " {" << endl;
      for (vector< pair<int,string> >::iterator it = positions.begin(); it < positions.end(); it++)
      { if (useIcons && files[it->first].icon != "\0") entryName = '"' + files[it->first].name + '"' + " " + files[it->first].icon;
        else entryName = '"' + files[it->first].name + "\" -";
        entryExec = files[it->first].exec;
        cout << "\tprog " + entryName + " " + entryExec << endl;
      }
      cout << "}\n" << endl;
      break;
  }
}

