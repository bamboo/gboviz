#include "processor.h"
#include <fstream>

// TODO copy sunburst.html to output directory
// TODO arg parser https://github.com/p-ranav/argparse/releases/tag/v3.1
// TODO introduce --depth <build operation tree depth> to limit data generation
// TODO introduce icicles visualization and copy it to output directory
// TODO introduce --output <dir>
// TODO consider grouping operations with no significant duration together
int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
    return 1;
  }

  std::ifstream file(argv[1]);

  if (!file.is_open()) {
    std::cerr << "Could not open the file: " << argv[1] << std::endl;
    return 1;
  }

  generate_data_file_from_build_operations(file, "data.js");

  return 0;
}
