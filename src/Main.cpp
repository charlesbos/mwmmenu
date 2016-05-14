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

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "boost/filesystem.hpp"
#include "DesktopFile.h"
#include "MenuWriter.h"

void usage()
{ cout << "mwmmenu - creates application menus for MWM and other window managers." << endl << endl;
  cout << "Usage:" << endl;
  cout << "  # Note: all options must be spaced." << endl;
  cout << "  mwmmenu [OPTIONS]" << endl << endl;
  cout << "Options:" << endl;
  cout << "  -h, -help: show this dialogue" << endl;
  cout << "  -n, -name: name used for the main menu - by default, use Applications" << endl;
  cout << "  -o, -only_show: hide entries with the OnlyShowIn key, false by default" << endl;
  cout << "  -i, -icons: use icons with menu entries, only compatible with some window managers" << endl << endl;
  cout << "  -icon_size: choose size of icons used in menus. Should be a value such as 16x16, 32x32 etc. Can also be scalable or all. Large icon sizes or all sizes should be used only in window managers which can scale icons down to an appropriate size. The default is 16x16." << endl << endl;
  cout << "  # Note: any names to be excluded that contain spaces must have quotes." << endl << endl;
  cout << "  -exclude: do not add desktop entries that have the names specified. Multiple names should be separated by commas, for instance: -exclude Firefox,XTerm,LibreOffice" << endl << endl;
  cout << "  -exclude_matching: do not add desktop entries where the entry name contains one of the strings specified. Multiple entries should be separated by commas, for instance -exclude_matching Term,Office" << endl << endl;
  cout << "  -exclude_categories: do not print category menus for the given strings. Multiple values should be separated by commas, for instance: -exclude_categories Internet,System" << endl << endl;
  cout << "  -add_desktop_paths: add extra search paths for desktop entries. Multiple paths should be separated by commas." << endl << endl;
  cout << "  -add_icon_paths: add extra search paths for icons. Multiple paths should be separated by commas." << endl << endl;
  cout << "Menu format options:" << endl;
  cout << "  # No format argument: produce menus for MWM" << endl;
  cout << "  -twm: produce menus for TWM" << endl;
  cout << "  -fvwm: produce menus for FVWM" << endl;
  cout << "  -fluxbox: produce menus for Fluxbox" << endl;
  cout << "  -openbox: produce menus for Openbox" << endl;
  cout << "  -blackbox: produce menus for Blackbox" << endl;
  cout << "  -olvwm: produce menus for Olvwm" << endl;
  cout << "  -windowmaker: produce menus for Windowmaker" << endl;
  cout << "  -icewm: produce menus for IceWM" << endl;
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
      vector<char> readChars;
      readChars.reserve(10);
      while (counter < line.size())
      { c = line[counter];
        if (c == '=') break;
        readChars.push_back(c);
        counter++;
      }
      string read_id = string(readChars.begin(), readChars.end());
      read_id.erase(remove(read_id.begin(), read_id.end(), ' '), read_id.end());
      if (id == read_id)
      { vector<char> themeNameChars;
        themeNameChars.reserve(10);
        while (counter < line.size())
        { c = line[counter];
          if (c != '"' && c != '=') themeNameChars.push_back(c);
          counter ++;
        }
        themefile.close();
        string themename = string(themeNameChars.begin(), themeNameChars.end());
        themename.erase(remove(themename.begin(), themename.end(), ' '), themename.end());
        return themename;
      }
      counter = 0;
    }
    themefile.close();
    return "\0";
  }
}

/* Function to split string argument separated by commas into
 * a string vector */
vector<string> splitCommaArgs(string arg)
{ vector<string> splitArgs;
  vector<char> buffer;
  splitArgs.reserve(5);
  buffer.reserve(10);

  for (unsigned int x = 0; x < arg.size(); x++)
  { if (arg[x] == ',') 
    { splitArgs.push_back(string(buffer.begin(), buffer.end()));
      buffer.clear();
      continue;
    }
    buffer.push_back(arg[x]);
  }
  if (!buffer.empty()) splitArgs.push_back(string(buffer.begin(), buffer.end()));

  return splitArgs;
}

