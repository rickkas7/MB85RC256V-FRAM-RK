
#include "Particle.h"
#include "MB85RC256V-FRAM-RK.h"


MB85RC::MB85RC(TwoWire &wire, size_t memorySize, int addr) :
	wire(wire), memorySize(memorySize), addr(addr) {
}

MB85RC::~MB85RC() {
}

void MB85RC::begin() {
	wire.begin();
}

bool MB85RC::erase() {

	WITH_LOCK(wire) {
		size_t framAddr = 0;
		size_t totalLen = memorySize;

		uint8_t zero[30];
		memset(zero, 0, sizeof(zero));

		while(totalLen > 0) {
			size_t count = totalLen;
			if (count > sizeof(zero)) {
				count = sizeof(zero);
			}

			bool result = writeData(framAddr, zero, count);
			if (!result) {
				Log.info("writeData failed during erase framAddr=%u", framAddr);
				return false;
			}

			totalLen -= count;
			framAddr += count;
		}
	}

	return true;
}


bool MB85RC::readData(size_t framAddr, uint8_t *data, size_t dataLen) {
	bool result = true;

	WITH_LOCK(wire) {

		while(dataLen > 0) {
			wire.beginTransmission(addr | DEVICE_ADDR);
			wire.write(framAddr >> 8);
			wire.write(framAddr);
			int stat = wire.endTransmission(false);
			if (stat != 0) {
				//Serial.printlnf("read set address failed %d", stat);
				result = false;
				break;
			}

			size_t bytesToRead = dataLen;
			if (bytesToRead > 32) {
				bytesToRead = 32;
			}

			wire.requestFrom((uint8_t)(addr | DEVICE_ADDR), bytesToRead, (uint8_t) true);

			if (Wire.available() < (int) bytesToRead) {
				result = false;
				break;
			}

			for(size_t ii = 0; ii < bytesToRead; ii++) {
				*data++ = Wire.read();    // receive a byte as character
				framAddr++;
				dataLen--;
			}
		}
	}
	return result;
}


bool MB85RC::writeData(size_t framAddr, const uint8_t *data, size_t dataLen) {
	bool result = true;

	WITH_LOCK(wire) {
		while(dataLen > 0) {
			wire.beginTransmission(addr | DEVICE_ADDR);
			wire.write(framAddr >> 8);
			wire.write(framAddr);

			for(size_t ii = 0; ii < 30 && dataLen > 0; ii++) {
				wire.write(*data);
				framAddr++;
				data++;
				dataLen--;
			}

			int stat = wire.endTransmission(true);
			if (stat != 0) {
				//Serial.printlnf("write failed %d", stat);
				result = false;
				break;
			}
		}
	}
	return result;
}


bool MB85RC::moveData(size_t framAddrFrom, size_t framAddrTo, size_t numBytes) {
	bool result = true;

	// Maximum number of bytes we can write is 30
	uint8_t buf[30];

	WITH_LOCK(wire) {
		if (framAddrFrom < framAddrTo) {
			// Moving to a higher address - copy from the end of the from buffer
			framAddrFrom += numBytes;
			framAddrTo += numBytes;
			while(numBytes > 0) {
				size_t count = numBytes;
				if (count > sizeof(buf)) {
					count = sizeof(buf);
				}
				framAddrFrom -= count;
				framAddrTo -= count;

				result = readData(framAddrFrom, buf, count);
				if (!result) {
					break;
				}
				result = writeData(framAddrTo, buf, count);
				if (!result) {
					break;
				}

				numBytes -= count;
			}

		}
		else
		if (framAddrFrom > framAddrTo) {
			// Moving to a lower address - copy from beginning of the from buffer
			while(numBytes > 0) {
				size_t count = numBytes;
				if (count > sizeof(buf)) {
					count = sizeof(buf);
				}
				result = readData(framAddrFrom, buf, count);
				if (!result) {
					break;
				}
				result = writeData(framAddrTo, buf, count);
				if (!result) {
					break;
				}
				framAddrFrom += count;
				framAddrTo += count;
				numBytes -= count;
			}
		}
	}
	return result;
}



//
// Special versions of readData and writeData are required for the MB85RC1M
//

bool MB85RC1M::readData(size_t framAddr, uint8_t *data, size_t dataLen) {
	bool result = true;

	WITH_LOCK(wire) {

		while(dataLen > 0) {
			size_t count = dataLen;
			if (count > 32) {
				// Don't read more than 32 bytes (limit of Wire implementation)
				count = 32;
			}
			if ((framAddr < 65536) && ((framAddr + count) >= 65536)) {
				// Crosses boundary at 65536, only write up to the boundary
				count = 65536 - framAddr;
			}

			wire.beginTransmission(getI2CAddr(framAddr));
			wire.write(framAddr >> 8);
			wire.write(framAddr);
			int stat = wire.endTransmission(false);
			if (stat != 0) {
				Log.info("read set address failed %d", stat);
				result = false;
				break;
			}

			wire.requestFrom(getI2CAddr(framAddr), count, (uint8_t) true);

			if (Wire.available() < (int) count) {
				Log.info("didn't receive enough bytes count=%u", count);
				result = false;
				break;
			}

			for(size_t ii = 0; ii < count; ii++) {
				*data++ = Wire.read();    // receive a byte as character
				framAddr++;
				dataLen--;
			}
		}
	}
	return result;
}

bool MB85RC1M::writeData(size_t framAddr, const uint8_t *data, size_t dataLen) {
	bool result = true;

	WITH_LOCK(wire) {
		while(dataLen > 0) {
			size_t count = dataLen;
			if (count > 30) {
				// Don't write more than 30 bytes (limit of Wire implementation)
				count = 30;
			}
			if ((framAddr < 65536) && ((framAddr + count) >= 65536)) {
				// Crosses boundary at 65536, only write up to the boundary
				count = 65536 - framAddr;
			}

			wire.beginTransmission(getI2CAddr(framAddr));
			wire.write(framAddr >> 8);
			wire.write(framAddr);

			for(size_t ii = 0; ii < count; ii++) {
				wire.write(*data);
				framAddr++;
				data++;
				dataLen--;
				count--;
			}

			int stat = wire.endTransmission(true);
			if (stat != 0) {
				Log.info("write failed %d", stat);
				result = false;
				break;
			}
		}
	}
	return result;
}

uint8_t MB85RC1M::getI2CAddr(size_t framAddr) const {
	return (uint8_t) (addr | DEVICE_ADDR | (framAddr >= 65536 ? 1 : 0));
}





