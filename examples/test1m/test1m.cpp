#include "MB85RC256V-FRAM-RK.h"

SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler;

// MB85RC1M connected to Wire (D0/D1), and address 0x0 on the A1/A2 pins.
MB85RC1M fram(Wire, 0);

void runTest();

void setup() {
	// Wait for a USB serial connection for up to 10 seconds
	waitFor(Serial.isConnected, 10000);

	fram.begin();
}

void loop() {
	runTest();
	delay(10000);
}

class TimeTest {
public:
	TimeTest(const char *name) : name(name), start(millis()) {
		Log.info("%s: starting", name);
	}

	~TimeTest() {
		unsigned long elapsed = millis() - start;

		int ms = (int)(elapsed % 1000);
		elapsed /= 1000;

		int sec = (int)(elapsed % 60);
		elapsed /= 60;

		int min = (int)(elapsed % 60);
		elapsed /= 60;

		Log.info("%s: completed in %d:%02d.%03d", name, min, sec, ms);
	}

protected:
	const char *name;
	unsigned long start;
};

void runTest() {
	{
		TimeTest timer("simple read/write");

		uint32_t d1;
		bool bResult = fram.readData(0, (uint8_t *)&d1, sizeof(d1));
		if (!bResult) {
			Log.info("readData failed line=%u", __LINE__);
			return;
		}
		Log.info("d1=%u", d1);

		d1++;
		bResult = fram.writeData(0, (const uint8_t *)&d1, sizeof(d1));
		if (!bResult) {
			Log.info("writeData failed line=%u", __LINE__);
			return;
		}
	}

	{
		TimeTest timer("boundary cross test x100");

		uint8_t buf1[128];
		uint8_t buf2[128];

		for(size_t testNum = 0; testNum < 100; testNum++) {
			for(size_t ii = 0; ii < sizeof(buf1); ii++) {
				buf1[ii] = (uint8_t) rand();
				buf2[ii] = 0;
			}

			size_t framAddr = 65535 - rand() % 120;

			bool bResult = fram.writeData(framAddr, buf1, sizeof(buf1));
			if (!bResult) {
				Log.info("writeData failed line=%u", __LINE__);
				return;
			}

			bResult = fram.readData(framAddr, buf2, sizeof(buf2));
			if (!bResult) {
				Log.info("readData failed line=%u", __LINE__);
				return;
			}

			for(size_t ii = 0; ii < sizeof(buf1); ii++) {
				if (buf1[ii] != buf2[ii]) {
					Log.error("data was %02x expected %02x ii=%u line=%u", buf2[ii], buf1[ii], ii, __LINE__);
					return;
				}
			}
		}
	}

	{
		TimeTest timer("moveTest");

		uint8_t buf1[128];
		uint8_t buf2[128];

		for(size_t ii = 0; ii < sizeof(buf1); ii++) {
			buf1[ii] = (uint8_t) rand();
			buf2[ii] = 0;
		}

		bool bResult = fram.writeData(0, buf1, sizeof(buf1));
		if (!bResult) {
			Log.info("writeData failed line=%u", __LINE__);
			return;
		}

		bResult = fram.readData(0, buf2, sizeof(buf2));
		if (!bResult) {
			Log.info("readData failed line=%u", __LINE__);
			return;
		}

		for(size_t ii = 0; ii < sizeof(buf1); ii++) {
			if (buf1[ii] != buf2[ii]) {
				Log.error("data was %02x expected %02x ii=%u line=%u", buf2[ii], buf1[ii], ii, __LINE__);
				return;
			}
		}

		// Move higher
		bResult = fram.moveData(50, 75, 40);
		if (!bResult) {
			Log.info("moveData failed line=%u", __LINE__);
			return;
		}

		bResult = fram.readData(0, buf2, sizeof(buf2));
		if (!bResult) {
			Log.info("readData failed line=%u", __LINE__);
			return;
		}

		for(size_t ii = 0; ii < 75; ii++) {
			if (buf2[ii] != buf1[ii]) {
				Log.error("data was %02x expected %02x ii=%u line=%u", buf2[ii], buf1[ii], ii, __LINE__);
				return;
			}
		}
		for(size_t ii = 0; ii < 40; ii++) {
			if (buf2[75 + ii] != buf1[50 + ii]) {
				Log.error("data was %02x expected %02x ii=%u line=%u", buf2[75 + ii], buf1[50 + ii], ii, __LINE__);
				return;
			}
		}
		for(size_t ii = 115; ii < 128; ii++) {
			if (buf2[ii] != buf1[ii]) {
				Log.error("data was %02x expected %02x ii=%u line=%u", buf2[ii], buf1[ii], ii, __LINE__);
				return;
			}
		}

		bResult = fram.writeData(0, buf1, sizeof(buf1));
		if (!bResult) {
			Log.info("writeData failed line=%u", __LINE__);
			return;
		}

		// Move lower
		bResult = fram.moveData(50, 25, 40);
		if (!bResult) {
			Log.info("moveData failed line=%u", __LINE__);
			return;
		}

		bResult = fram.readData(0, buf2, sizeof(buf2));
		if (!bResult) {
			Log.info("readData failed line=%u", __LINE__);
			return;
		}

		for(size_t ii = 0; ii < 25; ii++) {
			if (buf2[ii] != buf1[ii]) {
				Log.error("data was %02x expected %02x ii=%u line=%u", buf2[ii], buf1[ii], ii, __LINE__);
				return;
			}
		}
		for(size_t ii = 0; ii < 40; ii++) {
			if (buf2[25 + ii] != buf1[50 + ii]) {
				Log.error("data was %02x expected %02x ii=%u line=%u", buf2[25 + ii], buf1[50 + ii], ii, __LINE__);
				return;
			}
		}
		for(size_t ii = 65; ii < 128; ii++) {
			if (buf2[ii] != buf1[ii]) {
				Log.error("data was %02x expected %02x ii=%u line=%u", buf2[ii], buf1[ii], ii, __LINE__);
				return;
			}
		}


	}

	{
		TimeTest timer("erase");
		fram.erase();
	}

	{
		TimeTest timer("verify erase");
		// Make sure it was erased
		uint8_t buf[32];

		size_t framAddr = 0;
		size_t dataLen = 131072;
		while(dataLen > 0) {
			size_t count = dataLen;
			if (count > sizeof(buf)) {
				count = sizeof(buf);
			}
			bool bResult = fram.readData(framAddr, buf, count);
			if (!bResult) {
				Log.error("fram.readData failed framAddr=%u count=%u line=%u", framAddr, count, __LINE__);
				return;
			}
			for(size_t ii = 0; ii < count; ii++) {
				if (buf[ii] != 0) {
					Log.error("data was %02x expected 0 ii=%u framAddr=%u count=%u line=%u", buf[ii], ii, framAddr, count, __LINE__);
					return;
				}
			}
			framAddr += count;
			dataLen -= count;
		}
	}

}
