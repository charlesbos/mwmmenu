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
#include "Categories.h"

Categories::Categories() {}

Categories::Categories(const char *dirFile, vector<string> menuFiles)
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
  }
}

/* A function to get the category name and icon definition */
void Categories::getCategoryParams()
{ string line;

  while (!dir_f.eof())
  { getline(dir_f, line);
    string id = DesktopFile::getID(line);
    if (id == "Name") name = DesktopFile::getSingleValue(line);
    if (id == "Icon") icon = DesktopFile::getSingleValue(line);
  }

  /* Some wm's don't like category names with spaces and some
   * picked up categories will have spaces - looking at you 
   * 'Chromium Apps' !!! */
  replace(name.begin(), name.end(), ' ', '_');
}

/* A function to loop through xdg .menu files, looking for
 * files that correspond to this category and from that file
 * learning about any desktop entry filenames that belong to
 * this category */
void Categories::getIncludedFiles()
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
        incEntries.push_back(files[x]);
  }
}

/* An xdg .menu file specific function for getting the line
 * id. In thei case, the id will be tags like <Directory>
 * or <Include> */
string Categories::getID(string line)
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
string Categories::getSingleValue(string line)
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
