#include "aho_corasick.h"

int main()
{
	AhoCorasick<char> aho({ "a", "ab", "bab", "bc", "bca", "c", "caa" });
	auto entries = aho.execute_on_string("abccba");
	return 0;
}