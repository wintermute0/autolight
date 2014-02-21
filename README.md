autolight
=========

A very simple win32 command line utility that adjust screen brightness automatically according to whether you're running a full screen app,
so that you can have brighter screen while playing games or watching movies, and have dimmer screen while browsing or coding.

Basically you can just start this app and leave it be. It will check whether you're running a full screen app every 60 seconds, and then adjust screen brightness accordingly.

Notice that your monitor must support DDC/CI.

###Usage

The compiled executable file is under **executable** folder.

`autolight [low_brightness=25] [high_brightness=75] [check_interval_seconds=60]`

All arguments are optional, and the default value will be used if you don't provide one.

For example:

`autolight 20 80 30`

This will set the low brightness to 20, high brightness to 80 and check interval to 30 seconds.

This utility is compiled and tested under **VisualStudio 2013** and **Windows 7**.