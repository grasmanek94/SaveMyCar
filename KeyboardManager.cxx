#include <Windows.h>
#include "KeyboardManager.hxx"

void KeyboardManager::ResetKeyData()
{
	BufferAcceptOnlyNumeric = false;
	Keys.fill(KeyData{ false, false, false });
	key_buffer.clear();
}

KeyboardManager kbmgr;

KeyboardManager::KeyboardManager(
	const std::function<void(bool, unsigned char)>& onKeyStateChange)
	: onKeyStateChange(onKeyStateChange), key_buffer(boost::circular_buffer<unsigned char>(8))
{
	ResetKeyData();
}

void KeyboardManager::SetOnKeyStateChangeFunction(const std::function<void(bool, unsigned char)>& onKeyStateChange)
{
	this->onKeyStateChange = onKeyStateChange;
}

void KeyboardManager::CheckKeys(unsigned char key, bool pressed)
{
	KeyData *data = &Keys[key];
	if (pressed)
	{
		if (!data->DOWN)
		{
			data->DOWN = true;
			data->PRESSED = true;
			data->RELEASED = false;

			if (onKeyStateChange != nullptr)
			{
				if (BufferAcceptOnlyNumeric && key >= '0' && key <= '9')
				{
					key_buffer.push_front(key);
				}
				else if(!BufferAcceptOnlyNumeric)
				{
					key_buffer.push_front(key);
				}

				onKeyStateChange(true, key);
			}
		}
		else if (data->PRESSED)
		{
			data->PRESSED = false;
		}
	}
	else
	{
		if (data->DOWN)
		{
			data->DOWN = false;
			data->PRESSED = false;
			data->RELEASED = true;

			if (onKeyStateChange != nullptr)
			{
				onKeyStateChange(false, key);
			}
		}
		else if (data->RELEASED)
		{
			data->RELEASED = false;
		}
	}
}

void KeyboardHandlerFunction(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow)
{
	kbmgr.AntiCrashSychronization.push(KeyboardManager::TACS((unsigned char)key, !isUpNow));
}

bool KeyboardManager::Pressed(unsigned char key)
{
	return Keys[key].PRESSED;
}

bool KeyboardManager::Released(unsigned char key)
{
	return Keys[key].RELEASED;
}

bool KeyboardManager::Up(unsigned char key)
{
	return !Keys[key].DOWN;
}

bool KeyboardManager::Down(unsigned char key)
{
	return Keys[key].DOWN;
}

bool KeyboardManager::BufferContainsArray(const std::vector<unsigned char>& compare)
{
	if (compare.size() > key_buffer.size())
	{
		return false;
	}

	size_t compare_end = compare.size() - 1;

	for (size_t i = 0; i < compare.size(); ++i)
	{
		unsigned char at_buf = key_buffer[i];
		unsigned char at_com = compare[compare_end - i];
		if (at_com && at_com != at_buf)
		{
			return false;
		}
	}

	return true;
}

void KeyboardManager::ClearBuffer()
{
	key_buffer.clear();
}

const std::vector<unsigned char> KeyboardManager::GetSequence(size_t size, bool removelast)
{
	std::vector<unsigned char> retvec(key_buffer.begin(), key_buffer.begin() + ((key_buffer.size() > size) ? size : key_buffer.size()));
	std::reverse(retvec.begin(), retvec.end() - (size_t)removelast);
	retvec.push_back(0);
	return retvec;
}