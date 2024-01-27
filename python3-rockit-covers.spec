Name:           python3-rockit-covers
Version:        %{_version}
Release:        1
License:        GPL3
Summary:        Common backend code for the W1m mirror covers daemon
Url:            https://github.com/rockit-astro/coversd
BuildArch:      noarch
BuildRequires:  python3-devel

%description

%prep
rsync -av --exclude=build --exclude=.git --exclude=.github .. .

%generate_buildrequires
%pyproject_buildrequires -R

%build
%pyproject_wheel

%install
%pyproject_install
%pyproject_save_files rockit

%files -f %{pyproject_files}
