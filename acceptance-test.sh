#!/bin/sh

###
### Acceptance Test Script for metronome
###
### Usage
###   ./acceptance-test.sh
###
### Installation
###   1. Copy this shell script file into the /tmp directory in the Neutrino file system
###   2. Use sed (stream editor) to remove the carriage-return (^M) end-of-line symbol: sed 's/\r$//g' ./acceptance-test.sh > acceptance-test.sh
###   3. Change file permissions of the script file to be runnable: chmod +x acceptance-test.sh
###   4. Ensure the metronome is running before testing script.
###
### Author
###     Zachary Sabourin
###

echo "\n"
echo "Unit Test C: cat ./dev/local/metronome"
echo "Expected: [metronome: 120 beats/min, time signature 2/4, secs-per-interval: 0.25, nanoSecs: 250000000]"
cat ./dev/local/metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test D: cat ./dev/local/metronome-help"
echo "Expected: information regarding the metronome resmgr’s API"
cat ./dev/local/metronome-help
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test E: echo set 100 2 4 > ./dev/local/metronome"
echo "Expected: metronome regmgr changes settings to: 100 bpm in 2/4 time"
echo set 100 2 4 > ./dev/local/metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test F: cat ./dev/local/metronome"
echo "Expected: [metronome: 100 beats/min, time signature 2/4, secs-per-interval: 0.30, nanoSecs: 300000000]"
cat ./dev/local/metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test G: echo set 200 5 4 > ./dev/local/metronome"
echo "Expected: metronome regmgr changes settings to: 200 bpm in 5/4 time"
echo set 200 5 4 > ./dev/local/metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test H: cat ./dev/local/metronome"
echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per- interval: 0.15, nanoSecs: 150000000]"
cat ./dev/local/metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test I: echo stop > ./dev/local/metronome"
echo "Expected: metronome stops running; metronome resmgr is still running as a process"
echo stop > ./dev/local/metronome && pidin | grep metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test J: echo start > ./dev/local/metronome"
echo "Expected: metronome starts running again at 200 bpm in 5/4 time, which is the last setting"
echo start > ./dev/local/metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test K: cat ./dev/local/metronome"
echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per- interval: 0.15, nanoSecs: 150000000]"
cat ./dev/local/metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test L: echo stop > ./dev/local/metronome"
echo "Expected: metronome stops running;"
echo stop > ./dev/local/metronome && pidin | grep metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test M: echo stop > ./dev/local/metronome"
echo "Expected: metronome stays stopped;"
echo stop > ./dev/local/metronome && pidin | grep metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test N: echo start > ./dev/local/metronome"
echo "Expected: metronome starts running again at 200 bpm in 5/4 time, which is the last setting"
echo start > ./dev/local/metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test O: echo start > ./dev/local/metronome"
echo "Expected: metronome is still running again at 200 bpm in 5/4 time, which is the last setting"
echo start > ./dev/local/metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test P: cat ./dev/local/metronome"
echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per- interval: 0.15, nanoSecs: 150000000]"
cat ./dev/local/metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test Q: echo pause 3 > ./dev/local/metronome"
echo "Expected: metronome continues on next beat (not next measure)."
echo pause 3 > ./dev/local/metronome && pidin | grep metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test R: echo pause 10 > ./dev/local/metronome"
echo "Expected: properly formatted error message, and metronome continues to run."
echo pause 10 > ./dev/local/metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test S: echo bogus > ./dev/local/metronome"
echo "Expected: properly formatted error message, and metronome continues to run."
echo bogus > ./dev/local/metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test T: echo set 120 2 4 > ./dev/local/metronome"
echo "Expected: 1 measure per second."
echo set 120 2 4 > ./dev/local/metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test U: cat ./dev/local/metronome"
echo "Expected: [metronome: 120 beats/min, time signature 2/4, secs-per-interval: 0.25, nanoSecs: 250000000]"
cat ./dev/local/metronome
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test V: cat ./dev/local/metronome-help"
echo "Expected: information regarding the metronome resmgr’s API,"
cat ./dev/local/metronome-help
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test W: echo Writes-Not-Allowed > /dev/local/metronome-help"
echo "Expected: properly formatted error message, and metronome continues to run."
echo Writes-Not-Allowed > ./dev/local/metronome-help
read -rs $'Press enter to continue...\n'

echo "\n"
echo "Unit Test X: echo quit > ./dev/local/metronome && pidin | grep metronome"
echo "Expected: metronome gracefully terminates."
echo quit > ./dev/local/metronome && pidin | grep metronome
read -rs $'Press enter to continue...\n'

## end of unit tests
exit 0
