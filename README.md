> [!WARNING]
> **Unofficial, experimental fork for the Canon EOS 7D (firmware 2.0.6).**
>
> - This is **not** an official Magic Lantern release and is not affiliated
>   with, endorsed by, or supported by the Magic Lantern team or Canon.
> - Builds from this repository are **test builds**. They have been tried on
>   a single camera and only for a limited set of features. Anything not
>   explicitly described as tested should be treated as untested.
> - Running unofficial software on your camera carries real risk, including
>   crashes, corrupted recordings, lost data and, in the worst case, a camera
>   that needs service. **Use it entirely at your own risk.**
> - Before trying it: back up your memory card, use a card you can format,
>   and make sure you know how to remove the boot flag / return to Canon
>   firmware.
> - This software is provided **"as is", without warranty of any kind**, as
>   stated in the GNU GPL (see [COPYING](COPYING), sections 11 and 12). The
>   author of this fork accepts no responsibility or liability for any damage
>   or loss resulting from its use.
> - Please do **not** report problems with this fork to the upstream Magic
>   Lantern project. Use this repository's issue tracker instead.

Magic Lantern
=============

For instructions on building from source and preparing a CF card, see the
[EOS 7D 2.0.6 platform README](platform/7D.206/README.md#building).

Magic Lantern (ML) is a software enhancement that offers increased
functionality to the excellent Canon DSLR cameras.
  
It's an open framework, licensed under GPL, for developing extensions to the
official firmware.

Magic Lantern is not a *hack*, or a modified firmware, **it is an
independent program that runs alongside Canon's own software**. 
Each time you start your camera, Magic Lantern is loaded from your memory
card. Our only modification was to enable the ability to run software
from the memory card.

ML is being developed by photo and video enthusiasts, adding
functionality such as: HDR images and video, timelapse, motion
detection, focus assist tools, manual audio controls much more.

For more details on Magic Lantern please see [http://www.magiclantern.fm/](http://www.magiclantern.fm/)

There is a sibling repo for our patched version of Qemu that adds support
for emulating camera ROMs. This allows testing without access to a physical
camera, and automating tests across a suite of cameras.  
https://github.com/reticulatedpines/qemu-eos  
https://github.com/reticulatedpines/qemu-eos/tree/qemu-eos-v4.2.1 (current ML team supported branch)
