/*
 * async.cpp
 *
 *  Created on: 7 июл. 2019 г.
 *      Author: sveta
 */
#include "async.h"
#include <iostream>
#include <thread>
#include <mutex>
#include "bulkasync.h"
#ifdef _WIN32
#include <direct.h>
#endif

namespace async {
std::mutex cvMutex;
class AsyncReciever {
	CommandProcessor p;
	std::string folder;
	bool connected = false;
	int commandsCount;
	int blockCount;

	void WriteToConsole() {
		std::unique_lock<std::mutex> lk(cvMutex);
		for (std::size_t i = 0; i < p.bulks.size(); i++) {
			if (!p.bulks[i].GetOutputed()) {
				std::cout << p.bulks[i].GetBulkString();
				commandsCount += p.bulks[i].GetCommandCount();
				blockCount++;
				p.bulks[i].SetOutputed(true);
			}
		}
		lk.unlock();
	}

	void WriteToFile() {
		std::stringstream filestring;
		for (std::size_t i = 0; i < p.bulks.size(); i++) {
			if (!p.bulks[i].GetWritten()) {
				filestring.clear();
				filestring << folder << "bulk" << p.bulks[i].GetTime() << "_"
						<< std::this_thread::get_id() << ".log";
				std::ofstream file(filestring.str());
				file << p.bulks[i].GetBulkString();
				file.close();
				p.bulks[i].SetWritten(true);
				commandsCount += p.bulks[i].GetCommandCount();
				blockCount++;
				if (p.bulks[i].GetOutputed()) {
					std::swap(p.bulks[i], p.bulks.back());
					p.bulks.pop_back();
					i--;
				}
			}

		}
	}
	std::string trim(const std::string& str, const std::string& whitespace =
			" \t") {
		const auto strBegin = str.find_first_not_of(whitespace);
		if (strBegin == std::string::npos)
			return "";

		const auto strEnd = str.find_last_not_of(whitespace);
		const auto strRange = strEnd - strBegin + 1;

		return str.substr(strBegin, strRange);
	}

public:
	AsyncReciever(std::size_t bulk) :
			p(bulk) {
		connected = true;

		commandsCount = 0;
		blockCount = 0;
	}

	void Disconnect() {
		connected = false;
		p.SetState(State::Finish);

	}

	void ProcessBulk(const std::vector<std::string> commandsVector) {
		if (!connected)
			return;
		auto folderName = "bulkfiles";
#ifdef _WIN32
		auto status = _mkdir(folderName);
#else
		auto status = mkdir(folderName, S_IRWXU | S_IRWXG);
		;
#endif
		if (status == 0 || errno == static_cast<int>(std::errc::file_exists)) {
			folder = std::string(folderName) + "/";
		} else
			folder = "";

		for (auto cmd : commandsVector) {

			p.ProcessCommand(trim(cmd));
			if (p.GetState() == State::Processed) {
				WriteToConsole();
				WriteToFile();
			}
		}
	}
	std::string GetState() {

		switch (p.GetState()) {
		case State::Finish:
			return "Finish";
		case State::Processed:
			return "Processed";
		case State::WaitComand:
			return "WaitComand";
		}
		return "error";
	}
};

std::vector<std::string> split(const char *str, std::size_t size, char d) {
	std::vector<std::string> r;
	try {
		std::stringstream s("");
		for (size_t i = 0; i < size; i++) {
			if (str[i] != d) {
				s << str[i];
			} else {
				r.push_back(s.str());
				s.str("");
			}
		}
		if (s.str() != "") {
			r.push_back(s.str());
		}
	} catch (...) {
		r.clear();
	}

	return r;
}

std::string GetState(handle_t handle) {
	if (handle == nullptr)
		throw "Incorrect operation";
	;
	try {
		auto reciever = reinterpret_cast<AsyncReciever*>(handle);
		return (*reciever).GetState();
	} catch (...) {
		throw "Incorrect operation";
	}
}

handle_t connect(std::size_t bulk) {
	auto reciever = new AsyncReciever(bulk);
	return static_cast<handle_t>(reciever);
}

void receive(handle_t handle, const char *data, std::size_t size) {
	if (handle == nullptr)
		return;
	try {
		auto commandsVector = split(data, size, '\n');
		if (commandsVector.size() == 0)
			return;
		auto reciever = reinterpret_cast<AsyncReciever*>(handle);
		(*reciever).ProcessBulk(commandsVector);
	} catch (...) {
		throw "Incorrect operation";
	}
}

void disconnect(handle_t handle) {
	if (handle == nullptr)
		return;
	try {
		auto reciever = reinterpret_cast<AsyncReciever*>(handle);
		(*reciever).Disconnect();
		delete reciever;
	} catch (...) {
		throw "Incorrect operation";
	}

}

}

