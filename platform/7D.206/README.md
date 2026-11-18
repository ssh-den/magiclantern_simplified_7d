# EOS 7D 2.0.6

Experimental port of Magic Lantern to the Canon EOS 7D with firmware 2.0.6.
The 7D.203 port is the base; this directory only overrides what differs in
2.0.6. Use it at your own risk.

## Building

```
make -C platform/7D.206
```

This builds `build/magiclantern.zip` (autoexec.bin, `ML/modules/*.mo`,
`7D_206.sym`). For the 7D, modules are built into `modules/build/7d`;
`sd_clock` and `yolo` are skipped: the 7D has a CF slot and no Wi-Fi.
`raw_vidx`, `ime_base` and `ime_rot` build, but are not in `modules.included`.

## Status

Checked on a camera:

- boot, ML menus, saving and loading settings
- Canon H.264 video, intervalometer, focus peaking
- selftest "Stubs API test": all checks pass except the two noted below
- bench memory benchmarks (cache lockdown stays in place)
- mlv_rec at x1 and with crop_rec "RAW 1:1 (Canon x5)"
- dual_iso (photo)
- file_man, arkanoid

Builds, but not yet checked on a camera: mlv_lite, mlv_snd, mlv_play,
silent, pic_view, img_name, deflick, ettr, autoexpo, adv_int, dot_tune,
edmac, lua.

Known selftest differences:

- `snprintf(buf, 3, "%d", 1234)` returns 4. Canon's `vsnprintf` on 2.0.6
  returns the untruncated length (C99); the buffer is truncated correctly.
- One `HALFSHUTTER_PRESSED` check in the AF test can fail: with AF
  disabled, the test reads the state right after sending the key press,
  without the delay used elsewhere.

## Firmware facts

- Dual DIGIC 4. ML runs on the slave CPU (ROM at `0xFF010000`,
  signature `0xAB3E8EFF`). The sensor is driven by the master CPU,
  which has its own ROM and RAM. The same RAM address on the two CPUs is
  different memory; `MEM()` on the slave cannot read master RAM.
- `stubs.S` defines all stubs for 2.0.6 (172). Most other headers include
  the 7D.203 files; `consts.h` only overrides what moved.
- `GMT_FUNCTABLE` is `0xFF511914` (`0xFF51193C` on 2.0.3).
- `0xFF05A6DC` is not a cache flush-rate patch site on 2.0.6. The flush-rate
  and GOP cache hacks (slave and master) are not ported.
- `CONFIG_CRASH_LOG`, `CONFIG_AUTOBACKUP_ROM` and `CONFIG_TSKMON` are
  disabled.

### Talking to the master CPU

In order of preference:

1. Use a Canon property and let Canon coordinate both CPUs. For example,
   `PROP_LV_DISPSIZE` 1/5/10 gives native 1:1 RAW at x5 (2520x1226,
   no binning or skipping).
2. To read or write master data, use a bounded IPC transfer: check
   address and size, check that the request was queued, and wait for the
   completion callback with a timeout. Keep the buffer alive until the
   callback runs, even after a timeout. dual_iso does this for the ISO
   table (same master address as on 2.0.3).
3. Changing master code needs a handler running on the master and a
   protocol that confirms the change. The `ml_rpc_*` helpers expect the
   2.0.3 master payload, which this port does not load.

Do not wait inside PropMgr: property handlers only update state, and the
waiting request runs later in ShootTask when the camera is idle.

Master addresses found in the 2.0.6 master ROM (base `0xFF800000`):

| Object | Slave | Master |
| --- | --- | --- |
| ADTG write | `FF2C0918` | `FF90DECC` |
| CMOS write | `FF2C0B10` | `FF90E0C4` |
| Sensor parameters | `FF101F68` | `FF863880` |
| StartupLiveViewDevice | `FF1D4F04` | `FF8C4CC4` |
| ChangeSrModeLiveViewDevice | `FF1D5D90` | `FF8C5B50` |
| SIO root / enable flag | `18E08` / `18E0C` | `14FC4` / `14FC8` |
| LvTgDriver root | `97E8` | `7FC4` |
| ADTG / CMOS pointer slots | `981C` / `9820` | `7FF8` / `7FFC` |

The CMOS writer takes a `0xFFFF`-terminated list of `uint16_t` in `r0`.
The ADTG writer takes the chip select in `r0` and a `0xFFFFFFFF`-terminated
list of `uint32_t` in `r1`. Upstream crop_rec hooks these functions on
single-DIGIC cameras; on the 7D only the master writes to the sensor, so
those hooks do not apply on the slave.

These are matched functions for this firmware only. Do not port addresses
by a fixed offset from 2.0.3; check the instructions and callers first.

## Memory layout

ML sits at the top of Canon's AllocateMemory heap. Canon sets the heap end
with `MOV r1, #0xD00000` at `0xFF011F2C`; `boot-d45-ch.c` replaces it with
`MOV r1, #0xC60000` (`HIJACK_CACHE_HACK_BSS_END_*`), so ML owns
`0xC60000..0xD00000` (640 KiB).

- `RESTARTSTART` is `0xC60000`. If you change it, change the patched
  instruction and `heap-bounds.lds` together; the linker script fails the
  build if ML does not fit.
- Code and BSS use about 534 KiB, leaving about 100 KiB.
- After boot, AllocateMemory has about 1.3 MB free with many modules
  loaded. The module loader refuses to load if there is less than 1 MiB
  free or the largest free block is under 512 KiB.

## Core changes for this port

All are under `CONFIG_7D206_FULL_CORE` (set in the Makefile).

- Property reads (`property.c`, `property-contracts.h`): the dispatcher
  checks each property length against the size its readers need before
  calling them. A bad buffer is cleaned up and never reaches a handler.
  On the 7D, AF mode, shooting type, auto ISO and half-shutter are 2 bytes,
  WB shift is 1 byte. `0x80040040` and `0x8005002F` do not exist in this
  ROM and are not registered.
- Property writes (`property-writes.h`): `prop_request_change` only passes
  listed property IDs with the exact payload size, and never during
  shutdown. Folder numbers must be 100..999, file numbers 0..9999.
  Properties are saved in NVRAM, so extend this list only after checking
  the size and values the camera actually uses.
- Startup order (`init.c`): property handlers are registered after all
  core queues and semaphores exist, because Canon may send initial values
  during registration. The LiveView state object hook is installed last.
- `state-object.c`: hooks LVState only if the object matches the 2.0.6
  layout. Otherwise ML does not start its tasks.
- `reboot.c`: checks the CP15 cache geometry (8 KiB I and D caches,
  4 ways, 32-byte lines) before copying ML.
- Modules (`module.c`): loading waits for ML startup, stops on shutdown,
  and records its result. "Modules -> Loader status" shows why loading
  failed. Settings are not saved while modules are linking.
- Settings are saved when the ML menu is closed, not at shutdown.

## Modules on the 7D

- crop_rec: "RAW 1:1 (Canon x5)" switches LiveView to x5 when the menu
  closes and restores the previous zoom when turned off. While recording,
  the restore is deferred. Other upstream presets are not available.
- mlv_rec: a buffer slot stays locked until its DMA completes and then
  becomes full; the writer task must confirm it has exited before buffers
  are freed.
- pic_view and mlv_play decode lossless RAW in software
  (`modules/silent/lj92-preview.h`); the 7D lacks the hardware decoder
  these modules use.
- dual_iso: IPC transfers to the master have a timeout. After a timeout,
  dual ISO stays disabled until restart.
- bench: on 7D.206 the memory benchmark keeps the cache lockdown, because
  ML's boot patches live in locked cache lines.
