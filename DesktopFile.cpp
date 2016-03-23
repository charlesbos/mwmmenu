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
#include <set>
#include <string.h>
#include "DesktopFile.h"

DesktopFile::DesktopFile() {}

DesktopFile::DesktopFile(const char *filename, bool hideOSI, bool useIcons, vector<string> iconpaths) 
{ dfile.open(filename);
  this->name = "\0";
  this->exec = "\0";
  this->categories = vector<string>();
  this->nodisplay = false; //Always assume entries are displayed unless entry specifies otherwise
  this->onlyShowIn = false; //Assume false but if we find OnlyShowIn make it true
  this->useIcons = useIcons;
  this->iconpaths = iconpaths;
  this->icon = "\0";
  this->iconDef = "\0";
  this->filename = filename;
  if (!dfile); //If we cannot open the file, do nothing. The object will keep its initial values
  else
  { populate(hideOSI);
    dfile.close();
  }
}

/* This function fetches the required values (Name, Exec, Categories and NoDisplay)
 * and then assigns the results to the appropriate instance variables */
void DesktopFile::populate(bool hideOSI)
{ string line;
  bool started = false;

  while (!dfile.eof())
  { getline(dfile, line);
    if (strlen(line.c_str()) == 0) continue;
    string id = getID(line);
    /* .desktop files can contain more than just desktop entries. On getting
     * the id [Desktop Entry] we know we've started looking at an entry */
    if (strcmp(id.c_str(), "[Desktop Entry]") == 0)
    { started = true;
      continue;
    }
    /* If we get another line beginning with [, it probably means we've found
     * a desktop action as opposed to a desktop entry. We should break here 
     * to avoid the entry data being overwritten with action data */
    if (id[0] == '[' && started == true) break;
    if (strcmp(id.c_str(), "Name") == 0)
    { name = getSingleValue(line);
      continue;
    }
    if (strcmp(id.c_str(), "Exec") == 0)
    { exec = getSingleValue(line);
      continue;
    }
    if (strcmp(id.c_str(), "Categories") == 0)
    { categories = getMultiValue(line);
      continue;
    }
    if (strcmp(id.c_str(), "NoDisplay") == 0)
    { string value = getSingleValue(line);
      if (strcmp(value.c_str(), "True") == 0 || strcmp(value.c_str(), "true") == 0)
        nodisplay = true;
      continue;
    }
    if (strcmp(id.c_str(), "OnlyShowIn") == 0)
    { if (hideOSI) onlyShowIn = true;
      continue;
    }
    if (strcmp(id.c_str(), "Icon") == 0)
    { iconDef = getSingleValue(line);
      continue;
    }
  }

  processCategories(categories);
  if (useIcons) matchIcon();
}

/* This function is used to get the single value before the = sign.
 * Should be Name, Exec, Categories etc */
string DesktopFile::getID(string line)
{ char readChars[line.size() + 1] = {'\0'};
  char c = '\0';
  unsigned int counter = 0;

  while (counter < line.size())
  { c = line[counter];
    if (c == '=') break;
    readChars[counter] = c;
    counter++;
  }

  return readChars;
}

/* This function is used to get single values, for example: for the line 
 * Name=Firefox this function will return Firefox */
string DesktopFile::getSingleValue(string line)
{ char readChars[line.size() + 1] = {'\0'};
  string value;
  char c = '\0';
  bool startFilling = false;
  unsigned int counter = 0;
  int fillCounter = 0;

  while (counter < line.size())
  { c = line[counter];
    if (startFilling) 
    { readChars[fillCounter] = c;
      fillCounter++;
    }
    //Make sure we only get the value, not the id as well
    if (c == '=') startFilling = true;
    counter++;
  }

  //Throw away field codes like %F, MWM doesn't appear to handle these
  value = readChars;
  string::iterator fieldCode = find(value.begin(), value.end(), '%');
  if (fieldCode != value.end()) value.erase(fieldCode - 1, value.end());

  return value;
}

/* This function is used to get multiple semi-colon seperated values.
 * For instance, the line Categories=System;Settings; will return a string
 * vector with the items System and Settings */
vector<string> DesktopFile::getMultiValue(string line)
{ vector<string> values;
  values.reserve(10);
  char readChars[line.size() + 1] = {'\0'};
  char c = '\0';
  bool startFilling = false;
  int counter = 0;
  int fillCounter = 0;

  for (unsigned int x = 0; x < line.size(); x++)
  { c = line[x];
    if (startFilling && c != ';') //Add chars to our buffer array, but avoid the semi-colons
    { readChars[fillCounter] = c;
      fillCounter++;
    }
    if (c == '=') startFilling = true;
    if (c == ';') //Categories are seperated with semi-colons so add to vector when we encounter them
    { values.push_back(readChars);
      fill(readChars, readChars + line.size() + 1, '\0');
      fillCounter = 0;
      continue;
    }
    counter++;
  }

  return values;
}

/* This function is used to strip out categories that are not supposed to be
 * displayed in menus, to group multiple multimedia categories into one,
 * to strip out possible duplicates and to add a catchall category if no
 * base categories are present */
void DesktopFile::processCategories(vector<string> &categories)
{ const char *baseCatsArr[] = {"AudioVideo", "Audio", "Video", "Development", "Education", "Game", "Graphics", 
                               "Network", "Office", "Science", "Settings", "System", "Utility"};
  vector<string> baseCategories(baseCatsArr, baseCatsArr + sizeof(baseCatsArr) / sizeof(baseCatsArr[0]));
  vector<string>::iterator it = categories.begin();
  bool noCategory = true;

  while (it < categories.end())
  { //Convert some base categories to more commonly used categories
    if (*it == "AudioVideo" || *it == "Audio" || *it == "Video") 
    { *it = "Multimedia";
      ++it;
      continue;
    }
    if (*it == "Network")
    { *it = "Internet";
      ++it;
      continue;
    }
    if (*it == "Utility")
    { *it = "Accessories";
      ++it;
      continue;
    }
    //Throw away non-base categories
    if (find(baseCategories.begin(), baseCategories.end(), *it) == baseCategories.end()) it = categories.erase(it);
    else ++it;
  }

  //Convert to set and back again to remove duplicates
  set<string> temp(categories.begin(), categories.end());
  categories.assign(temp.begin(), temp.end());

  //If an entry ends up with no categories, give the entry the catchall category
  it = categories.begin();
  while (it < categories.end())
  { if (find(baseCategories.begin(), baseCategories.end(), *it) != baseCategories.end() || *it == "Multimedia" || *it == "Internet" || *it == "Accessories")
    { noCategory = false;
      break;
    }
  }
  if (noCategory) categories.push_back("Other");
}

/* Function which attempts to find the full path for a desktop entry by going
 * through a list of icons, attempting to match the icon entry in the entry
 * against each icon path */
void DesktopFile::matchIcon()
{ for (unsigned int x = 0; x < iconpaths.size(); x++)
  { string iconName = iconpaths[x].substr(iconpaths[x].find_last_of("/") + 1, iconpaths[x].find_last_of(".") - iconpaths[x].find_last_of("/") - 1);
    if (iconDef == iconName)
    { icon = iconpaths[x];
      break;
    }
  }
}
  
