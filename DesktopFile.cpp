#include <algorithm>
#include <set>
#include <string.h>
#include "DesktopFile.h"

DesktopFile::DesktopFile() {}

DesktopFile::DesktopFile(const char *filename) 
{ dfile.open(filename);
  this->name = "\0";
  this->exec = "\0";
  this->categories = vector<string>();
  this->nodisplay = false;
  if (!dfile);
  else
  { populate();
    close();
  }
}

void DesktopFile::close() { dfile.close(); }

void DesktopFile::populate()
{ string line;
  bool started = false;

  while (!dfile.eof())
  { getline(dfile, line);
    if (strlen(line.c_str()) == 0) continue;
    string id = getID(line);
    if (strcmp(id.c_str(), "[Desktop Entry]") == 0)
    { started = true;
      continue;
    }
    if (id[0] == '[' && started == true) break;
    if (strcmp(id.c_str(), "Name") == 0)
    { this->name = '"' + getSingleValue(line) + '"';
      continue;
    }
    if (strcmp(id.c_str(), "Exec") == 0)
    { this->exec = '"' + getSingleValue(line) + '"';
      continue;
    }
    if (strcmp(id.c_str(), "Categories") == 0)
    { vector<string> categories = getMultiValue(line);
      processCategories(categories);
      this->categories = categories;
      continue;
    }
    if (this->categories.size() == 0) this->categories.push_back("Other");
    if (strcmp(id.c_str(), "NoDisplay") == 0)
    { string value = getSingleValue(line);
      if (strcmp(value.c_str(), "True") == 0 || strcmp(value.c_str(), "true") == 0)
        this->nodisplay = true;
    }
  }
}

string DesktopFile::getID(string line)
{ char readChars[line.size() + 1] = {'\0'};
  char c = '\0';
  int counter = 0;

  while (counter < line.size())
  { c = line[counter];
    if (c == '=') break;
    readChars[counter] = c;
    counter++;
  }

  return readChars;
}

string DesktopFile::getSingleValue(string line)
{ char readChars[line.size() + 1] = {'\0'};
  string value;
  char c = '\0';
  bool startFilling = false;
  int counter = 0;
  int fillCounter = 0;

  while (counter < line.size())
  { c = line[counter];
    if (startFilling) 
    { readChars[fillCounter] = c;
      fillCounter++;
    }
    if (c == '=') startFilling = true;
    counter++;
  }

  value = readChars;
  string::iterator fieldCode = find(value.begin(), value.end(), '%');
  if (fieldCode != value.end()) value.erase(fieldCode - 1, value.end());

  return value;
}

vector<string> DesktopFile::getMultiValue(string line)
{ vector<string> values;
  values.reserve(10);
  char readChars[line.size() + 1] = {'\0'};
  char c = '\0';
  bool startFilling = false;
  int counter = 0;
  int fillCounter = 0;

  for (int x = 0; x < line.size(); x++)
  { c = line[x];
    if (startFilling && c != ';') 
    { readChars[fillCounter] = c;
      fillCounter++;
    }
    if (c == '=') startFilling = true;
    if (c == ';')
    { values.push_back(readChars);
      fill(readChars, readChars + line.size() + 1, '\0');
      fillCounter = 0;
      continue;
    }
    counter++;
  }

  return values;
}

void DesktopFile::processCategories(vector<string> &categories)
{ const char *baseCatsArr[] = {"AudioVideo", "Audio", "Video", "Development", "Education", "Game", "Graphics", 
                               "Network", "Office", "Science", "Settings", "System", "Utility"};
  vector<string> baseCategories(baseCatsArr, baseCatsArr + sizeof(baseCatsArr) / sizeof(baseCatsArr[0]));
  vector<string>::iterator it = categories.begin();
  bool noCategory = true;

  while (it < categories.end())
  { if (*it == "AudioVideo" || *it == "Audio" || *it == "Video") 
    { *it = "Multimedia";
      ++it;
      continue;
    }
    if (find(baseCategories.begin(), baseCategories.end(), *it) == baseCategories.end()) it = categories.erase(it);
    else ++it;
  }

  set<string> temp(categories.begin(), categories.end());
  categories.assign(temp.begin(), temp.end());

  it = categories.begin();
  while (it < categories.end())
  { if (find(baseCategories.begin(), baseCategories.end(), *it) != baseCategories.end() || *it == "Multimedia")
    { noCategory = false;
      break;
    }
  }
  if (noCategory) categories.push_back("Other");
}
