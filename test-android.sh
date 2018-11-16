#!/usr/bin/env bash

PACKAGENAME=com.defoldexample.videoplayer

BOB=bob.jar

set -e
java -jar $BOB --debug --archive --platform armv7-android clean build bundle -bo build/armv7-android --build-server=http://localhost:9000
echo "Uninstall"
adb uninstall $PACKAGENAME
echo "Install"
adb install -r build/armv7-android/VideoPlayer/VideoPlayer.apk
echo "Starting ..."
adb shell am start -a android.intent.action.MAIN -n $PACKAGENAME/com.dynamo.android.DefoldActivity

sleep 1

PID=`adb shell ps | grep $PACKAGENAME | awk '{ print $2 }'`

echo PID=$PID

adb logcat | grep $PID
