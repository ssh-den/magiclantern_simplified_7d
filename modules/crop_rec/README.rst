Crop mode recording
===================

This alters the 1080p and 720p video modes, transforming them
into 3x (1:1) crop modes, by tweaking the sensor registers.

All other behaviors are the same as with Canon's 1080p/720p
implementation: resolution, H.264, RAW, audio, HDMI, preview,
overlays and so on.

EOS 7D 2.0.6
------------

RAW 1:1 (Canon x5): close the menu in Movie LiveView to enter the native
sensor window. Canon configures both DIGIC processors. Use the focus box to
position the window. x10 changes preview magnification, keeping RAW 1:1.
Select OFF to restore zoom changed by the module. Changes wait for idle
LiveView with recording stopped and the menu closed. Toggle OFF/ON to retry
a failed zoom request. RAW video only; H.264 is not cropped.


:Author: a1ex
:License: GPL
:Summary: Turn the 1080p and 720p video modes into 1:1 sensor crop modes


