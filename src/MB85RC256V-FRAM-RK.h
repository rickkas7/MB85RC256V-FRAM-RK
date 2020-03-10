#ifndef __MB85RC256V_FRAM_RK
#define __MB85RC256V_FRAM_RK

#include "Particle.h"

class MB85RC {
public:
	/**
	 * You will normally use one the subclasses of this object like MB85RC64, MB85RC256V, MB85RC512, or MB85RC1M
	 *
	 * The first argument is typically Wire (pins D0 and D1). On the Electron it can also be Wire1 (pins C4 and C5)
	 *
	 * The addr argument is the address 0-7 based on the setting of A0, A1 and A2.
	 */
	MB85RC(TwoWire &wire, size_t memorySize, int addr = 0);
	virtual ~MB85RC();

	/**
	 * @brief Typically called during setup() to start the Wire interface.
	 */
	void begin();

	/**
	 * @brief Returns the length of the device in bytes
	 *
	 * This is determined from the class used (MB85RC64, etc.) and does not communicate with the device.
	 * This call just returns a variable and is fast.
	 */
	inline size_t length() { return memorySize; }

	/**
	 * @brief Erases the FRAM device
	 *
	 * This is generally a slow operation because it requires writing to every location.
	 */
	bool erase();

	/**
	 * @brief Read from FRAM using EEPROM-style API
	 *
	 * @param framAddr The address in the FRAM to read from
	 * @param t The variable to read to
	 */
    template <typename T> T &get(size_t framAddr, T &t) {
        readData(framAddr, (uint8_t *)&t, sizeof(T));
        return t;
    }

	/**
	 * @brief Write from FRAM using EEPROM-style API
	 *
	 * @param framAddr The address in the FRAM to write to
	 * @param t The variable to write from. t is not modified.
	 */
    template <typename T> const T &put(size_t framAddr, const T &t) {
        writeData(framAddr, (const uint8_t *)&t, sizeof(T));
        return t;
    }

    /**
     * @brief Low-level read call
     *
	 * @param framAddr The address in the FRAM to read from
	 *
	 * @param data The buffer to read into
	 *
	 * @param dataLen The number of bytes to read
	 *
	 * The dataLen can be larger than the maximum I2C read. Multiple reads will be done if necessary.
     */
	virtual bool readData(size_t framAddr, uint8_t *data, size_t dataLen);

    /**
     * @brief Low-level write call
     *
	 * @param framAddr The address in the FRAM to write to
	 *
	 * @param data The buffer containing the data to write
	 *
	 * @param dataLen The number of bytes to write
	 *
	 * The dataLen can be larger than the maximum I2C write. Multiple writes will be done if necessary.
     */
	virtual bool writeData(size_t framAddr, const uint8_t *data, size_t dataLen);

	/**
	 * @brief Move data within the FRAM. This is just a read then write operation.
	 *
	 * @param framAddrFrom address to read from
	 *
	 * @param framAddrTo address to write to
	 *
	 * @param numBytes number of bytes to move
	 */
	virtual bool moveData(size_t framAddrFrom, size_t framAddrTo, size_t numBytes);

	static const uint8_t DEVICE_ADDR = 0b1010000;

protected:
	TwoWire &wire;
	size_t memorySize;
	int addr; // This is just 0-7, the (0b1010000 of the 7-bit address is ORed in later)

};

class MB85RC64 : public MB85RC {
public:
	/**
	 * @brief Object to interface with a MB85RC64 I2C FRAM chip (8K x 8 bit)
	 *
	 * @param wire The I2C interface, typically Wire (pins D0 and D1). On the Electron, Argon, and Xenon it can also be Wire1 (pins C4 and C5)
	 *
	 * @param addr The address 0-7 based on the setting of A0, A1 and A2.
	 *
	 * You typically create one of these objects as a global variable.
	 */
	MB85RC64(TwoWire &wire, int addr = 0) : MB85RC(wire, 8192, addr) {};
};

class MB85RC256V : public MB85RC {
public:
	/**
	 * @brief Object to interface with a MB85RC256V I2C FRAM chip (32K x 8 bit)
	 *
	 * @param wire The I2C interface, typically Wire (pins D0 and D1). On the Electron, Argon, and Xenon it can also be Wire1 (pins C4 and C5)
	 *
	 * @param addr The address 0-7 based on the setting of A0, A1 and A2.
	 *
	 * You typically create one of these objects as a global variable.
	 */
	MB85RC256V(TwoWire &wire, int addr = 0) : MB85RC(wire, 32768, addr) {};
};

class MB85RC512 : public MB85RC {
public:
	/**
	 * @brief Object to interface with a MB85RC512 I2C FRAM chip (64K x 8 bit)
	 *
	 * @param wire The I2C interface, typically Wire (pins D0 and D1). On the Electron, Argon, and Xenon it can also be Wire1 (pins C4 and C5)
	 *
	 * @param addr The address 0-7 based on the setting of A0, A1 and A2.
	 *
	 * You typically create one of these objects as a global variable.
	 */
	MB85RC512(TwoWire &wire, int addr = 0) : MB85RC(wire, 65536, addr) {};
};

class MB85RC1M : public MB85RC {
public:
	/**
	 * @brief Object to interface with a MB85RC1M I2C FRAM chip (128K x 8 bit)
	 *
	 * @param wire The I2C interface, typically Wire (pins D0 and D1). On the Electron, Argon, and Xenon it can also be Wire1 (pins C4 and C5)
	 *
	 * @param addr The address based on the setting of A1 and A2. Because A0 is NC on the MB85RC1M, the only valid values are: 0, 2, 4, and 6.
	 *
	 * You typically create one of these objects as a global variable.
	 */
	MB85RC1M(TwoWire &wire, int addr = 0) : MB85RC(wire, 131072, addr & 6) {};

    /**
     * @brief Low-level read call
     *
	 * @param framAddr The address in the FRAM to read from
	 *
	 * @param data The buffer to read into
	 *
	 * @param dataLen The number of bytes to read
	 *
	 * The dataLen can be larger than the maximum I2C read. Multiple reads will be done if necessary.
	 *
	 * On the MB85RC1M reads across the framAddr 65536 page boundary are special but this call will break the
	 * read into multiple reads if necessary so you don't have to worry about it.
     */
	virtual bool readData(size_t framAddr, uint8_t *data, size_t dataLen);

	/**
     * @brief Low-level write call
     *
	 * @param framAddr The address in the FRAM to write to
	 *
	 * @param data The buffer containing the data to write
	 *
	 * @param dataLen The number of bytes to write
	 *
	 * The dataLen can be larger than the maximum I2C write. Multiple writes will be done if necessary.
	 *
	 * On the MB85RC1M writes across the framAddr 65536 page boundary are special but this call will break the
	 * read into multiple reads if necessary so you don't have to worry about it.
     */
	virtual bool writeData(size_t framAddr, const uint8_t *data, size_t dataLen);

	uint8_t getI2CAddr(size_t framAddr) const;
};



#endif /* __MB85RC256V_FRAM_RK */
