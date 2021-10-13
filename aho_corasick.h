#ifndef _AHO_CORASICK_H_
#define _AHO_CORASICK_H_

#include <initializer_list>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <queue>
#include <vector>
#include <deque>

//template is needed for different string types: string, wstring and other
template <typename CharType>
class AhoCorasick
{
public:
	//dictionary is set of strings algorithm is looking for in text 
	AhoCorasick(const std::initializer_list<const CharType*> dictionary): dictionary_size(dictionary.size()), trie(dictionary)
	{
	}
	//fuction for finding all entries of dictionaru string in input string
	//return deque of dictionary_size elements, filled with vector of entries for each string in dictionary, respectively
	std::deque<std::vector<std::size_t>> execute_on_string(const CharType* str)
	{
		std::deque<std::vector<std::size_t>> entries(dictionary_size);//create deque with dictionary_size vectors for each string
		
		const Trie::TrieNode* state = &trie.root;
		//in the created trie, change the states starting from the root for every symbol in str
		for (std::size_t i = 0; str[i] != '\0'; ++i)
		{
			state = trie.transition_function(state, str[i]);
			//if current state is the end of any string in dictionary,
			//push the index of symbol where string was found to corresponding vector
			if (state->end_of_string)
				entries[state->end_of_string - 1].push_back(i);
			
			//if in the same state there are more strings found, dictionary suffix link will point on them
			if (state->dict_suffix_link != nullptr)
			{
				auto dict_found_state = state->dict_suffix_link;
				do {
					//get the string number from state and push it in the same index array
					entries[dict_found_state->end_of_string - 1].push_back(i);
					dict_found_state = dict_found_state->dict_suffix_link;
				} while (dict_found_state != nullptr); //if there are more dictionary links, continue
			}
			//continue search
		}
		return entries;
	}
	~AhoCorasick()
	{

	}
private:
	//algorithm builds trie for the dictionary
	//there are two nested classes needed for algo implementation: trie and its node
	class Trie
	{
	public:
		Trie(const std::initializer_list<const CharType*> dictionary) : root(nullptr, '\0') //construct root (no parent and path_symbol)
		{
			//add every element into trie
			std::size_t string_num = 1;
			for (auto str_pointer: dictionary)
			{
				//for every character add node
				//pass the pointer to string and string number that can be 1 ... dictionary_size
				add_child(&root, str_pointer, string_num);
				++string_num;
			}
			//after building trie nodes, find and set all links using breadth-first search
			std::queue<TrieNode*> node_queue;
			//fill queue with root children for start
			for (auto& child : root.children)
				node_queue.push(&child.second);
			//for every child node that is not root find suffix link and dictionaty suffix link
			do 
			{
				auto& current_node = node_queue.front();

				for (auto& child : current_node->children)
					node_queue.push(&child.second);
				//calculate suffix links
				current_node->suffix_link = find_suffix_link(current_node);
				current_node->dict_suffix_link = find_dict_suffix_link(current_node);

				node_queue.pop();
			} while (!node_queue.empty());
			//after that, trie is ready for searching
		}
		~Trie()
		{

		}
		//trie node contatins children nodes, suffix link, dictionary suffix link,
		//end_of_string flag (if node is terminal for any string in dictionaty, contains number of string starting with 1, else 0),
		//parent link and path symbol(symbol used for parent to reach this node)
		//TrieNode is used for storing data only, therefore it's a struct and not a class
		struct TrieNode
		{
			std::size_t end_of_string;
			std::unordered_map<CharType, TrieNode> children;

			const TrieNode* suffix_link;
			const TrieNode* dict_suffix_link;

			TrieNode* parent;
			CharType path_symbol; //symbol, which leads to current node from parent

			//default constructor must NOT be called, used for unordered_map compatibility
			TrieNode(): end_of_string(0), suffix_link(nullptr), dict_suffix_link(nullptr), parent(nullptr), path_symbol('\0')
			{
			}

			TrieNode(TrieNode* parent, CharType path_symbol) : end_of_string(0), /*default value*/
				suffix_link(nullptr), /*nullptr value means value is not set yet*/
				dict_suffix_link(nullptr),
				parent(parent),
				path_symbol(path_symbol)
			{
			}
		};
		//node used by all TrieNodes
		TrieNode root;
		//calculates where transition from current trie node leads if path_symbol is the path
		const TrieNode* transition_function(const TrieNode* node_ptr, const CharType next_symbol)
		{
			auto child_iter = node_ptr->children.find(next_symbol);
			//node has child by symbol path_symbol
			if (child_iter != node_ptr->children.cend())
				return &node_ptr->children.at(next_symbol);
			else if (node_ptr == &root) //node has no child by symbol path_symbol and it is root
				return &root;
			else return transition_function(node_ptr->suffix_link, next_symbol); //recursively calculate transition function for suffix link
		}
		//operations implementation
	private:
		//fucntion used by trie to add new child for specified node
		void add_child(TrieNode* node_ptr, const CharType* path, const std::size_t string_number)
		{
			if (path == nullptr)
				throw std::invalid_argument("string is nullptr");

			const auto child_sym = *path;
			if (child_sym != '\0') //end of string
			{
				const auto child_iter = node_ptr->children.find(child_sym);

				if (child_iter == node_ptr->children.cend()) //construct new node for child
					node_ptr->children.insert({ child_sym, TrieNode{node_ptr, child_sym} });

				//recursively add new child to child node
				add_child(&node_ptr->children[child_sym], ++path, string_number);
			}
			else
				node_ptr->end_of_string = string_number;
		}
		const TrieNode* find_suffix_link(const TrieNode* node_ptr)
		{
			if (node_ptr->parent == &root)
				return &root;
			else if (node_ptr != &root)
			{
				return transition_function(node_ptr->parent->suffix_link, node_ptr->path_symbol);
			}
			else 
				return nullptr;
		}
		const TrieNode* find_dict_suffix_link(const TrieNode* node_ptr)
		{
			auto suffix_linked_node = node_ptr->suffix_link;
			if (suffix_linked_node->end_of_string)
				return suffix_linked_node;
			else if (suffix_linked_node == &root)
				return nullptr;
			else
				return find_dict_suffix_link(suffix_linked_node);
		}
	};

	std::size_t dictionary_size;
	//trie build in constructor for algorithm
	Trie trie;
};







#endif // !_AHO_CORASICK_H_