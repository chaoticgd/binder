#include <map>
#include <set>
#include <string>
#include <memory>
#include <vector>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <stdexcept>

static const std::size_t BUFFER_SIZE = 1024 * 1024;

enum token_type {
	XOR, AND, OR, SUBTRACT, OPEN_BRACKET, CLOSE_BRACKET, FILE_NAME
};

struct diff_table {
	std::set<std::size_t> offsets;
	std::array<char, BUFFER_SIZE>* values;
};

static const std::map<std::string, token_type> token_table = {
	{ "^", XOR           },
	{ ".", AND,          },
	{ "+", OR,           },
	{ "-", SUBTRACT      },
	{ "{", OPEN_BRACKET  },
	{ "}", CLOSE_BRACKET }
};
typedef std::map<std::string, std::array<char, BUFFER_SIZE>> buffer_map;
typedef std::vector<std::string>::iterator token_iterator;

std::vector<std::string> parse_args(int argc, char** argv);
std::map<std::string, std::ifstream> open_files(std::vector<std::string> args);
diff_table find_difference(token_iterator& expression, buffer_map& buffers);
template <typename T_op>
diff_table apply_operation(diff_table lhs, diff_table rhs, T_op op);
token_type get_token_type(std::string token);

int main(int argc, char** argv) {
	std::vector<std::string> args = parse_args(argc, argv);
	auto files = open_files(args);

	buffer_map buffers;
	for(std::size_t i = 0;; i++) {
		bool reached_end = false;
		for(auto& file : files) {
			auto dest = buffers[file.first].data();
			std::size_t bytes_read = file.second.readsome(dest, BUFFER_SIZE);
			if(bytes_read <= 0) {
				reached_end = true;
				break;
			}
			for(std::size_t j = bytes_read; j < BUFFER_SIZE; j++) {
				dest[j] = 0;
			}
		}

		if(reached_end) {
			break;
		}

		diff_table differences;
		try {
			auto expression = args.begin();
			differences = find_difference(expression, buffers);
		} catch(std::runtime_error& ex) {
			std::cerr << "Parsing error: " << ex.what() << std::endl;
			std::exit(1);
		}

		std::cout << "        ";
		for(auto& buffer : buffers) {
			std::cout << std::setw(buffer.first.size() + 1);
			std::cout << buffer.first;
		}
		std::cout << "\n";

		for(std::size_t offset : differences.offsets) {
			std::size_t absolute_offset = i * BUFFER_SIZE + offset;
			std::cout << std::hex << std::setfill('0') << std::setw(8);
			std::cout << absolute_offset << " ";
			for(auto& buffer : buffers) {
				std::cout << std::setfill(' ') << std::setw(buffer.first.size());
				std::cout << (int) buffer.second[offset] << " ";
			}
			std::cout << "\n";
		}
	}
}

std::vector<std::string> parse_args(int argc, char** argv) {
	std::vector<std::string> result;
	for(std::size_t i = 1; i < argc; i++) {
		result.push_back(argv[i]);
	}
	return result;
}

std::map<std::string, std::ifstream> open_files(std::vector<std::string> args) {
	std::map<std::string, std::ifstream> result;
	for(auto argument : args) {
		if(get_token_type(argument) == token_type::FILE_NAME) {
			result.emplace(argument, argument);
		}
	}
	return result;
}

bool xorOp(const diff_table& lhs, const diff_table& rhs, std::size_t i) {
	return (*lhs.values)[i] != (*rhs.values)[i];
}

bool andOp(const diff_table& lhs, const diff_table& rhs, std::size_t i) {
	return lhs.offsets.find(i) != lhs.offsets.end() &&
	       rhs.offsets.find(i) != rhs.offsets.end();
}

bool orOp(const diff_table& lhs, const diff_table& rhs, std::size_t i) {
	return lhs.offsets.find(i) != lhs.offsets.end() ||
	       rhs.offsets.find(i) != rhs.offsets.end();
}

bool subtractOp(const diff_table& lhs, const diff_table& rhs, std::size_t i) {
	return lhs.offsets.find(i) != lhs.offsets.end() &&
	       rhs.offsets.find(i) == rhs.offsets.end();
}

diff_table find_difference(token_iterator& expression, buffer_map& buffers) {

	auto parse_sub_expression = [](token_iterator& expression, buffer_map& buffers) {
		diff_table result;
		if(get_token_type(*expression) == OPEN_BRACKET) {
			result = find_difference(++expression, buffers);
			if(get_token_type(*expression++) != CLOSE_BRACKET) {
				throw std::runtime_error("Expected '}'.");
			}
		} else {
			result.offsets = {};
			result.values = &buffers.at(*expression++);
		}
		return result;
	};

	auto lhs = parse_sub_expression(expression, buffers);
	token_type op = get_token_type(*expression++);
	auto rhs = parse_sub_expression(expression, buffers);

	switch(op) {
		case XOR:      return apply_operation(lhs, rhs, xorOp);
		case AND:      return apply_operation(lhs, rhs, andOp);
		case OR:       return apply_operation(lhs, rhs, orOp);
		case SUBTRACT: return apply_operation(lhs, rhs, subtractOp);
	}
	throw std::runtime_error("Expected operator");
}

template <typename T_op>
diff_table apply_operation(diff_table lhs, diff_table rhs, T_op op) {
	diff_table result;
	for(std::size_t i = 0; i < BUFFER_SIZE; i++) {
		if(op(lhs, rhs, i)) {
			result.offsets.insert(i);
		}
	}
	result.values = lhs.values;
	return result;
}

token_type get_token_type(std::string token) {
	auto type = token_table.find(token);
	if(type != token_table.end()) {
		return type->second;
	} else {
		return FILE_NAME;
	}
}