int main(int argc, char *argv[])
{ //Handle args
  string homedir = getenv("HOME");
  string menuName;
  bool hideOSI = false;
  string windowmanager = "MWM";
  bool useIcons = false;
  string iconSize = "16x16";
  string exclude = "\0";
  string excludeMatching = "\0";
  string excludeCategories = "\0";
  string extraDesktopPaths = "\0";
  string extraIconPaths = "\0";

  for (int x = 0; x < argc; x++)
  { if (strcmp(argv[x], "-h") == 0 || strcmp(argv[x], "-help") == 0)
    { usage();
      return 0; 
    }
    if (strcmp(argv[x], "-n") == 0 || strcmp(argv[x], "-name") == 0) 
    { menuName = argv[x + 1];
      continue;
    }
    if (strcmp(argv[x], "-o") == 0 || strcmp(argv[x], "-only_show") == 0)
    { hideOSI = true;
      continue;
    }
    if (strcmp(argv[x], "-i") == 0 || strcmp(argv[x], "-icons") == 0)
    { useIcons = true;
      continue;
    }
    if (strcmp(argv[x], "-icon_size") == 0) 
    { iconSize = argv[x + 1];
      continue;
    }
    if (strcmp(argv[x], "-fvwm") == 0) 
    { windowmanager = "FVWM";
      continue;
    }
    if (strcmp(argv[x], "-twm") == 0) 
    { windowmanager = "TWM";
      continue;
    }
    if (strcmp(argv[x], "-fluxbox") == 0) 
    { windowmanager = "Fluxbox";
      continue;
    }
    if (strcmp(argv[x], "-openbox") == 0) 
    { windowmanager = "Openbox";
      continue;
    }
    if (strcmp(argv[x], "-blackbox") == 0) 
    { windowmanager = "Blackbox";
      continue;
    }
    if (strcmp(argv[x], "-olvwm") == 0) 
    { windowmanager = "Olvwm";
      continue;
    }
    if (strcmp(argv[x], "-windowmaker") == 0) 
    { windowmanager = "Windowmaker";
      continue;
    }
    if (strcmp(argv[x], "-icewm") == 0) 
    { windowmanager = "IceWM";
      continue;
    }
    if (strcmp(argv[x], "-exclude") == 0) 
    { exclude = argv[x + 1];
      continue;
    }
    if (strcmp(argv[x], "-exclude_matching") == 0) 
    { excludeMatching = argv[x + 1];
      continue;
    }
    if (strcmp(argv[x], "-exclude_categories") == 0) 
    { excludeCategories = argv[x + 1];
      continue;
    }
    if (strcmp(argv[x], "-add_desktop_paths") == 0) 
    { extraDesktopPaths = argv[x + 1];
      continue;
    }
    if (strcmp(argv[x], "-add_icon_paths") == 0) 
    { extraIconPaths = argv[x + 1];
      continue;
    }
  }
  if (menuName.size() == 0) menuName = "Applications";
  if (windowmanager == "MWM" || 
      windowmanager == "TWM" || 
      windowmanager == "Blackbox" || 
      windowmanager == "Olvwm" ||
      windowmanager == "Windowmaker") 
    useIcons = false;
  if (iconSize == "all") iconSize = "/"; //All paths will have forward slashes so this makes the check null and void

  //Get string vector of paths to .desktop files
  vector<string> paths;
  paths.reserve(300);
  vector<string> appdirs = {"/usr/share/applications/", "/usr/local/share/applications/"};
  if (homedir.c_str() != NULL) appdirs.push_back(homedir + "/.local/share/applications/");
  if (extraDesktopPaths != "\0")
  { vector<string> newDPaths = splitCommaArgs(extraDesktopPaths);
    for (unsigned int x = 0; x < newDPaths.size(); x++)
      appdirs.push_back(newDPaths[x]);
  }
  for (unsigned int x = 0; x < appdirs.size(); x++)
  { try
    { for (boost::filesystem::recursive_directory_iterator i(appdirs[x]), end; i != end; ++i)
        if (!is_directory(i->path())) paths.push_back(i->path().string());
    }
    catch (boost::filesystem::filesystem_error) { continue; }
  }

  /* Get vector of string pairs. Each pair contains the icon filename and the
   * full path to the icon */
  vector<string> iconpaths;
  if (useIcons)
  { iconpaths.reserve(500);
    vector<string> icondirs = {"/usr/share/icons/hicolor", "/usr/share/pixmaps", "/usr/local/share/pixmaps"};
    if (homedir.c_str() != NULL)
    { string themename = getIconTheme(homedir); 
      icondirs.push_back("/usr/share/icons/" + themename);
      if (find(icondirs.begin(), icondirs.end(), "/usr/share/icons/gnome") != icondirs.end()) icondirs.push_back("/usr/share/icons/gnome");
    }
    if (extraIconPaths != "\0")
    { vector<string> newIPaths = splitCommaArgs(extraIconPaths);
      for (unsigned int x = 0; x < newIPaths.size(); x++)
        icondirs.push_back(newIPaths[x]);
    }
    for (unsigned int x = 0; x < icondirs.size(); x++)
    { try
      { for (boost::filesystem::recursive_directory_iterator i(icondirs[x]), end; i != end; ++i)
        { if (!is_directory(i->path()))
          { string ipath = i->path().string();
            if (ipath.find(iconSize) != string::npos)
              iconpaths.push_back(i->path().string());
          }
        }
      }
      catch (boost::filesystem::filesystem_error) { continue; }
    }
  }

  //Create array of DesktopFile, using each path in the paths vector
  DesktopFile **files = new DesktopFile*[paths.size()];
  int counter = 0;
  for (vector<string>::iterator it = paths.begin(); it < paths.end(); it++)
  { DesktopFile *df = new DesktopFile((*it).c_str(), hideOSI, useIcons, iconpaths);
    /* If a name or exec wasn't found we cannot add an entry to our menu so ignore
     * these objects */
    if (df->name != "\0" && df->exec != "\0")
    { files[counter] = df;
      counter++;
    }
  }

  //Create MwmMenuWriter object, passing it the array of DesktopFile
  //This object will cause the menus to be printed
  MenuWriter *mw = new MenuWriter(files, 
                                  counter, 
                                  menuName, 
                                  windowmanager, 
                                  useIcons, 
                                  iconpaths, 
                                  splitCommaArgs(exclude), 
                                  splitCommaArgs(excludeMatching), 
                                  splitCommaArgs(excludeCategories),
                                  iconSize);

  delete mw;
  for (int x = 0; x < counter; x++) delete files[x];
  delete[] files;
  return 0;
}
  
