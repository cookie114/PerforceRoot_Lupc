Just take /usr/local/bin/file, transfer "file" and "file-5.04/magic/Magdir" to target boot, remove no-used files in Magdir (keep "filesystem"), then execute "file -C -m magic.mgc", then generated magic.mgc.

For identify filesystem, execute "file -sb -m magic.mgc /dev/sdx".

