#include <array>
#include <functional>
#include <boost/circular_buffer.hpp>
#include <vector>
#include <concurrent_queue.h>

class KeyboardManager
{
private:
	struct KeyData
	{
		bool PRESSED;
		bool RELEASED;
		bool DOWN;
	};
	std::array<KeyData, 0x100> Keys;
	void ResetKeyData();
	std::function<void(bool, unsigned char)> onKeyStateChange;
	boost::circular_buffer<unsigned char> key_buffer;
public:
	KeyboardManager(const std::function<void(bool, unsigned char)>& onKeyStateChange = nullptr);
	void CheckKeys(unsigned char key, bool pressed);
	bool Pressed(unsigned char key);
	bool Released(unsigned char key);
	bool Up(unsigned char key);
	bool Down(unsigned char key);
	bool BufferContainsArray(const std::vector<unsigned char>& compare);
	const std::vector<unsigned char> GetSequence(size_t size, bool removelast);
	void ClearBuffer();
	void SetOnKeyStateChangeFunction(const std::function<void(bool, unsigned char)>& onKeyStateChange);

	bool BufferAcceptOnlyNumeric;

	using TACS = std::pair<unsigned char, bool>;
	 
	Concurrency::concurrent_queue<TACS> AntiCrashSychronization;
};

void KeyboardHandlerFunction(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow);

extern KeyboardManager kbmgr;