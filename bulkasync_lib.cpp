/*
 * bulkmt_lib.cpp
 *
 *  Created on: 2 июл. 2019 г.
 *      Author: sveta
 */
#include "bulkasync.h"


	std::string BulkInfo::GetBulkString() {
		return bulkString;
	}
	int BulkInfo::GetCommandCount() {
		return  commandCount;
	}
	std::string BulkInfo::GetTime() {
		return recieveTime;
	}
	bool BulkInfo::GetOutputed() {
		return outputed;
	}
	void BulkInfo::SetOutputed(bool val) {
		outputed = val;
	}
	bool BulkInfo::GetWritten() {
		return written;
	}
	void BulkInfo::SetWritten(bool val) {
		written = val;
	}
	CommandCollector::CommandCollector() {
			commandsCount = 0;
			recieveTime = "";
		}
		size_t CommandCollector::size() {
			return commandsCount;
		}
		void CommandCollector::AddCommand(std::string command, std::string time) {
			if (commandsCount == 0) {
				recieveTime = time;
				bulkString.str("");
				bulkString << " bulk:" << command;
			}
			else {
				bulkString << ", " << command;
			}
			commandsCount++;
		}

		// Выполнить команды и записать их в файл;
		BulkInfo CommandCollector::Process() {
			bulkString << std::endl;
			auto result = BulkInfo(bulkString.str(), commandsCount, recieveTime);
			bulkString.str("");
			commandsCount = 0;
			recieveTime = "";
			return result;
		}

		void CommandCollector::Clear(){
				bulkString.clear();
				recieveTime="";
				commandsCount=0;
			}

		size_t CommandProcessor::GetBulkSize() {
			return commandCollector.size();
		}
		CommandProcessor::CommandProcessor(size_t n) :
			N(n) {
			openCount = 0;
			closeCount = 0;
			processorState = State::WaitComand;
		}
		void CommandProcessor::SetState(State st) {
			processorState = st;
			if (st==State::Finish){
				openCount = 0;
				closeCount = 0;
				 commandCollector.Clear();
			}
		}
		State CommandProcessor::GetState() {
			return processorState;
		}

		void CommandProcessor::ProcessCommand(std::string command) {

			if (command==""){
				processorState = State::WaitComand;
				return;
			}
			auto currentTime = sclock::now();
			auto recieveTime = std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(currentTime.time_since_epoch()).count());
			processorState = State::WaitComand;
			// новый блок.
			if (command == openBrace) {
				if (openCount == 0) {
					CallProcessor();
				}
				openCount++;
			}
			else {
				// закрывающая скобка.
				if (command == closeBrace && openCount>0) {
					closeCount++;
					// проверка на вложенность.
					if (closeCount == openCount) {
						CallProcessor();
						openCount = 0;
						closeCount = 0;
					}
				}
				else {
					commandCollector.AddCommand(command, recieveTime);
					// если блок команд полностью заполнен и размер блока не был изменен скобкой.
					if (commandCollector.size() == N && openCount == 0) {
						CallProcessor();
					}

				}
			}
		}

