# Routing Settings #
## For outgoing calls ##

Syntax: `datacard/callednumber;device=devicename`

The callednumber is, obvious, the called party's number. The device parameter is mandatory and indicates the device to use. devicename must be the name of an active device configured in datacard.conf.

**Example:**

`^555$=datacard/+381234567890;device=datacard0`

## For incoming calls ##

Syntax: `${device}^devicename$=target`

The devicename is, the datacard name from datacard.conf(like 'datacard0').

**Example:**

`${device}^datacard0$=sip/sip:123@192.168.1.1`

## DTMF detection ##

Since modem does not support DTMF detection, we advise you to use Yate tonedetect module.

The request for attaching a DTMF detector can be specified from routing, where a boolean true value stands equivalent to tone/**:**


DTMF detection on incoming calls

`${device}^datacard0$=;tonedetect_in=yes`

DTMF detection on outgoing calls

`^555$=;tonedetect_out=yes`

_Note: module tonedetect.yate must be installed and enabled_