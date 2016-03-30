cd ../../../../
CURRENT_PATH=$(pwd | sed 's/\//\\\//gp')
cd -
CURRENT_PATH=`echo $CURRENT_PATH | cut -d ' ' -f 1`
ROOT_PATH='${FORGE_ANDROID_ROOT}'
sed -i "s/$CURRENT_PATH/$ROOT_PATH/g" build.mak
sed -i "s/$CURRENT_PATH/$ROOT_PATH/g" ./pjlib/build/os-auto.mak
sed -i "s/$CURRENT_PATH/$ROOT_PATH/g" ./build/cc-auto.mak
sed -i "s/$CURRENT_PATH/$ROOT_PATH/g" ./build/os-auto.mak
