# Aurc
Fast Easy way to Install and Update Aur and Non-Aur Arch Linux Packages!
![aurcbanner](https://github.com/statulr/aurc/assets/122219240/218741a8-0faa-4693-9fa8-feeb5285bfa9)

## TODO

- [x] Complete the essential pacman functions
- [x] Ability to modify arch mirrorlist
- [x] Implement aur helper
- [x] Better inspection and secure aur package installation
- [x] Better optimization
- [ ] Fully Move To Using C Libraries rather than command line wrapping

## Requirements

* Dependencies -
  
   - less
   - pacman
   - curl
   - base-devel
   - make
   - gcc

## Usage

  * Actions :
    - update        - ( Update outdated system/user packages )
    - refresh       - ( Refresh Repository Database )
    - install       - ( Install a package )
    - install-local - ( Install a local package )
    - install-aur   - ( Install aur package )
    - install-force - ( Forcefully install a package )
    - modify-repo   - ( Modify arch repositories )
    - query         - ( Query if a package is installed )
    - search        - ( Search for a package in the base repository )
    - search-aur    - ( Search for a package in the aur repository )
    - remove        - ( Remove a package )
    - remove-dep    - ( Remove a package along with its dependencies )
    - remove-force  - ( Forcefully remove a package even if other packages depend on it )
    - remove-orp    - ( Remove orphan packages )
    
  * Options :
    - --version, -v - ( Display the version of the package manager )
    - --help,    -h - ( Display this help guide )

## Installation
### Make sure you are in the SRC Directory
  * Install dependencies : 

      ```bash
      sudo pacman -S less gcc pacman make base-devel
      ```

  * Method 1 :

      ```bash
      git clone https://github.com/statulr/aurc.git
      ```
      ```bash
      cd aurc/src
      ```
      ```bash
      sudo make install
      ```

   * Method 2 :

      ```bash
      curl https://github.com/statulr/aurc/releases/latest/download/aurc-pkgbuild.tar.gz -o aurc-pkgbuild.tar.gz
      ```
      ```bash
      tar xvzf aurc-pkgbuild.tar.gz
      ```
      ```bash
      cd aurc-pkgbuild
      ```
      ```bash
      makepkg -si
      ```
   
   * Method 3 :

      ```bash
      wget https://github.com/statulr/aurc/releases/latest/download/aurc-${version}-x86_64.pkg.tar.zst
      ```
      
      or download the file from <a href="https://github.com/statulr/aurc/releases/latest/">Github Releases</a></h1>

      ```bash
      sudo pacman -U aurc-${version}-x86_64.pkg.tar.zst
      ```
## Updating
### Make sure you are in the SRC Directory
   ```bash
   git fetch & git pull
   ```
   Then
   ```bash
   sudo rm -rd build && sudo make && sudo make clean install
   ```
