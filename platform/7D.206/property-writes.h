/* Reviewed core write payload extents for 7D.206. Notification lengths alone
 * are not a write contract. Unknown IDs remain closed, including requests
 * from external modules. */
PORT_WRITE(0x01000007, 8)   /* file prefix, zero-padded */
PORT_WRITE(0x02010000, 4)   /* CF folder number, 100..999 */
PORT_WRITE(0x02010003, 4)   /* CF file number, 0..9999 */
PORT_WRITE(0x02020000, 52)  /* custom WB: preserve template, edit gains */
PORT_WRITE(0x02040000, 4)   /* LCD brightness 1..7 */
PORT_WRITE(0x02040008, 4)   /* continuous file numbering counter, 0..9999 */
PORT_WRITE(0x0205000E, 4)   /* sound mode */
PORT_WRITE(0x80000000, 4)   /* shooting mode, P/Tv/Av/M/B only */
PORT_WRITE(0x80000003, 4)   /* drive mode */
PORT_WRITE(0x80000005, 4)   /* shutter in Canon APEX units */
PORT_WRITE(0x80000006, 4)   /* aperture, clamped to lens limits */
PORT_WRITE(0x80000007, 4)   /* ISO in Canon APEX units */
PORT_WRITE(0x80000008, 4)   /* exposure compensation */
PORT_WRITE(0x80000009, 4)   /* flash compensation */
PORT_WRITE(0x8000000A, 7)   /* AF point: preserve trailing 3 bytes */
PORT_WRITE(0x8000000D, 4)   /* photo WB mode */
PORT_WRITE(0x8000000E, 4)   /* photo Kelvin */
PORT_WRITE(0x80000010, 4)   /* WB shift GM, signed byte in word */
PORT_WRITE(0x80000011, 4)   /* WB shift BA, signed byte in word */
PORT_WRITE(0x80000028, 4)   /* Picture Style selection */
PORT_WRITE(0x8000002C, 4)   /* shutter auto, legacy core helper */
PORT_WRITE(0x80000034, 4)   /* exposure simulation */
PORT_WRITE(0x80010003, 4)   /* reboot after card format, value 0 */
PORT_WRITE(0x80010006, 23)  /* AFMA: template, mode and signed adjustment */
PORT_WRITE(0x80020009, 4)   /* UI lock request flag + lower mask */
PORT_WRITE(0x80020015, 2)   /* remote half-shutter 0/1 */
PORT_WRITE(0x80020016, 2)   /* remote shutter 0/1 */
PORT_WRITE(0x8003001D, 1)   /* battery poll command 31; reply is 96 bytes */
PORT_WRITE(0x80030024, 4)   /* power-save permit/prohibit/prolong 1/2/3 */
PORT_WRITE(0x8003002E, 32)  /* HDMI template; first word 2 or 5 */
PORT_WRITE(0x80040013, 4)   /* flash firing mode 0..2 */
PORT_WRITE(0x80050000, 56)  /* lens-data poll; preserve Canon template */
PORT_WRITE(0x80050007, 100) /* AF frame: template + bounded position */
PORT_WRITE(0x8005000B, 2)   /* DOF preview 0/1 */
PORT_WRITE(0x80050013, 4)   /* LV focus steps 0..3 or 8001..8003 */
PORT_WRITE(0x80050015, 4)   /* LV zoom 1/5/10 */
PORT_WRITE(0x80050018, 4)   /* LV WB mode */
PORT_WRITE(0x80050019, 4)   /* LV Kelvin */
PORT_WRITE(0x80050022, 4)   /* LV resume/pause 0/1 */
PORT_WRITE(0x02060001, 24)  /* Standard Picture Style */
PORT_WRITE(0x02060002, 24)  /* Portrait */
PORT_WRITE(0x02060003, 24)  /* Landscape */
PORT_WRITE(0x02060004, 24)  /* Neutral */
PORT_WRITE(0x02060005, 24)  /* Faithful */
PORT_WRITE(0x02060006, 24)  /* Monochrome */
PORT_WRITE(0x02060007, 24)  /* User 1 */
PORT_WRITE(0x02060008, 24)  /* User 2 */
PORT_WRITE(0x02060009, 24)  /* User 3 */
