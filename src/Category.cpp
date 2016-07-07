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
#include "Category.h"

Category::Category() {}

Category::Category(const char *dirFile, vector<string> menuFiles, bool useIcons, vector<string> iconpaths, string iconSize)
{ this->dirFile = dirFile;
  this->menuFiles = menuFiles;
  name = "\0";
  icon = "\0";
  dir_f.open(dirFile);
  if (!dir_f);
  else
  { getCategoryParams();
    getIncludedFiles();
    dir_f.close();
    if (useIcons) getCategoryIcon(iconpaths, iconSize);
  }
}

Category::Category(string name, bool useIcons, vector<string> iconpaths, string iconSize)
{ this->name = name;
  icon = "\0";
  if (useIcons) getCategoryIcon(iconpaths, iconSize);
}

Category::Category(const Category& c)
{ this->name = c.name;
  this->icon = c.icon;
  this->incEntries = c.incEntries;
  this->incEntryFiles = c.incEntryFiles;
}

Category& Category::operator=(const Category& c)
{ this->name = c.name;
  this->icon = c.icon;
  this->incEntries = c.incEntries;
  this->incEntryFiles = c.incEntryFiles;
  return *this;
}

bool Category::operator<(const Category& c)
{ if (this->name < c.name) return true;
  else return false;
}

/* A function to get the category name and icon definition */
void Category::getCategoryParams()
{ string line;

  while (!dir_f.eof())
  { getline(dir_f, line);
    string id = DesktopFile::getID(line);
    if (id == "Name") name = DesktopFile::getSingleValue(line);
    if (id == "Icon") icon = DesktopFile::getSingleValue(line);
  }
}

/* A function to loop through xdg .menu files, looking for
 * files that correspond to this category and from that file
 * learning about any desktop entry filenames that belong to
 * this category */
void Category::getIncludedFiles()
{ for (unsigned int x = 0; x < menuFiles.size(); x++)
  { menu_f.open(menuFiles[x]);
    if (!menu_f) continue;
    string line;
    vector<string> files;
    bool started = false;
    while (!menu_f.eof())
    { getline(menu_f, line);
      string id = getID(line);
      if (id == "<Directory>")
      { string dir = getSingleValue(line);
        if (dir != dirFile.substr(dirFile.find_last_of("/") + 1, dirFile.size() - dirFile.find_last_of("/") - 1))
        { menu_f.close();
          continue;
        }
      }
      if (id == "<Include>") started = true;
      if (started && id == "<Filename>") files.push_back(getSingleValue(line));
      if (id == "</Include>") started = false;
    }
    if (!files.empty()) 
      for (unsigned int x = 0; x < files.size(); x++)
        incEntryFiles.push_back(files[x]);
    menu_f.close();
  }
}

/* An xdg .menu file specific function for getting the line
 * id. In thei case, the id will be tags like <Directory>
 * or <Include> */
string Category::getID(string line)
{ vector<char> readChars;
  readChars.reserve(10);
  char c = '\0';
  unsigned int counter = 0;
  bool startFilling = false;

  while (counter < line.size())
  { c = line[counter];
    if (startFilling) readChars.push_back(c);
    if (c == '>') break;
    if (c == '<') 
    { startFilling = true;
      readChars.push_back(c);
    }
    counter++;
  }

  return string(readChars.begin(), readChars.end());
}

/* An xdg .menu file specific function for getting the line
 * value. This should be a value between two enclosing tags.
 * For instance, for the line <tag>value</tag> then this
 * should return the string "value" */
string Category::getSingleValue(string line)
{ vector<char> readChars;
  readChars.reserve(10);
  char c = '\0';
  unsigned int counter = 0;
  bool startFilling = false;

  while (counter < line.size())
  { c = line[counter];
    if (c == '>')
    { startFilling = true;
      counter++;
      continue;
    }
    if (startFilling && c == '<') break;
    if (startFilling) readChars.push_back(c);
    counter++;
  }

  return string(readChars.begin(), readChars.end());
}

/* Cycle through list of icon paths and attempt to match the category
 * name against a category icon of the appropriate size. Return null
 * character if we can't find one */
void Category::getCategoryIcon(vector<string> iconpaths, string iconSize)
{ string nameGuard = "categories";
  string iconDef;

  if (icon != "\0") iconDef = icon;
  else iconDef = name;

  /* This is a kludge. If we already have an icon definition and it is a full path instead of
   * a true definition, then there is nothing more to do so we exit here*/
  if (icon != "\0" && icon.find("/") != string::npos && icon.find(iconSize) != string::npos) return;

  if (icon != "\0") nameGuard = "/"; //If we already have a definition, don't bother with the guard

  //Workarounds
  //There is no icon for education so use the science one instead
  if (iconDef == "Education") iconDef = "Science";
  //Chromium App category has chromium-browser as its icon def but chromium does
  //not provide an icon called chromium-browser so change it to just chromium
  if (iconDef == "chromium-browser") iconDef = "chromium";

  /* The main search loop. Here we try to match the category name against icon paths, checking
   * that the word 'categories' appears somewhere in the path, as well as checking for size */
  iconDef.at(0) = tolower(iconDef.at(0));
  for (unsigned int x = 0; x < iconpaths.size(); x++)
  { if (iconpaths[x].find(iconSize) != string::npos
        && iconpaths[x].find(nameGuard) != string::npos
        && iconpaths[x].find(iconDef) != string::npos)
    { icon = iconpaths[x];
      return;
    }
  }
}
