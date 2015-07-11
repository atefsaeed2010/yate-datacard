# Installation #
**Basic Yate Installation
```
svn checkout http://voip.null.ro/svn/yate/trunk yate
cd yate
./autogen.sh
./configure
make debug
make install
```
_We recommend to use `make debug` because datacard module is in alpha stage._**

For more information see http://yate.null.ro

**Module installation
```
svn checkout http://yate-datacard.googlecode.com/svn/trunk/ yate-datacard-read-only
cd yate-datacard-read-only
./autogen.sh
./configure
make debug
make install
```
_Use `./configure --with-yate=path` if you installed Yate in not default folder._**

**NOTE: Please disable call waiting and PIN futures for SIM card, before use this module**