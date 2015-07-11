# Datacard module for Yate #

Yate datacard channel driver for Huawei UMTS modem.
Driver based on `chan_datacard` module for Asterisk.

This is an **alpha** version of the module. We hope that it will be useful, but there is no warranty for all cases of usage can be provided.

_Warning:_ this module can spend the money from your operator`s (SIM
card's) balance when you call or send/receive SMS messages via modem.

**What works now(basically):**
  * incoming calls
  * outgoing calls
  * sending/receiving DTMF

**Testing stage:**
  * sending USSD and receiving responses
  * sending/receiving SMS
  * SMS PDU mode support

**TODO:**
  * code refactoring

_If you find some kind of bug, please write an issue or send e-mail to project owners._

Tested with Huawei E1550 on Debian GNU/Linux 5.0.6 (lenny).
Also tested on Debian GNU/Linux 7.0 (wheezy)

&lt;wiki:gadget url="http://www.ohloh.net/p/487769/widgets/project\_basic\_stats.xml" height="220" border="1"/&gt;