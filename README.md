# mwmmenu
A utility to create application menus from freedesktop.org desktop entry files

The following window managers are supported (more may be added in the future):
* MWM (Motif Window Manager)
* TWM (Tab Window Manager)
* FVWM (F Virtual Window Manager
* Fluxbox
* Openbox
* Blackbox
* Olvwm (Open Look Virtual Window Manager)
* Windowmaker
* IceWM

Dependencies:
* boost-libs
* boost (makedep)
* gcc (makedep) 

Extra features:
* Supports user/vendor created categories/subcategories as defined in
  directory and menu files
* No dependencies on packages like gnome-menus. The base menu structure is
  defined internally
* Most operations defined in menu files, such an entry inclusion/exclusion
  or category mergers are also available as command line options. No need to
  mess with XML
* Define non-standard entry and icon locations from the command line
* Support for menu icons in all window managers that support them
* Support for terminal based applications, the default terminal used to launch
  an application is xterm -e, but this can be set using a command line option
* Support for selectively showing entries defined to be shown only in a certain
  desktop like GNOME or XFCE.
* Supports disabling all vendor/user defined categories and rules and adhering
  purely to the internal menu structure (which is essentially just the XDG base
  categories, with one or two categories renamed)

See 'mwmmenu --help' for a full list of options

Installation:

Just run make! This will create a single executable file which you can run
anywhere. In the Makefile we use static compilation, which means you won't
need to recompile on boost libs updates. If your distribution packages static
libs separately, make sure you install the static versions of boost libs and
glibc. As for the required GCC version, 4.4 and higher should be fine. Tested
on CentOS 6, CentOS 7 and Arch Linux.
