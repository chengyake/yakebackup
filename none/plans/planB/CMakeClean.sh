

del_src="`find -name cmake_install.cmake` `find -name CMakeFiles` `find -name Makefile` `find -name CMakeCache.txt` `find -name out`"





flag="`echo $del_src | grep "a" `"

if [ -n "$flag" ];then
    echo $del_src "need to clean"
    rm -rf $del_src
else
    echo "No file to clean"
fi


