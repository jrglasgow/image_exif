api = 2
core = 7.x

libraries[pjmt][type] = module
libraries[pjmt][download][type] = git
libraries[pjmt][download][url] = ssh://git@172.22.58.11:7999/WCMSH/pjmt.git
libraries[pjmt][directory_name] = "pjmt"
libraries[pjmt][destination] = "libraries"
libraries[pjmt][download][tag] = 1.12.01

libraries[pel][type] = module
libraries[pel][download][type] = git
libraries[pel][download][url] = ssh://git@172.22.58.11:7999/WCMSH/php_exif_library.git
libraries[pel][directory_name] = "pel"
libraries[pel][download][tag] = "0.9.6"