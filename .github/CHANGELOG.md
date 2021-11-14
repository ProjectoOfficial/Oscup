# Change Log


## [1.2.0-beta](https://github.com/ProjectoOfficial/Oscup/releases/tag/v1.2.0-beta)

### few changes in this new release involving packet length and available example. This is not only a bug fix release but we changed some things, so pay attention!

* packet length has been reduced from 260 bytes to 45 bytes, payload becomes 40 bytes -> more stability in sharing packets
* solved bugs inside pyOscup -> tested, and it works!
* packet length has been fixed at 45 bytes and it is variable no more
* reduced the number of example code to 2: write and read
* added mainRead.py which works with Test_write.ino
* updated setup.py for installing pyOscup


## [1.1.0-beta](https://github.com/ProjectoOfficial/Oscup/releases/tag/v1.1.0-beta) (2021-11-06)

### We're happy to announce that we finally reached a Beta version of Oscup. Now it provides a complete C library and a complementary complete Python library. For any issue do not hesitate to contact us!

* Oscup.c read completed and tested
* Oscup.c write completed and tested
* pyOscup read completed and tested
* pyOscup write completed and tested
* added more example source code


## [1.0.0-alpha] (https://github.com/ProjectoOfficial/Oscup/releases/tag/v1.0.0-alpha) (2021-10-15)

### this major change of OSCUP introduces a lot of new feature. We added a TIM on which we will implement ACK and NACK on packets. The protocol acquired more stability than last version.

* fixed a lot of bugs
* added begin function
* added testWrite function
* added get_timer function
* added get_APB_clk function
* first example working fine
* It is already possible to use it, please contact us for any bug!


## [0.1.1-alpha](https://github.com/ProjectoOfficial/Oscup/releases/tag/Oscup) (2021-10-11)
### this first version allows to write data on UART. Read was is still not completely tested!

