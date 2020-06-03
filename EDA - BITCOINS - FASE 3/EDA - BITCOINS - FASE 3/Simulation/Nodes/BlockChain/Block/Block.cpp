#include "Block.h"

Block::Block(const json& JSON) {
	block = JSON;
	header = block;
	tx = header["tx"];
	header.erase("tx");
	transformData();
};

void Block::transformData() {
	header["height"] = std::to_string(header["height"].get<unsigned int>());
	header["nonce"] = std::to_string(header["nonce"].get<unsigned int>());
	header["nTx"] = std::to_string(header["nTx"].get<unsigned int>());
}

/*Gets transaction IDs from json.*/
const std::list<std::string> Block::getIDs() {
	std::list<std::string> IDs;

	std::string tx_id;
	bool mustAdd;
	/*For every transaction...*/
	for (const auto& TX : tx) {
		/*Loops through every 'mini header' in header['vin'].*/
		for (const auto& miniJson : TX["vin"])
			/*Gets string from header.*/
			tx_id.append(miniJson["txid"].get<std::string>());

		/*Hashes id and appends it to IDs.*/
		IDs.push_back(hash(tx_id));

		tx_id.clear();
	}

	return IDs;
}

unsigned int Block::generateID(unsigned char* str) {
	unsigned int ID = 0;
	int c;
	while (c = *str++)
		ID = c + (ID << 6) + (ID << 16) - ID;
	return ID;
}
/*Transforms int into hex Coded ASCII.*/
inline const std::string Block::hexCodedASCII(unsigned int number) {
	char res[9];
	sprintf_s(res, "%08X", number);

	return res;
}

inline const std::string Block::hash(const std::string& code) {
	return hexCodedASCII(generateID((unsigned char*)code.c_str()));
}

void Block::buildTree(void) {
	/*Gets IDs from transactions.*/
	std::list<std::string> nodes = getIDs();

	std::list<std::string>::iterator itrTemp;

	/*While nodes list is not the Merkle Root...*/
	while (nodes.size() > 1) {
		/*If node amount is uneven, it copies the last one to the back of the list.*/
		if (nodes.size() % 2)
			nodes.push_back(nodes.back());

		tree.insert(tree.end(), nodes.begin(), nodes.end());

		/*For every node in the list...*/
		for (auto i = nodes.begin(); i != nodes.end(); i++) {
			/*Concats next node's content to the current node's content and hashes.*/
			*i = hash(*i + *std::next(i));

			/*Erases next node.*/
			nodes.erase(std::next(i));
		}
	}
	if (nodes.size()) {
		tree.push_back(nodes.back());
		calculatedMerkleRoot = nodes.back();
		if (nodes.back() == header["merkleroot"])
			isMROK = "True";
		else
			isMROK = "False";
	}
	else
		throw std::exception("Wrong data input for Merkle Tree");
}

const std::string Block::getData(const BlockInfo& data) {
	switch (data) {
	case BlockInfo::BLOCKID:
		return header["blockid"];
	case BlockInfo::BLOCK_NUMBER:
		return header["height"];
	case BlockInfo::SEE_MROOT:
		return header["merkleroot"];
	case BlockInfo::VALIDATE_MROOT:
		if (!isMROK.length())
			buildTree();
		return isMROK;
	case BlockInfo::NTX:
		return header["nTx"];
	case BlockInfo::NONCE:
		return header["nonce"];
	case BlockInfo::PREVIOUS_BLOCKID:
		return header["previousblockid"];
	case BlockInfo::PRINT_TREE:
		return printTree();
	}
}

/*Prints tree.*/
std::string Block::printTree(void) {
	if (!tree.size())
		buildTree();
	int levels = log2(tree.size() + 1);

	std::string result;

	/*Character between words. */
	const char* spacing = " ";
	int words_in_row, init, middle, abs_pos = 0;
	const int length = tree[0].length();

	/*Total number of positions in square row.*/
	const int row = (pow(2, levels) - 1) * length;

	/*Loops from higher to lower level.*/
	for (int i = levels; i > 0; i--) {
		/*Sets number of words in current row.*/
		words_in_row = pow(2, i - 1);

		/*Sets initial amount of characters before first word.*/
		init = (pow(2, levels - i) - 1) * length;

		/*Sets number of characters between words.*/
		if (words_in_row - 1)
			middle = (row - 2 * init - words_in_row * length) / (words_in_row - 1);
		else
			middle = 0;

		/*Adds 'init' amount of initial spacings.*/
		for (int j = 0; j < init; j++)
			result.append(spacing);

		/*Adds words plus characters between next word.*/
		for (int j = abs_pos; j < abs_pos + words_in_row; j++) {
			result.append(tree[j]);
			for (int k = 0; k < middle; k++)
				result.append(spacing);
		}

		/*Updates absolute position within tree.*/
		abs_pos += words_in_row;

		/*Goes down one level.*/
		result.append("\n\n");
	}

	return result;
}

Block::~Block() {};