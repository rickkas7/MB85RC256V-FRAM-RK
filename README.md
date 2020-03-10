# MB85RC256V-FRAM-RK

*Particle driver for MB85RC FRAMs (64, 256, 512, and 1M)*

This little board is a FRAM (Ferroelectric RAM). I got mine from [Adafruit](https://www.adafruit.com/products/1895).

It's 32 Kbytes in size, and the main benefit is that it's non-volatile like EEPROM so the contents are preserved if you remove power, but it's also very fast and doesn't have the limited wear cycles of EEPROM.

There are also a bunch of other variations in different sizes for just the chip part:

| Device | Size (bytes) | DigiKey | Price |
| :--- | ---: | :--- | ---: |
| MB85RC64 | 8K | [865-1274-1-ND](https://www.digikey.com/product-detail/en/fujitsu-electronics-america-inc/MB85RC64TAPNF-G-BDERE1/865-1274-1-ND/6802280) | $1.58 |
| MB85RC256 | 32K | [865-1249-1-ND](https://www.digikey.com/product-detail/en/fujitsu-electronics-america-inc/MB85RC256VPF-G-JNERE2/865-1249-1-ND/4022672) | $4.22 |
| MB85RC512 | 64K | [865-1269-1-ND](https://www.digikey.com/product-detail/en/fujitsu-electronics-america-inc/MB85RC512TPNF-G-JNERE1/865-1269-1-ND/5456443) | $5.28 |
| MB85RC1M | 128K | [865-1268-1-ND](https://www.digikey.com/product-detail/en/fujitsu-electronics-america-inc/MB85RC1MTPNF-G-JNERE1/865-1268-1-ND/5456442) | $5.65 |


## Circuit

The pins on the Adafruit breakout connect as typical for an I2C device:

- VCC to 3V3 (can also use VIN for a 5V I2C bus)
- GND to GND
- WP not connected (connect to VCC to prevent writes to the memory)
- SCL connect to D1 (SCL) (blue in the picture)
- SDA connect to D0 (SDA) (green in the picture)
- A2 not connected. Connect to VCC to change the I2C address. 
- A1 not connected. Connect to VCC to change the I2C address. 
- A0 not connected. Connect to VCC to change the I2C address. 

![Circuit](images/circuit.jpg)

The Adafruit board has built-in pull-ups on SDA and SCL so you don't need to add them. You will need pull-ups with a bare chip.
 
It also has pull-downs on A2, A1, and A0, so you can leave them unconnected.

## Using the code

Typically you create a global variable for the FRAM:

```
MB85RC256V fram(Wire, 0);
```

The first parameter is the Wire interface to use, typically Wire (D0/D1). On the Electron you can also use Wire1 (C4/C5).

The second parameter is the address. Pass 0 if you left the A0-A3 pins unconnected. It can be 0-7 depending on the value set on A0-A3.

From setup you typically call:

```
fram.begin();
```

To read and write data you use the readData and writeData methods:

```
bool readData(size_t framAddr, uint8_t *data, size_t dataLen);
bool writeData(size_t framAddr, const uint8_t *data, size_t dataLen);
```

You can also use get and put like the [EEPROM API](https://docs.particle.io/reference/firmware/photon/#eeprom). This makes it easy to convert code between using the built-in EEPROM and FRAM as you can use the calls nearly identically.

This also works with other chips:

```
MB85RC64 fram(Wire, 0);
MB85RC512 fram(Wire, 0);
MB85RC1M fram(Wire, 0);
```

Note that with the MB85RC1M chip, the A0 pin is N/C. You can leave it unconnected, or connect it to VCC or GND. Because of this, the only acceptable address values for the MB85RC1M are 0, 2, 4, and 6.

## Version History

#### 0.0.5 (2020-03-10)

- Fix compile error for ambiguous receiveFrom() with 1.5.0.rc.2.

#### 0.0.4 (2019-11-18)

- Added moveData() method to efficiently move data. Supports overlap.

#### 0.0.3 (2019-11-18)

- Added support for MB85RC64, MB85RC512, MB85RC1M
- Now surrounds transactions with WITH_LOCK(wire) for thread safety
- Minimum support Device OS version is now 1.0.0

