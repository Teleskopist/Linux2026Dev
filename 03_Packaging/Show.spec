Name:           Show
Version:        1.0
Release:        alt1
Summary:        Simple ncurses file viewer
License:        MIT
Group:          Other

Source:         %name-%version.tar.gz

BuildRequires:  libncursesw-devel

%description
Simple ncurses program for viewing text files.

%prep
%setup

%build
%make_build

%install
%makeinstall_std

%files
%_bindir/Show
