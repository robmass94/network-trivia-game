#ifndef _TRIVIABOT_H_
#define _TRIVIABOT_H_

#include <map>
#include <fstream>
#include <string>
#include "stringutils.h"
#include <sstream>

class TriviaBot {
public:
	TriviaBot();

	// question bank
	void ImportQuestions(const char* path, bool overwrite = false);

	// player interactions
	void AddPlayer    (const int& sd);
	void RemovePlayer (const int& sd);

	// game functions
	std::string GetRandomQuestion ()              const;
	std::string GetAnswer         (const std::string& question) const;
	std::string GetHint           (const std::string& question) const;
	void        IncreaseScore     (const int& sd);
	std::string SendScore	      (const int& sd);
private:
	std::map<int, int>                 player_scores_;
	std::map<std::string, std::string> questions_;
};

#endif
