#include "../src/processor.h"
#include <cstdio> // For std::remove
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

std::string read_all_text(std::istream &input) {
  std::ostringstream ss;
  ss << input.rdbuf();
  return ss.str();
}

class temp_file_guard {
public:
  explicit temp_file_guard(const std::string &file_name)
      : file_name_(file_name) {
    // Create an empty file or open it
    std::ofstream(file_name_).close();
  }

  ~temp_file_guard() {
    if (std::filesystem::exists(file_name_)) {
      std::filesystem::remove(file_name_);
      std::cout << "Temporary file '" << file_name_ << "' deleted.\n";
    }
  }

  // Return the file name for use
  const std::string &file_name() const { return file_name_; }

private:
  std::string file_name_;
};

TEST(ProcessJsonTest, single_parent_single_child) {
  // given:
  std::string json_lines =
      R"({"displayName": "Run build", "id": 85, "startTime": 1}
{"displayName": "Load configuration cache state", "id": 88, "parentId": 85, "startTime": 2}
{"id": 88, "endTime": 4}
{"id": 85, "endTime": 4}
)";

  //  and:
  const char *temp_output_file_name = "output.js";
  temp_file_guard temp_file{temp_output_file_name};

  // when:
  std::istringstream json_stream{json_lines};
  generate_data_file_from_build_operations(json_stream, temp_output_file_name);

  // then:
  std::ifstream output_file(temp_file.file_name());
  ASSERT_TRUE(output_file.is_open()) << "Expecting output file!";

  std::string actual_output = read_all_text(output_file);
  output_file.close();

  // and:
  std::string expected_output = R"(const data = {
  name: "$",
  children: [
    {
      name: "Run build",
      children: [
        {
          name: "Load configuration cache state",
          value: 2
        }
      ]
    }
  ]
}
)";
  EXPECT_EQ(actual_output, expected_output);
}
