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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CATEGORY_H_
#define _CATEGORY_H_

#include "DesktopFile.h"

class Category
{
	public:
		Category();
		Category(const char *dirFile, const vector<string>& menuFiles, bool useIcons, const vector<IconSpec>& iconpaths);
		Category(const string& name, bool useIcons, const vector<IconSpec>& iconpaths);
		Category(const Category& c);

		Category& operator=(const Category& c);
		bool operator<(const Category& c);

		string name;
		string icon;
		vector<string> incEntries;
		vector<string> incEntryFiles;

		static string getID(const string& line);
		static string getSingleValue(const string& line);

	private:
		string dirFile;
		vector<string> menuFiles;
		ifstream dir_f;
		ifstream menu_f;

		void getCategoryParams();
		void getIncludedFiles();
		void getCategoryIcon(const vector<IconSpec>& iconpaths);
};

#endif
