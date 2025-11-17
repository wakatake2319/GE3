#include "Logger.h"
namespace Logger {
void Log(const std::string& message) {
	// 出力先のファイルパスを取得
	std::filesystem::path logFilePath = std::filesystem::current_path() / "log.txt";
	// ファイルに追記モードで書き込む
	std::ofstream logFile(logFilePath, std::ios::app);
	if (logFile.is_open()) {
		logFile << message;
		logFile.close();
	}
	// デバッグ出力にも表示
	OutputDebugStringA(message.c_str());
}

}