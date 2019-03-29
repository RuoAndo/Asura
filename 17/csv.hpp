#include <string>
#include <vector>

class Csv
{
 std::string csv_file;

 public:
 	Csv(std::string);
	bool getCsv(std::vector<std::vector<std::string>>&, const char delim=',');
};
