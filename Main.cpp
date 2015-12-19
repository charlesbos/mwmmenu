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

#include <iostream>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "boost/filesystem.hpp"
#include "DesktopFile.h"
#include "MenuWriter.h"

#define NUMBER_OF_DIRS 3 //If more search dirs are added, increase by 1 for each dir
#define NUMBER_OF_ICON_DIRS 3 //Likewise, increase for each search dir

void usage()
{ cout << "mwmmenu - a program to produce application menus for the MWM window manager." << endl << endl;
  cout << "The menus will be printed out into the console. To use them, paste them into" << endl;
  cout << "your ~/.mwmrc file and then add an entry for the main menu into the root menu" << endl;
  cout << "defined in ~/.mwmrc" << endl << endl;
  cout << "The MWM menus should also be compatible with TWM." << endl << endl;
  cout << "Support for FVWM is now also available and other window managers may be" << endl;
  cout << "supported at a later date." << endl << endl;
  cout << "Usage:" << endl;
  cout << "  mwmmenu [OPTIONS]" << endl << endl;
  cout << "Options:" << endl;
  cout << "  -h, --help: show this dialogue" << endl;
  cout << "  -n: name used for the main menu - by default, use 'Applications'" << endl;
  cout << "  -o: display entries with the OnlyShowIn key, false by default" << endl;
  cout << "  -i: use icons with menu entries, only compatible with some window managers" << endl;
  cout << "  -fvwm: produce menus for FVWM instead of MWM/TWM" << endl;
}

//Function that attempts to get the user icon theme from ~/.gtkrc-2.0
string getIconTheme(string homedir)
{ ifstream themefile;
  string path = homedir + "/.gtkrc-2.0";
  string id = "gtk-icon-theme-name";
  themefile.open(path.c_str());
  if (!themefile) return "\0";
  else
  { string line;
    char c = '\0';
    unsigned int counter = 0;
    while (!themefile.eof())
    { getline(themefile, line);
      char readChars[line.size() + 1] = {'\0'};
      while (counter < line.size())
      { c = line[counter];
        if (c == '=') break;
        readChars[counter] = c;
        counter++;
      }
      if (strcmp(id.c_str(), readChars) == 0)
      { char themeNameChars[line.size() + 1] = {'\0'};
        int selector = 0;
        while (counter < line.size())
        { c = line[counter];
          if (c != '"' && c != '=')
          { themeNameChars[selector] = c;
            selector++;
          }
          counter ++;
        }
        themefile.close();
        return string(themeNameChars);
      }
      counter = 0;
    }
    themefile.close();
    return "\0";
  }
}

int main(int argc, char *argv[])
{ //Handle args
  string homedir = getenv("HOME");
  string menuName;
  bool displayOSI = false;
  string windowmanager = "MWM";
  bool useIcons = false;
  for (int x = 0; x < argc; x++)
  { if (strcmp(argv[x], "-h") == 0 || strcmp(argv[x], "--help") == 0)
    { usage();
      return 0; 
    }
    if (strcmp(argv[x], "-n") == 0) 
    { menuName = argv[x + 1];
      continue;
    }
    if (strcmp(argv[x], "-o") == 0)
    { displayOSI = true;
      continue;
    }
    if (strcmp(argv[x], "-i") == 0)
    { useIcons = true;
      continue;
    }
    if (strcmp(argv[x], "-fvwm") == 0) 
    { windowmanager = "FVWM";
      continue;
    }
  }
  if (menuName.size() == 0) menuName = "Applications";
  if (windowmanager == "MWM") useIcons = false;

  //Get string vector of paths to .desktop files
  vector<string> paths;
  paths.reserve(300);
  int counter = 0;
  string appdirs[NUMBER_OF_DIRS] = {"/usr/share/applications/", "/usr/local/share/applications/"};
  if (homedir.c_str() != NULL) appdirs[NUMBER_OF_DIRS - 1] = homedir + "/.local/share/applications/";
  else appdirs[NUMBER_OF_DIRS - 1] = '\0';
  dirent *f;
  for (int x = 0; x < NUMBER_OF_DIRS; x++)
  { DIR *d = opendir(appdirs[x].c_str());
    if (d != NULL)
    { while ((f = readdir(d)) != NULL)
      { string file = f->d_name;
        string fullPath = appdirs[x] + file;
        if (fullPath.find(".desktop") != string::npos)
        { paths.push_back(fullPath);
          counter++;
        }
      }
    }
  }

  /* Get vector of string pairs. Each pair contains the icon filename and the
   * full path to the icon */
  vector<string> iconpaths;
  if (useIcons)
  { iconpaths.reserve(500);
    string icondirs[NUMBER_OF_ICON_DIRS] = {"/usr/share/icons/hicolor"};
    if (homedir.c_str() != NULL)
    { string themename = getIconTheme(homedir); 
      icondirs[NUMBER_OF_ICON_DIRS - 2] = "/usr/share/icons/" + themename;
      if (icondirs[NUMBER_OF_ICON_DIRS - 2] != "/usr/share/icons/gnome") icondirs[NUMBER_OF_ICON_DIRS - 1] = "/usr/share/icons/gnome";
      else icondirs[NUMBER_OF_ICON_DIRS - 1] = "\0";
    }
    else icondirs[NUMBER_OF_DIRS - 1] = '\0';
    for (int x = 0; x < NUMBER_OF_ICON_DIRS; x++)
    { try
      { for (boost::filesystem::recursive_directory_iterator i(icondirs[x]), end; i != end; ++i)
        { if (!is_directory(i->path()))
          { string ipath = i->path().string();
            if (ipath.find("16x16") != string::npos)
              iconpaths.push_back(i->path().string());
          }
        }
      }
      catch (boost::filesystem::filesystem_error) { continue; }
    }
  }

  //Create array of DesktopFile, using each path in the paths vector
  DesktopFile *files[counter];
  counter = 0;
  for (vector<string>::iterator it = paths.begin(); it < paths.end(); it++)
  { DesktopFile *df = new DesktopFile((*it).c_str(), displayOSI, useIcons, iconpaths);
    /* If a name or exec wasn't found we cannot add an entry to our menu so ignore
     * these objects */
    if (df->name != "\0" && df->exec != "\0")
    { files[counter] = df;
      counter++;
    }
  }

  //Create MwmMenuWriter object, passing it the array of DesktopFile
  //This object will cause the menus to be printed
  new MenuWriter(files, counter, menuName, windowmanager, useIcons, iconpaths);

  return 0;
}
  
